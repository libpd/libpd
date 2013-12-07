#!/bin/sh

autoreconf -fiv || exit -1

echo "if everything above succeeded, you can now run './configure'"
echo "for options see './configure --help=recursive'"
