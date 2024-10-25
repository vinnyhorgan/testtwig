local Areas = Object:extend()

local Area = require("core.Area")

function Areas:new()
    self.editing = false
    self.buffer = ""
end

function Areas:enter()

end

function Areas:exit()
end

function Areas:update()

end

function Areas:draw()
    if self.editing then
        if button((WIDTH - 100) / 2, 10, 100, 30, self.buffer) then

        end
    else
        if button((WIDTH - 100) / 2, 10, 100, 30, areas[currentArea].name) then
            self.editing = true
        end
    end

    if #areas > 1 then
        if button(20, (HEIGHT - 24) / 2, 24, 24, "<") then
            currentArea = currentArea - 1
            if currentArea < 1 then
                currentArea = #areas
            end
        end

        if button(WIDTH - 40, (HEIGHT - 24) / 2, 24, 24, ">") then
            currentArea = currentArea + 1
            if currentArea > #areas then
                currentArea = 1
            end
        end
    end

    if button(WIDTH - 40, HEIGHT - 40, 30, 30, "+") then
        table.insert(areas, Area("Area " .. #areas + 1))
        currentArea = #areas
    end
end

function Areas:textinput(text)
    if self.editing and renderer.text_width(self.buffer) < 90 then
        self.buffer = self.buffer .. text
    end
end

function Areas:keypressed(key)
    if self.editing then
        if key == "return" then
            areas[currentArea].name = self.buffer
            self.buffer = ""
            self.editing = false
        elseif key == "backspace" then
            local byteoffset = utf8.offset(self.buffer, -1)
            if byteoffset then
                self.buffer = string.sub(self.buffer, 1, byteoffset - 1)
            end
        end
    end
end

return Areas
