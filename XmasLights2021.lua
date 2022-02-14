local bluesee = require "bluesee"

-- Config service definition
local xmas_service_uuid = bluesee.UUID.new('81bea2b7-ad1a-493a-bf19-123596b3328b')
bluesee.set_display_name(xmas_service_uuid, 'Xmas Lights 2021')
bluesee.set_display_category(xmas_service_uuid, bluesee.ui)

local runLights_uuid = bluesee.UUID.new('3a6d65bb-ed42-4443-a23b-4225e76f10d8')
local nbrOfLights_uuid = bluesee.UUID.new('a9497a4a-4735-4b50-b10c-da941ac7b51b')
local candyStripWidth_uuid = bluesee.UUID.new('672b85ce-5175-485e-a9e2-739ebae601d9')
local trainCarLength_uuid = bluesee.UUID.new('9307a368-8d50-48dd-92e8-9f65bb15f98f')
local secondsBetweenEffects_uuid = bluesee.UUID.new('f0d754d8-a042-4d39-9cec-5b2243a2de86')

-- Create a collection of controls to process
-- readable and/or writeable text strings
-- If the characteristic is readable a label
-- is included whos value is the characteristic
-- If the characteristic is writable a textfield
-- control is included and the write button will
-- update the characteristic
function add_ReadWriteTextControl(span, title, ch)

    local label = nil
    local textfield = nil
    local read_btn = nil
    local write_btn = nil

    if ch.readable then
        label = bluesee.new_widget(bluesee.label)
        label.title = title
        read_btn = bluesee.new_widget(bluesee.button)
        read_btn.title = "Read"
        read_btn.on_click = function()
            ch:read()
        end
    end

    if ch.writeable then
        textfield = bluesee.new_widget(bluesee.textfield)
        if ch.readable then
            textfield.title = "New Value"
        else
            textfield.title = title
        end
        write_btn = bluesee.new_widget(bluesee.button)
        write_btn.title = "Write"
        write_btn.on_click = function()
            ch:write_binary(textfield.value)
            if ch.readable then
                ch:read()   -- force a read to refresh the label
            end
        end
    end

    span:add_widget(bluesee.new_widget(bluesee.hr))
    if label ~= nil then
        span:add_widget(label)
    end
    if read_btn ~= nil then
        span:add_widget(read_btn)
    end
    if textfield ~= nil then
        span:add_widget(textfield)
    end
    if write_btn ~= nil then
        span:add_widget(write_btn)
    end

    if ch.readable then 
        local update_fn = function()
            local val = tostring(ch.value)
            if val == nil then
                val = ch.value:as_hex_string()
            end
            label.value = val
        end
        ch:add_read_callback(update_fn)
        ch:read()
    end

end

function add_ReadWriteUnsignedIntegerControl(span, title, ch)

    local label = nil
    local textfield = nil
    local read_btn = nil
    local write_btn = nil

    if ch.readable then
        label = bluesee.new_widget(bluesee.label)
        label.title = title
        read_btn = bluesee.new_widget(bluesee.button)
        read_btn.title = "Read"
        read_btn.on_click = function()
            ch:read()
        end
    end

    if ch.writeable then
        textfield = bluesee.new_widget(bluesee.textfield)
        if ch.readable then
            textfield.title = "New Value"
        else
            textfield.title = title
        end
        write_btn = bluesee.new_widget(bluesee.button)
        write_btn.title = "Write"
        write_btn.on_click = function()
            local val = tonumber(textfield.value)
            ch:write(bluesee.Data.new(val, 4, bluesee.Data.endian_little))
            if ch.readable then
                ch:read()   -- force a read to refresh the label
            end
        end
    end

    span:add_widget(bluesee.new_widget(bluesee.hr))
    if label ~= nil then
        span:add_widget(label)
    end
    if read_btn ~= nil then
        span:add_widget(read_btn)
    end
    if textfield ~= nil then
        span:add_widget(textfield)
    end
    if write_btn ~= nil then
        span:add_widget(write_btn)
    end

    if ch.readable then 
        local update_fn = function()
            local val = tostring(ch.value:unsigned_integer(bluesee.Data.endian_little))
            if val == nil then
                val = ch.value:as_hex_string()
            end
            label.value = val
        end
        ch:add_read_callback(update_fn)
        if ch.subscribable then
            ch:subscribe(update_fn)
        end
        ch:read()
    end

end

function add_ReadWriteBoolSwitchControl(span, title, ch)

    local label = nil
    local runBtn = nil
    local stopBtn = nil

    label = bluesee.new_widget(bluesee.label)
    label.title = title

    runBtn = bluesee.new_widget(bluesee.button)
    runBtn.title = "On"
    runBtn.on_click = function()
        span:log("ON pressed")
        ch:write(bluesee.Data.new(1, 1, bluesee.Data.endian_little))
    end

    stopBtn = bluesee.new_widget(bluesee.button)
    stopBtn.title = "Off"
    stopBtn.on_click = function()
        span:log("Off pressed")
        ch:write(bluesee.Data.new(0, 1, bluesee.Data.endian_little))
    end

    span:add_widget(bluesee.new_widget(bluesee.hr))
    span:add_widget(label)
    span:add_widget(runBtn)
    span:add_widget(stopBtn)

end

-- Register the Config service
bluesee.register_service(xmas_service_uuid, function(span)

    
    -- Build the UI and add handlers based on the available and known
    -- characteristics
    span.on_ch_discovered = function(ch)

        if ch.uuid == runLights_uuid then
            add_ReadWriteBoolSwitchControl(span, "Run/Stop", ch)
        elseif ch.uuid == nbrOfLights_uuid then
            add_ReadWriteUnsignedIntegerControl(span, "Number of LEDS", ch)
        elseif ch.uuid == candyStripWidth_uuid then
            add_ReadWriteUnsignedIntegerControl(span, "Candy Cane Strip Width", ch)
        elseif ch.uuid == trainCarLength_uuid then
            add_ReadWriteUnsignedIntegerControl(span, "Train Car Length", ch)
        elseif ch.uuid == secondsBetweenEffects_uuid then
            add_ReadWriteUnsignedIntegerControl(span, "Seconds Between Effects", ch)
        else
            span:log("Error - unknown characteristic" .. ch.uuid)
        end
    end

 end)