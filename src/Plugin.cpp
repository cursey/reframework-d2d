#include <memory>
#include <vector>
#include <chrono>

#include <reframework/API.hpp>
#include <sol/sol.hpp>

#include "D3D12Renderer.hpp"
#include "DrawList.hpp"

using API = reframework::API;
using Clock = std::chrono::high_resolution_clock;

std::unique_ptr<D3D12Renderer> g_d3d12{};
D2DPainter* g_d2d{};
std::vector<sol::protected_function> g_draw_fns{};
std::vector<sol::protected_function> g_init_fns{};
lua_State* g_lua{};
bool g_needs_init{};
DrawList g_drawlist{};
DrawList::CommandLock* g_cmds{};
Clock::time_point g_d2d_next_frame_time{Clock::now()};
const std::chrono::duration<double> DEFAULT_UPDATE_INTERVAL{1.0 / 60.0};
std::chrono::duration<double> g_d2d_update_interval{DEFAULT_UPDATE_INTERVAL};
bool g_update_d2d{};

auto get_d2d_max_updaterate() { return 1.0 / g_d2d_update_interval.count(); }
auto set_d2d_max_updaterate(double hz) { g_d2d_update_interval = std::chrono::duration<double>{1.0 / hz}; }

void on_ref_lua_state_created(lua_State* l) try {
    g_lua = l;
    sol::state_view lua{l};

    auto d2d = lua.create_table();
    auto detail = lua.create_table();

    d2d.new_usertype<D2DFont>(
        "Font", sol::meta_function::construct,
        [](const char* name, int size, sol::object bold_obj, sol::object italic_obj) {
            auto bold = false;
            auto italic = false;

            if (bold_obj.is<bool>()) {
                bold = bold_obj.as<bool>();
            }

            if (italic_obj.is<bool>()) {
                italic = italic_obj.as<bool>();
            }

            return std::make_shared<D2DFont>(g_d2d->dwrite(), name, size, bold, italic);
        },
        "measure", &D2DFont::measure);

    d2d.new_usertype<D2DImage>(
        "Image", sol::meta_function::construct,
        [](const char* filepath) {
            std::string modpath{};

            modpath.resize(1024, 0);
            modpath.resize(GetModuleFileName(nullptr, modpath.data(), modpath.size()));

            auto images_path = std::filesystem::path{modpath}.parent_path() / "reframework" / "images";
            auto image_path = images_path / filepath;

            std::filesystem::create_directories(images_path);

            return std::make_shared<D2DImage>(g_d2d->wic(), g_d2d->context(), image_path);
        },
        "size", &D2DImage::size);

    detail["get_max_updaterate"] = []() { return get_d2d_max_updaterate(); };
    detail["set_max_updaterate"] = [](double fps) { set_d2d_max_updaterate(fps); };
    d2d["detail"] = detail;
    d2d["register"] = [](sol::protected_function init_fn, sol::protected_function draw_fn) {
        g_init_fns.emplace_back(init_fn);
        g_draw_fns.emplace_back(draw_fn);
        g_needs_init = true;
    };
    d2d["create_font"] = [](const char* name, int size, sol::object bold_obj, sol::object italic_obj) {
        auto bold = false;
        auto italic = false;

        if (bold_obj.is<bool>()) {
            bold = bold_obj.as<bool>();
        }

        if (italic_obj.is<bool>()) {
            italic = italic_obj.as<bool>();
        }

        return std::make_shared<D2DFont>(g_d2d->dwrite(), name, size, bold, italic);
    };
    d2d["text"] = [](std::shared_ptr<D2DFont>& font, const char* text, float x, float y, unsigned int color) {
        g_cmds->text(font, text, x, y, color);
    };
    d2d["measure_text"] = [](sol::this_state s, std::shared_ptr<D2DFont>& font, const char* text) {
        auto [w, h] = font->measure(text);
        sol::variadic_results results{};
        results.push_back(sol::make_object(s, w));
        results.push_back(sol::make_object(s, h));
        return results;
    };
    d2d["fill_rect"] = [](float x, float y, float w, float h, unsigned int color) { g_cmds->fill_rect(x, y, w, h, color); };
    d2d["filled_rect"] = d2d["fill_rect"];
    d2d["outline_rect"] = [](float x, float y, float w, float h, float thickness, unsigned int color) {
        g_cmds->outline_rect(x, y, w, h, thickness, color);
    };
    d2d["line"] = [](float x1, float y1, float x2, float y2, float thickness, unsigned int color) {
        g_cmds->line(x1, y1, x2, y2, thickness, color);
    };
    d2d["image"] = [](std::shared_ptr<D2DImage>& image, float x, float y, sol::object w_obj, sol::object h_obj) {
        auto [w, h] = image->size();

        if (w_obj.is<float>()) {
            w = w_obj.as<float>();
        }

        if (h_obj.is<float>()) {
            h = h_obj.as<float>();
        }

        g_cmds->image(image, x, y, w, h);
    };
    d2d["surface_size"] = [](sol::this_state s) {
        auto [w, h] = g_d2d->surface_size();
        sol::variadic_results results{};
        results.push_back(sol::make_object(s, w));
        results.push_back(sol::make_object(s, h));
        return results;
    };
    lua["d2d"] = d2d;
    g_needs_init = true;
} catch (const std::exception& e) {
    OutputDebugStringA(e.what());
    API::get()->log_error("[reframework-d2d] [on_ref_lua_state_created] %s", e.what());
}

void on_ref_lua_state_destroyed(lua_State* l) try {
    g_drawlist.acquire().commands.clear();
    g_draw_fns.clear();
    g_init_fns.clear();
    g_lua = nullptr;
} catch (const std::exception& e) {
    OutputDebugStringA(e.what());
    API::get()->log_error("[reframework-d2d] [on_ref_lua_state_destroyed] %s", e.what());
}

void on_ref_device_reset() try {
    g_drawlist.acquire().commands.clear();
    g_d2d = nullptr;
    g_d3d12.reset();
} catch (const std::exception& e) {
    OutputDebugStringA(e.what());
    API::get()->log_error("[reframework-d2d] [on_ref_lua_device_reset] %s", e.what());
}

void on_ref_frame() try {
    if (g_draw_fns.empty()) {
        return;
    }

    if (g_d3d12 == nullptr) {
        auto renderer_data = API::get()->param()->renderer_data;
        g_d3d12 = std::make_unique<D3D12Renderer>((IDXGISwapChain*)renderer_data->swapchain, (ID3D12Device*)renderer_data->device,
            (ID3D12CommandQueue*)renderer_data->command_queue);
        g_d2d = g_d3d12->get_d2d().get();
        g_needs_init = true;
    }

    // Just return if we need init since its not ready yet.
    if (g_needs_init) {
        return;
    }

    g_d3d12->render([](D2DPainter& d2d) {
        auto cmds_lock = g_drawlist.acquire();

        for (auto&& cmd : cmds_lock.commands) {
            switch (cmd.type) {
            case DrawList::CommandType::TEXT:
                g_d2d->text(cmd.font_resource, cmd.str, cmd.text.x, cmd.text.y, cmd.text.color);
                break;

            case DrawList::CommandType::FILL_RECT:
                g_d2d->fill_rect(cmd.fill_rect.x, cmd.fill_rect.y, cmd.fill_rect.w, cmd.fill_rect.h, cmd.fill_rect.color);
                break;

            case DrawList::CommandType::OUTLINE_RECT:
                g_d2d->outline_rect(cmd.outline_rect.x, cmd.outline_rect.y, cmd.outline_rect.w, cmd.outline_rect.h,
                    cmd.outline_rect.thickness, cmd.outline_rect.color);
                break;

            case DrawList::CommandType::LINE:
                g_d2d->line(cmd.line.x1, cmd.line.y1, cmd.line.x2, cmd.line.y2, cmd.line.thickness, cmd.line.color);
                break;

            case DrawList::CommandType::IMAGE:
                g_d2d->image(cmd.image_resource, cmd.image.x, cmd.image.y, cmd.image.w, cmd.image.h);
                break;
            }
        }
    }, g_update_d2d);

    g_update_d2d = false;
} catch (const std::exception& e) {
    OutputDebugStringA(e.what());
    // g_ref->functions->log_error(e.what());
}

void on_begin_rendering() try {
    if (g_d3d12 == nullptr) {
        return;
    }

    if (g_needs_init) {
        auto _ = API::LuaLock{};

        for (const auto& init_fn : g_init_fns) {
            try {
                auto result = init_fn();

                if (!result.valid()) {
                    sol::script_throw_on_error(g_lua, std::move(result));
                }
            } catch (const std::exception& e) {
                MessageBox(nullptr, e.what(), "[reframework-d2d] [init_fn] error", MB_ICONERROR | MB_OK);
                OutputDebugStringA(e.what());
                API::get()->log_error("[reframework-d2d] [on_ref_lua_device_reset] %s", e.what());
            }
        }

        g_needs_init = false;
    }

    auto now = Clock::now();

    if (now >= g_d2d_next_frame_time) {
        auto lua_lock = API::LuaLock{};
        auto cmds_lock = g_drawlist.acquire();
        g_cmds = &cmds_lock;

        cmds_lock.commands.clear();

        for (const auto& draw_fn : g_draw_fns) {
            auto result = draw_fn();

            if (!result.valid()) {
                sol::script_throw_on_error(g_lua, std::move(result));
            }
        }

        g_cmds = nullptr;
        g_d2d_next_frame_time = now + std::chrono::duration_cast<std::chrono::milliseconds>(g_d2d_update_interval);
        g_update_d2d = true;
    }
} catch (const std::exception& e) {
    OutputDebugStringA(e.what());
    // g_ref->functions->log_error(e.what());
}

extern "C" __declspec(dllexport) void reframework_plugin_required_version(REFrameworkPluginVersion* version) {
    version->major = REFRAMEWORK_PLUGIN_VERSION_MAJOR;
    version->minor = REFRAMEWORK_PLUGIN_VERSION_MINOR;
    version->patch = REFRAMEWORK_PLUGIN_VERSION_PATCH;
}

extern "C" __declspec(dllexport) bool reframework_plugin_initialize(const REFrameworkPluginInitializeParam* param) {
    if (param->renderer_data->renderer_type != REFRAMEWORK_RENDERER_D3D12) {
        return false;
    }

    reframework::API::initialize(param);

    param->functions->on_lua_state_created(on_ref_lua_state_created);
    param->functions->on_lua_state_destroyed(on_ref_lua_state_destroyed);
    param->functions->on_frame(on_ref_frame);
    param->functions->on_device_reset(on_ref_device_reset);
    param->functions->on_pre_application_entry("BeginRendering", on_begin_rendering);

    return true;
}
