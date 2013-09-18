#!/bin/bash

## TODO:
##  find Gem (either in ../src or ../)
##  if it is not there, assume it is split into externals

if [ "x${PD}" = "x" ]
then
 PD=pd
fi


#SUFFIX="$$"
SUFFIX=$(date +%y%m%d-%H%M%S)

RUNTESTS_TXT=runtests.txt
RUNTESTS_LOG=log-runtests.${SUFFIX}

LIBFLAGS="-path ../src:../ -lib Gem -path ../abstractions/"

list_tests() {
#  find . -mindepth 2  -name "*.pd" | sed 's|\.pd$|;|' 
 ls -1 */*.pd | sed 's|\.pd$|;|'
}

debug() {
 :
if [ "x${DEBUG}" = "xyes" ]; then echo $@; fi
}


evaluate_tests() {
 local logfile
 local testfile
 local numtests

 testfile=$1
 logfile=$2

 debug "now evaluating results in ${logfile} (${testfile}"

 numtests=$(grep -c . ${testfile})
 numpass=$(egrep -c "regression-test: (.*/fail.*: failed|.*: OK)$" ${logfile})
 let numfail=0
 failtests=""
 for t in $(egrep "regression-test: .*: (failed|OK)$" ${logfile} | egrep -v "regression-test: (.*/fail.*: failed|.*: OK)$" | awk '{print $2}')
 do
  failtests="${failtests} ${t%:}"
  let numfail=numfail+1
 done
 debug "number of tests = ${numtests}"
 echo "regression-test: ======================================" >>  ${logfile}
 echo "regression-test: ${numtests} regression-tests total" >>  ${logfile}
 echo "regression-test: ${numpass} regression-tests passed" >>  ${logfile}
 echo "regression-test: ${numfail} regression-tests failed" >>  ${logfile}
 echo "regression-test: ======================================" >>  ${logfile}
 echo "regression-test: failed tests: ${failtests}" >> ${logfile}
 debug "show results"
 cat ${logfile} | egrep "^regression-test: " | sed -e 's/^regression-test: //'
}


run_nogui() {
 debug "running test without gui"
 ${PD} ${LIBFLAGS} -nogui runtests_nogui.pd > ${RUNTESTS_LOG} 2>&1 
 debug "testing done"
 evaluate_tests ${RUNTESTS_TXT} ${RUNTESTS_LOG}
 debug "testing finished"
}

run_withgui() {
 debug "running test with gui"
 ${PD} ${LIBFLAGS} -stderr runtests.pd 2>&1 | tee ${RUNTESTS_LOG}
 echo "testing completed, no evaluation will be done; see ${RUNTESTS_LOG} for results"
}

list_tests > ${RUNTESTS_TXT}

USEGUI=""
DEBUG=""

while [ "$@" ]
do
 if test "x$1" = "x-gui"; then
  USEGUI="yes"
 fi
 if test "x$1" = "x-debug"; then
  DEBUG="yes"
 fi
 if test "x$1" = "x-d"; then
  DEBUG="yes"
 fi
 shift
done

if [ "x${USEGUI}" = "xyes" ]; then
 run_withgui
else
 run_nogui
fi

echo $@
