#!/bin/sh
cp -v -f "`basename $1 .l`.c" lex.yy.c
touch --no-create "`basename $1 .l`.h"
