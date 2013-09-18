-- reimplementation of [pipe] for any message
-- claude(+fbar) 2008

local M = pd.Class:new():register("lpipe")

function M:initialize(name, atoms)
    self.inlets = 2
    self.outlets = 1 
    self.nextID = 0
    return true
end

function M:in_2_float(f)                                                                            
   self.deltatime = math.max(0, f)                                                                  
end                                                                                                 
                                                                                                    
function M:in_1(sel, atoms)                                                                         
    -- we need unique method names for our clock callbacks                                           
    self.nextID = self.nextID + 1                                                                    
    local id = "trigger" .. self.nextID                                                              
    local clock = pd.Clock:new():register(self, id)                                                  
    -- the clock callback outlets the data and cleans up                                             
    self[id] = function(self)                                                                        
        self:outlet(1, sel, atoms)                                                                     
        clock:destruct()                                                                               
        self[id] = nil                                                                                 
    end                                                                                              
    -- now start the clock                                                                           
    clock:delay(self.deltatime)                                                                      
end
