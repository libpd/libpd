from pylibpd import *

def pd_receive(*s):
  print 'received:', s

ptr = libpd_bind('eggs')

libpd_set_print_callback(pd_receive)
libpd_set_float_callback(pd_receive)
libpd_set_symbol_callback(pd_receive)
libpd_set_noteon_callback(pd_receive)

libpd_message('pd', 'dsp', 1)
libpd_message('pd', 'open', 'test.pd', '.')

libpd_init_audio(1, 2, 44100, 1)

libpd_float('spam', 42)
libpd_symbol('spam', "don't panic")

inp = float_array(64)
outp = float_array(128)

for i in range(64):
  inp[i] = i

libpd_process_float(inp, outp)

for i in range(64):
  print outp[2*i], outp[2*i+1],

