--[[ 
  lfloat2bytes
  Output a float as a sequence of bytes
  author Martin Peach 20120420
--]]

-- Pd class

local lfloat2bytes = pd.Class:new():register("lfloat2bytes")
local digitstr = "%g" -- defaut format string gives up to 6 significant digits

function lfloat2bytes:initialize(name, atoms)
  self.inlets = 1
  self.outlets = 1
  if (#atoms > 0) then
    --pd.post(#atoms .. " args")
    --pd.post("atom 1: " .. atoms[1])
    --pd.post(type(atoms[1]))
    if (type(atoms[1]) ~= "number") then
      self:error("lfloat2bytes: Digit count not a number")
    elseif (atoms[1] > 0) and (atoms[1] <= 24) then
      digitstr = "%0." .. atoms[1] .. "g"
    else
      self:error("lfloat2bytes: Digit count out of range")
    end
  end

  return true
end

-- lfloat2bytes:drip: output characters from buf as floats on [0..255]
function lfloat2bytes:drip(buf)
  --pd.post(buf.. " size " .. #buf)
  local i = 1, b
  local len = #buf

  for i = 1, len do
    self:outlet(1, "float", {string.byte(buf, i)})
  end
end

function lfloat2bytes:in_1(sel, atoms) -- anything
  --pd.post("sel is " .. sel);
  --pd.post("number of atoms is ".. #atoms)
  --for i,v in ipairs(atoms) do
  --  pd.post(i .. " = " .. v)
  --end
  local buf
  if sel == "float" then -- the usual case
    --buf = tostring(atoms[1])
    buf = string.format(digitstr, atoms[1])
    self:drip(buf)
  else -- reject non-floats
    self:error("lfloat2bytes: input not a float")
  end
end
-- end of lfloat2bytes

