narrator = require("narrator.narrator")
text = require("text")
Object = require("classic")

local StateManager = require("core.StateManager")
local Areas = require("states.Areas")
local Demo = require("states.Demo")

local core = {}

-- TODO: move all asset control in lua: font and cursor

local state_manager = StateManager()
state_manager:change(Demo(state_manager))

function core.init()

end

local cur = renderer.image.load("cursor.png")
mouseX, mouseY = 0, 0

function core.run()
    while true do
        local frame_start = system.get_time()

        for type, a,b,c,d in system.poll_event do
            if type == "quit" then
                os.exit()
            elseif type == "textinput" then

            elseif type == "mousemoved" then
                mouseX, mouseY = a, b
                state_manager:mousemoved(a, b)
            elseif type == "keypressed" then
                state_manager:keypressed(a)
            elseif type == "mousepressed" then
                state_manager:mousepressed(a, b, c)
            end
        end

        state_manager:update()

        renderer.clear({ 255 / 2, 255 / 2, 255 / 2 })

        state_manager:draw()

        if mouseX > 0 and mouseX < 319 and mouseY > 0 and mouseY < 179 then
            cur:draw(mouseX, mouseY)
        end

        renderer.update()

        local elapsed = system.get_time() - frame_start
        system.sleep(math.max(0, 1/60 - elapsed))
    end
end

return core
