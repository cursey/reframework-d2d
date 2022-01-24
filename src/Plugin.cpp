#include <memory>
#include <vector>

#include <reframework/API.hpp>
#include <sol/sol.hpp>

#include "D3D12Renderer.hpp"

const REFrameworkPluginInitializeParam* g_ref{};
std::unique_ptr<D3D12Renderer> g_d3d12{};
std::vector<sol::function> g_draw_fns{};
std::vector<sol::function> g_init_fns{};

void on_ref_lua_state_created(lua_State* l) try { 
    sol::state_view lua{l}; 
    auto d2d = lua.create_table();

    d2d["register"] = [](sol::function init_fn, sol::function draw_fn) {
        g_init_fns.emplace_back(init_fn);
        g_draw_fns.emplace_back(draw_fn);
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

        return g_d3d12->get_d2d()->create_font(name, size, bold, italic);
    };
    d2d["color"] = [](float r, float g, float b, float a) {
        g_d3d12->get_d2d()->color(r, g, b, a);
    };
    d2d["text"] = [](int font, int x, int y, const char* text) {
        g_d3d12->get_d2d()->text(font, x, y, text);
    };
    lua["d2d"] = d2d;
} catch (const std::exception& e) {
    OutputDebugStringA(e.what());
    throw e;
}

void on_ref_lua_state_destroyed(lua_State* l) try { 
    g_draw_fns.clear();
    g_init_fns.clear();
} catch (const std::exception& e) {
    OutputDebugStringA(e.what());
    throw e;
}

void on_ref_device_reset() try {
    g_d3d12.reset();
} catch(const std::exception& e) {
    OutputDebugStringA(e.what());
    throw e;
}

void on_ref_frame() try {
    if (g_draw_fns.empty()) {
        return;
    }

    if (g_d3d12 == nullptr) {
        g_d3d12 = std::make_unique<D3D12Renderer>((IDXGISwapChain*)g_ref->renderer_data->swapchain,
            (ID3D12Device*)g_ref->renderer_data->device, (ID3D12CommandQueue*)g_ref->renderer_data->command_queue);

        g_ref->functions->lock_lua();

        for (const auto& init_fn : g_init_fns) {
            init_fn();
        }

        g_ref->functions->unlock_lua();
    }

    g_d3d12->render([](D2DRenderer& d2d) {
        g_ref->functions->lock_lua();

        for (const auto& draw_fn : g_draw_fns) {
            draw_fn();
        }

        g_ref->functions->unlock_lua();
    });
} catch(const std::exception& e) {
    OutputDebugStringA(e.what());
    throw e;
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

    g_ref = param;

    param->functions->on_lua_state_created(on_ref_lua_state_created);
    param->functions->on_lua_state_destroyed(on_ref_lua_state_destroyed);
    param->functions->on_frame(on_ref_frame);
    param->functions->on_device_reset(on_ref_device_reset);

    return true;
}
