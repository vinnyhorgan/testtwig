local Demo = Object:extend()

function Demo:new()
    self.box = text.new("left", {
        color = { 1, 1, 1, 1 },
        print_speed = 0.03,
        adjust_line_height = 3
    })
    self.choice_boxes = {}
    self.can_choose = false
end

function Demo:enter()
    self.book = narrator.parse_file("test")
    self.story = narrator.init_story(self.book)

    self.story:begin()
end

function Demo:exit()

end

function Demo:update()
    self.box:update(0.016)

    if self.box:is_finished() then
        for _, choice_box in ipairs(self.choice_boxes) do
            choice_box:update(0.016)
        end
    end
end

function Demo:draw()
    renderer.draw_rect(160, 0, 160, 180, { 0, 0, 0, 255 / 2 })
    self.box:draw(170, 10)

    if self.box:is_finished() then
        local yoff = self.box.get.height + 20

        for _, choice_box in ipairs(self.choice_boxes) do
            if mouseX > 170 and mouseY < 170 + choice_box.get.width and mouseY > yoff and mouseY < yoff + choice_box.get.height then
                renderer.draw_rect(170 - 5, yoff - 5, choice_box.get.width + 10, choice_box.get.height + 10, { 255 / 2, 255 / 2, 255 / 2, 255}) -- investigate hard crash on wrong input here
            end

            choice_box:draw(170, yoff)
            yoff = yoff + choice_box.get.height + 10
        end
    end
end

function Demo:mousepressed(button, x, y)
    if self.can_choose then
        local yoff = self.box.get.height + 20

        for _, choice_box in ipairs(self.choice_boxes) do
            if x > 170 and x < 170 + choice_box.get.width and y > yoff and y < yoff + choice_box.get.height then
                self.story:choose(choice_box.index)
                self.choice_boxes = {}
                self.can_choose = false

                self:step_story()
            end

            yoff = yoff + choice_box.get.height + 10
        end
    else
        self:step_story()
    end
end

function Demo:step_story()
    if self.story:can_continue() then
        local paragraphs = self.story:continue()
        local fullText = ""

        for _, paragraph in ipairs(paragraphs) do
            fullText = fullText .. paragraph.text .. "\n"
        end

        self.box:send(fullText, 160 - 20, false)

        self.can_choose = self.story:can_choose()

        if self.can_choose then
            local choices = self.story:get_choices()

            for i, choice in ipairs(choices) do
                local choice_box = text.new("left", { color = { 1, 1, 1, 1 }, print_speed = 0.03, adjust_line_height = 3 })
                choice_box:send(i .. ". " .. choice.text, 160 - 20, false)
                choice_box.index = i
                table.insert(self.choice_boxes, choice_box)
            end
        end
    end
end

return Demo
