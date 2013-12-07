--[[ 

list-pak
    
    Like [pack] for any kind of type. Argument specifies number of inlets. All
    inlets are hot (as in Max' [pak])

    --Written by Frank Barknecht 

--]]

-- Some footils --

-- selectors
local selectors = {"float", "list", "symbol", "bang", "pointer"}

-- check if item x is in table t:
local function contains(t, x)
  for _,v in ipairs(t) do
    if v == x then return true end
  end
  return false
end

-- Pd class

local ListPak = pd.Class:new():register("list-pak")

function ListPak:initialize(name, atoms)
    self.outlets = 1
    self.stored = {}
    if atoms[1] == nil then 
        self.inlets = 1
    elseif type(atoms[1]) == "number" and atoms[1] >= 0 then
        self.inlets = math.max(atoms[1], 1)
    else
        pd.post("list-pak: First arg must be a positive float or empty")
        return false
    end
    for i=1,self.inlets do
        table.insert(self.stored, 0)
    end
    return true
end

function ListPak:in_n(i, sel, atoms)
    if not contains(selectors, sel) then
        -- insert selector
        self.stored[i] =  sel
    else
        if table.getn(atoms) > 0 then
            self.stored[i] =  atoms[1]
        end
    end
    self:outlet(1, "list", self.stored)
end
