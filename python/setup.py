#!/usr/bin/python

from distutils.core import setup, Extension

setup(name='pypdlib',
      version='0.2',
      py_modules = [
        'pylibpd'
      ],
      ext_modules = [
        Extension("_pylibpd",
                  define_macros = [
                    ('PD', 1),
                    ('HAVE_UNISTD_H', 1),
                    ('HAVE_LIBDL', 1),
                    ('USEAPI_DUMMY', 1),
                    ('LIBPD_EXTRA', 1)
                  ],
                  include_dirs = [
                    '../libpd_wrapper',
                    '../pure-data/src'
                  ],
                  libraries = [
                    'm',
                    'dl',
                    'pthread'
                  ],
                  sources = [
                    'pylibpd.i',
                    '../libpd_wrapper/s_libpdmidi.c',
                    '../libpd_wrapper/x_libpdreceive.c',
                    '../libpd_wrapper/z_hooks.c',
                    '../libpd_wrapper/z_libpd.c',
                    '../pure-data/src/d_arithmetic.c',
                    '../pure-data/src/d_array.c',
                    '../pure-data/src/d_ctl.c',
                    '../pure-data/src/d_dac.c',
                    '../pure-data/src/d_delay.c',
                    '../pure-data/src/d_fft.c',
                    '../pure-data/src/d_fft_fftsg.c',
                    '../pure-data/src/d_filter.c',
                    '../pure-data/src/d_global.c',
                    '../pure-data/src/d_math.c',
                    '../pure-data/src/d_misc.c',
                    '../pure-data/src/d_osc.c',
                    '../pure-data/src/d_resample.c',
                    '../pure-data/src/d_soundfile.c',
                    '../pure-data/src/d_ugen.c',
                    '../pure-data/src/g_all_guis.c',
                    '../pure-data/src/g_array.c',
                    '../pure-data/src/g_bang.c',
                    '../pure-data/src/g_canvas.c',
                    '../pure-data/src/g_clone.c',
                    '../pure-data/src/g_editor.c',
                    '../pure-data/src/g_editor_extras.c',
                    '../pure-data/src/g_graph.c',
                    '../pure-data/src/g_guiconnect.c',
                    '../pure-data/src/g_hdial.c',
                    '../pure-data/src/g_hslider.c',
                    '../pure-data/src/g_io.c',
                    '../pure-data/src/g_mycanvas.c',
                    '../pure-data/src/g_numbox.c',
                    '../pure-data/src/g_readwrite.c',
                    '../pure-data/src/g_rtext.c',
                    '../pure-data/src/g_scalar.c',
                    '../pure-data/src/g_template.c',
                    '../pure-data/src/g_text.c',
                    '../pure-data/src/g_toggle.c',
                    '../pure-data/src/g_traversal.c',
                    '../pure-data/src/g_undo.c',
                    '../pure-data/src/g_vdial.c',
                    '../pure-data/src/g_vslider.c',
                    '../pure-data/src/g_vumeter.c',
                    '../pure-data/src/m_atom.c',
                    '../pure-data/src/m_binbuf.c',
                    '../pure-data/src/m_class.c',
                    '../pure-data/src/m_conf.c',
                    '../pure-data/src/m_glob.c',
                    '../pure-data/src/m_memory.c',
                    '../pure-data/src/m_obj.c',
                    '../pure-data/src/m_pd.c',
                    '../pure-data/src/m_sched.c',
                    '../pure-data/src/s_audio.c',
                    '../pure-data/src/s_audio_dummy.c',
                    '../pure-data/src/s_inter.c',
                    '../pure-data/src/s_loader.c',
                    '../pure-data/src/s_main.c',
                    '../pure-data/src/s_path.c',
                    '../pure-data/src/s_print.c',
                    '../pure-data/src/s_utf8.c',
                    '../pure-data/src/x_acoustics.c',
                    '../pure-data/src/x_array.c',
                    '../pure-data/src/x_arithmetic.c',
                    '../pure-data/src/x_connective.c',
                    '../pure-data/src/x_gui.c',
                    '../pure-data/src/x_interface.c',
                    '../pure-data/src/x_list.c',
                    '../pure-data/src/x_midi.c',
                    '../pure-data/src/x_misc.c',
                    '../pure-data/src/x_net.c',
                    '../pure-data/src/x_scalar.c',
                    '../pure-data/src/x_text.c',
                    '../pure-data/src/x_time.c',
                    '../pure-data/src/x_vexp.c',
                    '../pure-data/src/x_vexp_if.c',
                    '../pure-data/src/x_vexp_fun.c',
                    '../pure-data/extra/bob~/bob~.c',
                    '../pure-data/extra/bonk~/bonk~.c', \
                    '../pure-data/extra/choice/choice.c', \
                    '../pure-data/extra/fiddle~/fiddle~.c', \
                    '../pure-data/extra/loop~/loop~.c', \
                    '../pure-data/extra/lrshift~/lrshift~.c', \
                    '../pure-data/extra/pique/pique.c', \
                    '../pure-data/extra/sigmund~/sigmund~.c', \
                    '../pure-data/extra/stdout/stdout.c'
                  ]
        )
      ]
)

