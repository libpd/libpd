#!/bin/sh

EXTENSIONS=".pd -help.pd .pd_linux .pd_darwin .pd_freebsd .dll .b_i386 .l_ia64 .l_i386 .d_fat .d_i386 .d_ppc .m_i386" 

ALIASFILE=$1
shift

if [ "x$ALIASFILE" = "x-clean" ]
then
  CLEANMODE=yes
  ALIASFILE=$1
  shift
else
  CLEANMODE=
fi


if [ -e "${ALIASFILE}" ]; then :; else
 echo cannot find alias-file ${ALIASFILE}
 exit 1
fi


debug() {
 :
# echo $@
}

do_makealias() {
 local source
 local dest

 source=$1
 dest=$2

 if [ "x${CLEANMODE}" = "xyes" ]
 then
   if [ -e "${dest}" ]; then
    debug "removing alias ${dest}"
    rm ${dest}
   else
     debug "alias ${dest} does not exist"
   fi
 else
   debug "aliasing ${source} to ${dest}"
   if [ "x${COPYMODE}" = "xyes" ]
   then
    cp ${source} ${dest}
   else
    source=${source##*/}
    ln -s ${source} ${dest}
   fi
 fi
}

do_makealiases() {
  local dir
  local master
  local slave
  local extension

  dir=$1
  master=$2
  shift; shift

  if [ "x${master}" = "x" ]; then
  # no realname provided
    return
  fi

  if [ "x$@" = "x" ]; then
  # no aliases provided...
    return
  fi

  for extension in ${EXTENSIONS}
  do
#   echo "checking aliases for ${dir}/${master}${extension}"
   if [ -f "${dir}/${master}${extension}" ]
   then
     for slave in $@
     do
      do_makealias ${dir}/${master}${extension} ${dir}/${slave}${extension}
     done
   fi
  done
}

for d in $@
do
  if [ -d "$d" ]
  then
    debug "scanning directory $d for aliases"
    cat ${ALIASFILE} | while read line
    do
     do_makealiases $d $line
    done
  else
    echo "skipping non-directory $d"
  fi
done

