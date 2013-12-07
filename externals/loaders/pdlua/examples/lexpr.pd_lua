local lexpr = pd.Class:new():register("lexpr")

local function sandbox(e, f) -- only supports nullary f() with one return
  local g = getfenv(f)
  setfenv(f, e)
  local r = f()
  setfenv(f, g)
  return r
end

local lexpr_globals = {
  abs    = math.abs,
  acos   = math.acos,
  asin   = math.asin,
  atan   = math.atan,
  atan2  = math.atan2,
  ceil   = math.ceil,
  cos    = math.cos,
  cosh   = math.cosh,
  deg    = math.deg,
  exp    = math.exp,
  floor  = math.floor,
  fmod   = math.fmod,
  log    = math.log,
  log10  = math.log10,
  max    = math.max,
  min    = math.min,
  int    = function (x) i,f = math.modf(x) ; return i end,
  wrap   = function (x) i,f = math.modf(x) ; return f end,
  pi     = math.pi,
  pow    = math.pow,
  rad    = math.rad,
  sin    = math.sin,
  sinh   = math.sinh,
  sqrt   = math.sqrt,
  tan    = math.tan,
  tanh   = math.tanh,
  val    = function (s) return (pd.getvalue(s) or 0) end,
  choose = function (b,t,f) if b then return t else return f end end
}

function lexpr:readexpr(atoms)
  local vname = { }
  local context = { }
  local expr
  local i
  local k
  local v
  local j = 1
  local inlets
  local f
  local phase = "vars"
  for k,v in pairs(lexpr_globals) do
    context[k] = v
  end
  for i,v in ipairs(atoms) do
    if phase == "vars" then        -- create variables
      if v == "->" then
        inlets = i - 1
        phase = "expr"
        expr = ""
      else
        if type(v) == "string" then
          vname[j] = v
          context[v] = 0
          j = j + 1
        else
          self:error("lexpr: variable names must be symbols")
          return -1
        end
      end
    else if phase == "expr" then  -- build string
      expr = expr .. " " .. v
    else
      self:error("lexpr: internal error parsing expression")
      return -1
    end end
  end
  f = assert(loadstring("return {" .. expr .. " }"))
  local outlets = #(sandbox(context, f))
  return inlets, vname, context, f, outlets
end

function lexpr:initialize(sel, atoms)
  self.vname = { }
  self.context = { }
  self.hot = { }
  self.f = function () return 0 end
  function self:in_1_bang()
    local r = sandbox(self.context, self.f)
    local i
    for i = self.outlets,1,-1 do
      if type(r[i]) == "number" then
        self:outlet(i, "float", { r[i] })
      else if type(r[i]) == "string" then
        self:outlet(i, "symbol", { r[i] })
      else
        self:error("calculated a " .. type(r[i]) .. " but expected a number or a string")
      end end
    end
  end
  function self:in_n_float(i, f)
    self.context[self.vname[i]] = f
    if self.hot[i] then self:in_1_bang() end
  end
  function self:in_n_symbol(i, s)
    self.context[self.vname[i]] = s
    if self.hot[i] then self:in_1_bang() end
  end
  function self:in_n_hot(i, atoms)
    if type(atoms[1]) == "number" then
      self.hot[i] = atoms[1] ~= 0
    else
      self:error("hot method expects a float")
    end
  end
  function self:in_1_lexpr(atoms)
    local inlets
    local vname
    local context
    local f
    local outlets
    inlets, vname, context, f, outlets = self:readexpr(atoms)
    if (inlets == self.inlets) and (outlets == self.outlets) then
      self.vname = vname
      self.context = context
      self.f = f
    else
      self:error("new expression has different inlet/outlet count")
    end
  end
  self.inlets, self.vname, self.context, self.f, self.outlets = self:readexpr(atoms)
  if self.inlets < 1 then
    pd.post("lexpr: error: would have no inlets")
    return false
  end
  if self.outlets < 1 then
    pd.post("lexpr: error: would have no outlets")
    return false
  end
  for i = 1,self.inlets,1 do
    self.hot[i] = i == 1
  end
  return true
end
