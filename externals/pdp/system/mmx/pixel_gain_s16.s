#    Pure Data Packet mmx routine.
#    Copyright (c) by Tom Schouten <tom@zwizwa.be>
# 
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
# 
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
# 
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
.globl pixel_gain_s16
.type  pixel_gain_s16,@function

# gain is integer, shift count is down	
# void pixel_gain_s16(int *buf, int nb_8pixel_vectors, short int gain[4], unsigned long long *shift)

pixel_gain_s16:
	pushl %ebp
	movl %esp, %ebp
	push %esi
	push %edi

	movl 20(%ebp), %edi
	movq (%edi), %mm6	# get shift vector

	movl 16(%ebp), %edi
	movq (%edi), %mm7	# get gain vector
	
	movl 8(%ebp),  %esi	# input array
	movl 12(%ebp), %ecx	# pixel count

	
	.align 16
	.loop_gain:	

	movq (%esi), %mm0	# load 4 pixels from memory
	movq %mm0, %mm1		
	pmulhw %mm7, %mm1	# apply gain (s15.0) fixed point, high word
	pmullw %mm7, %mm0	# low word

	movq %mm0, %mm2		# copy
	movq %mm1, %mm3

	punpcklwd %mm1, %mm0	# unpack lsw components
	punpckhwd %mm3, %mm2	# unpack msw components

	psrad %mm6, %mm0	# apply signed shift
	psrad %mm6, %mm2

	packssdw %mm2, %mm0	# pack result & saturate
	movq %mm0, (%esi)	# store result
	

	addl $8, %esi		# increment source pointer
	decl %ecx
	jnz .loop_gain		# loop

	emms
	
	pop %edi
	pop %esi
	leave
	ret
	
