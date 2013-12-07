Mutatee = pd.Class:new():register("mutatee")

function Mutatee:initialize()
  self.inlets = 1
  return true
end

function Mutatee:in_1_bang()
  pd.post("I'm happy and carefree!")
end
