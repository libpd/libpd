#!/bin/bash

##-- hack CFLAGS
for arg in "$@"; do
 case "$arg" in
  CFLAGS=*)
    axsf_safe=
    for axsf_flag in `echo $arg | sed -e 's/^[^=]*=//'`
    do
      #echo "checking: $axsf_flag"
      axsf_flag_tmp=`echo $axsf_flag | sed -e 's/[ '\''\"\(\)]//g'`
      if test "${axsf_flag_tmp}" = "${axsf_flag}"; then
        axsf_safe="$axsf_safe $axsf_flag"
      else
        axsf_unsafe="$axsf_unsafe $axsf_flag"
      fi
    done
    #echo "new CFLAGS: $axsf_safe"
    args=("${args[@]}" "CFLAGS=$axsf_safe") ##-- only retain 'safe' flags for gfsm
    ;;
  *)
    #echo "default: $arg"
    args=("${args[@]}" "$arg")
    ;;
 esac
done

#echo "`dirname $0`"/configure "${args[@]}" FLEX=no BISON=no --disable-doc --disable-programs --disable-shared --prefix="$PWD/../../extended/build.moo/noinstall"
exec "`dirname $0`"/configure "${args[@]}" FLEX=no BISON=no --disable-doc --disable-programs --disable-shared --prefix="$PWD/../../extended/build.moo/noinstall"

