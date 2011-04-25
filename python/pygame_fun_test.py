from pylibpd import *
import array
import pygame
import numpy
from os import environ

BUFFERSIZE = 1024
BLOCKSIZE = 64

SCREENSIZE = (640, 480)

environ['SDL_VIDEO_CENTERED'] = '1'
pygame.init()
screen = pygame.display.set_mode(SCREENSIZE)

m = PdManager(1, 2, pygame.mixer.get_init()[0], 1)
patch = libpd_open_patch('funtest.pd', '.')
print "$0: ", patch

# this is basically a dummy since we are not actually going to read from the mic
inbuf = array.array('h', range(BLOCKSIZE))

# the pygame channel that we will use to queue up buffers coming from pd
ch = pygame.mixer.Channel(0)
# python writeable sound buffers
sounds = [pygame.mixer.Sound(numpy.zeros((BUFFERSIZE, 2), numpy.int16)) for s in range(2)]
samples = [pygame.sndarray.samples(s) for s in sounds]

rectangles = []
rectcolor = (255, 0, 0)
bg = (255, 255, 255)
rectsize = 200

def updatexy(event):
	libpd_float('x', float(event.pos[1]) / SCREENSIZE[1])
	libpd_float('y', float(event.pos[0]) / SCREENSIZE[0])
	libpd_bang('trigger')
	rectangles.append([event.pos, 0])

# we go into an infinite loop selecting alternate buffers and queueing them up
# to be played each time we run short of a buffer
selector = 0
quit = False
while not quit:
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
	
	for event in pygame.event.get():
		if event.type == pygame.QUIT or event.type == pygame.KEYDOWN and event.key == 27:
			quit = True
		
		if event.type == pygame.MOUSEBUTTONDOWN:
			updatexy(event)
	
	screen.fill(bg)
	delrects = []
	for r in rectangles:
		dr = pygame.Rect(r[0][0], r[0][1], r[1], r[1])
		dr.center = r[0]
		cv = 255 * (rectsize - r[1]) / rectsize
		pygame.draw.rect(screen, (255, 255 - cv, 255 - cv), dr, 2)
		r[1] += 1
		if r[1] >= rectsize:
			delrects.append(r)
	
	for r in delrects:
		rectangles.remove(r)
	
	pygame.display.flip()

libpd_release()
