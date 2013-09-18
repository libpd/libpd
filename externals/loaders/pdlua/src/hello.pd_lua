pd.post("Hello, universe!")

local Hello = pd.Class:new():register("hello")

function Hello:initialize(name, atoms)
  pd.post("Hello, world!")
  return true
end

function Hello:finalize()
  pd.post("Bye bye, world!")
end
