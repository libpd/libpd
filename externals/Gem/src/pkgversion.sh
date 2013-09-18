#!/bin/sh

VERSIONFILE=${0%/*}/pkgversion.txt
if test -e "${VERSIONFILE}"; then
 . ${VERSIONFILE}
fi

if test "x${PKGVERSION_MAJOR}" = "x"; then
 PKGVERSION_MAJOR="0"
fi
if test "x${PKGVERSION_MINOR}" = "x"; then
 PKGVERSION_MINOR="0"
fi

pkgversion_cvs () {
  if [ -d CVS ]; then
    PKGVERSION_BUGFIX="CVS"
    PKGVERSION_CODENAME=
  else
    return 1
  fi
}

pkgversion_svn () {
  if [ -d .svn ]; then
    PKGVERSION_BUGFIX="SVN"
    if which svnversion 2>&1 > /dev/null; then
      PKGVERSION_CODENAME="rev$(svnversion .)"
    else
      PKGVERSION_CODENAME="unknown"
    fi
  else
    return 1
  fi
}

pkgversion_git () {
  local version
  if version=$(git describe --always --abbrev=0 2>/dev/null); then
    PKGVERSION_BUGFIX="git"
    PKGVERSION_CODENAME="${version}"
  else
     return 1
  fi
}


fullversion() {
                if test "x${PKGVERSION_BUGFIX}" = "x"; then
		  echo "${PKGVERSION_MAJOR}.${PKGVERSION_MINOR} ${PKGVERSION_CODENAME}"
		else
		  echo "${PKGVERSION_MAJOR}.${PKGVERSION_MINOR}.${PKGVERSION_BUGFIX} ${PKGVERSION_CODENAME}"
		fi
}

substitute_file() {
 local INFILE
 INFILE=$1

 if [ -e "${INFILE}" ]; then
   sed  -e "s|@PKGVERSION_MAJOR@|${PKGVERSION_MAJOR}|g" \
	-e "s|@PKGVERSION_MINOR@|${PKGVERSION_MINOR}|g" \
	-e "s|@PKGVERSION_BUGFIX@|${PKGVERSION_BUGFIX}|g" \
	-e "s|@PKGVERSION_CODENAME@|${PKGVERSION_CODENAME}|g" \
	${INFILE}
 else
  echo "${INFILE} not found!" 1>&2
 fi
}

if test "x${PKGVERSION_BUGFIX}" = "x"; then
## try to autodetect the current version
  pkgversion_git || pkgversion_svn || pkgversion_cvs
fi

if test "x$1" = "x"; then
  fullversion
else
 case $1 in
	--major)
		echo ${PKGVERSION_MAJOR}
	;;
	--minor)
		echo ${PKGVERSION_MINOR}
	;;
	--bugfix)
		echo ${PKGVERSION_BUGFIX}
	;;
	--codename)
		echo ${PKGVERSION_CODENAME}
	;;
	*)
		substitute_file $1	
	;;
 esac
fi
