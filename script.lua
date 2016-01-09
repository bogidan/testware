
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
local auxdac = require("auxdac")
local dbgctl = require("dbgctrl")

function long()
	while(true) do tx(".") sleep(500) end
end

function adsb_toggle()
	tx("* T\r")
end
register("^t", "Transmit Toggle", adsb_toggle)

--auxdac.register_menu( register );
--dbgctl.register_menu( register );

