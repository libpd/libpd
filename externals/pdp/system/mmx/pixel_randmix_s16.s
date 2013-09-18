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
.globl pixel_randmix_s16
.type  pixel_randmix_s16,@function

# mmx rgba pixel gain
# void pixel_randmix_s16(int *left, int *right, int nb_4pixel_vectors, short int random_seed[4], short int threshold[4])

pixel_randmix_s16:
	pushl %ebp
	movl %esp, %ebp
	push %esi
	push %edi

	movl 20(%ebp), %edi	# int16[4] array of random seeds
	movq (%edi), %mm6

	movl 24(%ebp), %edi	# int16[4] array of thresholds
	movq (%edi), %mm7
	
	movl 8(%ebp),  %edi	# left array
	movl 12(%ebp), %esi	# right array
	movl 16(%ebp), %ecx	# pixel count

	pcmpeqw %mm3, %mm3
	psrlw $15, %mm3		# get bit mask 4 times 0x0001
	
	.align 16
	.loop_randmix:	

#	prefetch 128(%esi)	
	movq (%esi), %mm1	# load right 4 pixels from memory
	movq (%edi), %mm0	# load 4 left pixels from memory

	movq %mm6, %mm2		# get random vector
	pcmpgtw %mm7, %mm2	# compare random vector with threshold
	movq %mm2, %mm5
	
	pand %mm0, %mm2		# get left array's components
	pandn %mm1, %mm5	# get right array's components
	por %mm2, %mm5
	
	movq %mm5, (%edi)	# store pixels

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
	
	addl $8, %esi
	addl $8, %edi
	decl %ecx
	jnz .loop_randmix	# loop


	movl 20(%ebp), %edi	# int16[4] array of random seeds
	movq %mm6, (%edi)	# store random seeds

	emms
	
	pop %edi
	pop %esi
	leave
	ret
	
