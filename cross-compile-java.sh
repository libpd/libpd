#!/bin/bash

make CC='gcc -m32' PDNATIVE_ARCH=x86    clean javalib
make CC='gcc -m64' PDNATIVE_ARCH=x86_64 clean javalib

