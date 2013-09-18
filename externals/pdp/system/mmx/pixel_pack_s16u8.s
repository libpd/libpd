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
.globl pixel_pack_s16u8_y
.type  pixel_pack_s16u8_y,@function

# mmx rgba pixel gain
# void pixel_pack_s16u8_y(int *input, int *output, int nb_8pixel_vectors)

pixel_pack_s16u8_y:
	pushl %ebp
	movl %esp, %ebp
	push %esi
	push %edi

#	movl 20(%ebp), %edi	# int16[4] array of gains
#	movq (%edi), %mm7	# get gain array
#	psllw $1, %mm7		# adjust for shifted sign bit
	
	movl 8(%ebp),  %esi	# input array
	movl 12(%ebp), %edi	# output array
	movl 16(%ebp), %ecx	# pixel count

	pxor %mm6, %mm6
	
	.align 16
	.loop_pack_y:	

#	prefetch 128(%esi)	
	movq (%esi), %mm0	# load 4 pixels from memory
#	pmulhw %mm7, %mm0	# apply gain
	movq 8(%esi), %mm1	# load 4 pixels from memory
#	pmulhw %mm7, %mm1	# apply gain

#	movq %mm0, %mm2
#	pcmpgtw %mm6, %mm2	# mm2 > 0 ?  0xffff :	0
#	pand %mm2, %mm0 

#	movq %mm1, %mm3
#	pcmpgtw %mm6, %mm3	# mm3 > 0 ?  0xffff :	0
#	pand %mm3, %mm1 

#	psllw $1, %mm0		# shift out sign bit
#	psllw $1, %mm1		# shift out sign bit

	psraw $7, %mm0		# shift to lsb
	psraw $7, %mm1		# shift to lsb
	
	packuswb %mm1, %mm0	# pack & saturate to 8bit vector
	movq %mm0, (%edi)	# store result in memory

	addl $16, %esi		# increment source pointer
	addl $8, %edi		# increment dest pointer
	decl %ecx
	jnz .loop_pack_y	# loop

	emms
	
	pop %edi
	pop %esi
	leave
	ret
	
.globl pixel_pack_s16u8_uv
.type  pixel_pack_s16u8_uv,@function

pixel_pack_s16u8_uv:
	pushl %ebp
	movl %esp, %ebp
	push %esi
	push %edi

#	movl 20(%ebp), %edi	# int16[4] array of gains
#	movq (%edi), %mm7	# get gain array
	movl 8(%ebp),  %esi	# pixel array offset
	movl 12(%ebp), %edi	# nb of elements
	movl 16(%ebp), %ecx	# pixel count

	pcmpeqw %mm6, %mm6
	psllw $15, %mm6
	movq %mm6, %mm5
	psrlw $8, %mm5
	por %mm5, %mm6		# mm6 <- 8 times 0x80
	
	.align 16
	.loop_pack_uv:	

#	prefetch 128(%esi)	
	movq (%esi), %mm0	# load 4 pixels from memory
#	pmulhw %mm7, %mm0	# apply gain
	movq 8(%esi), %mm1	# load 4 pixels from memory
#	pmulhw %mm7, %mm1	# apply gain

	psraw $8, %mm0		# shift to msb
	psraw $8, %mm1
	
	packsswb %mm1, %mm0	# pack & saturate to 8bit vector
	pxor %mm6, %mm0		# flip sign bits
	movq %mm0, (%edi)	# store result in memory

	addl $16, %esi		# increment source pointer
	addl $8, %edi		# increment dest pointer
	decl %ecx
	jnz .loop_pack_uv	# loop

	emms
	
	pop %edi
	pop %esi
	leave
	ret
	
