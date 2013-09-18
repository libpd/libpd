local R = pd.Class:new():register("reverb-calculator")

function R:initialize(sel, atoms)
  if type(atoms[1]) ~= "number" then
    return false
  end
  self.count = math.max(atoms[1], 1)
  self.inlets = 1
  self.outlets = 1
  return true
end

local gcd = function(m, n)
  while m ~= 0 do m, n = math.mod(n, m), m end
  return n
end

function R:fundamental(nx, ny, nz)
  if (nx == 0 and ny == 0 and nz == 0) then return false end
  if (nx == 0 and ny == 0 and nz == 1) then return true  end
  if (nx == 0 and ny == 1 and nz == 0) then return true  end
  if (nx == 1 and ny == 0 and nz == 0) then return true  end
  return gcd(gcd(nx, ny), nz) == 1
end

function R:delay(lx, ly, lz, nx, ny, nz)
  return 1000 / ((340/2) * math.sqrt((nx*nx)/(lx*lx)+(ny*ny)/(ly*ly)+(nz*nz)/(lz*lz)))
end

function R:in_1_list(atoms)
  local lx = atoms[1]
  local ly = atoms[2]
  local lz = atoms[3]
  local delays = { }
  for nx = 0, 16 do
  for ny = 0, 16 do
  for nz = 0, 16 do
    if self:fundamental(nx, ny, nz) then
      table.insert(delays, self:delay(lx, ly, lz, nx, ny, nz))
    end
  end end end
  table.sort(delays, function (a, b) return a > b end)
  local out = { }
  local j = 1
  for i = 1, self.count do
    out[i] = delays[j]
    repeat
      j = j + 1
    until delays[j] ~= delays[j-1]
  end
  self:outlet(1, "list", out)
end
