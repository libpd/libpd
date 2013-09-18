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
#TODO:	 fix out of bound acces in conv_ver and conv_hor
	
	# intermediate function
	
	# input in register:	
	# %mm0:	top 4 pixels
	# %mm1:	middle 4 pixels
	# %mm2:	bottom 4 pixels

	# %mm5:	top 4 pixel mask
	# %mm6:	middle 4 pixel mask
	# %mm7:	bottom 4 pixel mask
	
	# return in register:	 
	# %mm0:	middle 4 pixels result

	
	.conv_ver_4_pixels:	
	.align 16
	
	# compute quadruplet

	# get top pixel
	pmulhw %mm5, %mm0
	psllw $1, %mm0
	
	# get middle pixel
	movq %mm1, %mm4
	pmulhw %mm6, %mm4
	psllw $1, %mm4
	paddsw %mm4, %mm0

	# get bottom pixel
	movq %mm2, %mm3
	pmulhw %mm7, %mm3
	psllw $1, %mm3			# mm3 <- mm3/4
	paddsw %mm3, %mm0

	ret
	
.globl pixel_conv_ver_s16
.type  pixel_conv_ver_s16,@function


# pixel_conv_ver_s16(short int *pixel_array, int nb_4_pixel_vectors, int row_byte_size, short int border[4])
# horizontal unsigned pixel conv (1/4 1/2 1/4) not tested
# NOT TESTED

	
pixel_conv_ver_s16: 

		
	pushl %ebp
	movl %esp, %ebp
	push %esi
	push %edi

	movl 8(%ebp),  %esi		# pixel array offset
	movl 12(%ebp), %ecx		# nb of 4 pixel vectors in a row (at least 2)
	movl 16(%ebp), %edx		# rowsize in bytes

	movl 24(%ebp), %edi		# mask vector
	movq (%edi), %mm5
	movq 8(%edi), %mm6
	movq 16(%edi), %mm7
	
	movl 20(%ebp), %edi		# edge vector


	shll $1, %edx
	decl %ecx			# loop has a terminator stub
	decl %ecx			# loop has another terminator stub
	

	movq (%edi), %mm0		# init regs (left edge, so mm0 is zero)
	movq (%esi), %mm1
	movq (%esi,%edx,1), %mm2
	jmp .conv_line_loop


	.align 16
	.conv_line_loop:	
	call .conv_ver_4_pixels		# compute conv 
	movq %mm0, (%esi)		# store result
	movq %mm1, %mm0			# mm0 <- prev (%esi)
	movq %mm2, %mm1			# mm1 <- (%esi,%edx,1)
	movq (%esi,%edx,2), %mm2	# mm2 <- (%esi,%edx,2)
	
	addl %edx, %esi			# increase pointer
	decl %ecx
	jnz .conv_line_loop

	call .conv_ver_4_pixels		# compute conv 
	movq %mm0, (%esi)		# store result
	movq %mm1, %mm0			# mm0 <- prev (%esi)
	movq %mm2, %mm1			# mm1 <- (%esi,%edx,1)
	movq (%edi), %mm2		# clear invalid edge vector

	addl %edx, %esi			# increase pointer
	call .conv_ver_4_pixels		# compute last vector
	movq %mm0, (%esi)		# store it
	
	emms
	
	pop %edi
	pop %esi
	leave
	ret



