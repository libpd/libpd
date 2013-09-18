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
.globl pixel_rand_s16
.type  pixel_rand_s16,@function

# mmx rgba pixel gain
# void pixel_rand_s16(int *dst, nb_4pixel_vectors, short int random_seed[4])

pixel_rand_s16:
	pushl %ebp
	movl %esp, %ebp
	push %esi
	push %edi

	movl 16(%ebp), %esi	# int16[4] array of random seeds
	movl 8(%ebp),  %edi	# dst array
	movl 12(%ebp), %ecx	# pixel count

	movq (%esi), %mm6


	pcmpeqw %mm3, %mm3
	psrlw $15, %mm3		# get bit mask 4 times 0x0001
	
	.align 16
	.loop_rand:	

#	prefetch 128(%esi)	


	movq %mm6, %mm4		# get random vector
	psrlw $15, %mm4		# get first component
	movq %mm6, %mm5
	psrlw $14, %mm5		# get second component
	pxor %mm5, %mm4
	movq %mm6, %mm5
	psrlw $12, %mm5		# get third component
	pxor %mm5, %mm4
	movq %mm6, %mm5
	psrlw $3, %mm5		# get forth component
	pxor %mm5, %mm4

	psllw $1, %mm6		# shift left original random vector
	pand %mm3, %mm4		# isolate new bit
	por %mm4, %mm6		# combine into new random vector

	movq %mm6, (%edi)
	addl $8, %edi
	decl %ecx
	jnz .loop_rand	# loop


	movq %mm6, (%esi)	# store random seeds

	emms
	
	pop %edi
	pop %esi
	leave
	ret
	
