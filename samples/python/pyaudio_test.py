# This sample code is licensed under https://creativecommons.org/licenses/by-sa/3.0/
# source: http://stackoverflow.com/a/29589319/426990
# author: Paul John Leonard - http://stackoverflow.com/users/1934792/paul-john-leonard
# adaptations: make use of existing libpd bloopy.pd sample

import pyaudio
from pylibpd import *
import time

def callback(in_data,frame_count,time_info,status):
    outp = m.process(data)
    return (outp,pyaudio.paContinue)

p  = pyaudio.PyAudio()
bs = libpd_blocksize()

stream = p.open(format = pyaudio.paInt16,
               channels = 1,
               rate = 44100,
               input = False,
               output = True,
               frames_per_buffer = bs,
               stream_callback=callback)

m = PdManager(1, 1 , 44100, 1)

libpd_open_patch('bloopy.pd')

data=array.array('B',[0]*bs)

while stream.is_active():
    time.sleep(.1)

stream.close()
p.terminate()
libpd_release()
