@echo off

pd -stderr -nogui -path .. -path ../../examples/data -path ../abstractions -lib Gem -open existence.pd -send "pd quit" > existence.log 2>&1
