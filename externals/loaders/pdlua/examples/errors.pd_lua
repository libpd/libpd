-- no methods, deliberately: test the error reporting!
local errors = pd.Class:new():register("errors")
function errors:initialize(sel, atoms)
  self.inlets = 4
  self.outlets = 0
  return true
end
