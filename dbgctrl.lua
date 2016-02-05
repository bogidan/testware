if not sleep then sleep = function() print("sleep") end end
if not tx then tx = function(s) print(s) end end
if not register then register = function(a,b,c) print("register",a,b,c) end end

local dbg_ctrl = {
	disable_100v      = 0x80000000,
	disable_53v_n     = 0x40000000,
	disable_5v_n      = 0x20000000,
	bot_1030_en       = 0x10000000,
	bot_1090_en       = 0x08000000,
	top_1030_en       = 0x04000000,
	top_1090_en       = 0x02000000,
}
dbg_ctrl.rcv_top  = dbg_ctrl.top_1030_en | dbg_ctrl.top_1090_en
dbg_ctrl.rcv_bot  = dbg_ctrl.bot_1030_en | dbg_ctrl.bot_1090_en
dbg_ctrl.rcv_mask = dbg_ctrl.rcv_top     | dbg_ctrl.rcv_bot
dbg_ctrl.val      = dbg_ctrl.disable_100v | dbg_ctrl.disable_5v_n | dbg_ctrl.rcv_mask -- 0xE3000000

function dbg_ctrl.send()
	tx("* WF 43c2fff0 " .. string.format("%08X", dbg_ctrl.val) .. "\r")
end

local dbg_tx = {
	disable_wait_for_pll      = 0x80000000,
	final_ena                 = 0x40000000,
	driver_ena                = 0x20000000,
	ant_sel_bot               = 0x10000000,
	ant_sel_top               = 0x08000000,
	t_r_mode                  = 0x04000000,
	filter_sel_1090           = 0x02000000,
	filter_sel_rf_test        = 0x01000000,
	filter_sel_1030           = 0x00800000,
	rf_test_top_monoport_sel  = 0x00400000,
	bot_monoport_sel          = 0x00200000,
	rf_test_sel               = 0x00100000,
	sel_mode_ac_tune_interval = 0x00080000,
	sel_mode_s_tune_interval  = 0x00040000,
	mode_s_interr_short       = 0x00020000,
}
dbg_tx.ant_mask = dbg_tx.ant_sel_bot | dbg_tx.ant_sel_top
dbg_tx.val = dbg_tx.t_r_mode | dbg_tx.filter_sel_1030 | dbg_tx.ant_sel_bot | dbg_tx.driver_ena;

function dbg_tx.send()
	tx("* WF 43c10ff0 " .. string.format("%08X", dbg_tx.val) .. "\r")
end


function bit_on( tbl, bits )
  tbl.val = tbl.val |  bits
  tbl.send()
end
function bit_off( tbl, bits )
  tbl.val = tbl.val & ~bits
  tbl.send()
end

function recieve_select( bits )
  dbg_ctrl.val = (dbg_ctrl.val & ~dbg_ctrl.rcv_mask) | bits
  dbg_ctrl.send()
end
function antenna_select( bits )
  dbg_tx.val = (dbg_tx.val & ~ant_mask) | bits
  dbg_tx.send()
end

register(nil,  "Debug/100v/enable",   function() bit_off(dbg_ctrl, dbg_ctrl.disable_100v ) end )
register(nil,  "Debug/100v/disable",  function() bit_on (dbg_ctrl, dbg_ctrl.disable_100v ) end )

register(nil,  "Debug/53v/enable",    function() bit_on (dbg_ctrl, dbg_ctrl.disable_53v_n) end )
register(nil,  "Debug/53v/disable",   function() bit_off(dbg_ctrl, dbg_ctrl.disable_53v_n) end )

register(nil,  "Debug/ANT/top",       function() antenna_select(dbg_tx.ant_sel_top) end )
register(nil,  "Debug/ANT/bot",       function() antenna_select(dbg_tx.ant_sel_bot) end )
register(nil,  "Debug/ANT/none",      function() antenna_select(0                   ) end )

register(nil,  "Debug/bias/driver/enable",  function() bit_on (dbg_tx, dbg_tx.driver_ena) end )
register(nil,  "Debug/bias/driver/disable", function() bit_off(dbg_tx, dbg_tx.driver_ena) end )
register(nil,  "Debug/bias/final/enable",   function() bit_on (dbg_tx, dbg_tx.final_ena) end )
register(nil,  "Debug/bias/final/disable",  function() bit_off(dbg_tx, dbg_tx.final_ena) end )

register(nil,  "Debug/Rcv/top",       function() recieve_select(dbg_ctrl.rcv_top ) end )
register(nil,  "Debug/Rcv/bot",       function() recieve_select(dbg_ctrl.rcv_bot ) end )
register(nil,  "Debug/Rcv/both",      function() recieve_select(dbg_ctrl.rcv_mask) end )
register(nil,  "Debug/Rcv/none",      function() recieve_select(0                ) end )

