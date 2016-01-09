
local addr = "43c12008 "
local scale_ring   = 0x4
local driver_scale = 0x3FF
local final_scale  = 0x3FF

function gen( offst, scale )
    if( scale > 0x3ff ) then scale = 0x3ff end
    if( offst > 0x004 ) then offst = 0x004 end

    return string.format("%02X", scale & 0xff ) .. string.format("%02X", 0x80 | (offst<<2) | ((scale >> 8) & 0x03))
end

function aux_dac()
    local value = gen(scale_ring, final_scale) .. gen(scale_ring, driver_scale)
    tx("* WF " .. addr .. value .. "\r")
    --sleep(1)
    --tx("* R 43c12008" .. addr_aux_dac .. "\r")
end

function final_adjust( change )
    final_scale = final_scale + change
    if( final_scale < 0x000 ) then final_scale = 0x000 end
    if( final_scale > 0x3FF ) then final_scale = 0x3FF end
    aux_dac()
end

function driver_adjust( change )
    driver_scale = driver_scale + change
    if( driver_scale < 0x000 ) then driver_scale = 0x000 end
    if( driver_scale > 0x3FF ) then driver_scale = 0x3FF end
    aux_dac()
end

function driver_ramp()
    driver_scale = 0x3FF
    final_scale  = 0x3FF
    while( driver_scale > 0 ) do
        driver_scale = driver_scale - 1
        sleep(100)
        aux_dac()
    end
end

register(nil, "Aux Dac/final/++",   function()  final_adjust( 16) end)
register(nil, "Aux Dac/final/+",    function()  final_adjust(  1) end)
register(nil, "Aux Dac/final/-",    function()  final_adjust( -1) end)
register(nil, "Aux Dac/final/--",   function()  final_adjust(-16) end)
register(nil, "Aux Dac/driver/++",  function() driver_adjust( 16) end)
register(nil, "Aux Dac/driver/+",   function() driver_adjust(  1) end)
register(nil, "Aux Dac/driver/-",   function() driver_adjust( -1) end)
register(nil, "Aux Dac/driver/--",  function() driver_adjust(-16) end)
register(nil, "Aux Dac/ramp/driver",function() driver_ramp()      end)
register(nil, "Aux Dac/ramp/final", function() driver_ramp()      end)
register(nil, "Aux Dac/reset",      function() final_scale = 0x3FF driver_scale = 0x3FF aux_dac() end)

