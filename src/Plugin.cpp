#include <chrono>
#include <memory>
#include <vector>

#include "reframework/API.hpp"
#include "sol/sol.hpp"

#include "D3D12Renderer.hpp"
#include "DrawList.hpp"

using API = reframework::API;
using Clock = std::chrono::high_resolution_clock;

struct Plugin {
    std::unique_ptr<D3D12Renderer> d3d12{};
    D2DPainter* d2d{};
    std::vector<sol::protected_function> draw_fns{};
    std::vector<sol::protected_function> init_fns{};
    lua_State* lua{};
    bool needs_init{};
    DrawList drawlist{};
    DrawList::CommandLock* cmds{};
    Clock::time_point d2d_next_frame_time{Clock::now()};
    const std::chrono::duration<double> DEFAULT_UPDATE_INTERVAL{1.0 / 60.0};
    std::chrono::duration<double> d2d_update_interval{DEFAULT_UPDATE_INTERVAL};
    bool update_d2d{};
    std::string last_script_error{};
};

Plugin* g_plugin{};

BOOL APIENTRY DllMain(HMODULE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_plugin = new Plugin{};
    }

    return TRUE;
}

void handle_error_message(const std::string& msg) {
    OutputDebugStringA(msg.c_str());

    g_plugin->last_script_error = msg;
}

auto get_d2d_max_updaterate() {
    return 1.0 / g_plugin->d2d_update_interval.count();
}
auto set_d2d_max_updaterate(double hz) {
    g_plugin->d2d_update_interval = std::chrono::duration<double>{1.0 / hz};
}

void on_ref_lua_state_created(lua_State* l) try {
    g_plugin->lua = l;
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

            return std::make_shared<D2DFont>(g_plugin->d2d->dwrite(), name, size, bold, italic);
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

            return std::make_shared<D2DImage>(g_plugin->d2d->wic(), g_plugin->d2d->context(), image_path);
        },
        "size", &D2DImage::size);

    detail["get_max_updaterate"] = []() { return get_d2d_max_updaterate(); };
    detail["set_max_updaterate"] = [](double fps) { set_d2d_max_updaterate(fps); };
    detail["get_last_error"] = []() {
        return g_plugin->last_script_error;
    };
    d2d["detail"] = detail;
    d2d["register"] = [](sol::protected_function init_fn, sol::protected_function draw_fn) {
        g_plugin->init_fns.emplace_back(init_fn);
        g_plugin->draw_fns.emplace_back(draw_fn);
        g_plugin->needs_init = true;
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

        return std::make_shared<D2DFont>(g_plugin->d2d->dwrite(), name, size, bold, italic);
    };
    d2d["text"] = [](std::shared_ptr<D2DFont>& font, const char* text, float x, float y, unsigned int color) {
        g_plugin->cmds->text(font, text, x, y, color);
    };
    d2d["measure_text"] = [](sol::this_state s, std::shared_ptr<D2DFont>& font, const char* text) {
        auto [w, h] = font->measure(text);
        sol::variadic_results results{};
        results.push_back(sol::make_object(s, w));
        results.push_back(sol::make_object(s, h));
        return results;
    };
    d2d["fill_rect"] = [](float x, float y, float w, float h, unsigned int color) { g_plugin->cmds->fill_rect(x, y, w, h, color); };
    d2d["filled_rect"] = d2d["fill_rect"];
    d2d["outline_rect"] = [](float x, float y, float w, float h, float thickness, unsigned int color) {
        g_plugin->cmds->outline_rect(x, y, w, h, thickness, color);
    };
    d2d["rounded_rect"] = [](float x, float y, float w, float h, float rX, float rY, float thickness, unsigned int color) {
        g_plugin->cmds->rounded_rect(x, y, w, h, rX, rY, thickness, color);
    };
    d2d["fill_rounded_rect"] = [](float x, float y, float w, float h, float rX, float rY, unsigned int color) {
        g_plugin->cmds->fill_rounded_rect(x, y, w, h, rX, rY, color);
    };
    d2d["line"] = [](float x1, float y1, float x2, float y2, float thickness, unsigned int color) {
        g_plugin->cmds->line(x1, y1, x2, y2, thickness, color);
    };
    d2d["image"] = [](std::shared_ptr<D2DImage>& image, float x, float y, sol::object w_obj, sol::object h_obj) {
        auto [w, h] = image->size();

        if (w_obj.is<float>()) {
            w = w_obj.as<float>();
        }

        if (h_obj.is<float>()) {
            h = h_obj.as<float>();
        }

        g_plugin->cmds->image(image, x, y, w, h);
    };
    d2d["fill_circle"] = [](float x, float y, float r, unsigned int color) { g_plugin->cmds->fill_circle(x, y, r, r, color); };
    d2d["circle"] = [](float x, float y, float r, float thickness, unsigned int color) {
        g_plugin->cmds->circle(x, y, r, r, thickness, color);
    };
    d2d["fill_oval"] = [](float x, float y, float rX, float rY, unsigned int color) { g_plugin->cmds->fill_circle(x, y, rX, rY, color); };
    d2d["oval"] = [](float x, float y, float rX, float rY, float thickness, unsigned int color) {
        g_plugin->cmds->circle(x, y, rX, rY, thickness, color);
    };
    d2d["pie"] = [](float x, float y, float r, float startAngle, float sweepAngle, unsigned int color) {
        g_plugin->cmds->pie(x, y, r, startAngle, sweepAngle, color);
    };
    d2d["ring"] = [](float x, float y, float outerR, float innerR, float startAngle, float sweepAngle, unsigned int color) {
        g_plugin->cmds->ring(x, y, outerR, innerR, startAngle, sweepAngle, color);
    };
    d2d["surface_size"] = [](sol::this_state s) {
        auto [w, h] = g_plugin->d2d->surface_size();
        sol::variadic_results results{};
        results.push_back(sol::make_object(s, w));
        results.push_back(sol::make_object(s, h));
        return results;
    };
    lua["d2d"] = d2d;
    g_plugin->needs_init = true;
} catch (const std::exception& e) {
    handle_error_message(e.what());
    API::get()->log_error("[reframework-d2d] [on_ref_lua_state_created] %s", e.what());
}

void on_ref_lua_state_destroyed(lua_State* l) try {
    g_plugin->drawlist.acquire().commands.clear();
    g_plugin->draw_fns.clear();
    g_plugin->init_fns.clear();
    g_plugin->last_script_error.clear();
    g_plugin->lua = nullptr;
} catch (const std::exception& e) {
    handle_error_message(e.what());
    API::get()->log_error("[reframework-d2d] [on_ref_lua_state_destroyed] %s", e.what());
}

void on_ref_device_reset() try {
    g_plugin->drawlist.acquire().commands.clear();
    g_plugin->d2d = nullptr;
    g_plugin->d3d12.reset();
} catch (const std::exception& e) {
    handle_error_message(e.what());
    API::get()->log_error("[reframework-d2d] [on_ref_lua_device_reset] %s", e.what());
}

void on_ref_frame() try {
    if (g_plugin->draw_fns.empty()) {
        return;
    }

    if (g_plugin->d3d12 == nullptr) {
        auto renderer_data = API::get()->param()->renderer_data;
        g_plugin->d3d12 = std::make_unique<D3D12Renderer>((IDXGISwapChain*)renderer_data->swapchain, (ID3D12Device*)renderer_data->device,
            (ID3D12CommandQueue*)renderer_data->command_queue);
        g_plugin->d2d = g_plugin->d3d12->get_d2d().get();
        g_plugin->needs_init = true;
    }

    // Just return if we need init since its not ready yet.
    if (g_plugin->needs_init) {
        return;
    }

    g_plugin->d3d12->render(
        [](D2DPainter& d2d) {
            auto cmds_lock = g_plugin->drawlist.acquire();

            for (auto&& cmd : cmds_lock.commands) {
                switch (cmd.type) {
                case DrawList::CommandType::TEXT:
                    g_plugin->d2d->text(cmd.font_resource, cmd.str, cmd.text.x, cmd.text.y, cmd.text.color);
                    break;

                case DrawList::CommandType::FILL_RECT:
                    g_plugin->d2d->fill_rect(cmd.fill_rect.x, cmd.fill_rect.y, cmd.fill_rect.w, cmd.fill_rect.h, cmd.fill_rect.color);
                    break;

                case DrawList::CommandType::OUTLINE_RECT:
                    g_plugin->d2d->outline_rect(cmd.outline_rect.x, cmd.outline_rect.y, cmd.outline_rect.w, cmd.outline_rect.h,
                        cmd.outline_rect.thickness, cmd.outline_rect.color);
                    break;

                case DrawList::CommandType::ROUNDED_RECT:
                    g_plugin->d2d->rounded_rect(cmd.rounded_rect.x, cmd.rounded_rect.y, cmd.rounded_rect.w, cmd.rounded_rect.h,
                        cmd.rounded_rect.rX, cmd.rounded_rect.rY, cmd.rounded_rect.thickness, cmd.rounded_rect.color);
                    break;

                case DrawList::CommandType::FILL_ROUNDED_RECT:
                    g_plugin->d2d->fill_rounded_rect(cmd.rounded_rect.x, cmd.rounded_rect.y, cmd.rounded_rect.w, cmd.rounded_rect.h,
                        cmd.rounded_rect.rX, cmd.rounded_rect.rY, cmd.rounded_rect.color);
                    break;

                case DrawList::CommandType::LINE:
                    g_plugin->d2d->line(cmd.line.x1, cmd.line.y1, cmd.line.x2, cmd.line.y2, cmd.line.thickness, cmd.line.color);
                    break;

                case DrawList::CommandType::IMAGE:
                    g_plugin->d2d->image(cmd.image_resource, cmd.image.x, cmd.image.y, cmd.image.w, cmd.image.h);
                    break;

                case DrawList::CommandType::FILL_CIRCLE:
                    g_plugin->d2d->fill_circle(
                        cmd.fill_circle.x, cmd.fill_circle.y, cmd.fill_circle.radiusX, cmd.fill_circle.radiusY, cmd.fill_circle.color);
                    break;

                case DrawList::CommandType::CIRCLE:
                    g_plugin->d2d->circle(cmd.circle.x, cmd.circle.y, cmd.circle.radiusX, cmd.circle.radiusY, cmd.circle.thickness, cmd.circle.color);
                    break;

                case DrawList::CommandType::PIE:
                    g_plugin->d2d->pie(cmd.pie.x, cmd.pie.y, cmd.pie.r, cmd.pie.startAngle, cmd.pie.sweepAngle, cmd.pie.color);
                    break;

                case DrawList::CommandType::RING:
                    g_plugin->d2d->ring(cmd.ring.x, cmd.ring.y, cmd.ring.outerRadius, cmd.ring.innerRadius, cmd.ring.startAngle, cmd.ring.sweepAngle, cmd.ring.color);
                    break;
                }
            }
        },
        g_plugin->update_d2d);

    g_plugin->update_d2d = false;
} catch (const std::exception& e) {
    handle_error_message(e.what());
    // g_plugin->ref->functions->log_plugin->error(e.what());
}

void on_begin_rendering() try {
    if (g_plugin->d3d12 == nullptr) {
        return;
    }

    if (g_plugin->needs_init) {
        auto _ = API::LuaLock{};

        for (const auto& init_fn : g_plugin->init_fns) {
            try {
                auto result = init_fn();

                if (!result.valid()) {
                    sol::script_throw_on_error(g_plugin->lua, std::move(result));
                }
            } catch (const std::exception& e) {
                MessageBox(nullptr, e.what(), "[reframework-d2d] [init_fn] error", MB_ICONERROR | MB_OK);
                handle_error_message(e.what());
                API::get()->log_error("[reframework-d2d] [on_ref_lua_device_reset] %s", e.what());
            }
        }

        g_plugin->needs_init = false;
    }

    auto now = Clock::now();

    if (now >= g_plugin->d2d_next_frame_time) {
        auto lua_lock = API::LuaLock{};
        auto cmds_lock = g_plugin->drawlist.acquire();
        g_plugin->cmds = &cmds_lock;

        cmds_lock.commands.clear();

        for (const auto& draw_fn : g_plugin->draw_fns) {
            try {
                auto result = draw_fn();

                if (!result.valid()) {
                    sol::script_throw_on_error(g_plugin->lua, std::move(result));
                }
            } catch (const std::exception& e) {
                handle_error_message(e.what());
            }
        }

        g_plugin->cmds = nullptr;
        g_plugin->d2d_next_frame_time = now + std::chrono::duration_cast<std::chrono::milliseconds>(g_plugin->d2d_update_interval);
        g_plugin->update_d2d = true;
    }
} catch (const std::exception& e) {
    handle_error_message(e.what());
    // g_plugin->ref->functions->log_error(e.what());
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
    param->functions->on_present(on_ref_frame);
    param->functions->on_device_reset(on_ref_device_reset);
    param->functions->on_pre_application_entry("BeginRendering", on_begin_rendering);

    return true;
}
