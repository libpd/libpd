IFILE = pylibpd.i
CFILE = pylibpd_wrap.c
PYFILE = pylibpd.py
LIBPD = ../libs/libpd.so
PYLIBPD = _pylibpd.so

.PHONY: all clean clobber

CFLAGS = -I../libpd_wrapper -I../pure-data/src -I/usr/include/python2.6

all: $(LIBPD) ${CFILE:.c=.o}
	gcc -shared -lm -o $(PYLIBPD) $^

$(CFILE): $(IFILE)
	swig -python $^

$(LIBPD):
	cd .. && make

clean:
	rm -f $(CFILE) $(PYFILE) *.o *.pyc

clobber: clean
	rm -f $(PYLIBPD)

