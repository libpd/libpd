-- contributed by Frank Barknecht

local LDemux = pd.Class:new():register("ldemux")

function LDemux:initialize(name, atoms)
    local n = atoms[1] or 2 -- default to 2 outlets.
    if type(n) ~= "number" or n < 2 then
        pd.post("ldemux: wrong outlet-count argument, using 2 outlets instead")
        n = 2
    end
    self.outlets = n
    self.inlets = 2
    self.to = 1
    -- second arg, if a number, selects default outlet
    if type(atoms[2]) == "number" then
        self:in_2_float(atoms[2])
    end
    return true
end

function LDemux:in_2_float(f)
    -- clip selection between left- and rightmost outlet
    self.to = math.max(1, math.min(self.outlets, f))
end

function LDemux:in_1(s, m)
  self:outlet(self.to, s, m)
end
