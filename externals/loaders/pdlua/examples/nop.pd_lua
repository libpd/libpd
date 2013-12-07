local Nop = pd.Class:new():register("nop")

function Nop:initialize()
  self.inlets = 1
  self.outlets = 1
  return true
end

function Nop:in_1(s, m)
  self:outlet(1, s, m)
end
