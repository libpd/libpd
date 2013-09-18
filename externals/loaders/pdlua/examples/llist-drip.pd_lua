--[[ 
  llist-drip
  Output a list as a sequence of elements, same as [list-abs/list-drip]
  author Martin Peach 20120403
--]]

-- Pd class

local LlistDrip = pd.Class:new():register("llist-drip")
local selectormap = {string = "symbol", number="float", userdata="pointer"} -- for lua/Pd type conversion

function LlistDrip:initialize(name, atoms)
  self.inlets = 1
  self.outlets = 1
  return true
end

function LlistDrip:in_1(sel, atoms) -- anything
  --pd.post("sel is " .. sel);
  --pd.post("number of atoms is ".. #atoms)
  --for i,v in ipairs(atoms) do
  --  pd.post(i .. " = " .. v)
  --end
  if sel == "list" then -- the usual case
    for i=1, #atoms do
      -- map lua types to pd selectors
      self:outlet(1, selectormap[type(atoms[i])], {atoms[i]})
    end
  elseif sel == "float" or sel == "symbol" or sel == "pointer" or sel == "bang" then -- single element "lists"
    self:outlet(1, sel, {atoms[1]})
  else -- messages are lists beginning with a selector
    self:outlet(1, selectormap[type(sel)], {sel})
    for i=1, #atoms do
      self:outlet(1, selectormap[type(atoms[i])], {atoms[i]})
    end
  end  
end
-- end of llist-drip

