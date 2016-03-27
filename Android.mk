LOCAL_PATH := $(call my-dir)

# PD-specific flags

PD_SRC_FILES := \
  pure-data/src/d_arithmetic.c pure-data/src/d_array.c pure-data/src/d_ctl.c \
  pure-data/src/d_dac.c pure-data/src/d_delay.c pure-data/src/d_fft.c \
  pure-data/src/d_fft_fftsg.c \
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
  pure-data/src/x_gui.c pure-data/src/x_list.c pure-data/src/x_midi.c \
  pure-data/src/x_misc.c pure-data/src/x_net.c pure-data/src/x_array.c \
  pure-data/src/x_time.c pure-data/src/x_interface.c pure-data/src/x_scalar.c \
  pure-data/src/x_text.c libpd_wrapper/s_libpdmidi.c \
  libpd_wrapper/x_libpdreceive.c libpd_wrapper/z_libpd.c \
  libpd_wrapper/util/ringbuffer.c libpd_wrapper/util/z_queued.c \
  libpd_wrapper/z_hooks.c
PD_C_INCLUDES := $(LOCAL_PATH)/pure-data/src $(LOCAL_PATH)/libpd_wrapper \
  $(LOCAL_PATH)/libpd_wrapper/util
PD_CFLAGS := -DPD -DHAVE_UNISTD_H -DHAVE_LIBDL -DUSEAPI_DUMMY -w
PD_JNI_CFLAGS := -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast
PD_LDLIBS := -ldl


# Build libpd

include $(CLEAR_VARS)

LOCAL_MODULE := pd
LOCAL_C_INCLUDES := $(PD_C_INCLUDES)
LOCAL_CFLAGS := $(PD_CFLAGS)
LOCAL_LDLIBS := $(PD_LDLIBS)
LOCAL_SRC_FILES := $(PD_SRC_FILES)
include $(BUILD_SHARED_LIBRARY)


# Build plain JNI binary

include $(CLEAR_VARS)

LOCAL_MODULE := pdnative
LOCAL_C_INCLUDES := $(PD_C_INCLUDES)
LOCAL_CFLAGS := $(PD_JNI_CFLAGS)
LOCAL_SRC_FILES := jni/z_jni_plain.c
LOCAL_SHARED_LIBRARIES := pd
include $(BUILD_SHARED_LIBRARY)


# Build OpenSL JNI binary

include $(CLEAR_VARS)

LOCAL_MODULE := pdnativeopensl
LOCAL_C_INCLUDES := $(PD_C_INCLUDES) $(LOCAL_PATH)/jni
LOCAL_CFLAGS := $(PD_JNI_CFLAGS)
LOCAL_LDLIBS := -lOpenSLES -llog
LOCAL_SRC_FILES := jni/opensl_stream/opensl_stream.c jni/z_jni_opensl.c
LOCAL_SHARED_LIBRARIES := pd
include $(BUILD_SHARED_LIBRARY)


# Build libchoice.so

include $(CLEAR_VARS)

LOCAL_MODULE := choice
LOCAL_C_INCLUDES := $(LOCAL_PATH)/pure-data/src
LOCAL_CFLAGS := -DPD
LOCAL_SRC_FILES := pure-data/extra/choice/choice.c
LOCAL_SHARED_LIBRARIES := pd

include $(BUILD_SHARED_LIBRARY)


# Build libbonk_tilde.so

include $(CLEAR_VARS)

LOCAL_MODULE := bonk_tilde
LOCAL_C_INCLUDES := $(LOCAL_PATH)/pure-data/src
LOCAL_CFLAGS := -DPD
LOCAL_SRC_FILES := pure-data/extra/bonk~/bonk~.c
LOCAL_SHARED_LIBRARIES := pd

include $(BUILD_SHARED_LIBRARY)


# Build liblrshift_tilde.so

include $(CLEAR_VARS)

LOCAL_MODULE := lrshift_tilde
LOCAL_C_INCLUDES := $(LOCAL_PATH)/pure-data/src
LOCAL_CFLAGS := -DPD
LOCAL_SRC_FILES := pure-data/extra/lrshift~/lrshift~.c
LOCAL_SHARED_LIBRARIES := pd

include $(BUILD_SHARED_LIBRARY)


# Build libfiddle_tilde.so

include $(CLEAR_VARS)

LOCAL_MODULE := fiddle_tilde
LOCAL_C_INCLUDES := $(LOCAL_PATH)/pure-data/src
LOCAL_CFLAGS := -DPD
LOCAL_SRC_FILES := pure-data/extra/fiddle~/fiddle~.c
LOCAL_SHARED_LIBRARIES := pd

include $(BUILD_SHARED_LIBRARY)


# Build libsigmund_tilde.so

include $(CLEAR_VARS)

LOCAL_MODULE := sigmund_tilde
LOCAL_C_INCLUDES := $(LOCAL_PATH)/pure-data/src
LOCAL_CFLAGS := -DPD
LOCAL_SRC_FILES := pure-data/extra/sigmund~/sigmund~.c
LOCAL_SHARED_LIBRARIES := pd

include $(BUILD_SHARED_LIBRARY)


# Build libpique.so

include $(CLEAR_VARS)

LOCAL_MODULE := pique
LOCAL_C_INCLUDES := $(LOCAL_PATH)/pure-data/src
LOCAL_CFLAGS := -DPD
LOCAL_SRC_FILES := pure-data/extra/pique/pique.c
LOCAL_SHARED_LIBRARIES := pd

include $(BUILD_SHARED_LIBRARY)


# Build libloop_tilde.so

include $(CLEAR_VARS)

LOCAL_MODULE := loop_tilde
LOCAL_C_INCLUDES := $(LOCAL_PATH)/pure-data/src
LOCAL_CFLAGS := -DPD
LOCAL_SRC_FILES := pure-data/extra/loop~/loop~.c
LOCAL_SHARED_LIBRARIES := pd

include $(BUILD_SHARED_LIBRARY)


# Build libexpr.so

include $(CLEAR_VARS)

LOCAL_MODULE := expr
LOCAL_C_INCLUDES := $(LOCAL_PATH)/pure-data/src
LOCAL_CFLAGS := -DPD
LOCAL_SRC_FILES := pure-data/extra/expr~/vexp.c \
          pure-data/extra/expr~/vexp_fun.c pure-data/extra/expr~/vexp_if.c
LOCAL_SHARED_LIBRARIES := pd

include $(BUILD_SHARED_LIBRARY)

# Build bob_tilde.so

include $(CLEAR_VARS)

LOCAL_MODULE := bob_tilde
LOCAL_C_INCLUDES := $(LOCAL_PATH)/pure-data/src
LOCAL_CFLAGS := -DPD
LOCAL_SRC_FILES := pure-data/extra/bob~/bob~.c
LOCAL_SHARED_LIBRARIES := pd

include $(BUILD_SHARED_LIBRARY)

# Build stdout.so

include $(CLEAR_VARS)

LOCAL_MODULE := stdout
LOCAL_C_INCLUDES := $(LOCAL_PATH)/pure-data/src
LOCAL_CFLAGS := -DPD
LOCAL_SRC_FILES := pure-data/extra/stdout/stdout.c
LOCAL_SHARED_LIBRARIES := pd

include $(BUILD_SHARED_LIBRARY)

