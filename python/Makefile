# override via: make PYTHON=python3.9, etc
PYTHON = python3

# detect platform
UNAME = $(shell uname)

ifeq ($(UNAME), Darwin)  # Mac
  # fix duplicate symbol error by allowing common blocks, silence some warnings
  export CFLAGS := $(CFLAGS) -fcommon \
                  -Wno-unused-variable -Wno-unused-function -Wno-unused-label
else
  ifeq ($(OS), Windows_NT) # Windows, use Mingw
    export CFLAGS := $(CFLAGS) -DWINVER=0x502 -DWIN32 -D_WIN32 -DPD_INTERNAL
    export LDFLAGS := $(LDFLAGS) -shared -Wl,--export-all-symbols -lws2_32 -lkernel32 -static-libgcc
  endif
endif

# keep track of installed files in order to remove them later
INSTALLED = installed.txt

.PHONY: build install clean clobber

all: build

build:
	$(PYTHON) setup.py build

install:
	$(PYTHON) setup.py install --record $(INSTALLED)

# use tr to handle spaces in file names
uninstall:
	tr '\n' '\0' < $(INSTALLED) | xargs -0 rm -vf --
	rm -f $(INSTALLED)

clean:
	$(PYTHON) setup.py clean

clobber:
	rm -rfv build
