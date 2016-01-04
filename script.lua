
if not tx then
  tx = function(msg) print(msg) end
end
if not register then
  register = function(func) print("register: ", func) end
end



function Enable()
	tx("Ena\r")
end

function Disable()
	tx("Dis\r")
end

function long()
	while(true) do
		tx(".")
		sleep(500)
	end
end

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
register("^e", "Heater/Enable",  Enable )
register("^d", "Heater/Disable", Disable)
register(nil,  "Long", long)
