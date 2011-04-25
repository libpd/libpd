from pylibpd import *
import array
import pygame
import numpy

BUFFERSIZE = 4096
SAMPLERATE = 44100
BLOCKSIZE = 64

pygame.mixer.init(frequency=SAMPLERATE)

m = PdManager(1, 2, SAMPLERATE, 1)
patch = libpd_open_patch('bloopy.pd', '.')
print "$0: ", patch

# this is basically a dummy since we are not actually going to read from the mic
inbuf = array.array('h', range(BLOCKSIZE))

# the pygame channel that we will use to queue up buffers coming from pd
ch = pygame.mixer.Channel(0)
# python writeable sound buffers
sounds = [pygame.mixer.Sound(numpy.zeros((BUFFERSIZE, 2), numpy.int16)) for s in range(2)]
samples = [pygame.sndarray.samples(s) for s in sounds]

# we go into an infinite loop selecting alternate buffers and queueing them up
# to be played each time we run short of a buffer
selector = 0
while(1):
	# we have run out of things to play, so queue up another buffer of data from Pd
	if not ch.get_queue():
		# make sure we fill the whole buffer
		for x in range(BUFFERSIZE):
			# let's grab a new block from Pd each time we're out of BLOCKSIZE data
			if x % BLOCKSIZE == 0:
				outbuf = m.process(inbuf)
			# de-interlace the data coming from libpd
			samples[selector][x][0] = outbuf[(x % BLOCKSIZE) * 2]
			samples[selector][x][1] = outbuf[(x % BLOCKSIZE) * 2 + 1]
		# queue up the buffer we just filled to be played by pygame
		ch.queue(sounds[selector])
		# next time we'll do the other buffer
		selector = int(not selector)

libpd_release()
