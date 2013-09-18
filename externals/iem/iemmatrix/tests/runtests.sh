#!/bin/sh

if [ "x${PD}" = "x" ]
then
 PD=pd
fi

RUNTESTS_TXT=runtests.txt
RUNTESTS_LOG=runtests.log

ls -1 */*.pd | sed 's/\.pd/;/' > $RUNTESTS_TXT

IEMMATRIX="-lib ../iemmatrix -path ../abs/"

function run_nogui() {
 pd $IEMMATRIX -nogui runtests_nogui.pd > ${RUNTESTS_LOG}.$$ 2>&1 
 NUMTESTS=`grep -c . $RUNTESTS_TXT`
 echo "regression-test: ${NUMTESTS} tests total" >>  ${RUNTESTS_LOG}.$$
 
 cat ${RUNTESTS_LOG}.$$ | egrep "^regression-test: " | sed -e 's/^regression-test: //'
 FAILEDTESTS=$(cat ${RUNTESTS_LOG}.$$ | egrep "^regression-test: .*: failed$" | sed -e 's|^regression-test: ||' -e 's|: failed$||')
 echo -n "failed tests: "
 for ft in ${FAILEDTESTS}; do
   echo -n "${ft} "
 done
 echo
}

function run_withgui() {
 pd $IEMMATRIX -stderr runtests.pd 2>&1 | tee ${RUNTESTS_LOG}
}

if test "x$1" = "x-gui"; then
 run_withgui
else
 run_nogui
fi


