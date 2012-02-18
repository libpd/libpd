# Note:  If you are using a Mac with Xcode 4 or later, you will need to
#        set processor architecture flags before running make:
#           export ARCHFLAGS="-arch i386 -arch x86_64"
#           make
#        Background: Xcode 4 removed support for PPC, but the Python
#        distutils still think they're supposed to build for PPC,
#        causing the build to fail with a most unhelpful error message.

.PHONY: build install clean

build:
	python setup.py build

install:
	python setup.py install

clean:
	python setup.py clean

