from pylibpd import *
import array

def pd_receive(*s):
  print 'received:', s


libpd_set_print_callback(pd_receive)
libpd_set_float_callback(pd_receive)
libpd_set_list_callback(pd_receive)
libpd_set_symbol_callback(pd_receive)
libpd_set_noteon_callback(pd_receive)

libpd_subscribe('eggs')

m = PdManager(1, 2, 44100, 1)
patch = libpd_open_patch('test.pd', '.')
print "$0: ", patch

libpd_float('spam', 42)
libpd_symbol('spam', "don't panic")
libpd_list('spam', 'test', 1, 'foo', 2)

inbuf = array.array('h', range(64)).tostring()

outbuf = m.process(inbuf)
print array.array('h', outbuf)

outbuf = m.process(inbuf)
print array.array('h', outbuf)

libpd_release()
