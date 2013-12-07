#!/bin/sh

GLEW_DIR=$1
OUTPUT_DIR=Base

usage () {
 echo "usage: $0 </path/to/glew/>"
 echo "get GLEW from http://sourceforge.net/projects/glew/files/glew"
 exit 255
}

test_glew_path () {
 if test "x" = "x${GLEW_DIR}"; then
  usage
 fi

 if test -d "${GLEW_DIR}" -a -f "${GLEW_DIR}/auto/src/glew.rc"; then :; else
  usage
 fi

# OUTPUT_DIR=$(echo ${OUTPUT_DIR} | sed -e 's/\///g')
 OUTPUT_DIR_ESC=$(echo ${OUTPUT_DIR%/} | sed -e 's/\//\\\//g')
 if test -d "${OUTPUT_DIR}"; then :; else
   echo "$0: no valid output-directory: ${OUTPUT_DIR}"
   exit 255
 fi
}

remake_glew () {
 # delete previously downloaded extensions
 make -C ${GLEW_DIR}/auto destroy
 # get extensions from http://oss.sgi.com/
 make -C ${GLEW_DIR}/auto
}

gemify_glew () {
 if test -f "$1"; then
   sed -e "s/<GL\/\(.*gl.*ew.*\)>/\"${OUTPUT_DIR_ESC}\/\1\"/" $1
 fi
}

gemify_glew_c () {
 if test -f "$1"; then
cat <<EOF
#ifdef HAVE_CONFIG_H
# include "Base/config.h"
#endif
#ifndef HAVE_LIBGLEW
EOF

 gemify_glew $1

echo "#endif /* HAVE_LIBGLEW */"

 fi
}

# test whether the user has provided enough information
test_glew_path

# rebuild glew
remake_glew

gemify_glew_c ${GLEW_DIR}/src/glew.c > ${OUTPUT_DIR}/glew.cpp

for f in glew.h glxew.h wglew.h
do
 gemify_glew ${GLEW_DIR}/include/GL/${f} > ${OUTPUT_DIR}/${f}
done




