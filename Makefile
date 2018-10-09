UNAME = $(shell uname)
SOLIB_PREFIX = lib
LIBPD_IMPLIB =
LIBPD_DEF =

ifeq ($(UNAME), Darwin)  # Mac
  SOLIB_EXT = dylib
  PDNATIVE_SOLIB_EXT = jnilib
  PDNATIVE_PLATFORM = mac
  PDNATIVE_ARCH =
  PLATFORM_CFLAGS = -DHAVE_LIBDL -arch x86_64 -arch i386 \
    -I/System/Library/Frameworks/JavaVM.framework/Headers
  LDFLAGS = -arch x86_64 -arch i386 -dynamiclib -ldl -Wl,-no_compact_unwind
  CSHARP_LDFLAGS = $(LDFLAGS)
  JAVA_LDFLAGS = -framework JavaVM $(LDFLAGS)
else
  ifeq ($(OS), Windows_NT)  # Windows, use Mingw
    CC ?= gcc
    SOLIB_EXT = dll
    SOLIB_PREFIX =
    LIBPD_IMPLIB = libs/libpd.lib
    LIBPD_DEF = libs/libpd.def
    PDNATIVE_PLATFORM = windows
    PDNATIVE_ARCH = $(shell $(CC) -dumpmachine | sed -e 's,-.*,,' -e 's,i[3456]86,x86,' -e 's,amd64,x86_64,')
    PLATFORM_CFLAGS = -DWINVER=0x502 -DWIN32 -D_WIN32 -DPD_INTERNAL \
      -I"$(JAVA_HOME)/include" -I"$(JAVA_HOME)/include/win32"
    MINGW_LDFLAGS = -shared -Wl,--export-all-symbols -lws2_32 -lkernel32 -static-libgcc
    LDFLAGS = $(MINGW_LDFLAGS) -Wl,--output-def=$(LIBPD_DEF) \
      -Wl,--out-implib=$(LIBPD_IMPLIB)
    CSHARP_LDFLAGS = $(MINGW_LDFLAGS) -Wl,--output-def=libs/libpdcsharp.def \
      -Wl,--out-implib=libs/libpdcsharp.lib
    JAVA_LDFLAGS = $(MINGW_LDFLAGS) -Wl,--kill-at
  else  # Linux or *BSD
    SOLIB_EXT = so
    PDNATIVE_ARCH = $(shell $(CC) -dumpmachine | sed -e 's,-.*,,' -e 's,i[3456]86,x86,' -e 's,amd64,x86_64,')
    PLATFORM_CFLAGS = -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast -fPIC
    LDFLAGS = -shared -Wl,-Bsymbolic
    ifeq ($(UNAME), Linux)
      PDNATIVE_PLATFORM = linux
      JAVA_HOME ?= /usr/lib/jvm/default-java
      PLATFORM_CFLAGS += -I"$(JAVA_HOME)/include/linux" -DHAVE_LIBDL
      LDFLAGS += -ldl
    else ifeq ($(UNAME), FreeBSD)
      PDNATIVE_PLATFORM = FreeBSD
      JAVA_HOME ?= /usr/local/openjdk8
      PLATFORM_CFLAGS += -I"$(JAVA_HOME)/include/"
    endif
    CSHARP_LDFLAGS = $(LDFLAGS)
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
    pure-data/src/g_editor_extras.c \
    pure-data/src/g_graph.c pure-data/src/g_guiconnect.c pure-data/src/g_hdial.c \
    pure-data/src/g_hslider.c pure-data/src/g_io.c pure-data/src/g_mycanvas.c \
    pure-data/src/g_numbox.c pure-data/src/g_readwrite.c \
    pure-data/src/g_rtext.c pure-data/src/g_scalar.c pure-data/src/g_template.c \
    pure-data/src/g_text.c pure-data/src/g_toggle.c pure-data/src/g_traversal.c \
    pure-data/src/g_undo.c \
    pure-data/src/g_vdial.c pure-data/src/g_vslider.c pure-data/src/g_vumeter.c \
    pure-data/src/m_atom.c pure-data/src/m_binbuf.c pure-data/src/m_class.c \
    pure-data/src/m_conf.c pure-data/src/m_glob.c pure-data/src/m_memory.c \
    pure-data/src/m_obj.c pure-data/src/m_pd.c pure-data/src/m_sched.c \
    pure-data/src/s_audio.c pure-data/src/s_audio_dummy.c pure-data/src/s_inter.c \
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

PDJAVA_JAR_CLASSES = \
    java/org/puredata/core/PdBase.java \
    java/org/puredata/core/PdBaseloader.java \
    java/org/puredata/core/NativeLoader.java \
    java/org/puredata/core/PdListener.java \
    java/org/puredata/core/PdMidiListener.java \
    java/org/puredata/core/PdMidiReceiver.java \
    java/org/puredata/core/PdReceiver.java \
    java/org/puredata/core/utils/IoUtils.java \
    java/org/puredata/core/utils/PdDispatcher.java

# additional Java source jar files
PDJAVA_SRC_FILES = \
	.classpath \
	.project

JNI_SOUND = jni/z_jni_plain.c

# conditional libpd_wrapper/util compilation
UTIL_FILES = $(LIBPD_UTILS)
ifeq ($(UTIL), false)
    UTIL_FILES =
endif

# conditional pure-data/extra externals compilation
EXTRA_FILES = $(PD_EXTRA_FILES)
EXTRA_CFLAGS = -DLIBPD_EXTRA
ifeq ($(EXTRA), false)
    EXTRA_FILES =
    EXTRA_CFLAGS =
endif

# conditional multi-instance support
MULTI_CFLAGS =
ifeq ($(MULTI), true)
    MULTI_CFLAGS = -DPDINSTANCE -DPDTHREADS
endif

# conditional optimizations or debug settings
OPT_CFLAGS = -ffast-math -funroll-loops -fomit-frame-pointer -O3
ifeq ($(DEBUG), true)
    OPT_CFLAGS = -g -O0
endif

# conditional to set numeric locale to default "C"
LOCALE_CFLAGS =
ifeq ($(SETLOCALE), false)
    LOCALE_CFLAGS = -DLIBPD_NO_NUMERIC
endif

# portaudio backend?
ifeq ($(PORTAUDIO), true)
    JNI_SOUND = jni/z_jni_pa.c
    JAVA_LDFLAGS := $(JAVA_LDFLAGS) -lportaudio
    ifeq ($(UNAME), Darwin)  # Mac
        JAVA_LDFLAGS := $(JAVA_LDFLAGS) \
                        -framework CoreAudio -framework AudioToolbox \
                        -framework AudioUnit -framework CoreServices
    endif
endif

# object files which are somehow generated but not from sources listed above,
# there is probably a better fix but this works for now
PD_EXTRA_OBJS = \
    pure-data/src/d_fft_fft_fftsg.o pure-data/src/d_fft_fftw.o \
    pure-data/src/d_fft_fftsg_h.o pure-data/src/x_qlist.o

# default install location
prefix ?= /usr/local
includedir ?= $(prefix)/include
libdir ?= $(prefix)/lib

JNI_FILE = libpd_wrapper/util/ringbuffer.c libpd_wrapper/util/z_queued.c $(JNI_SOUND)
JNIH_FILE = jni/z_jni.h
JAVA_BASE = java/org/puredata/core/PdBase.java
ifeq ($(OS), Windows_NT)
	LIBPD = libs/pd.$(SOLIB_EXT)
else
	LIBPD = libs/libpd.$(SOLIB_EXT)
endif
PDCSHARP = libs/libpdcsharp.$(SOLIB_EXT)

PDJAVA_BUILD = java-build
PDJAVA_DIR = $(PDJAVA_BUILD)/org/puredata/core/natives/$(PDNATIVE_PLATFORM)/$(PDNATIVE_ARCH)
PDJAVA_NATIVE = $(PDJAVA_DIR)/$(SOLIB_PREFIX)pdnative.$(PDNATIVE_SOLIB_EXT)
PDJAVA_JAR = libs/libpd.jar
PDJAVA_SRC = libs/libpd-sources.jar
PDJAVA_DOC = javadoc

CFLAGS = -DPD -DHAVE_UNISTD_H -DUSEAPI_DUMMY -I./pure-data/src \
         -I./libpd_wrapper -I./libpd_wrapper/util $(PLATFORM_CFLAGS) \
         $(OPT_CFLAGS) $(EXTRA_CFLAGS) $(MULTI_CFLAGS) $(LOCALE_CFLAGS) \
         $(ADDITIONAL_CFLAGS)
LDFLAGS += $(ADDITIONAL_LDFLAGS)
CSHARP_LDFLAGS += $(ADDITIONAL_LDFLAGS)
JAVA_LDFLAGS += $(ADDITIONAL_LDFLAGS)

.PHONY: libpd csharplib cpplib javalib javadoc javasrc install uninstall clean clobber

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
	javac -classpath java -d $(PDJAVA_BUILD) $(PDJAVA_JAR_CLASSES)
	jar -cvf $(PDJAVA_JAR) -C $(PDJAVA_BUILD) org/puredata/

javadoc: $(PDJAVA_JAR_CLASSES)
	javadoc -d $(PDJAVA_DOC) -sourcepath java org.puredata.core

javasrc: $(PDJAVA_SRC)

$(PDJAVA_SRC): $(PDJAVA_JAR_FILES)
	jar -cvf $(PDJAVA_SRC) $(PDJAVA_SRC_FILES) -C java org

csharplib: $(PDCSHARP)

$(PDCSHARP): ${PD_FILES:.c=.o} ${EXTRA_FILES:.c=.o}
	$(CC) -o $(PDCSHARP) $^ $(CSHARP_LDFLAGS) -lm -lpthread

clean:
	rm -f ${PD_FILES:.c=.o} ${PD_EXTRA_OBJS} ${JNI_FILE:.c=.o}
	rm -f ${UTIL_FILES:.c=.o} ${PD_EXTRA_FILES:.c=.o}

clobber: clean
	rm -f $(LIBPD) $(LIBPD_IMPLIB) $(LIBPD_DEF)
	rm -f $(PDCSHARP) ${PDCSHARP:.$(SOLIB_EXT)=.lib} ${PDCSHARP:.$(SOLIB_EXT)=.def}
	rm -f $(PDJAVA_JAR) $(PDJAVA_NATIVE) libs/`basename $(PDJAVA_NATIVE)`
	rm -rf $(PDJAVA_BUILD) $(PDJAVA_SRC) $(PDJAVA_DOC)

# optional install headers & libs based on build type: UTIL=true and/or windows
install:
	install -d $(includedir)/libpd
	install -m 644 libpd_wrapper/z_libpd.h $(includedir)/libpd
	install -m 644 pure-data/src/m_pd.h $(includedir)/libpd
	if [ -e libpd_wrapper/util/z_queued.o ]; then \
	    install -d $(includedir)/libpd/util; \
	    install -m 644 libpd_wrapper/util/z_print_util.h $(includedir)/libpd/util; \
	    install -m 644 libpd_wrapper/util/z_queued.h $(includedir)/libpd/util; \
		install -m 644 cpp/*hpp $(includedir)/libpd; \
	fi
	install -d $(libdir)
	install -m 755 $(LIBPD) $(libdir)
	if [ -e '$(LIBPD_IMPLIB)' ]; then install -m 755 $(LIBPD_IMPLIB) $(libdir); fi
	if [ -e '$(LIBPD_DEF)' ]; then install -m 755 $(LIBPD_DEF) $(libdir); fi

uninstall:
	rm -rf $(includedir)/libpd
	rm -f $(libdir)/`basename $(LIBPD)` $(libdir)/`basename $(LIBPD_IMPLIB)` $(libdir)/`basename $(LIBPD_DEF)`
