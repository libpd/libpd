--[[ 
  lsymbol2bytes
  Output a symbol as a sequence of bytes
  author Martin Peach 20120420
--]]

-- Pd class

local Lsymbol2Bytes = pd.Class:new():register("lsymbol2bytes")

function Lsymbol2Bytes:initialize(name, atoms)
  self.inlets = 1
  self.outlets = 1
  return true
end

-- Lsymbol2Bytes:drip: output possibly multibyte characters from buf and output them as floats on [0..255]
function Lsymbol2Bytes:drip(buf)
  --pd.post(buf.. " size " .. #buf)
  local i = 1, b
  local len = #buf

  for i = 1, len do
    self:outlet(1, "float", {string.byte(buf, i)})
  end
end

function Lsymbol2Bytes:in_1(sel, atoms) -- anything
  --pd.post("sel is " .. sel);
  --pd.post("number of atoms is ".. #atoms)
  --for i,v in ipairs(atoms) do
  --  pd.post(i .. " = " .. v)
  --end
  local buf
  if sel == "symbol" then -- the usual case
    buf = tostring(atoms[1])
    self:drip(buf)
  elseif #atoms == 0 then -- a selector
    buf = tostring(sel)
    self:drip(buf)
  else -- reject non-symbols
    self:error("lsymbol2bytes: input not a symbol")
  end
end
-- end of lsymbol2bytes

