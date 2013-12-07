#!/bin/sh

aclocal && \
autoconf && \
echo "now run './configure'
for help on args run './configure --help'"
