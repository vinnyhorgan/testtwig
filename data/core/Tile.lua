local Tile = Object:extend()

function Tile:new(spritesheet, sheetX, sheetY)
    self.spritesheet = spritesheet
    self.quad = love.graphics.newQuad(sheetX * CELL_SIZE, sheetY * CELL_SIZE, CELL_SIZE, CELL_SIZE, self.spritesheet:getDimensions())
end
