local ltabfill = pd.Class:new():register("ltabfill")

local function sandbox(e, f, x) -- only supports unary f() with one return
  local g = getfenv(f)
  setfenv(f, e)
  local r = f(x)
  setfenv(f, g)
  return r
end

local ltabfill_globals = {
  abs   = math.abs,
  acos  = math.acos,
  asin  = math.asin,
  atan  = math.atan,
  atan2 = math.atan2,
  ceil  = math.ceil,
  cos   = math.cos,
  cosh  = math.cosh,
  deg   = math.deg,
  exp   = math.exp,
  floor = math.floor,
  fmod  = math.fmod,
  log   = math.log,
  log10 = math.log10,
  max   = math.max,
  min   = math.min,
  int   = function (x) i,f = math.modf(x) ; return i end,
  wrap  = function (x) i,f = math.modf(x) ; return f end,
  pi    = math.pi,
  pow   = math.pow,
  rad   = math.rad,
  sin   = math.sin,
  sinh  = math.sinh,
  sqrt  = math.sqrt,
  tan   = math.tan,
  tanh  = math.tanh,
  val   = function (s) return (pd.getvalue(s) or 0) end
}

function ltabfill:readexpr(atoms)
  local vname = { }
  local context = { }
  local expr
  local i
  local k
  local v
  local j = 2
  local inlets
  local f
  local phase = "table"
  for k,v in pairs(ltabfill_globals) do
    context[k] = v
  end
  for i,v in ipairs(atoms) do
    if phase == "table" then
      if type(v) == "string" then
        self.tabname = v
        phase = "vars"
      else
        self:error("ltabfill: table name must be a symbol")
        return -1
      end
    else if phase == "vars" then        -- create variables
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
          self:error("ltabfill: variable names must be symbols")
          return -1
        end
      end
    else if phase == "expr" then  -- build string
      expr = expr .. " " .. v
    else
      self:error("ltabfill: internal error parsing expression")
      return -1
    end end end
  end
  f = assert(loadstring("return function(x) return " .. expr .. " end"))()
  return inlets, vname, context, f, 0
end

function ltabfill:initialize(sel, atoms)
  self.tabname = nil
  self.vname = { }
  self.context = { }
  self.hot = { }
  self.f = function (x) return 0 end
  function self:in_1_bang()
    if self.tabname ~= nil then
      local t = pd.Table:new():sync(self.tabname)
      if t ~= nil then
        local i
        local l = t:length()
        for i = 1,l do
          local y = sandbox(self.context, self.f, (i-1)/l)
          t:set(i-1, y)
        end
        t:redraw()
      end
    end
  end
  function self:in_1_symbol(s)
    self.tabname = s
    if self.hot[1] then self:in_1_bang() end
  end
  function self:in_1_float(f)
    self:error("table name expected, got a float")
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
  function self:in_1_ltabfill(atoms)
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
    pd.post("ltabfill: error: would have no inlets")
    return false
  end
  for i = 1,self.inlets,1 do
    self.hot[i] = i == 1
  end
  return true
end
