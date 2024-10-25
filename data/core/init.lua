-- GLOBALS

narrator = require("narrator.narrator")
text = require("text")
Object = require("classic")

state_manager = nil
WIDTH = 384
HEIGHT = 216
CELL_SIZE = 16
mouseX, mouseY = 0, 0
areas = {}

currentArea = 1

mouse_down_state = {}
mouse_pressed_state = {}

function mouse_down(button)
    return mouse_down_state[button] or false
end

function mouse_pressed(button)
    return mouse_pressed_state[button] or false
end

function button(x, y, w, h, text)
    local color = { 30, 30, 30, 100 }

    if mouseX >= x and mouseX <= x + w and mouseY >= y and mouseY <= y + h and mouse_down("left") then
        color = { 0, 0, 0, 100 }
    end

    renderer.draw_rect(x + 2, y + 2, w - 4, h - 4, color)
    renderer.draw_line(x + 2, y + 1, x + w - 3, y + 1, color)
    renderer.draw_line(x + 1, y + 2, x + 1, y + h - 3, color)
    renderer.draw_line(x + 2, y + h - 2, x + w - 3, y + h - 2, color)
    renderer.draw_line(x + w - 2, y + 2, x + w - 2, y + h - 3, color)

    renderer.draw_line(x, y + 2, x, y + h - 3, { 0, 0, 0 })
    renderer.draw_line(x + 2, y, x + w - 3, y, { 0, 0, 0 })
    renderer.draw_point(x + 1, y + 1, { 0, 0, 0 })
    renderer.draw_line(x + 2, y + h - 1, x + w - 3, y + h - 1, { 0, 0, 0 })
    renderer.draw_point(x + 1, y + h - 2, { 0, 0, 0 })
    renderer.draw_line(x + w - 1, y + 2, x + w - 1, y + h - 3, { 0, 0, 0 })
    renderer.draw_point(x + w - 2, y + 1, { 0, 0, 0 })
    renderer.draw_point(x + w - 2, y + h - 2, { 0, 0, 0 })

    renderer.draw_text(text, x + w / 2 - math.floor(renderer.text_width(text) / 2), y + h / 2 - math.floor(renderer.text_height() / 2), { 255, 255, 255 })

    return mouseX >= x and mouseX <= x + w and mouseY >= y and mouseY <= y + h and mouse_pressed("left")
end

-- END GLOBALS

local StateManager = require("core.StateManager")
local Areas = require("states.Areas")
local Demo = require("states.Demo")
local Area = require("core.Area")

local core = {}

-- TODO: move all asset control in lua: font and cursor

local cur

function core.init()
    cur = renderer.image.load("cursor.png")
    state_manager = StateManager()
    state_manager:change(Areas())

    table.insert(areas, Area("Test Area"))
end

function core.run()
    while true do
        local frame_start = system.get_time()

        for button in pairs(mouse_pressed_state) do
            mouse_pressed_state[button] = false
        end

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
                if not mouse_down_state[a] then
                    mouse_pressed_state[a] = true
                end
                mouse_down_state[a] = true
                state_manager:mousepressed(a, b, c)
            elseif type == "mousereleased" then
                mouse_down_state[a] = false
            end
        end

        state_manager:update()

        renderer.clear({ 255 / 2, 255 / 2, 255 / 2 })

        state_manager:draw()

        if mouseX > 0 and mouseX < WIDTH - 1 and mouseY > 0 and mouseY < HEIGHT - 1 then
            cur:draw(mouseX, mouseY)
        end

        renderer.update()

        local elapsed = system.get_time() - frame_start
        system.sleep(math.max(0, 1/60 - elapsed))
    end
end

return core
