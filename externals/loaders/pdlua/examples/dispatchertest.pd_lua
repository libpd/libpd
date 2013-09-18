local DispatcherTest = pd.Class:new():register("dispatchertest")

function DispatcherTest:initialize(name, atoms)
  self.inlets = 3
  return true
end

function DispatcherTest:in_1_float(f)
  pd.post("float: " .. f)
end

function DispatcherTest:in_2_symbol(s)
  pd.post("symbol: " .. s)
end

function DispatcherTest:in_3(sel, atoms)
  pd.post(sel .. ":")
  for i,v in ipairs(atoms) do
    pd.post(i .. " = " .. v)
  end
end
