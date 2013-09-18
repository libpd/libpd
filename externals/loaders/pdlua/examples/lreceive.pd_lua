-- test receive support

local LReceive = pd.Class:new():register("lreceive")

function LReceive:initialize(name, atoms)
  if type(atoms[1]) ~= "string" then
    pd.post("lreceive needs a prefix for the receive names!")
    return false
  else
    self.prefix = atoms[1]
  end
  if type(atoms[2]) ~= "number" or atoms[2] < 1 then
    self.start = 1
  else
    self.start = math.floor(atoms[2])
  end
  if type(atoms[3]) ~= "number" or atoms[3] < 1 then
    self.count = 1
  else
    self.count = math.floor(atoms[3])
  end
  self.receives = { }
  self.inlets = 0
  self.outlets = 2
  return true
end

function LReceive:postinitialize()
  local i = 0
  while (i < self.count) do
    local n = self.start + i
    self.receives[i+1] = pd.Receive:new():register(self, self.prefix .. n, "receive_" .. n)
    self["receive_" .. n] = function (self, sel, atoms)
      self:outlet(2, "float", { n })
      self:outlet(1, sel, atoms)
    end
    i = i + 1
  end
end

function LReceive:finalize()
  for _,r in ipairs(self.receives) do r:destruct() end
end

