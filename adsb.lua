if not sleep then sleep = function() print("sleep") end end
if not tx then tx = function(s) print(s) end end
if not register then register = function(a,b,c) print("register",a,b,c) end end

local adsb_timeout = 100 -- milliseconds

local msg_install = {
  0xA5, 0x01, 0x1A,
  -- ICAO Address
  0x1C, 0xA6, 0xB2,
  -- Aircraft Registration
  0x4E, 0x43, 0x31, 0x39, 0x38, 0x5A, 0x20, 0x20,
  -- Max Airspeed, COM Port 0, COM Port 1
  0x00, 0x00, 0x00,
  -- GPS WoW, GPS Integrity, Emitter Category Set, Emitter Category
  0x00, 0x01, 0x06, 0x00,
  -- Aircraft Size, Altitude Encode offset {2}
  0x00, 0x00, 0x06,
  -- ADS-B {Control, In Reported Participants, Out DF Field} 
  0x64, 0x12, 0x01,
  -- Reserved
  0x00, 0x00,
  -- Checksum
  0x85
}
local msg_gps = {
  0xA5, 0x04, 0x3F,
  -- GPS Longitude {11}
  0x30, 0x35, 0x39, 0x33, 0x30, 0x2E, 0x30, 0x30, 0x30, 0x30, 0x30,
  -- GPS Latitude {10}
  0x33, 0x30, 0x30, 0x30, 0x2E, 0x30, 0x30, 0x30, 0x30, 0x30,
  -- GPS Speed Over Ground {6}
  0x30, 0x39, 0x39, 0x2E, 0x30, 0x30,
  -- GPS Coarse Over Ground {8}
  0x31, 0x38, 0x30, 0x2E, 0x30, 0x30, 0x30, 0x30,
  0x03, -- Hemisphere
  -- GPS Time of Fix {10}
  0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x2E, 0x37, 0x38, 0x39,
  -- Height
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, --Reserved
  0xAD, -- Checksum
}


function adsb_load(fn)
  file = io.open(fn, "r")
  if file then
    for line in file:lines() do repeat
      line = line:gsub("//.+", "")
      line = line:gsub("%s+$","")
      if line == "" then break end
      
      local ts, adsb_str = line:match("(%d+)(.+)")
      if ts == nil then break end
      
      adsb_str = adsb_str:gsub("%s","")
      sleep(adsb_timeout)
      tx("*AA "..ts.." "..adsb_str.."\r")
    until true end
  else
    print("No File")
  end
end

function adsb_play () tx("*AP\r") end
function adsb_reset() tx("*AS\r") end

-- Lua implementation of PHP scandir function
function scandir(directory)
    local i, t, popen = 0, {}, io.popen
    --for filename in popen('ls -a "'..directory..'"'):lines() do
    for filename in popen('dir "'..directory..'" /b /a'):lines() do
      i = i + 1
      t[i] = filename
    end
    return t
end

function load_tests(dir)
  local tests = scandir(dir)
  table.sort(tests)
  for k,fn in pairs(tests) do
    register(nil, "ADS-B/Tests/"..fn, function()
      adsb_reset()
      adsb_load(dir..fn)
    end)
  end
end

local default_fn = "tests\\INTC-006.txt"--"./INTC-069-tiny.txt"

load_tests(".\\ADS-B Tests\\")

--register(nil, "ADS-B/Load File",adsb_load)
--register(nil, "ADS-B/Insert",   function() tx("*AA 100000 98690001A8058400000000\r") end)
register(nil, "ADS-B/Play",     adsb_play)
register(nil, "ADS-B/Reset",    adsb_reset)



