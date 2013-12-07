if [ -z "$1" ]; then
  # no argument - run all tests found in current dir
  set -e
  for i in *.ref; do sh $0 "${i/.ref/}"; done
  exit 0
fi
KEEP_OUTPUT=0
if [ "x$1" = "x-k" ]; then
  KEEP_OUTPUT=1
  shift
fi
if [ ! -f "$1.pd" ]; then
  echo -e "error: $1.pd does not exist" 1>&2
  exit 1
fi
if [ ! -f "$1.ref" ]; then
  echo -e "error: $1.ref does not exist" 1>&2
  exit 1
fi
sed -e "s|%TESTCASE%|$1|" -e "s|%OUTPUT%|$1.out|" runtest.pd.in > "runtest-$1.pd" || exit 1
echo -n "Running test '$1'... ";
"$PD_PATH/bin/pd" -noprefs -nogui -path .. -lib tclpd "runtest-$1.pd"
diff --strip-trailing-cr "$1.ref" "$1.out" 1>/dev/null 2>&1
RESULT=$?
if [ $RESULT -eq 0 ]; then
  echo "OK"
else
  echo "FAIL"
  # show differences:
  diff -u --strip-trailing-cr "$1.ref" "$1.out"
fi
rm -f "runtest-$1.pd"
if [ $KEEP_OUTPUT -eq 0 ]; then
  rm -f "$1.out"
fi
exit $RESULT
