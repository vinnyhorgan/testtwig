local narrator = require("narrator.narrator")

local core = {}

local img

function core.init()
    img = renderer.image.load("test.png")
    print(img:get_width(), img:get_height())
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

        img:draw(10, 10)

        renderer.draw_point(10, 10, { 255, 0, 0 })

        renderer.draw_text("Porco dio! Cazzi per tutti bro", 20, 50, { 0, 0, 0})

        renderer.update()

        local elapsed = system.get_time() - frame_start
        system.sleep(math.max(0, 1/60 - elapsed))
    end
end

return core
