local Revalue = pd.Class:new():register("revalue")

function Revalue:initialize(sel, atoms)
  self.inlets = 2
  self.outlets = 2
  if type(atoms[1]) == "string" then
    self.name = atoms[1]
  else
    self.name = nil
  end
  return true
end

-- get a value or report if it doesn't exist
function Revalue:in_1_bang()
  if self.name ~= nil then
    local r = pd.getvalue(self.name)
    if r ~= nil then
      self:outlet(1, "float", { r })
      return
    end
  end
  self:outlet(2, "bang", { })
end

-- set a value or report if it doesn't exist
function Revalue:in_1_float(f)
  if self.name ~= nil then
    if pd.setvalue(self.name, f) then
      return
    end
  end
  self:outlet(2, "bang", { })
end

-- set the [value] name
function Revalue:in_2_symbol(s)
  self.name = s
end
