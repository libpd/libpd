-- reimplementation of [metro] with a twist
-- fbar 2007

-- we need an urn:
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

local modes = {"alea", "series", "sequence", "rota"}

local function contains(t,x)
    for k,v in pairs(t) do
        if v == x then return true end
    end
    return false
end

-- pd:

local M = pd.Class:new():register("luametro")

function M:initialize(name, atoms)
    self.periods = {}
    self.i = 1
    self.mode = "sequence"
    self.direction = 1
    self.u = urn.new(math.max(1, #atoms))
    if not self:setperiods(atoms) then return false end
    self.inlets = 2
    self.outlets = 2 
    return true
end

function M:postinitialize()
    self.clock = pd.Clock:new():register(self, "tick")
end

function M:finalize()
    self.clock:destruct()
end

function M:setperiods(periods)
    self.periods = {}
    for i,j in ipairs(periods) do
        if type(j) == "number" then
            self.periods[i] = math.max(0.01, j)
        else
            self:error("non-number in period list!")
            return false
        end
    end
    -- start over at front:
    self.i = 1
    return true
end

function M:in_2(sel, atoms)
    if sel == "list" or sel == "float" then
        self:setperiods(atoms)
    end
end

function M:in_1_float(f)
    if f ~= 0 then
        self:tick()
    else
        self.clock:unset()
    end
end

function M:in_1_bang()
    self.i = 1
    self:tick()
end

function M:in_1_start()
    self.i = 1
    self:tick()
end 

function M:in_1_stop()
    self.clock:unset()
end    

function M:in_1_mode(atoms)
    if type(atoms[1]) == "number" then
        self.mode = modes[atoms[1]]
        if self.mode == "series" then
            self.u = urn.new(#self.periods)
        end
        self.i = 1
    elseif type(atoms[1]) == "string" then
        if contains(modes, atoms[1]) then
            self.mode = atoms[1]
        else 
            self:error("no such mode: " .. atoms[1])
        end
    end
    pd.post("Mode set to: " .. self.mode)
end    


function M:tick()
    self:outlet(1, "bang", {})
    -- pd.post("selection: " .. tostring(self.i))
    if type(self.periods[self.i]) == "number" then
        self.clock:delay(self.periods[self.i])
    end
    if self.mode == "sequence" then
        self.i = self.i + 1 
        if self.i > #self.periods then 
            self.i = 1 
            self:outlet(2, "bang", {})
        end
    elseif self.mode == "alea" then
        self.i = math.random(#self.periods)
    elseif self.mode == "series" then
        local f = urn.get(self.u)
        if not f then
            urn.reset(self.u)
            f = urn.get(self.u)
            self:outlet(2, "bang", {})
        end
        self.i = f
    elseif self.mode == "rota" then
        self.i = self.i + self.direction
        if self.i > #self.periods or self.i < 1 then 
            self.direction = -self.direction
            self.i = self.i + self.direction
            self:outlet(2, "bang", {})
        end
    else
        self:error("Unknown mode")
    end
end
