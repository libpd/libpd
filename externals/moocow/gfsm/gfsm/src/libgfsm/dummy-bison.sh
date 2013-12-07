#!/bin/sh

base=`basename $1 .y`
cp -v -f "$base.c" y.tab.c
cp -v -f "$base.h" y.tab.h
if test -f "$base.output"; then cp -v -f "$base.output" y.output ; fi
