local cfg = json.load_file("reframework-d2d.json")

re.on_config_save(
    function()
        json.dump_file("reframework-d2d.json", cfg)
    end
)

re.on_frame(function ()
    if not cfg.Debug then return end

    imgui.text(string.format("Draw function cost: %.2f us",  d2d.detail.get_draw_function_cost()))
    imgui.text(string.format("Cache build cost: %.2f",  d2d.detail.get_cache_build_cost()))
    imgui.text(string.format("Draw calls: %d, cost: %.2f us", d2d.detail.get_draw_calls_count(), d2d.detail.get_draw_calls_cost()))
    imgui.text(string.format("Render cost: %.2f us",  d2d.detail.get_render_cost()))
    local hit, miss = d2d.detail.get_cache_status()
    imgui.text(string.format("Cache hit: %d, cache miss: %d", hit, miss))
end)

re.on_draw_ui(
    function()
        if not imgui.collapsing_header("REFramework D2D") then return end

        local changed, value = imgui.slider_int("Max Update Rate", cfg.max_update_rate, 1, 300)
        if changed then 
            cfg.max_update_rate = value 
        end
        
        changed, value = imgui.checkbox("Debug", cfg.Debug)
        if changed then 
            cfg.Debug = value
        end

        local last_error = d2d.detail.get_last_script_error()
        if last_error ~= "" then
            imgui.text("Last Script Error:")
            imgui.text_colored(last_error, 0xFF0000FF)
        end
    end
)

d2d.register(
    function()
        if not cfg then
            cfg = {
                max_update_rate = d2d.detail.get_max_updaterate() 
            }
        end
    end,
    function()
        d2d.detail.set_max_updaterate(cfg.max_update_rate)
    end
)
