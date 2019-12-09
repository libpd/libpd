# Note:  If you are using a Mac with Xcode 4 or later and you run into problems
#        related to building for PPC, you will need to set processor
#        architecture flags before running make:
#           export ARCHFLAGS="-arch i386 -arch x86_64"
#           make
#        Background: Xcode 4 removed support for PPC, but the Python
#        distutils still think they're supposed to build for PPC,
#        causing the build to fail with a most unhelpful error message.

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
	python setup.py build

install:
	python setup.py install

clean:
	python setup.py clean

clobber:
	rm -rfv build
