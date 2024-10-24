local narrator = require("narrator.narrator")

local core = {}

function core.init()

end

function core.run()
    while true do
        local frame_start = system.get_time()

        for type, a,b,c,d in system.poll_event do
            if type == "quit" then
                os.exit()
            end
        end

        renderer.clear({ 255, 255, 255 })

        renderer.draw_point(10, 10, { 255, 0, 0 })

        renderer.draw_rect(50, 50, 100, 100, { 0, 255, 0 })

        renderer.update()

        local elapsed = system.get_time() - frame_start
        system.sleep(math.max(0, 1/60 - elapsed))
    end
end

return core
