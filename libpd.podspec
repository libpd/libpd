Pod::Spec.new do |s|
  s.name = "libpd"
  s.version = "0.8.3"

  s.license = { :type => 'Standard Improved BSD License', :file => 'License.txt' }

  s.summary = "Pure Data embeddable audio synthesis library, useful as a sound engine in mobile phone apps, games, web pages, and art projects"
  s.homepage = "https://github.com/libpd/libpd"
  s.authors = "Peter Brinkmann", "Dan Wilcox", "Rich Eakin", "Miller Puckette (Pure Data)"

  s.source = { :git => "https://github.com/libpd/libpd.git", :tag => s.version.to_s, :submodules => true }
  s.source_files = 'cpp/**/*.{hpp,cpp}', 'libpd_wrapper/**/*.{h,c}', 'objc/**/*.{h,m}', 'pure-data/src/**/*.{h,c}', 'pure-data/extra/**/*.{h,c}'
  s.ios.deployment_target = '6.0'
  s.requires_arc = false
  s.frameworks = 'Foundation', 'AudioToolbox', 'AVFoundation'
  s.compiler_flags = '-DPD', '-DUSEAPI_DUMMY', '-DHAVE_UNISTD_H', '-DHAVE_ALLOCA_H', '-DLIBPD_EXTRA'
  s.exclude_files = 'pure-data/src/s_audio_alsa.h',
                    'pure-data/src/s_audio_alsa.c',
                    'pure-data/src/s_audio_alsamm.c',
                    'pure-data/src/s_audio_audiounit.c',
                    'pure-data/src/s_audio_esd.c',
                    'pure-data/src/s_audio_jack.c',
                    'pure-data/src/s_audio_mmio.c',
                    'pure-data/src/s_audio_oss.c',
                    'pure-data/src/s_audio_pa.c',
                    'pure-data/src/s_audio_paring.h',
                    'pure-data/src/s_audio_paring.c',
                    'pure-data/src/s_midi_alsa.c',
                    'pure-data/src/s_midi_dummy.c',
                    'pure-data/src/s_midi_mmio.c',
                    'pure-data/src/s_midi_oss.c',
                    'pure-data/src/s_midi_pm.c',
                    'pure-data/src/s_midi.c',
                    'pure-data/src/d_fft_fftw.c',
                    'pure-data/src/s_entry.c',
                    'pure-data/src/s_watchdog.c',
                    'pure-data/src/u_pdreceive.c',
                    'pure-data/src/u_pdsend.c'
end
