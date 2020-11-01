# override via: make PYTHON=python3, etc
PYTHON = python

# detect platform
UNAME = $(shell uname)

ifeq ($(UNAME), Darwin)  # Mac
  # fix duplicate symbol error by allowing common blocks, silence some warnings
  export CFLAGS = $(CFLAGS) -fcommon \
                  -Wno-unused-variable -Wno-unused-function -Wno-unused-label
else
  ifeq ($(OS), Windows_NT) # Windows, use Mingw
    export CFLAGS = $(CFLAGS) -DWINVER=0x502 -DWIN32 -D_WIN32 -DPD_INTERNAL
    export LDFLAGS = $(LDFLAGS) -shared -Wl,--export-all-symbols -lws2_32 -lkernel32 -static-libgcc
  endif
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
