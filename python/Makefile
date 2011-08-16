
UNAME := $(shell uname -s)
ifeq ($(UNAME),Darwin)
  CFLAGS = -I../libpd_wrapper -I../pure-data/src -I/usr/include/python2.5 \
		-fPIC -arch x86_64 -arch i386 -O3
  LDFLAGS = -dynamiclib -lm -arch x86_64 -arch i386 -lpython2.5
  SOLIB_EXT = dylib
  LIBPD =  ../pure-data/src/*.o ../libpd_wrapper/*.o
else
  CFLAGS = -I../libpd_wrapper -I../pure-data/src -I/usr/include/python2.6 \
		-fPIC -O3
  LDFLAGS = -shared -lm -lpython2.6
  SOLIB_EXT = so
  LIBPD = ../libs/libpd.$(SOLIB_EXT)
endif

IFILE = pylibpd.i
CFILE = pylibpd_wrap.c
PYFILE = pylibpd.py
PYLIBPD = _pylibpd.so

.PHONY: all clean clobber

all: $(LIBPD) ${CFILE:.c=.o}
	gcc $(LDFLAGS) -o $(PYLIBPD) $^

$(CFILE): $(IFILE)
	swig -python $^

$(LIBPD):
	make -C ..

clean:
	rm -f $(CFILE) $(PYFILE) ${CFILE:.c=.o} *.pyc

clobber: clean
	rm -f $(PYLIBPD)

