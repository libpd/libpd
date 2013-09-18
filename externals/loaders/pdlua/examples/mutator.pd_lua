Mutator = pd.Class:new():register("mutator")

function Mutator:initialize()
  self.inlets = 1
  return true
end

function Mutator:in_1_bang()
  pd.post("Radiation Alert!")
  Mutatee.in_1_bang = function(self)
    pd.post("Oh noes! I've been mutated!")
  end
end
