local SimpleCounter = pd.Class:new():register("simplecounter")

function SimpleCounter:initialize(sel, atoms)
  self.inlets = 1
  self.outlets = 1
  self.count = 0
  if type(atoms[1]) == "number" then self.count = atoms[1] end
  return true
end

function SimpleCounter:in_1_bang()
  self:outlet(1, "float", {self.count})
  self.count = self.count + 1
end
