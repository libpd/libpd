local R = pd.Class:new():register("requirer")

function R:initialize(sel, atoms)
  if type(atoms[1]) ~= "string" then 
    pd.post(self._name .. ": Missing module name")
    return false
  end
  pd.post (self._name .. ": Looking for " .. atoms[1])  
  require(atoms[1]) -- load the module
  if _G[atoms[1]] == nil then
      pd.post (self._name .. ": No such module (" .. atoms[1] .. ")")
  else -- print out module contents
      for k,v in pairs(_G[atoms[1]]) do
        pd.post(atoms[1].. "." .. tostring(k) .. " = " .. tostring(v))
      end
  end
  self.inlets = 0
  self.outlets = 0
  return true
end

