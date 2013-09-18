#!/bin/sh

if [ "$(uname -s)" = "Darwin" ]; then
  echo "Forcing the use of built-in autotools"
  echo /usr/bin/autoreconf --install --force --verbose
  /usr/bin/autoreconf --install --force --verbose
else
  echo autoreconf --install --force --verbose
  autoreconf --install --force --verbose
fi
