# override via: make PYTHON=python3, etc
PYTHON = python

# detect platform
UNAME = $(shell uname)

ifeq ($(UNAME), Darwin)  # Mac
  # fix duplicate symbol error by allowing common blocks, silence some warnings
  export CFLAGS = -fcommon \
                  -Wno-unused-variable -Wno-unused-function -Wno-unused-label
endif

.PHONY: build install clean clobber

build:
	$(PYTHON) setup.py build

install:
	$(PYTHON) setup.py install

clean:
	$(PYTHON) setup.py clean

clobber:
	rm -rfv build
