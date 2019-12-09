Pod::Spec.new do |spec|
  spec.name = "libpd"
  spec.version = "0.11.0"

  spec.license = { :type => 'Standard Improved BSD License', :file => 'License.txt' }

  spec.summary = "Pure Data embeddable audio synthesis library, useful as a sound engine in mobile phone apps, games, web pages, and art projects"
  spec.homepage = "https://github.com/libpd/libpd"
  spec.authors = "Peter Brinkmann", "Dan Wilcox", "Rich Eakin", "Miller Puckette (Pure Data)"

  spec.source = { :git => "https://github.com/libpd/libpd.git", :tag => spec.version.to_s, :submodules => true }
  spec.source_files = 'pure-data/src/**/*.{h,c}',
                      'pure-data/extra/**/*.{h,c}',
                      'libpd_wrapper/**/*.{h,c}',
                      'objc/**/*.{h,m}'
  spec.public_header_files = 'objc/**/*.{h}'
  spec.ios.deployment_target = '8.0'
  spec.requires_arc = false
  spec.frameworks = 'Foundation', 'AudioToolbox', 'AVFoundation'
  spec.compiler_flags = '-DPD', '-DUSEAPI_DUMMY', '-DHAVE_UNISTD_H', '-DLIBPD_EXTRA', '-fcommon'
  spec.exclude_files = 'pure-data/src/s_audio_alsa.h',
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
                       'pure-data/src/s_file.c',
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
