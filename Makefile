UNAME = $(shell uname)
SOLIB_PREFIX = lib

ifeq ($(UNAME), Darwin)  # Mac
  SOLIB_EXT = dylib
  PDNATIVE_SOLIB_EXT = jnilib
  PDNATIVE_PLATFORM = mac
  PDNATIVE_ARCH = 
  PLATFORM_CFLAGS = -DHAVE_LIBDL -O3 -arch x86_64 -arch i386 -g \
    -I/System/Library/Frameworks/JavaVM.framework/Headers
  LDFLAGS = -arch x86_64 -arch i386 -dynamiclib -ldl
  CSHARP_LDFLAGS = $(LDFLAGS)
  JAVA_LDFLAGS = -framework JavaVM $(LDFLAGS)
  STRIP = strip -x
else
  ifeq ($(OS), Windows_NT)  # Windows, use Mingw
    CC = gcc
    SOLIB_EXT = dll
    SOLIB_PREFIX = 
    PDNATIVE_PLATFORM = windows
    PDNATIVE_ARCH = $(shell $(CC) -dumpmachine | sed -e 's,-.*,,' -e 's,i[3456]86,x86,' -e 's,amd64,x86_64,')
    PLATFORM_CFLAGS = -DWINVER=0x502 -DWIN32 -D_WIN32 -DPD_INTERNAL -O3 \
      -I"$(JAVA_HOME)/include" -I"$(JAVA_HOME)/include/win32"
    MINGW_LDFLAGS = -shared -lws2_32 -lkernel32
    LDFLAGS = $(MINGW_LDFLAGS) -Wl,--output-def=libs/libpd.def \
      -Wl,--out-implib=libs/libpd.lib
    CSHARP_LDFLAGS = $(MINGW_LDFLAGS) -Wl,--output-def=libs/libpdcsharp.def \
      -Wl,--out-implib=libs/libpdcsharp.lib
    JAVA_LDFLAGS = $(MINGW_LDFLAGS) -Wl,--kill-at
    STRIP = strip --strip-unneeded -R .note -R .comment
  else  # Assume Linux
    SOLIB_EXT = so
    PDNATIVE_PLATFORM = linux
    PDNATIVE_ARCH = $(shell $(CC) -dumpmachine | sed -e 's,-.*,,' -e 's,i[3456]86,x86,' -e 's,amd64,x86_64,')
    JAVA_HOME ?= /usr/lib/jvm/default-java
    PLATFORM_CFLAGS = -DHAVE_LIBDL -Wno-int-to-pointer-cast \
      -Wno-pointer-to-int-cast -fPIC -I"$(JAVA_HOME)/include" \
      -I"$(JAVA_HOME)/include/linux" -O3 -g -D_GNU_SOURCE -Wall
    LDFLAGS = -shared -ldl -Wl,-Bsymbolic
    CSHARP_LDFLAGS = $(LDFLAGS)
    JAVA_LDFLAGS = $(LDFLAGS)
    STRIP = strip --strip-unneeded -R .note -R .comment	
  endif
endif

PDNATIVE_SOLIB_EXT ?= $(SOLIB_EXT)

# from the makefile of pd-extended
pd_SOURCES = \
    pure-data/src/g_canvas.c pure-data/src/g_graph.c pure-data/src/g_text.c pure-data/src/g_rtext.c pure-data/src/g_array.c pure-data/src/g_template.c \
    pure-data/src/g_io.c pure-data/src/g_scalar.c pure-data/src/g_traversal.c pure-data/src/g_guiconnect.c pure-data/src/g_readwrite.c pure-data/src/g_editor.c \
    pure-data/src/g_all_guis.c \
    pure-data/src/m_pd.c pure-data/src/m_class.c pure-data/src/m_obj.c pure-data/src/m_atom.c pure-data/src/m_memory.c pure-data/src/m_binbuf.c \
    pure-data/src/m_conf_pdextended.c pure-data/src/m_glob.c pure-data/src/m_sched.c \
    pure-data/src/s_main.c pure-data/src/s_inter.c pure-data/src/s_file.c \
    pure-data/src/s_loader.c pure-data/src/s_path.c pure-data/src/s_entry.c pure-data/src/s_audio.c \
    pure-data/src/s_utf8.c \
    pure-data/src/d_ugen.c pure-data/src/d_ctl.c pure-data/src/d_arithmetic.c pure-data/src/d_osc.c pure-data/src/d_filter.c \
    pure-data/src/d_math.c pure-data/src/d_array.c pure-data/src/d_global.c \
    pure-data/src/d_delay.c \
    pure-data/src/d_resample.c \
    pure-data/src/x_arithmetic.c pure-data/src/x_connective.c \
    pure-data/src/x_acoustics.c pure-data/src/d_soundfile.c \
    pure-data/src/e_fft.c pure-data/src/e_gfxstub.c pure-data/src/e_dac.c pure-data/src/e_midi.c \
    pure-data/src/g_magicglass.c \
    pure-data/src/import.c pure-data/src/path.c pure-data/src/closebang.c pure-data/src/initbang.c pure-data/src/s_print.c

#removed from pd_Sources for various reasons:
#  pure-data/src/s_midi.c
# removed from pd_Sources because they are in externals/vanilla:
#  pure-data/src/loadbang.c
#  

#added to pd_Sources:
pd_SOURCES_added = \
	pure-data/src/d_fft_mayer.c pure-data/src/d_fftroutine.c \
	pure-data/src/s_audio_dummy.c \
	pure-data/extra/expr~/vexp_fun.c pure-data/extra/expr~/vexp_if.c pure-data/extra/expr~/vexp.c


PD_FILES = $(pd_SOURCES) $(pd_SOURCES_added) \
	libpd_wrapper/s_libpdmidi.c libpd_wrapper/x_libpdreceive.c \
	libpd_wrapper/z_libpd.c loader-file.c

# Not used. Dont use.
PD_FILES_ORG = \
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
	pure-data/src/g_magicglass.c \
	pure-data/src/import.c pure-data/src/path.c pure-data/src/initbang.c pure-data/src/loadbang.c pure-data/src/closebang.c pure-data/src/print.c \
	pure-data/src/e_midi.c	pure-data/src/e_gfxstub.c \
	pure-data/src/m_atom.c pure-data/src/m_binbuf.c pure-data/src/m_class.c \
	pure-data/src/m_conf_pdextended.c pure-data/src/m_glob.c pure-data/src/m_memory.c \
	pure-data/src/m_obj.c pure-data/src/m_pd.c pure-data/src/m_sched.c \
	pure-data/src/s_audio.c pure-data/src/s_audio_dummy.c \
	pure-data/src/s_file.c pure-data/src/s_inter.c \
	pure-data/src/s_loader.c pure-data/src/s_main.c pure-data/src/s_path.c \
	pure-data/src/s_print.c pure-data/src/s_utf8.c pure-data/src/x_acoustics.c \
	pure-data/src/x_connective.c \
	pure-data/src/x_interface.c pure-data/src/x_list.c \
	pure-data/src/x_misc.c pure-data/src/x_net.c \
	pure-data/src/x_qlist.c pure-data/src/x_time.c \
	pure-data/src/x_arithmetic.c \
	libpd_wrapper/s_libpdmidi.c libpd_wrapper/x_libpdreceive.c \
	pure-data/src/e_fft.c \
	libpd_wrapper/z_libpd.c
# /home/kjetil/temp/pd-extended/externals/vanilla/dac~.c
#/home/kjetil/temp/pd-extended//externals/vanilla/lib_d_arithmetic.c \
#/home/kjetil/temp/pd-extended/externals/vanilla/lib_x_arithmetic.c

# removed in pdextended:
# pure-data/src/x_gui.c pure-data/src/x_midi.c

LIBPDS_FILES = libpds/libpds.c

PDJAVA_JAR_CLASSES = \
	java/org/puredata/core/PdBase.java \
	java/org/puredata/core/NativeLoader.java \
	java/org/puredata/core/PdListener.java \
	java/org/puredata/core/PdMidiListener.java \
	java/org/puredata/core/PdMidiReceiver.java \
	java/org/puredata/core/PdReceiver.java \
	java/org/puredata/core/utils/IoUtils.java \
	java/org/puredata/core/utils/PdDispatcher.java


JNI_FILE = libpd_wrapper/util/ringbuffer.c libpd_wrapper/util/z_queued.c \
	jni/z_jni_plain.c
JNIH_FILE = jni/z_jni.h
JAVA_BASE = java/org/puredata/core/PdBase.java
HOOK_SET = libpd_wrapper/util/z_hook_util.c
LIBPD = libs/libpd.$(SOLIB_EXT)
LIBPDS = libs/libpds.$(SOLIB_EXT)
LIBPDS_STATIC = libs/libpds.a
PDCSHARP = libs/libpdcsharp.$(SOLIB_EXT)

PDJAVA_BUILD = java-build
PDJAVA_DIR = $(PDJAVA_BUILD)/org/puredata/core/natives/$(PDNATIVE_PLATFORM)/$(PDNATIVE_ARCH)/
PDJAVA_NATIVE = $(PDJAVA_DIR)/$(SOLIB_PREFIX)pdnative.$(PDNATIVE_SOLIB_EXT)
PDJAVA_JAR = libs/libpd.jar

CFLAGS = -DPD -DLIBPD -DHAVE_UNISTD_H -DUSEAPI_DUMMY -I./pure-data/src \
         -I./libpd_wrapper -I./libpd_wrapper/util $(PLATFORM_CFLAGS)

.PHONY: libpd csharplib javalib clean clobber

all: $(LIBPDS) $(LIBPDS_STATIC) $(LIBPD) vanilla_install

$(LIBPDS): libpds/libpds_autogenerated.c libpds/libpds.h libpds/libpd_so.c $(LIBPDS_FILES) $(LIBPD)
	$(CC) $(CFLAGS) -o $(LIBPDS) $(LIBPDS_FILES) $(LDFLAGS) -lm -ldl 

$(LIBPDS_STATIC): libpds/libpds_autogenerated.c libpds/libpds.h libpds/libpd_so.c $(LIBPDS_FILES) $(LIBPD)
	$(CC) $(CFLAGS) -c -o libpds.o $(LIBPDS_FILES) -lm -ldl
	ar rcs $(LIBPDS_STATIC) libpds.o

loader-file.c: make.scm
	bash -c "if ! ./make.scm gen-loader-file >loader-file.c ; then rm -f loader-file.c ; echo 'could not make loader-file.c' ; failed-failed-failed ; fi "
	bash -c "if [[ ! -s loader-file.c ]] ; then rm -f loader-file.c ; echo 'could not make loader-file.c' ; failed-failed-failed ; fi "

libpds/libpd_so.c: libpds/bin2c $(LIBPD)
	bash -c "if ! libpds/bin2c libs/libpd.so >libpds/libpd_so.c ; then rm -f loader-file.c ; echo 'could not make loader-file.c' ; failed-failed-failed ; fi "
	bash -c "if [[ ! -s libpds/libpd_so.c ]] ; then rm -f libpds/libpd_so.c ; echo 'could not make libpds/libpd_so.c' ; failed-failed-failed ; fi "

libpds/bin2c: libpds/bin2c.c
	$(CC) libpds/bin2c.c -Wall -o libpds/bin2c

$(LIBPD): make.scm ${PD_FILES:.c=.o}
	./make.scm compile
	$(CC) -o $(LIBPD) ${PD_FILES:.c=.o} `./make.scm print-object-files` $(LDFLAGS) -lm -lpthread -lfftw3f -lfftw3 -logg -lvorbis -lvorbisfile -lvorbisenc -lspeex

libpds/libpds_autogenerated.c: libpds/gen.scm
	which guile
	bash -c "if ! libpds/gen.scm c-file >libpds/libpds_autogenerated.c ; then rm -f loader-file.c ; echo 'could not make loader-file.c' ; failed-failed-failed ; fi "
	bash -c "if [[ ! -s libpds/libpds_autogenerated.c ]] ; then rm -f libpds/libpds_autogenerated.c ; echo 'could not make libpds/libpds_autogenerated.c' ; failed-failed-failed ; fi "

libpds/libpds.h: libpds/gen.scm
	which guile
	bash -c "if ! libpds/gen.scm h-file >libpds/libpds.h ; then rm -f libpds/libpds.h ; echo 'could not make libpds/libpds.h' ; failed-failed-failed ; fi "
	bash -c "if [[ ! -s libpds/libpds.h ]] ; then rm -f libpds/libpds.h ; echo 'could not make libpds/libpds.h' ; failed-failed-failed ; fi "

vanilla_install: vanilla
	make -C externals/vanilla STRIP="$(STRIP)" \
		DESTDIR=../../pure-data/extra objectsdir="$(objectsdir)" install

vanilla: extra
	make -j8 -C externals/vanilla CFLAGS="$(CFLAGS) -fno-tree-vectorize -fno-strict-aliasing" \
		PD_PATH=../../pure-data/src PD_INCLUDE=../../pure-data/src

vanilla_clean:
	make -C externals/vanilla clean DESTDIR=../../pure-data/extra
	rm -f pure-data/extra/vanilla/*

extra: pure-data/extra/choice/choice.pd_linux

pure-data/extra/choice/choice.pd_linux:
	cd pure-data/extra && make all

javalib: $(JNIH_FILE) $(PDJAVA_JAR)

$(JNIH_FILE): $(JAVA_BASE)
	javac -classpath java $^
	javah -o $@ -classpath java org.puredata.core.PdBase

$(PDJAVA_NATIVE): ${PD_FILES:.c=.o} ${JNI_FILE:.c=.o}
	mkdir -p $(PDJAVA_DIR)
	$(CC) -o $(PDJAVA_NATIVE) $^ -lm -lpthread $(JAVA_LDFLAGS) 
	cp $(PDJAVA_NATIVE) libs/

$(PDJAVA_JAR): $(PDJAVA_NATIVE) $(PDJAVA_JAR_CLASSES)
	javac -d $(PDJAVA_BUILD) $(PDJAVA_JAR_CLASSES)
	jar -cvf $(PDJAVA_JAR) -C $(PDJAVA_BUILD) org/puredata/

csharplib: $(PDCSHARP)

$(PDCSHARP): ${PD_FILES:.c=.o} ${HOOK_SET:.c=.o}
	gcc -o $(PDCSHARP) $^ $(CSHARP_LDFLAGS) -lm -lpthread

clean: vanilla_clean
	rm -f ${PD_FILES:.c=.o} ${JNI_FILE:.c=.o} ${HOOK_SET:.c=.o} libpds/libpds_autogenerated.c libpds/libpds.h libpds/libpd_so.c libpds/bin2c loader-file.o externalobjs/*.o

clobber: clean
	rm -f $(LIBPD) $(LIBPDS) $(PDCSHARP) $(PDJAVA_NATIVE) $(PDJAVA_JAR)
	rm -f libs/`basename $(PDJAVA_NATIVE)`
	rm -rf $(PDJAVA_BUILD)
