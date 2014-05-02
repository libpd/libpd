Pod::Spec.new do |s|
  s.name         = "libpd"
  s.version      = "0.0.1"

  s.license      = { :type => 'Standard Improved BSD License', :file => 'License.txt' }

  s.summary      = "libpd turns Pd into an embeddable library, so you can use Pd as a sound engine in mobile phone apps, games, web pages, and art projects."
  s.homepage     = "https://github.com/libpd/libpd"
  s.author       = { "name or nickname" => "email@address.com" }

  s.source       = { :git => "https://github.com/libpd/libpd.git", :commit => "HEAD" }
  s.source_files = 'cpp/**/*.{hpp,cpp}', 'libpd_wrapper/**/*.{h,c}', 'objc/**/*.{h,m}', 'pure-data/src/**/*.{h,c}'

  s.ios.deployment_target = '4.0'

  s.frameworks = 'Foundation'
  s.compiler_flags = '-w -DPD -DUSEAPI_DUMMY -DHAVE_LIBDL -DHAVE_UNISTD_H'
end
