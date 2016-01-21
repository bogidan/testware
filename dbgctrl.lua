  
local dbg_ctrl = {
	disable_100v      = (1 << 31),
	disable_53v_n     = (1 << 30),
	disable_5v_n      = (1 << 29),
	bot_1030_en       = (1 << 28),
	bot_1090_en       = (1 << 27),
	top_1030_en       = (1 << 26),
	top_1090_en       = (1 << 25),
}
dbg_ctrl.ant_top  = dbg_ctrl.top_1030_en | dbg_ctrl.top_1090_en
dbg_ctrl.ant_bot  = dbg_ctrl.bot_1030_en | dbg_ctrl.bot_1090_en
dbg_ctrl.ant_mask = dbg_ctrl.ant_top     | dbg_ctrl.ant_bot
dbg_ctrl.val      = dbg_ctrl.disable_100v | dbg_ctrl.disable_5v_n | dbg_ctrl.ant_top -- 0xE3000000

function dbg_ctrl.send()
	tx("* WF 43c2fff0 " .. string.format("%08X", dbg_ctrl.val) .. "\r")
end

function bit_on( tbl, bits )
  tbl.val = tbl.val |  bits
  tbl.send()
end
function bit_off( tbl, bits )
  tbl.val = tbl.val & ~bits
  tbl.send()
end
function antenna_select( tbl, bits )
  tbl.val = (tbl.val & ~tbl.ant_mask) | bits
  tbl.send()
end

register(nil,  "Debug/100v/enable",   function() bit_off(dbg_ctrl, dbg_ctrl.disable_100v ) end )
register(nil,  "Debug/100v/disable",  function() bit_on (dbg_ctrl, dbg_ctrl.disable_100v ) end )
register(nil,  "Debug/53v/enable",    function() bit_on (dbg_ctrl, dbg_ctrl.disable_53v_n) end )
register(nil,  "Debug/53v/disable",   function() bit_off(dbg_ctrl, dbg_ctrl.disable_53v_n) end )
register(nil,  "Debug/ANT/top",       function() antenna_select(dbg_ctrl, dbg_ctrl.ant_top) end )
register(nil,  "Debug/ANT/bot",       function() antenna_select(dbg_ctrl, dbg_ctrl.ant_bot) end )
register(nil,  "Debug/ANT/none",      function() antenna_select(dbg_ctrl,                0) end )

