# File extension for shared libraries.
SOLIB_EXT = so

# Extra gcc flags.
PLATFORM_CFLAGS = -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast -fPIC \
	 -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/linux

# Extra flags for linking shared libraries.
LDFLAGS = -shared

JAVA_LDFLAGS =

include common.mk
