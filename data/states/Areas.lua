local Areas = Object:extend()

function Areas:new(state_manager)
    self.state_manager = state_manager
end

function Areas:enter()
    print("Hello bithces")
end

function Areas:exit()
    print("Bye bithces")
end

function Areas:update()
end

function Areas:draw()

end

function Areas:keypressed(key)
    print("Bitch i'm pressing")
end

return Areas
