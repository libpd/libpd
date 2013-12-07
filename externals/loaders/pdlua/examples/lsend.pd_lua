-- test send support

local LSend = pd.Class:new():register("lsend")

function LSend:initialize(name, atoms)
  if type(atoms[1]) ~= "string" then
    pd.post("lsend needs a symbol")
    return false
  else
    self.sendto = atoms[1]
  end
  self.inlets = 2
  self.outlets = 0
  return true
end

function LSend:in_2_symbol(s)
  self.sendto = s
end

function LSend:in_1(sel, atoms)
  pd.send(self.sendto, sel, atoms)
end
