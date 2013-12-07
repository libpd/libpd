#!/bin/sh

USAGE="$0 {copy|link} [COMMON_SRC=../common] [COMMON_DST=./common]"

if test -n "$1"; then
  case "$1" in
    copy)
      CP="cp -a";
      ;;
    link)
      CP="ln -s";
      ;;
    *)
      echo "Usage: $USAGE"
      exit 1;
      ;;
  esac
else
  CP="ln -s"
fi

if test -n "$2"; then
  COMMON_SRC="$2";
else
  COMMON_SRC="../common";
fi

if test -n "$3"; then
  COMMON_DST="$3";
else
  COMMON_DST="./common";
fi

##-- copy or link in common dir
runcmd() {
  echo "$0[`basename \`pwd\``]:" "$@"
  $@
}

runcmd rm -rf "$COMMON_DST";
runcmd $CP "$COMMON_SRC" "$COMMON_DST";
