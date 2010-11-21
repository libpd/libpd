PD_FILES = \
	d_arithmetic.c d_array.c d_ctl.c d_dac.c d_delay.c d_fft.c d_fft_mayer.c \
	d_fftroutine.c d_filter.c d_global.c d_math.c d_misc.c d_osc.c d_resample.c \
	d_soundfile.c d_ugen.c g_all_guis.c g_array.c g_bang.c g_canvas.c \
	g_editor.c g_graph.c g_guiconnect.c g_hdial.c g_hslider.c g_io.c \
	g_mycanvas.c g_numbox.c g_readwrite.c g_rtext.c g_scalar.c g_template.c \
	g_text.c g_toggle.c g_traversal.c g_vdial.c g_vslider.c g_vumeter.c \
	m_atom.c m_binbuf.c m_class.c m_conf.c m_glob.c m_memory.c m_obj.c m_pd.c \
	m_sched.c s_audio.c s_audio_dummy.c s_entry.c s_file.c s_inter.c s_loader.c \
	s_main.c s_midi.c s_midi_dummy.c s_path.c s_print.c s_utf8.c x_acoustics.c \
	x_arithmetic.c x_connective.c x_gui.c x_interface.c x_libpdreceive.c \
	x_list.c x_midi.c x_misc.c x_net.c x_qlist.c x_time.c z_libpd.c
JNI_FILES = z_jni.c
LIBPD = libpd.so
PDJAVA = libpdnative.so

CFLAGS = -DPD -DHAVE_UNISTD_H -DHAVE_LIBDL -DUSEAPI_DUMMY \
			-Wno-int-to-pointer-cast -Wno-pointer-to-int-cast \
			-I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/linux

.PHONY: all clean clobber

all: $(LIBPD) $(PDJAVA)

$(LIBPD): ${PD_FILES:.c=.o}
	gcc -shared -ldl -lm -lpthread -o $(LIBPD) $^

$(PDJAVA): ${PD_FILES:.c=.o} ${JNI_FILES:.c=.o}
	gcc -shared -ldl -lm -lpthread -o $(PDJAVA) $^

clean:
	-rm *.o

clobber: clean
	-rm $(LIBPD) $(PDJAVA)

