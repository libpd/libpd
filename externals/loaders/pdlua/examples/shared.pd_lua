local Shared = pd.Class:new():register("shared")
local sharedStore = { }

function Shared:initialize(sel, atoms)
  if type(atoms[1]) == "string" then
    if sharedStore[atoms[1]] == nil then
      sharedStore[atoms[1]] = { sel = "bang", msg = { }, count = 1 }
    else
      sharedStore[atoms[1]].count = sharedStore[atoms[1]].count + 1
    end
    self.name = atoms[1]
    self.inlets = 2
    self.outlets = 1
    return true
  else
    return false
  end
end

function Shared:in_1_bang()
  self:outlet(1, sharedStore[self.name].sel, sharedStore[self.name].msg)
end

function Shared:in_2(s, m)
  local c = sharedStore[self.name].count
  sharedStore[self.name] = { sel = s, msg = m, count = c }
end

function Shared:finalize()
  sharedStore[self.name].count = sharedStore[self.name].count - 1
  if sharedStore[self.name].count == 0 then
    sharedStore[self.name] = nil
  end
end
