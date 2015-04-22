import pyaudio
import wave
import sys
from pylibpd import *

p = pyaudio.PyAudio()

ch = 1
sr = 44100
tpb = 6
bs = libpd_blocksize()

stream = p.open(format = pyaudio.paInt16,
                channels = ch,
                rate = sr,
                input = True,
                output = True,
                frames_per_buffer = bs * tpb)

m = PdManager(ch, ch, sr, 1)
libpd_open_patch('echo.pd')

while 1:
    data = stream.read(bs)
    outp = m.process(data)
    stream.write(outp)

stream.close()
p.terminate()
libpd_release()

