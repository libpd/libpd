# File extension for shared libraries.
SOLIB_EXT = so

JAVA_HOME ?= /usr/lib/jvm/default-java  # Guess at the default path to the JDK.

# Extra gcc flags.
PLATFORM_CFLAGS = -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast -fPIC \
	 -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/linux

# Extra flags for linking shared libraries.
LDFLAGS = -shared

JAVA_LDFLAGS =

include common.mk
