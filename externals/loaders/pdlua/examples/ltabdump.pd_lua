local LTabDump = pd.Class:new():register("ltabdump")

function LTabDump:initialize(sel, atoms)
  self.inlets = 1
  self.outlets = 2
  if type(atoms[1]) == "string" then
    self.name = atoms[1]
  else
    self.name = nil
  end
  return true
end

-- output table length and data
function LTabDump:in_1_bang()
  -- need to sync each time before accessing if ...
  -- ... control-flow has gone outside of our scope
  local t = pd.Table:new():sync(self.name)
  if t ~= nil then
    local l = t:length()
    local a = { }
    local i
    for i = 1,l do
      a[i] = t:get(i-1)
    end
    -- copied above before outlet() to avoid race condition
    self:outlet(2, "float", { l })
    self:outlet(1, "list", a)
  end
end

-- choose a table name, then output
function LTabDump:in_1_symbol(s)
  self.name = s
  self:in_1_bang()
end
