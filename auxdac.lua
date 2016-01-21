
local addr = "43c12008 "
local driver = {
  control = 0x90,
  scale   = 0x3FF,
  voltage = 2.9,
}
local final = {
  control = 0x90,
  scale   = 0x3FF,
  voltage = 2.9,
}

function auxdac_gen( offst, scale )
    if( scale > 0x3ff ) then scale = 0x3ff end
    if( offst > 0x004 ) then offst = 0x004 end

    return string.format("%02X", scale & 0xff ) .. string.format("%02X", 0x80 | (offst<<2) | ((scale >> 8) & 0x03))
end

function auxdac()
    local value = auxdac_gen(final.ring, final.scale) .. auxdac_gen(driver.ring, driver.scale)
    tx("* WF " .. addr .. value .. "\r")
    --sleep(1)
    --tx("* R 43c12008" .. addr_aux_dac .. "\r")
end

function auxdac_reset()
	driver.control = 0x90
	driver.scale   = 0x3FF
	driver.voltage = 2.9
	final.control = 0x90
	final.scale   = 0x3FF
	final.voltage = 2.9
	auxdac()
end

function auxdac_set_voltage( tbl, v )
	-- Clamp Voltage
	if( v > 2.9 ) then v = 2.9 elseif( v < 0 ) then v = 0 end
	if( v < 1.5 ) then -- Use 1.5v Top of Range, 2v range
    tbl.voltage = v
    tbl.scale   = math.ceil(0x3FF * ((v + 0.5) / 2))
    tbl.control = 0x84
	else               -- Use 2.9v Top of Range, 2V range
    tbl.voltage = v
    tbl.scale   = math.ceil(0x3FF * ((v - 0.9) / 2))
    tbl.control = 0x90
	end
  aux_dac()
end

function auxdac_adjust( tbl, dv )
  auxdac_set_voltage( tbl, tbl.voltage + dv )
end

function ramp_driver()
	auxdac_reset();
	while(driver.voltage > 0) do
		sleep(250)
		auxdac_adjust_voltage( driver, -0.02 )
	end
end
function ramp_final()
	auxdac_reset();
	while(driver.voltage > 0) do
		sleep(250)
		auxdac_adjust_voltage(final, -0.02 )
	end
end


register(nil, "Aux Dac/final/++",   function() auxdac_adjust(final,   0.10) end)
register(nil, "Aux Dac/final/+",    function() auxdac_adjust(final,   0.01) end)
register(nil, "Aux Dac/final/-",    function() auxdac_adjust(final,  -0.01) end)
register(nil, "Aux Dac/final/--",   function() auxdac_adjust(final,  -0.10) end)
register(nil, "Aux Dac/driver/++",  function() auxdac_adjust(driver,  0.10) end)
register(nil, "Aux Dac/driver/+",   function() auxdac_adjust(driver,  0.01) end)
register(nil, "Aux Dac/driver/-",   function() auxdac_adjust(driver, -0.01) end)
register(nil, "Aux Dac/driver/--",  function() auxdac_adjust(driver, -0.10) end)
register(nil, "Aux Dac/ramp/driver",function() ramp_driver() end)
register(nil, "Aux Dac/ramp/final", function() ramp_final()  end)
register(nil, "Aux Dac/reset",      auxdac_reset) 


