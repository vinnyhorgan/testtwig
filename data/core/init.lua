local narrator = require("narrator.narrator")
local Text = require("text")

local core = {}

local img

local box = Text.new(nil, {})
box:send("Helloooooo! [rainbow][bounce]THIS STUFF WORKS[/bounce][/rainbow] It's so [rainbow]COOL[/rainbow] you have no [shake]IDEA[/shake]", 100)

function core.init()
    img = renderer.image.load("test.png")
end

function core.run()
    while true do
        local frame_start = system.get_time()

        box:update(0.016)

        for type, a,b,c,d in system.poll_event do
            if type == "quit" then
                os.exit()
            end
        end

        renderer.clear({ 255, 255, 255 })

        box:draw(50, 50)

        img:draw(10, 10)

        renderer.draw_point(10, 10, { 255, 0, 0 })

        renderer.draw_text("Welcome to twig", 20, 30, { 0, 0, 0 })

        renderer.update()

        local elapsed = system.get_time() - frame_start
        system.sleep(math.max(0, 1/60 - elapsed))
    end
end

return core
