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
.globl pixel_gain
.type  pixel_gain,@function

# mmx rgba pixel gain
# void asmtest(char *pixelarray, int32 nbpixels, int *rgba_gain)
# gains are 7.9 fixed point for rgba

pixel_gain:
	pushl %ebp
	movl %esp, %ebp
	push %esi
	push %edi

	movl 8(%ebp),  %esi	# pixel array offset
	movl 12(%ebp), %ecx	# nb of elements
	movl 16(%ebp), %edi	# int16[4] array of gains

	prefetch (%esi)

	emms
	sarl $2, %ecx		# process 4 pixels per loop iteration
	jz .exit
	movq (%edi), %mm7	# read gain array from memory
	jmp .loop_gain

	.align 16
	.loop_gain:	

	prefetch 128(%esi)	
	movq (%esi), %mm5	# load pixel 1-2  from memory
	movq 8(%esi), %mm6	# load pixel 3-4  from memory
	pxor %mm0, %mm0		# zero mm0 - mm3
	pxor %mm1, %mm1
	pxor %mm2, %mm2
	pxor %mm3, %mm3
	punpcklbw %mm5, %mm0	# unpack 1st pixel into 8.8 bit ints
	punpckhbw %mm5, %mm1	# unpack 2nd
	punpcklbw %mm6, %mm2	# unpack 3rd
	punpckhbw %mm6, %mm3	# unpack 4th
	psrlw $0x1, %mm0	# shift right to clear sign bit 9.7
	psrlw $0x1, %mm1
	psrlw $0x1, %mm2
	psrlw $0x1, %mm3
	
	pmulhw %mm7, %mm0	# multiply 1st pixel 9.7 * 7.9 -> 16.0
	pmulhw %mm7, %mm1	# multiply 2nd  
	pmulhw %mm7, %mm2	# multiply 3rd
	pmulhw %mm7, %mm3	# multiply 4th 

	packuswb %mm1, %mm0	# pack & saturate to 8bit vector
	movq %mm0, (%esi)	# store result in memory
	packuswb %mm3, %mm2	# pack & saturate to 8bit vector
	movq %mm2, 8(%esi)	# store result in memory

	addl $16, %esi		# increment source pointer
	decl %ecx
	jnz .loop_gain		# loop

	.exit:
	emms
	
	pop %edi
	pop %esi
	leave
	ret
	
