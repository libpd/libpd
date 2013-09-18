-- reimplementation of [delay] in Lua to test clock support

local LDelay = pd.Class:new():register("ldelay")

function LDelay:initialize(name, atoms)
  if type(atoms[1]) ~= "number" or atoms[1] < 0 then
    self.delay = 1000
  else
    self.delay = atoms[1]
  end
  self.inlets = 2
  self.outlets = 1
  return true
end

function LDelay:postinitialize()
  self.clock = pd.Clock:new():register(self, "trigger")
end

function LDelay:finalize()
  self.clock:destruct()
end

function LDelay:in_2_float(f)
  self.delay = math.max(0, f)
end

function LDelay:in_1_float(f)
  self:in_2_float(f)
  self:in_1_bang()
end

function LDelay:in_1_bang()
  self.clock:delay(self.delay)
end

function LDelay:in_1_stop()
  self.clock:unset()
end  

function LDelay:trigger()
  self:outlet(1, "bang", {})
end
