"""
A filter (in the sense of Unix, not DSP) for audio files, using Pd.  The idea
is to read a wav file (either from stdin or from a file given with -i), send
it into a Pd patch (given by the file name and an option path) and to write
the result to stdin or to a file given with -o.
"""

try:
    # On Linux dlopenflags need to be set for Pd plugin loading to work.
    import DLFCN
    import sys
    sys.setdlopenflags(DLFCN.RTLD_LAZY | DLFCN.RTLD_GLOBAL)
except ImportError:
    pass


import wave
import pylibpd

def pdfilter(inp, outp, patch, folder = '.'):
  inw = wave.open(inp, 'rb')
  outw = wave.open(outp, 'wb')
  try:
    w = inw.getsampwidth()
    if w != 2: raise Exception('wrong sample width')
    ch = inw.getnchannels()
    sr = inw.getframerate()
    n = inw.getnframes()
    outw.setsampwidth(w)
    outw.setnchannels(ch)
    outw.setframerate(sr)
    outw.setnframes(n)
    p = pylibpd.libpd_open_patch(patch, folder)
    m = pylibpd.PdManager(ch, ch, sr, 1)
    for i in xrange(n / 64):
      x = inw.readframes(64)
      y = m.process(x)
      outw.writeframesraw(y)
    r = n % 64
    x = inw.readframes(r) + b'\x00' * w * ch * (64 - r)
    y = m.process(x)
    outw.writeframesraw(y[: w * ch * r])
    pylibpd.libpd_close_patch(p)
  finally:
    inw.close()
    outw.close()

if __name__ == '__main__':
  import getopt
  import sys
  opts, args = getopt.getopt(sys.argv[1:], 'i:o:')
  if not len(args) in (1, 2):
    raise Exception('usage: filter.py [-i input.wav] [-o output.wav] foo.pd [path]')
  dopts = dict(opts)
  inp = dopts.get('-i', sys.stdin)
  outp = dopts.get('-o', sys.stdout)
  pdfilter(inp, outp, *args)

