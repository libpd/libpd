-- urn class: random selection without repetitions.
-- fbar 2007

-- urn interface:

local urn = {}

function urn.new(size)
    assert(size > 0, "Error: size of urn must be greater than 0")
    local t = {size=size, last=size}
    for i=1,size do
        t[i] = i
    end
    return t
end

function urn.get(u)
    if u.last > 0 then
        local i = math.random(u.last)
        local res = u[i]
        u[i] = u[u.last]
        u.last = u.last - 1
        return res
    else
        return nil
    end
end

function urn.reset(u)
    u.last = u.size
    for i=1,u.size do
        u[i] = i
    end
end

-- Pd class: 

local M = pd.Class:new():register("lurn")

function M:initialize(name, atoms)
    if type(atoms[1]) == "number" and atoms[1] >= 1 then
        self.u = urn.new(math.floor(math.max(atoms[1]), 1))
    else
        self.u = urn.new(1)
    end
    self.inlets = 2
    self.outlets = 2 
    return true
end

function M:finalize()
    self.u = nil
end

function M:in_2_float(f)
    if f >= 1 then
        self.u = urn.new(math.floor(f))
    else
        self:error("size of urn too small. needs to be 1 at least")
    end
end

function M:in_1_clear(atoms)
    urn.reset(self.u)
end

function M:in_1_seed(atoms)
    if type(atoms[1]) == "number" then
        math.randomseed(atoms[1])
    else
        self:error("seed needs a number")
    end
end

function M:in_1_bang()
    local f = urn.get(self.u)
    if type(f) == "number" then
        self:outlet(1, "float", {f - 1})
    else
        self:outlet(2, "bang", {})
    end
end
