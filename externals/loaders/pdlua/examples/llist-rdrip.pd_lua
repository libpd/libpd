--[[ 
  llist-rdrip
  Output a list as a sequence of elements in reverse order
  author Martin Peach 20120419
--]]

-- Pd class

local LlistRdrip = pd.Class:new():register("llist-rdrip")
local selectormap = {string = "symbol", number="float", userdata="pointer"} -- for lua/Pd type conversion

function LlistRdrip:initialize(name, atoms)
  self.inlets = 1
  self.outlets = 1
  return true
end

function LlistRdrip:in_1(sel, atoms) -- anything
  --pd.post("sel is " .. sel);
  --pd.post("number of atoms is ".. #atoms)
  --for i,v in ipairs(atoms) do
  --  pd.post(i .. " = " .. v)
  --end
  if sel == "list" then -- the usual case
    for i = #atoms, 1, -1 do
      -- map lua types to pd selectors
      self:outlet(1, selectormap[type(atoms[i])], {atoms[i]})
    end
  elseif sel == "float" or sel == "symbol" or sel == "pointer" or sel == "bang" then -- single element "lists"
    self:outlet(1, sel, {atoms[1]})
  else -- messages are lists beginning with a selector
    self:outlet(1, selectormap[type(sel)], {sel})
    for i = #atoms , 1 , -1 do
      self:outlet(1, selectormap[type(atoms[i])], {atoms[i]})
    end
  end  
end
-- end of llist-rdrip

