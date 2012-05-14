UNAME = $(shell uname)
SOLIB_PREFIX = lib

ifeq ($(UNAME), Darwin)  # Mac
  SOLIB_EXT = dylib
  PLATFORM_CFLAGS = -DHAVE_LIBDL -O3 -arch x86_64 -arch i386 -g \
    -I/System/Library/Frameworks/JavaVM.framework/Headers
  LDFLAGS = -arch x86_64 -arch i386 -dynamiclib -ldl
  CSHARP_LDFLAGS = $(LDFLAGS)
  JAVA_LDFLAGS = -framework JavaVM $(LDFLAGS)
else
  ifeq ($(OS), Windows_NT)  # Windows, use Mingw
    CC = gcc
    SOLIB_EXT = dll
    SOLIB_PREFIX = 
    PLATFORM_CFLAGS = -DWINVER=0x502 -DWIN32 -D_WIN32 -DPD_INTERNAL -O3 \
      -I"$(JAVA_HOME)/include" -I"$(JAVA_HOME)/include/win32"
    MINGW_LDFLAGS = -shared -lws2_32 -lkernel32
    LDFLAGS = $(MINGW_LDFLAGS) -Wl,--output-def=libs/libpd.def \
      -Wl,--out-implib=libs/libpd.lib
    CSHARP_LDFLAGS = $(MINGW_LDFLAGS) -Wl,--output-def=libs/libpdcsharp.def \
      -Wl,--out-implib=libs/libpdcsharp.lib
    JAVA_LDFLAGS = $(MINGW_LDFLAGS) -Wl,--output-def=libs/$(SOLIB_PREFIX)pdnative.def \
      -Wl,--out-implib=libs/$(SOLIB_PREFIX)pdnative.lib
  else  # Assume Linux
    SOLIB_EXT = so
    JAVA_HOME ?= /usr/lib/jvm/default-java
    PLATFORM_CFLAGS = -DHAVE_LIBDL -Wno-int-to-pointer-cast \
      -Wno-pointer-to-int-cast -fPIC -I"$(JAVA_HOME)/include" \
      -I"$(JAVA_HOME)/include/linux" -O3
    LDFLAGS = -shared -ldl -Wl,-Bsymbolic
    CSHARP_LDFLAGS = $(LDFLAGS)
    JAVA_LDFLAGS = $(LDFLAGS)
  endif
endif

PD_FILES = \
	pure-data/src/d_arithmetic.c pure-data/src/d_array.c pure-data/src/d_ctl.c \
	pure-data/src/d_dac.c pure-data/src/d_delay.c pure-data/src/d_fft.c \
	pure-data/src/d_fft_mayer.c pure-data/src/d_fftroutine.c \
	pure-data/src/d_filter.c pure-data/src/d_global.c pure-data/src/d_math.c \
	pure-data/src/d_misc.c pure-data/src/d_osc.c pure-data/src/d_resample.c \
	pure-data/src/d_soundfile.c pure-data/src/d_ugen.c \
	pure-data/src/g_all_guis.c pure-data/src/g_array.c pure-data/src/g_bang.c \
	pure-data/src/g_canvas.c pure-data/src/g_editor.c pure-data/src/g_graph.c \
	pure-data/src/g_guiconnect.c pure-data/src/g_hdial.c \
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
	pure-data/src/x_arithmetic.c pure-data/src/x_connective.c \
	pure-data/src/x_gui.c pure-data/src/x_interface.c pure-data/src/x_list.c \
	pure-data/src/x_midi.c pure-data/src/x_misc.c pure-data/src/x_net.c \
	pure-data/src/x_qlist.c pure-data/src/x_time.c \
	libpd_wrapper/s_libpdmidi.c libpd_wrapper/x_libpdreceive.c \
	libpd_wrapper/z_libpd.c 
	
JNI_FILE = libpd_wrapper/z_jni.c
JNIH_FILE = libpd_wrapper/z_jni.h
JAVA_BASE = java/org/puredata/core/PdBase.java
HOOK_SET = libpd_wrapper/z_csharp_helper.c
LIBPD = libs/libpd.$(SOLIB_EXT)
PDCSHARP = libs/libpdcsharp.$(SOLIB_EXT)
PDJAVA = libs/$(SOLIB_PREFIX)pdnative.$(SOLIB_EXT)

CFLAGS = -DPD -DHAVE_UNISTD_H -DUSEAPI_DUMMY -I./pure-data/src \
         -I./libpd_wrapper $(PLATFORM_CFLAGS)

.PHONY: libpd csharplib javalib clean clobber

libpd: $(LIBPD)

$(LIBPD): ${PD_FILES:.c=.o}
	gcc -o $(LIBPD) $^ $(LDFLAGS) -lm -lpthread 

javalib: $(JNIH_FILE) $(PDJAVA)

$(JNIH_FILE): $(JAVA_BASE)
	javac -classpath java $^
	javah -o $@ -classpath java org.puredata.core.PdBase

$(PDJAVA): ${PD_FILES:.c=.o} ${JNI_FILE:.c=.o}
	gcc -o $(PDJAVA) $^ -lm -lpthread $(JAVA_LDFLAGS) 

csharplib: $(PDCSHARP)

$(PDCSHARP): ${PD_FILES:.c=.o} ${HOOK_SET:.c=.o}
	gcc -o $(PDCSHARP) $^ $(CSHARP_LDFLAGS) -lm -lpthread

clean:
	rm -f ${PD_FILES:.c=.o} ${JNI_FILE:.c=.o} ${HOOK_SET:.c=.o}

clobber: clean
	rm -f $(LIBPD) $(PDCSHARP) $(PDJAVA)
