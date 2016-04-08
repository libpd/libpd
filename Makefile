UNAME = $(shell uname)
SOLIB_PREFIX = lib

ifeq ($(UNAME), Darwin)  # Mac
  SOLIB_EXT = dylib
  PDNATIVE_SOLIB_EXT = jnilib
  PDNATIVE_PLATFORM = mac
  PDNATIVE_ARCH = 
  PLATFORM_CFLAGS = -DHAVE_LIBDL -DHAVE_ALLOCA_H -arch x86_64 -arch i386 -g \
    -I/System/Library/Frameworks/JavaVM.framework/Headers
  LDFLAGS = -arch x86_64 -arch i386 -dynamiclib -ldl
  CSHARP_LDFLAGS = $(LDFLAGS)
  CPP_FLAGS = -stdlib=libc++
  CPP_LDFLAGS = $(LDFLAGS) -stdlib=libc++
  JAVA_LDFLAGS = -framework JavaVM $(LDFLAGS)
else
  ifeq ($(OS), Windows_NT)  # Windows, use Mingw
    CC = gcc
    SOLIB_EXT = dll
    SOLIB_PREFIX = 
    PDNATIVE_PLATFORM = windows
    PDNATIVE_ARCH = $(shell $(CC) -dumpmachine | sed -e 's,-.*,,' -e 's,i[3456]86,x86,' -e 's,amd64,x86_64,')
    PLATFORM_CFLAGS = -DWINVER=0x502 -DWIN32 -D_WIN32 -DPD_INTERNAL \
      -I"$(JAVA_HOME)/include" -I"$(JAVA_HOME)/include/win32"
    MINGW_LDFLAGS = -shared -lws2_32 -lkernel32
    LDFLAGS = $(MINGW_LDFLAGS) -Wl,--output-def=libs/libpd.def \
      -Wl,--out-implib=libs/libpd.lib
    CSHARP_LDFLAGS = $(MINGW_LDFLAGS) -Wl,--output-def=libs/libpdcsharp.def \
      -Wl,--out-implib=libs/libpdcsharp.lib
    CPP_LDFLAGS = $(LDFLAGS)
    JAVA_LDFLAGS = $(MINGW_LDFLAGS) -Wl,--kill-at
  else  # Assume Linux
    SOLIB_EXT = so
    PDNATIVE_PLATFORM = linux
    PDNATIVE_ARCH = $(shell $(CC) -dumpmachine | sed -e 's,-.*,,' -e 's,i[3456]86,x86,' -e 's,amd64,x86_64,')
    JAVA_HOME ?= /usr/lib/jvm/default-java
    PLATFORM_CFLAGS = -DHAVE_LIBDL -Wno-int-to-pointer-cast \
      -Wno-pointer-to-int-cast -fPIC -I"$(JAVA_HOME)/include" \
      -I"$(JAVA_HOME)/include/linux"
    LDFLAGS = -shared -ldl -Wl,-Bsymbolic
    CSHARP_LDFLAGS = $(LDFLAGS)
    CPP_LDFLAGS = $(LDFLAGS)
    JAVA_LDFLAGS = $(LDFLAGS)
  endif
endif

PDNATIVE_SOLIB_EXT ?= $(SOLIB_EXT)

PD_FILES = \
	pure-data/src/d_arithmetic.c pure-data/src/d_array.c pure-data/src/d_ctl.c \
	pure-data/src/d_dac.c pure-data/src/d_delay.c pure-data/src/d_fft.c \
	pure-data/src/d_fft_fftsg.c \
	pure-data/src/d_filter.c pure-data/src/d_global.c pure-data/src/d_math.c \
	pure-data/src/d_misc.c pure-data/src/d_osc.c pure-data/src/d_resample.c \
	pure-data/src/d_soundfile.c pure-data/src/d_ugen.c \
	pure-data/src/g_all_guis.c pure-data/src/g_array.c pure-data/src/g_bang.c \
	pure-data/src/g_canvas.c pure-data/src/g_clone.c pure-data/src/g_editor.c \
	pure-data/src/g_graph.c pure-data/src/g_guiconnect.c pure-data/src/g_hdial.c \
	pure-data/src/g_hslider.c pure-data/src/g_io.c pure-data/src/g_mycanvas.c \
	pure-data/src/g_numbox.c pure-data/src/g_readwrite.c \
	pure-data/src/g_rtext.c pure-data/src/g_scalar.c pure-data/src/g_template.c \
	pure-data/src/g_text.c pure-data/src/g_toggle.c pure-data/src/g_traversal.c \
	pure-data/src/g_vdial.c pure-data/src/g_vslider.c pure-data/src/g_vumeter.c \
	pure-data/src/m_atom.c pure-data/src/m_binbuf.c pure-data/src/m_class.c \
	pure-data/src/m_conf.c pure-data/src/m_glob.c pure-data/src/m_memory.c \
	pure-data/src/m_obj.c pure-data/src/m_pd.c pure-data/src/m_sched.c \
	pure-data/src/s_audio.c pure-data/src/s_audio_dummy.c \
	pure-data/src/s_file.c pure-data/src/s_inter.c \
	pure-data/src/s_loader.c pure-data/src/s_main.c pure-data/src/s_path.c \
	pure-data/src/s_print.c pure-data/src/s_utf8.c pure-data/src/x_acoustics.c \
	pure-data/src/x_arithmetic.c pure-data/src/x_array.c pure-data/src/x_connective.c \
	pure-data/src/x_gui.c pure-data/src/x_interface.c pure-data/src/x_list.c \
	pure-data/src/x_midi.c pure-data/src/x_misc.c pure-data/src/x_net.c \
	pure-data/src/x_scalar.c pure-data/src/x_text.c pure-data/src/x_time.c \
	pure-data/src/x_vexp.c pure-data/src/x_vexp_if.c pure-data/src/x_vexp_fun.c \
	libpd_wrapper/s_libpdmidi.c libpd_wrapper/x_libpdreceive.c \
	libpd_wrapper/z_hooks.c libpd_wrapper/z_libpd.c

PD_EXTRA_FILES = \
	pure-data/extra/bob~/bob~.c pure-data/extra/bonk~/bonk~.c \
	pure-data/extra/choice/choice.c \
	pure-data/extra/fiddle~/fiddle~.c pure-data/extra/loop~/loop~.c \
	pure-data/extra/lrshift~/lrshift~.c pure-data/extra/pique/pique.c \
	pure-data/extra/sigmund~/sigmund~.c pure-data/extra/stdout/stdout.c

LIBPD_UTILS = \
	libpd_wrapper/util/z_print_util.c \
	libpd_wrapper/util/z_queued.c \
	libpd_wrapper/util/ringbuffer.c

CPP_FILES = \
	cpp/PdBase.cpp \
	cpp/PdTypes.cpp

PDJAVA_JAR_CLASSES = \
	java/org/puredata/core/PdBase.java \
	java/org/puredata/core/NativeLoader.java \
	java/org/puredata/core/PdListener.java \
	java/org/puredata/core/PdMidiListener.java \
	java/org/puredata/core/PdMidiReceiver.java \
	java/org/puredata/core/PdReceiver.java \
	java/org/puredata/core/utils/IoUtils.java \
	java/org/puredata/core/utils/PdDispatcher.java

# conditional libpd_wrapper/util compilation
ifeq ($(UTIL), true)
	UTIL_FILES = $(LIBPD_UTILS)
endif

# conditional pure-data/extra externals compilation
ifeq ($(EXTRA), true)
	EXTRA_FILES = $(PD_EXTRA_FILES)
	EXTRA_CFLAGS = -DLIBPD_EXTRA
endif

# conditional optimizations or debug settings
OPT_CFLAGS = -O3
ifeq ($(DEBUG), true)
	OPT_CFLAGS = -Wall
endif

# object files which are somehow generated but not from sources listed above,
# there is probably a better fix but this works for now
PD_EXTRA_OBJS = \
	pure-data/src/d_fft_fft_fftsg.o pure-data/src/d_fft_fftw.o \
	pure-data/src/d_fft_fftsg_h.o pure-data/src/x_qlist.o

# default install location
prefix=/usr/local

JNI_FILE = libpd_wrapper/util/ringbuffer.c libpd_wrapper/util/z_queued.c jni/z_jni_plain.c
JNIH_FILE = jni/z_jni.h
JAVA_BASE = java/org/puredata/core/PdBase.java
LIBPD = libs/libpd.$(SOLIB_EXT)
PDCSHARP = libs/libpdcsharp.$(SOLIB_EXT)
PDCPP = libs/libpdcpp.$(SOLIB_EXT)

PDJAVA_BUILD = java-build
PDJAVA_DIR = $(PDJAVA_BUILD)/org/puredata/core/natives/$(PDNATIVE_PLATFORM)/$(PDNATIVE_ARCH)/
PDJAVA_NATIVE = $(PDJAVA_DIR)/$(SOLIB_PREFIX)pdnative.$(PDNATIVE_SOLIB_EXT)
PDJAVA_JAR = libs/libpd.jar

CFLAGS = -DPD -DHAVE_UNISTD_H -DUSEAPI_DUMMY -I./pure-data/src -I./libpd_wrapper \
         -I./libpd_wrapper/util $(EXTRA_CFLAGS) $(PLATFORM_CFLAGS) $(OPT_CFLAGS)

CXXFLAGS = $(CFLAGS) $(CPP_FLAGS)

.PHONY: libpd csharplib cpplib javalib install uninstall clean clobber

libpd: $(LIBPD)

$(LIBPD): ${PD_FILES:.c=.o} ${UTIL_FILES:.c=.o} ${EXTRA_FILES:.c=.o}
	$(CC) -o $(LIBPD) $^ $(LDFLAGS) -lm -lpthread 

javalib: $(JNIH_FILE) $(PDJAVA_JAR)

$(JNIH_FILE): $(JAVA_BASE)
	javac -classpath java $^
	javah -o $@ -classpath java org.puredata.core.PdBase

$(PDJAVA_NATIVE): ${PD_FILES:.c=.o} ${LIBPD_UTILS:.c=.o} ${EXTRA_FILES:.c=.o} ${JNI_FILE:.c=.o}
	mkdir -p $(PDJAVA_DIR)
	$(CC) -o $(PDJAVA_NATIVE) $^ -lm -lpthread $(JAVA_LDFLAGS) 
	cp $(PDJAVA_NATIVE) libs/

$(PDJAVA_JAR): $(PDJAVA_NATIVE) $(PDJAVA_JAR_CLASSES)
	javac -d $(PDJAVA_BUILD) $(PDJAVA_JAR_CLASSES)
	jar -cvf $(PDJAVA_JAR) -C $(PDJAVA_BUILD) org/puredata/

csharplib: $(PDCSHARP)

$(PDCSHARP): ${PD_FILES:.c=.o} ${EXTRA_FILES:.c=.o}
	$(CC) -o $(PDCSHARP) $^ $(CSHARP_LDFLAGS) -lm -lpthread

cpplib: $(PDCPP)

# build with LIBPD_UTILS since cpp wrapper uses the ringbuffer
$(PDCPP): ${PD_FILES:.c=.o} ${LIBPD_UTILS:.c=.o} ${EXTRA_FILES:.c=.o} ${CPP_FILES:.cpp=.o} 
	g++ -o $(PDCPP) $^ $(CPP_LDFLAGS) -lm -lpthread

clean:
	rm -f ${PD_FILES:.c=.o} ${PD_EXTRA_OBJS} ${CPP_FILES:.cpp=.o} ${JNI_FILE:.c=.o}
	rm -f ${PD_UTIL_FILES:.c=.o} ${PD_EXTRA_FILES:.c=.o}

clobber: clean
	rm -f $(LIBPD) $(PDCSHARP) $(PDCPP) $(PDJAVA_NATIVE) $(PDJAVA_JAR)
	rm -f libs/`basename $(PDJAVA_NATIVE)`
	rm -rf $(PDJAVA_BUILD)

install:
	mkdir -p $(prefix)/include/libpd
	install libpd_wrapper/z_libpd.h $(prefix)/include/libpd
	install pure-data/src/m_pd.h $(prefix)/include/libpd
	mkdir -p $(prefix)/include/libpd/util
	install libpd_wrapper/util/z_print_util.h $(prefix)/include/libpd/util
	install libpd_wrapper/util/z_queued.h $(prefix)/include/libpd/util
	install $(LIBPD) $(prefix)/lib
	if [ -e $(PDCPP) ]; then \
		install cpp/*.hpp $(prefix)/include/libpd; \
		install $(PDCPP) $(prefix)/lib; \
	fi

uninstall:
	rm -rf $(prefix)/include/libpd
	rm -f $(prefix)/lib/`basename $(LIBPD)`
	rm -f $(prefix)/lib/`basename $(PDCPP)`
