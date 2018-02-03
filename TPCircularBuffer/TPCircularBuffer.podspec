license = <<-END
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE."
END

Pod::Spec.new do |s|
  s.name               = "TPCircularBuffer"
  s.version            = '1.5'
  s.summary            = 'A simple, fast circular buffer implementation.'
  s.homepage           = 'https://github.com/michaeltyson/TPCircularBuffer'
  s.authors            = { 'Michael Tyson' => 'michael@atastypixel.com' }
  s.license            = { :type => 'MIT', :text => license }
  s.source             = { :git => 'https://github.com/michaeltyson/TPCircularBuffer.git', :tag => '1.5' }
  s.source_files       = '*.{c,h}'
  s.requires_arc       = false
  s.frameworks         = 'AudioToolbox'
  s.ios.deployment_target = '4.3'
  s.osx.deployment_target = '10.8'
end
