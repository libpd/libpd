--[[ 
  lsymbol-drip
  Output a symbol as a sequence of characters
  author Martin Peach 20120403
--]]

-- Pd class

local LsymbolDrip = pd.Class:new():register("lsymbol-drip")

function LsymbolDrip:initialize(name, atoms)
  self.inlets = 1
  self.outlets = 1
  return true
end

-- LsymbolDrip:drip: accumulate possibly multibyte characters from buf and output them as symbols
function LsymbolDrip:drip(buf)
  --pd.post(buf.. " size " .. #buf)
  local i = 1, j, b
  local len = #buf

  while i <= len do
    j = i -- character is one byte long
    b = string.byte(buf, i)
    --pd.post("*b is " .. b .. " i is " .. i)
    while b > 0x7f and j < len do -- multibyte characters have their high bit set
      j = j + 1 
      b = string.byte(buf, j)
      --pd.post("_b is " .. b .. " j is " .. j)
    end
    if j ~= i and j < len then j = j - 1 end -- j was pointing to the next character
    self:outlet(1, "symbol", {string.sub(buf, i, j)})
    i = j + 1 -- start of next character
  end
end

function LsymbolDrip:in_1(sel, atoms) -- anything
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
    self:error("lsymbol-drip: input not a symbol")
  end
end
-- end of lsymbol-drip

