local Area = Object:extend()

function Area:new(name)
    self.name = name
    self.background = { 255 / 2, 255 / 2, 255 / 2 }

    self.grid = {}

    for x = 1, WIDTH / CELL_SIZE do
        self.grid[x] = {}
        for y = 1, HEIGHT / CELL_SIZE do
            self.grid[x][y] = nil
        end
    end

    self.entities = {}

    for x = 1, WIDTH / CELL_SIZE do
        self.entities[x] = {}
        for y = 1, HEIGHT / CELL_SIZE do
            self.entities[x][y] = nil
        end
    end

    self.collision = {}

    for x = 1, WIDTH / CELL_SIZE do
        self.collision[x] = {}
        for y = 1, HEIGHT / CELL_SIZE do
            self.collision[x][y] = false
        end
    end
end

function Area:setTile(x, y, tile)
    if x >= 1 and x <= WIDTH / CELL_SIZE and y >= 1 and y <= HEIGHT / CELL_SIZE then
        self.grid[x][y] = tile
    end
end

function Area:setEntity(x, y, entity)
    if x >= 1 and x <= WIDTH / CELL_SIZE and y >= 1 and y <= HEIGHT / CELL_SIZE then
        self.entities[x][y] = entity
    end
end

function Area:setCollision(x, y, collision)
    if x >= 1 and x <= WIDTH / CELL_SIZE and y >= 1 and y <= HEIGHT / CELL_SIZE then
        self.collision[x][y] = collision
    end
end

function Area:draw()
    love.graphics.clear(self.background[1], self.background[2], self.background[3])

    for x = 1, WIDTH / CELL_SIZE do
        for y = 1, HEIGHT / CELL_SIZE do
            local tile = self.grid[x][y]
            if tile then
                love.graphics.draw(tile.spritesheet, tile.quad, (x - 1) * CELL_SIZE, (y - 1) * CELL_SIZE)
            end
        end
    end

    for x = 1, WIDTH / CELL_SIZE do
        for y = 1, HEIGHT / CELL_SIZE do
            local entity = self.entities[x][y]
            if entity then
                love.graphics.draw(entity.spritesheet, entity.quad, (x - 1) * CELL_SIZE, (y - 1) * CELL_SIZE)
            end
        end
    end
end

return Area
