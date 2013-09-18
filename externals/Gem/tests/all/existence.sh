#!/bin/sh

if test "x${PD}" = "x"
then
 PD=pd
fi

${PD} -stderr -nogui -path ../../src:../../examples/data -lib Gem -open existence.pd -send "pd quit"> existence.log 2>&1

cat existence.log | egrep -v "^creating: " | egrep -v "^destroying"
