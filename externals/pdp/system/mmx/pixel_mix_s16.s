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
.globl pixel_mix_s16
.type  pixel_mix_s16,@function

# mmx rgba pixel gain
# void pixel_mix_s16(int *left, int *right, int nb_4pixel_vectors, 
#	short int gain_left[4], short int gain_right[4])

pixel_mix_s16:
	pushl %ebp
	movl %esp, %ebp
	push %esi
	push %edi

	movl 20(%ebp), %edi	# int16[4] array of gains
	movq (%edi), %mm6	# get left gain array

	movl 24(%ebp), %edi	# int16[4] array of gains
	movq (%edi), %mm7	# get right gain array
	
	movl 8(%ebp),  %edi	# left array
	movl 12(%ebp), %esi	# right array
	movl 16(%ebp), %ecx	# pixel count

	
	.align 16
	.loop_mix:	

#	prefetch 128(%esi)	
	movq (%esi), %mm1	# load right 4 pixels from memory
	pmulhw %mm7, %mm1	# apply right gain
	movq (%edi), %mm0	# load 4 left pixels from memory
	pmulhw %mm6, %mm0	# apply left gain
#	pslaw $1, %mm1		# shift left ((s).15 x (s).15 -> (s0).14))
#	pslaw $1, %mm0
	paddsw %mm0, %mm0	# no shift left arithmic, so use add instead
	paddsw %mm1, %mm1
	paddsw %mm1, %mm0	# mix
	movq %mm0, (%edi)
	addl $8, %esi
	addl $8, %edi
	decl %ecx
	jnz .loop_mix		# loop

	emms

	
	pop %edi
	pop %esi
	leave
	ret
	
