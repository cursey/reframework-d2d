local cfg = json.load_file("reframework-d2d.json")

re.on_config_save(
    function()
        json.dump_file("reframework-d2d.json", cfg)
    end
)

re.on_draw_ui(
    function()
        if not imgui.collapsing_header("REFramework D2D") then return end

        local changed, value = imgui.slider_int("Max Update Rate", cfg.max_update_rate, 1, 300)
        if changed then 
            cfg.max_update_rate = value 
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
