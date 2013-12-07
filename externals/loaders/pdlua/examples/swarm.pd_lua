-- see also: http://www.vergenet.net/~conrad/boids/pseudocode.html
local swarm = pd.Class:new():register("swarm")

function swarm:initialize(sel, atoms) -- constructor
  if type(atoms[1]) ~= "number" or atoms[1] < 2 then return false end
  if type(atoms[2]) ~= "number" or atoms[2] < 3 then return false end
  self.dim = math.floor(atoms[1])
  self.count = math.floor(atoms[2])
  self.cluster = 0.05 -- magic values look ok in the help patch..
  self.distance2 = 0.2
  self.similar2 = 0.1
  self.friction = 0.96
  self.flock = { }
  self:in_1_randomize()
  self.inlets = 2
  self.outlets = 2
  return true
end

function swarm:in_1_randomize() -- randomize positions, no movement
  for i = 1, self.count do
    self.flock[i] = { x = { }, dx = { } }
    for j = 1, self.dim do
      self.flock[i].x[j] = math.random() - 0.5
      self.flock[i].dx[j] = 0
    end
    self.flock[i].w = math.random() + 0.5
  end
end

function swarm:in_1_bang()  -- update and output
  local c = self:center()
  for i = 1, self.count do
    f = self.flock[i]                    -- update
    local v1 = self:rule1(c, f)
    local v2 = self:rule2(i, f)
    local v3 = self:rule3(i, f)
    for k = 1, self.dim do f.dx[k] = f.dx[k] + v1[k] + v2[k] + v3[k] end
    for k = 1, self.dim do f.dx[k] = f.dx[k] * self.friction end
    for k = 1, self.dim do f.x[k] = f.x[k] + f.dx[k] end
    self:outlet(2, "float", { i })      -- output
    self:outlet(1, "list", f.x)
  end
end

function swarm:center() -- center of mass
  local c = { }
  local w = 0
  for k = 1, self.dim do c[k] = 0 end
  for i = 1, self.count do
    w = w + self.flock[i].w
    for k = 1, self.dim do c[k] = c[k] + self.flock[i].w * self.flock[i].x[k] end
  end
  for k = 1, self.dim do c[k] = c[k] / w end
  return c
end

function swarm:rule1(c, f) -- clustering
  local v = { }
  for k = 1, self.dim do v[k] = self.cluster * (c[k] - (1 + f.w) * f.x[k]) end
  return v
end

function swarm:rule2(i, f) -- avoidance
  local v = { }
  for k = 1, self.dim do v[k] = 0 end
  for j = 1, self.count do
    if i ~= j then
      g = self.flock[j]
      local d = { }
      local m = 0
      for k = 1, self.dim do d[k] = g.x[k] - f.x[k] ; m = m + d[k] * d[k] end
      if m < self.distance2 then
        for k = 1, self.dim do v[k] = v[k] - d[k] end
      end
    end
  end
  for k = 1, self.dim do v[k] = 0.01 * v[k] end
  return v
end

function swarm:rule3(i, f) -- similarity
  local v = { }
  for k = 1, self.dim do v[k] = 0 end
  for j = 1, self.count do
    if i ~= j then
      g = self.flock[j]
      local d = { }
      local m = 0
      for k = 1, self.dim do d[k] = g.dx[k] - f.dx[k] ; m = m + d[k] * d[k] end
      if m < self.similar2 then
        for k = 1, self.dim do v[k] = v[k] + d[k] end
      end
    end
  end
  for k = 1, self.dim do v[k] = 0.004 * v[k] end
  return v
end

-- exercise: make the right inlet control individual elements
