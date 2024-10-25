local StateManager = Object:extend()

function StateManager:new()
    self.current_state = nil
end

function StateManager:change(state)
    if self.current_state and self.current_state.exit then
        self.current_state:exit()
    end

    self.current_state = state

    if self.current_state and self.current_state.enter then
        self.current_state:enter()
    end
end

function StateManager:update()
    if self.current_state and self.current_state.update then
        self.current_state:update()
    end
end

function StateManager:draw()
    if self.current_state and self.current_state.draw then
        self.current_state:draw()
    end
end

function StateManager:keypressed(key)
    if self.current_state and self.current_state.keypressed then
        self.current_state:keypressed(key)
    end
end

function StateManager:mousepressed(button, x, y)
    if self.current_state and self.current_state.mousepressed then
        self.current_state:mousepressed(button, x, y)
    end
end

function StateManager:mousemoved(x, y)
    if self.current_state and self.current_state.mousemoved then
        self.current_state:mousemoved(x, y)
    end
end

return StateManager
