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

buf = array.array('f', range(64))
print "array size:", libpd_arraysize("array1")
print "array size:", libpd_arraysize("?????")  # doesn't exist
print libpd_read_array(buf, "array1", 0, 64)
print buf

inbuf = array.array('h', range(64))
outbuf = m.process(inbuf)
print outbuf

buf = array.array('f', map(lambda x : x / 64.0, range(64)))
print libpd_write_array("array1", 0, buf, 64)

outbuf = m.process(inbuf)
print outbuf

libpd_release()
