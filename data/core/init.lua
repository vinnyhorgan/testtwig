local narrator = require("narrator.narrator")

local core = {}

print(lpeg.version)

function core.init()
    print("init")

    local book = narrator.parse_file("test.ink")
    local story = narrator.init_story(book)

    story:begin()

    while story:can_continue() do
        local paragraphs = story:continue()
        for _, paragraph in ipairs(paragraphs) do
            local text = paragraph.text

            if paragraph.tags then
                text = text .. " #" .. table.concat(paragraph.tags, " #")
            end

            print(text)
        end

        if not story:can_choose() then break end

        local choices = story:get_choices()
        for i, choice in ipairs(choices) do
            print(i .. ") " .. choice.text)
        end

        local answer = tonumber(io.read())
        story:choose(answer)
    end
end

function core.run()
    print("run")
end

return core
