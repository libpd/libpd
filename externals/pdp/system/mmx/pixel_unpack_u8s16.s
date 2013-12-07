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
.globl pixel_unpack_u8s16_y
.type  pixel_unpack_u8s16_y,@function

# mmx rgba pixel gain
# void pixel_unpack_u8s16_y(char *input, char *output, int32 nb_pixels_div8)

pixel_unpack_u8s16_y:
	pushl %ebp
	movl %esp, %ebp
	push %esi
	push %edi

#	movl 20(%ebp), %edi	# int16[4] array of gains
#	movq (%edi), %mm7	# get gain array
	
	movl 8(%ebp),  %esi	# input uint8 pixel array
	movl 12(%ebp), %edi	# output sint16 pixel array
	movl 16(%ebp), %ecx	# nb of elements div 8


	.align 16
	.loop_unpack_y:	

	movq (%esi), %mm5	# load 8 pixels from memory
	pxor %mm0, %mm0		# zero mm0 - mm3
	pxor %mm1, %mm1
	punpcklbw %mm5, %mm0	# unpack 1st 4 pixels
	punpckhbw %mm5, %mm1	# unpack 2nd 4 pixles
	psrlw $0x1, %mm0	# shift right to clear sign bit 9.7
	psrlw $0x1, %mm1
#	pmulhw %mm7, %mm0	# apply gain
#	pmulhw %mm7, %mm1
#	paddsw %mm0, %mm0	# correct factor 2
#	paddsw %mm1, %mm1
	movq %mm0, (%edi)	# store
	movq %mm1, 8(%edi)
	
	addl $8, %esi		# increment source pointer
	addl $16, %edi		# increment dest pointer
	decl %ecx
	jnz .loop_unpack_y	# loop

	emms
	
	pop %edi
	pop %esi
	leave
	ret
	
.globl pixel_unpack_u8s16_uv
.type  pixel_unpack_u8s16_uv,@function
pixel_unpack_u8s16_uv:
	pushl %ebp
	movl %esp, %ebp
	push %esi
	push %edi

#	movl 20(%ebp), %edi	# int16[4] array of gains
#	movq (%edi), %mm7	# get gain array

	movl 8(%ebp),  %esi	# input uint8 pixel array
	movl 12(%ebp), %edi	# output sint16 pixel array
	movl 16(%ebp), %ecx	# nb of elements div 8

	pcmpeqw %mm6, %mm6
	psllw $15, %mm6
	
	.align 16
	.loop_unpack_uv:	

	movq (%esi), %mm5	# load 8 pixels from memory
	pxor %mm0, %mm0		# zero mm0 - mm3
	pxor %mm1, %mm1
	punpcklbw %mm5, %mm0	# unpack 1st 4 pixels
	punpckhbw %mm5, %mm1	# unpack 2nd 4 pixles
	pxor %mm6, %mm0		# flip sign bit (Cr and Cb are ofset by 128)
	pxor %mm6, %mm1
#	pmulhw %mm7, %mm0	# apply gain
#	pmulhw %mm7, %mm1
#	paddsw %mm0, %mm0	# correct factor 2
#	paddsw %mm1, %mm1
	movq %mm0, (%edi)	# store
	movq %mm1, 8(%edi)
	
	addl $8, %esi		# increment source pointer
	addl $16, %edi		# increment dest pointer
	decl %ecx
	jnz .loop_unpack_uv	# loop

	emms
	
	pop %edi
	pop %esi
	leave
	ret
	
