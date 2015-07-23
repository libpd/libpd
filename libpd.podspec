Pod::Spec.new do |s|
  s.name = "libpd"
  s.version = "0.8.2"

  s.license = { :type => 'Standard Improved BSD License', :file => 'License.txt' }

  s.summary = "Pure Data embeddable audio synthesis library, useful as a sound engine in mobile phone apps, games, web pages, and art projects"
  s.homepage = "https://github.com/libpd/libpd"
  s.authors = "Peter Brinkmann", "Dan Wilcox", "Rich Eakin", "Miller Puckette (Pure Data)"

  s.source = { :git => "https://github.com/libpd/libpd.git", :tag => s.version.to_s}
  s.source_files = 'cpp/**/*.{hpp,cpp}', 'libpd_wrapper/**/*.{h,c}', 'objc/**/*.{h,m}', 'pure-data/src/**/*.{h,c}'

  s.ios.deployment_target = '6.0'

  s.frameworks = 'Foundation', 'AudioToolbox', 'AVFoundation'
  s.compiler_flags = '-DPD', '-DUSEAPI_DUMMY', '-DHAVE_UNISTD_H', '-DHAVE_ALLOCA_H', '-DLIBPD_EXTRA'
end
