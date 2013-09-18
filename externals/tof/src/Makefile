# 
# SETUP
#

# Points to the folder that contains pd's /src dir
PD_SRC = ../../../pd

# Points to the folder that contains pd's compiled /bin dir (for OS X)
PD_BIN = /Applications/Pd-extended.app/Contents/Resources

# Points to the folder to which you want to move the built file
OUTPUT = ~/Make/pd/tof


#
# FIND OS
#

UNAME := $(shell uname -s)
ifeq ($(UNAME),Linux)
  OS_NAME = linux
  EXTENSION = pd_linux
  DYLIB_EXTENSION = so
endif
ifeq ($(UNAME),Darwin)
  OS_NAME = darwin
  EXTENSION = pd_darwin
  DYLIB_EXTENSION = dylib
endif
ifeq (MINGW,$(findstring MINGW,$(UNAME)))
  OS_NAME = windows
  EXTENSION = dll
  DYLIB_EXTENSION = dll
endif
ifeq (CYGWIN,$(findstring CYGWIN,$(UNAME)))
  OS_NAME = windows
  EXTENSION = dll
  DYLIB_EXTENSION = dll
endif
# which CPU to compile for
UNAME_MACHINE := $(shell uname -m)
ifeq ($(UNAME_MACHINE),i386)
  ARCH = i386
endif
ifeq ($(UNAME_MACHINE),i686)
  ARCH = i386
endif
ifeq ($(UNAME_MACHINE),ppc)
  ARCH = powerpc
endif



# 
# COMPILE OPTIONS
#

CWD := $(shell pwd)


# turn on weak linking and dlopen support
export MACOSX_DEPLOYMENT_TARGET = 10.3

# DEFAULT TARGET # Find a way to list all c files target
ALL = $(patsubst %.c,%.$(EXTENSION),$(wildcard *.c))
default: $(ALL)

clean:
	rm *.$(EXTENSION)

.SUFFIXES: .$(EXTENSION) .$(SHARED_LIB)

CCWARNINGS = -Waddress -Wchar-subscripts -Wimplicit-int -Wimplicit-function-declaration -Wcomment -Wformat -Wmain -Wmissing-braces -Wnonnull -Wparentheses -Wpointer-sign -Wreturn-type -Wsequence-point -Wsign-compare -Wstrict-aliasing -Wstrict-overflow=1 -Wswitch -Wtrigraphs -Wunknown-pragmas -Wunused-label -Wunused-value -Wvolatile-register-var 

# this variable is to support old "win" directories, rather than "windows"
BUILDSRC_OS_NAME = $(OS_NAME)

#-Wall -W $(DEBUG_CFLAGS)

CFLAGS = -DPD -I$(PD_SRC)/src $(CCWARNINGS)
LDFLAGS =  
LIBS = -lm

ifeq ($(OS_NAME),darwin)
# 10.4 Tiger
  FAT_FLAGS = -arch ppc -arch ppc64 -arch i386
# 10.5 Leopard
#  FAT_FLAGS = -arch ppc -arch ppc7400 -arch ppc64 -arch i386 -arch x86_64
  CFLAGS += -I/sw/include -DMACOSX -DUNIX -Dunix -DDL_OPEN -arch $(ARCH)
  LDFLAGS += -bundle -bundle_loader $(PD_BIN)/bin/pd -undefined dynamic_lookup \
		-weak_framework Carbon -arch $(ARCH)
  LIBS += -lc 
  DYLIB_LDFLAGS = -dynamiclib -undefined dynamic_lookup -read_only_relocs warning -L/sw/lib
  STRIP = strip -x
endif
ifeq ($(OS_NAME),linux)
  CFLAGS +=  -DUNIX -Dunix -DDL_OPEN -fPIC
  LDFLAGS += -Wl,--export-dynamic  -shared -fPIC
  LIBS += -lc
  DYLIB_LDFLAGS = $(LDFLAGS)
  STRIP = strip --strip-unneeded -R .note -R .comment
endif
ifeq ($(OS_NAME),windows)
  BUILDSRC_OS_NAME = win
  WINDOWS_HACKS = -D'O_NONBLOCK=1' -D'srand48(n)=srand((n))' \
    -D'drand48()=((double)rand()/RAND_MAX)' -D'bzero(p,n)=memset(p,0,n)'
# These don't seem to be needed:
#	-D'PROT_READ=1' \
#	-D'MAP_PRIVATE=2' \
#	-D'O_NDELAY=O_NONBLOCK'
  CFLAGS += -mms-bitfields -DMSW -DNT $(WINDOWS_HACKS)
  LDFLAGS += -s -shared
# all of these included libs are part of libc in UNIX platforms.  All except
# libregex are in DLLs, so they get stripped from the external's .dll binary
  LIBS += -L$(PD_SRC)/src -L$(PD_SRC)/bin -L$(PD_SRC)/obj -lpd \
    -lwsock32 -liphlpapi -lpthreadGC2 -lkernel32 -luser32 -lgdi32 -lregex
  DYLIB_LDFLAGS = -shared
  STRIP = strip --strip-unneeded -R .note -R .comment
endif

CXXFLAGS = $(CFLAGS)

# Special rules
param.$(EXTENSION): param.h paramDump.h paramGui.h paramRoute.h paramCustom.h paramFile.h paramPath.h


### C++ files
%.$(EXTENSION): %.cpp
%.$(EXTENSION): %.cc
	$(CXX) $(OPT_CFLAGS) $(CXXFLAGS) -o "$*.o" -c "$<"
	$(CXX) $(LDFLAGS) -o "$*.$(EXTENSION)" "$*.o" $(LIBS)
	chmod a-x "$*.$(EXTENSION)"
#	$(STRIP) $*.$(EXTENSION)
#	rm -f -- $*.o
	cp $*.$(EXTENSION) $(OUTPUT)

%.o: %.cpp
%.o: %.cc
	$(CXX) $(OPT_CFLAGS) $(CXXFLAGS) -o "$*.o" -c "$<"


### C files
%.o: %.c
	@echo "- Building $*.o"
	@$(CC) $(OPT_CFLAGS) $(CFLAGS) -o "$*.o" -c "$*.c"



%.$(EXTENSION): %.o
	@echo "- Linking $*.$(EXTENSION)"
	@$(CC) $(LDFLAGS) -o "$*.$(EXTENSION)" "$*.o"  $(LIBS) \
		`test -f $*.libs && cat $*.libs`	\
		`my_dylib=$(patsubst $(externals_src)/%,%,$(@D)); test -f $(@D)/lib$${my_dylib}.$(DYLIB_EXTENSION) && echo -L$(@D) -l$$my_dylib` \
		`my_obj=$(patsubst $(externals_src)/%,%,$(@D)); test -f $(@D)/shared/$${my_obj}.o && echo $(@D)/shared/$${my_obj}.o` \
		`test -f $(dir $*)../$(BUILDSRC_OS_NAME)/$(notdir $*).libs && \
			cat $(dir $*)../$(BUILDSRC_OS_NAME)/$(notdir $*).libs`
	@chmod a-x "$*.$(EXTENSION)"
	@$(STRIP) $*.$(EXTENSION)
	@rm -f -- $*.o
	@cp $*.$(EXTENSION) $(OUTPUT)






