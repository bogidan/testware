
if not tx then
  tx = function(msg) print(msg) end
end
if not register then
  register = function(key,menu,func) print("register:", menu, key) func() end
end
if not sleep then sleep = function() print("sleep") end end

--[[ Shortcuts - use prefixes comnined with ascii codes/characters
Prefixes:
  # - Alt
  + - Shift
  ^ - Control
  ! - Meta
  @ - Command (Ctrl on linux/win, Meta on OSX)
 
Examples:
  "c"      -- Uses 'c' as the shortcut
  "#^c"    -- Same as FL_ALT|FL_CTRL|'c'
  "#^!c"   -- Same as FL_ALT|FL_CTRL|FL_META|'c'
  "@c"     -- Same as FL_COMMAND|'c' (see FL_COMMAND for platform specific behavior)
  "0x63"   -- Same as "c" (hex 63=='c')
  "99"     -- Same as "c" (dec 99=='c')
  "0143"   -- Same as "c" (octal 0143=='c')
  "^0x63"  -- Same as (FL_CTRL|'c'), or (FL_CTRL|0x63)
  "^99"    -- Same as (FL_CTRL|'c'), or (FL_CTRL|99)
  "^0143"  -- Same as (FL_CTRL|'c'), or (FL_CTRL|0143)
  nil      -- No shortcut
]]
package.path = "./?.lua;" .. package.path
local adsb   = require("adsb")
--local auxdac = require("auxdac")
local dbgctl = require("dbgctrl")

function temp_check()
	tx("*P\r")
end
function temp_monitor()
	while(true) do temp_check() sleep(1000) end
end

function adsb_toggle()
	tx("* T\r")
end
--register("^t", "Transmit Toggle", adsb_toggle)
register("^p", "Temperature/Check",   temp_check)
register(nil,  "Temperature/Monitor", temp_monitor)
register(nil,  "Temperature/Standby", function() tx("*Ps") end)
register(nil,  "Temperature/Enable",  function() tx("*Pe") end)

register(nil,  "Altitude/Enable",  function() tx("*DA\r")  end)
register(nil,  "Altitude/Disable", function() tx("*DAe\r") end)

--auxdac.register_menu( register );
--dbgctl.register_menu( register );

