local Tile = require("core.Tile")

local Entity = Tile:extend()

function Entity:new(spritesheet, sheetX, sheetY, dialogue)
    Entity.super.new(self, spritesheet, sheetX, sheetY)
    self.dialogue = dialogue
end
