-- test multiple clocks

local LDelay = pd.Class:new():register("ldelay2")

function LDelay:initialize(name, atoms)
  if type(atoms[1]) ~= "number" or atoms[1] < 0 then
    self.delay = 1000
  else
    self.delay = atoms[1]
  end
  self.inlets = 2
  self.outlets = 2
  return true
end

function LDelay:postinitialize()
  self.clock1 = pd.Clock:new():register(self, "trigger1")
  self.clock2 = pd.Clock:new():register(self, "trigger2")
end

function LDelay:finalize()
  self.clock1:destruct()
  self.clock2:destruct()
end

function LDelay:in_2_float(f)
  self.delay = math.max(0, f)
end

function LDelay:in_1_float(f)
  self:in_2_float(f)
  self:in_1_bang()
end

function LDelay:in_1_bang()
  self.clock1:delay(self.delay)
  self.clock2:delay(self.delay * 2)
end

function LDelay:in_1_stop()
  self.clock1:unset()
  self.clock2:unset()
end  

function LDelay:trigger1()
  self:outlet(1, "bang", {})
end

function LDelay:trigger2()
  self:outlet(2, "bang", {})
end
