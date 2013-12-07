-- contributed by Frank Barknecht

local PeekBag = pd.Class:new():register("peekbag")

function PeekBag:initialize(name, atoms)
  self.inlets = 2
  self.outlets = 1
  self.bag = {}
  self.add = false
  return true
end

function PeekBag:in_1_float(f)
    if self.add then
        table.insert(self.bag, f)
    else
        for i=table.getn(self.bag),1,-1 do
            if self.bag[i]==f then
                table.remove(self.bag, i)
                break
            end
        end
    end
end

function PeekBag:in_1_list(l)
    if type(l[2])  == "number" then
        self:in_2_float(l[2])
    end
    if type(l[1])  == "number" then
        self:in_1_float(l[1])
    end
end

function PeekBag:in_1_clear(l)
  self.bag = {}
  self.add = false
end

function PeekBag:in_1_flush(l)
    self:in_1_bang()
    self:in_1_clear()
end

function PeekBag:in_2_float(f)
    if f == 0 then
        self.add = false
    else
        self.add = true
    end
end

function PeekBag:in_1_bang()
    -- print all values of array 
    for i,v in ipairs(self.bag) do 
        self:outlet(1, "float", {v})
    end
end

function PeekBag:in_1_aslist()
    -- print all values of array as list
    if table.getn(self.bag) == 1 then
        self:outlet(1, "float", {self.bag[1]})
    elseif table.getn(self.bag) > 1 then
        self:outlet(1, "list", self.bag)
    end
end


