--[[ 

list-unpack
    Like [unpack] for any kind of type. Argument specifies number of outlets.
    pointers untested rsp. not supported.
    --Written by Frank Barknecht 
    --Fixed for pdlua-0.2svn by Clahde Heiland-Allen
--]]

-- Some footils --

-- outlet selectors
local selectors = {"float", "list", "symbol", "bang", "pointer"}

-- check if item x is in table t:
local function contains(t, x)
  for _,v in ipairs(t) do
    if v == x then return true end
  end
  return false
end

-- Pd class

local ListUnpack = pd.Class:new():register("list-unpack")

function ListUnpack:initialize(name, atoms)
    self.inlets = 1
    if atoms[1] == nil then 
        self.outlets = 1
        return true
    elseif type(atoms[1]) == "number" and atoms[1] >= 0 then
        self.outlets = math.max(atoms[1], 1)
        return true
    else
        pd.post("list-unpack: First arg must be a positive float or empty")
        return false
    end
end

function ListUnpack:Outlet(num, atom)
    -- a better outlet: automatically selects selector
    -- map lua types to pd selectors
    local selectormap = {string = "symbol", number="float", userdata="pointer"}
    self:outlet(num, selectormap[type(atom)], {atom})
end

function ListUnpack:in_1(sel, atoms)
    if not contains(selectors, sel) then
        -- also unpack selector of anythings
        table.insert(atoms, 1, sel)
    end
    local size = math.min(self.outlets, table.getn(atoms))
    for i=size, 1, -1 do
        self:Outlet(i, atoms[i])
    end
end
