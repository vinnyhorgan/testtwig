local Areas = Object:extend()

local Area = require("core.Area")

function Areas:new()

end

function Areas:enter()

end

function Areas:exit()
end

function Areas:update()

end

function Areas:draw()
    if button((WIDTH - 100) / 2, 10, 100, 30, areas[currentArea].name) then

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

function Areas:keypressed(key)
end

return Areas
