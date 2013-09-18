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
	# intermediate function
	
	# input in register:	
	# %mm0:	left 4 pixels
	# %mm1:	middle 4 pixels
	# %mm2:	right 4 pixels
	
	# %mm5:	left 4 pixel masks
	# %mm6:	middle 4 pixel masks
	# %mm7:	right 4 pixel masks
	
	# return in register:	 
	# %mm0:	middle 4 pixels result

	
	.conv_hor_4_pixels:	
	.align 16
	
	# compute quadruplet

	# get left pixels
	psrlq $48, %mm0			# shift word 3 to byte 0
	movq %mm1, %mm4
	psllq $16, %mm4			# shift word 0,1,2 to 1,2,3
	por %mm4, %mm0			# combine
	pmulhw %mm5, %mm0
	psllw $1, %mm0

	
	# get middle pixels
	movq %mm1, %mm4
	pmulhw %mm6, %mm4
	psllw $1, %mm4
	paddsw %mm4, %mm0	


	# get right pixels
	movq %mm2, %mm3
	psllq $48, %mm3			# shift word 0 to word 3
	movq %mm1, %mm4
	psrlq $16, %mm4			# shift word 1,2,3 to 0,1,2
	por %mm4, %mm3			# combine
	pmulhw %mm7, %mm3
	psllw $1, %mm3
	paddsw %mm3, %mm0		# accumulate
	
	ret
	
.globl pixel_conv_hor_s16
.type  pixel_conv_hor_s16,@function


# pixel_conv_hor_s16(short int *pixel_array, int nb_4_pixel_vectors, short int border[4], short int mask[12])
# horizontal unsigned pixel conv (1/4 1/2 1/4) not tested
# NOT TESTED

	
pixel_conv_hor_s16: 

		
	pushl %ebp
	movl %esp, %ebp
	push %esi
	push %edi

	movl 8(%ebp),  %esi	# pixel array offset
	movl 12(%ebp), %ecx	# nb of 8 pixel vectors in a row (at least 2)

	movl 20(%ebp), %edi	# mask vector
	movq (%edi), %mm5
	movq 8(%edi), %mm6
	movq 16(%edi), %mm7
	
	movl 16(%ebp), %edi	# boundary pixel vector
	
	

	movq (%edi), %mm0	# init regs (left edge, so mm0 is zero)
	movq (%esi), %mm1
	movq 8(%esi), %mm2

	decl %ecx		# loop has 2 terminator stubs
	decl %ecx		# todo:	 handle if ecx < 3
	
	jmp .conv_line_loop


	.align 16
	.conv_line_loop:	
	call .conv_hor_4_pixels	# compute conv 
	movq %mm0, (%esi)	# store result
	movq %mm1, %mm0		# mm0 <- prev (%esi)
	movq %mm2, %mm1		# mm1 <- 8(%esi)
	movq 16(%esi), %mm2	# mm2 <- 16(%esi)
	
	addl $8, %esi		# increase pointer
	decl %ecx
	jnz .conv_line_loop

	call .conv_hor_4_pixels	# compute conv 
	movq %mm0, (%esi)	# store result
	movq %mm1, %mm0		# mm0 <- prev (%esi)
	movq %mm2, %mm1		# mm1 <- 8(%esi)
	movq (%edi), %mm2	# mm2 <- border

	call .conv_hor_4_pixels	# compute last vector
	movq %mm0, 8(%esi)	# store it
	
	emms
	
	pop %edi
	pop %esi
	leave
	ret



