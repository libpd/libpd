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
.globl pixel_crot3d_s16
.type  pixel_crot3d_s16,@function


# 3 dimensional colour space rotation
# 3x3 matrix is column encoded, each coefficient is a 4x16 bit fixed point vector
	
# void pixel_crot3d_s16(int *buf, int nb_4pixel_vectors_per_plane, short int *matrix)

pixel_crot3d_s16:
	pushl %ebp
	movl %esp, %ebp
	push %esi
	push %edi

	
	movl 8(%ebp),  %esi	# input array
	movl 12(%ebp), %ecx	# pixel count
	movl 16(%ebp), %edi	# rotation matrix
	movl %ecx, %edx
	shll $3, %edx		# %edx = plane spacing

	
	.align 16
	.loop_crot3d:	

	movq (%esi), %mm0		# get 1st component
	movq (%esi,%edx,1), %mm6	# get 2nd component
	movq (%esi,%edx,2), %mm7	# get 3rd component

	movq %mm0, %mm1			# copy 1st component
	movq %mm0, %mm2

	pmulhw (%edi), %mm0		# mul first column
	pmulhw 8(%edi), %mm1
	pmulhw 16(%edi), %mm2

	movq %mm6, %mm5			# copy 2nd component
	movq %mm6, %mm3

	pmulhw 24(%edi), %mm6		# mul second column
	pmulhw 32(%edi), %mm5
	pmulhw 40(%edi), %mm3

	paddsw %mm6, %mm0		# accumulate
	paddsw %mm5, %mm1
	paddsw %mm3, %mm2

	movq %mm7, %mm4			# copy 3rd component
	movq %mm7, %mm6

	pmulhw 48(%edi), %mm4		# mul third column
	pmulhw 56(%edi), %mm6
	pmulhw 64(%edi), %mm7

	paddsw %mm4, %mm0		# accumulate
	paddsw %mm6, %mm1
	paddsw %mm7, %mm2

	paddsw %mm0, %mm0		# double (fixed point normalization)
	paddsw %mm1, %mm1
	paddsw %mm2, %mm2

	movq %mm0, (%esi)		# store
	movq %mm1, (%esi, %edx, 1)
	movq %mm2, (%esi, %edx, 2)

	addl $8, %esi			# increment source pointer
	decl %ecx
	jnz .loop_crot3d		# loop

	emms
	
	pop %edi
	pop %esi
	leave
	ret
	

.globl pixel_crot2d_s16
.type  pixel_crot2d_s16,@function
	
# 2 dimensional colour space rotation
# 2x2 matrix is column encoded, each coefficient is a 4x16 bit fixed point vector
	
# void pixel_crot2d_s16(int *buf, int nb_4pixel_vectors_per_plane, short int *matrix)

pixel_crot2d_s16:
	pushl %ebp
	movl %esp, %ebp
	push %esi
	push %edi

	
	movl 8(%ebp),  %esi	# input array
	movl 12(%ebp), %ecx	# pixel count
	movl 16(%ebp), %edi	# rotation matrix
	movl %ecx, %edx
	shll $3, %edx		# %edx = plane spacing

	
	.align 16
	.loop_crot2d:	

	movq (%esi), %mm0		# get 1st component
	movq (%esi,%edx,1), %mm2	# get 2nd component

	movq %mm0, %mm1			# copy 1st component
	movq %mm2, %mm3			# copy 2nd component

	pmulhw (%edi), %mm0		# mul first column
	pmulhw 8(%edi), %mm1

	pmulhw 16(%edi), %mm2		# mul second column
	pmulhw 24(%edi), %mm3

	paddsw %mm2, %mm0		# accumulate
	paddsw %mm3, %mm1

	paddsw %mm0, %mm0		# fixed point gain correction
	paddsw %mm1, %mm1

	movq %mm0, (%esi)		# store
	movq %mm1, (%esi, %edx, 1)

	addl $8, %esi			# increment source pointer
	decl %ecx
	jnz .loop_crot2d		# loop

	emms
	
	pop %edi
	pop %esi
	leave
	ret
	
