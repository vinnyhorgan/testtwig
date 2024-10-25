local narrator = require("narrator.narrator")
local text = require("text")

local book = narrator.parse_file("test")
local story = narrator.init_story(book)

story:begin()

local core = {}

-- TODO: move all asset control in lua: font and cursor

local box = text.new("left", {
    color = { 1, 1, 1, 1 },
    print_speed = 0.03,
    adjust_line_height = 3
})
local choice_boxes = {}
local can_choose = false

function stepStory()
    if story:can_continue() then
        local paragraphs = story:continue()
        local fullText = ""

        for _, paragraph in ipairs(paragraphs) do
            fullText = fullText .. paragraph.text .. "\n"
        end

        box:send(fullText, 160 - 20, false)

        can_choose = story:can_choose()

        if can_choose then
            local choices = story:get_choices()

            for i, choice in ipairs(choices) do
                local choice_box = text.new("left", { color = { 1, 1, 1, 1 }, print_speed = 0.03, adjust_line_height = 3 })
                choice_box:send(i .. ". " .. choice.text, 160 - 20, false)
                choice_box.index = i
                table.insert(choice_boxes, choice_box)
            end
        end
    end
end

function core.init()
    img = renderer.image.load("test.png")
end

local inp = ""

local cur = renderer.image.load("cursor.png")

local mouseX, mouseY = 0, 0

local input_box = text.new("left", {
    adjust_line_height = 3
})

function core.run()
    while true do
        local frame_start = system.get_time()

        box:update(0.016)
        input_box:update(0.016)

        if box:is_finished() then
            for _, choice_box in ipairs(choice_boxes) do
                choice_box:update(0.016)
            end
        end

        for type, a,b,c,d in system.poll_event do
            if type == "quit" then
                os.exit()
            elseif type == "textinput" then
                inp = inp .. a
                input_box:send(inp, 160 - 20, true)
            elseif type == "mousemoved" then
                mouseX, mouseY = a, b
            elseif type == "keypressed" then
                if a == "backspace" then
                    local byteoffset = utf8.offset(inp, -1)
                    if byteoffset then
                        inp = string.sub(inp, 1, byteoffset - 1)
                        input_box:send(inp, 160 - 20, true)
                    end
                end
            elseif type == "mousepressed" then
                local button = a
                local x = b
                local y = c

                if can_choose then
                    local yoff = box.get.height + 20

                    for _, choice_box in ipairs(choice_boxes) do
                        if x > 170 and x < 170 + choice_box.get.width and y > yoff and y < yoff + choice_box.get.height then
                            story:choose(choice_box.index)
                            choice_boxes = {}
                            can_choose = false

                            stepStory()
                        end

                        yoff = yoff + choice_box.get.height + 10
                    end
                else
                    stepStory()
                end
            end
        end

        renderer.clear({ 255 / 2, 255 / 2, 255 / 2 })

        renderer.draw_rect(160, 0, 160, 180, { 0, 0, 0, 255 / 2 })
        box:draw(170, 10)

        input_box:draw(10, 10)

        if box:is_finished() then
            local yoff = box.get.height + 20

            for _, choice_box in ipairs(choice_boxes) do
                if mouseX > 170 and mouseY < 170 + choice_box.get.width and mouseY > yoff and mouseY < yoff + choice_box.get.height then
                    renderer.draw_rect(170 - 5, yoff - 5, choice_box.get.width + 10, choice_box.get.height + 10, { 255 / 2, 255 / 2, 255 / 2, 255}) -- investigate hard crash on wrong input here
                end

                choice_box:draw(170, yoff)
                yoff = yoff + choice_box.get.height + 10
            end
        end

        if mouseX > 0 and mouseX < 319 and mouseY > 0 and mouseY < 179 then
            cur:draw(mouseX, mouseY)
        end

        renderer.update()

        local elapsed = system.get_time() - frame_start
        system.sleep(math.max(0, 1/60 - elapsed))
    end
end

return core
