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


	# TODO MOVE TO DIRECT FORM II
	# y[k]  = b0 * x[k] + u1[k-1]
	# u1[k] = b1 * x[k] + u2[k-1] - a1 * y[k]
	# u2[k] = b2 * x[k]           - a2 * y[k]
	
	# input in register:	
	# %mm0-mm3:	input 4x4 pixels {x0 x1 x2 x3}
	# %esi:		coef memory  (a1, a2, b0, b1, b2)
	# %edi:		state memory (u1, u2)

	
	# return in register:	 
	# %mm0-mm4:	4x4 pixels result

	
	.biquad_4x4_pixels:	
	.align 16
	# prescale
	movq -8(%esi), %mm4
	pmulhw %mm4, %mm0
	pmulhw %mm4, %mm1
	pmulhw %mm4, %mm2
	pmulhw %mm4, %mm3
	psllw $1, %mm0
	psllw $1, %mm1
	psllw $1, %mm2
	psllw $1, %mm3

	
	# first vector
	movq 0(%edi), %mm4		# mm4 <- u[-1]
	movq 8(%edi), %mm5		# mm5 <- u[-2]
	movq %mm4, %mm6
	movq %mm5, %mm7

	pmulhw 0(%esi), %mm6		# multiply by a1
	pmulhw 8(%esi), %mm7		# multiply by a2

	paddsw %mm6, %mm0		# accumulate
	paddsw %mm7, %mm0		# accumulate
	paddsw %mm0, %mm0		# scale by 2 (since all fixed point muls are x*y/2)

	movq %mm0, %mm6			# mm6 <- u[0]
	movq %mm4, %mm7			# mm7 <- u[-1]
	pmulhw 16(%esi), %mm0		# multiply by b0
	pmulhw 24(%esi), %mm4		# multiply by b1
	pmulhw 32(%esi), %mm5		# multiply by b2

	paddsw %mm4, %mm0		# accumulate
	paddsw %mm5, %mm0		# accumulate

					# mm0 is result 0

	# second vector
	movq %mm6, %mm4			# mm4 <- u[0]
	movq %mm7, %mm5			# mm5 <- u[-1]

	pmulhw 0(%esi), %mm6		# multiply by a1
	pmulhw 8(%esi), %mm7		# multiply by a2

	paddsw %mm6, %mm1		# accumulate
	paddsw %mm7, %mm1		# accumulate
	paddsw %mm1, %mm1		# scale by 2

	
	movq %mm1, %mm6			# mm6 <- u[1]
	movq %mm4, %mm7			# mm7 <- u[0]
	pmulhw 16(%esi), %mm1		# multiply by b0
	pmulhw 24(%esi), %mm4		# multiply by b1
	pmulhw 32(%esi), %mm5		# multiply by b2

	paddsw %mm4, %mm1		# accumulate
	paddsw %mm5, %mm1		# accumulate

					# mm1 is result 1

	# third vector
	movq %mm6, %mm4			# mm4 <- u[1]
	movq %mm7, %mm5			# mm5 <- u[0]

	pmulhw 0(%esi), %mm6		# multiply by a1
	pmulhw 8(%esi), %mm7		# multiply by a2

	paddsw %mm6, %mm2		# accumulate
	paddsw %mm7, %mm2		# accumulate
	paddsw %mm2, %mm2		# scale by 2

	
	movq %mm2, %mm6			# mm6 <- u[2]
	movq %mm4, %mm7			# mm7 <- u[1]
	pmulhw 16(%esi), %mm2		# multiply by b0
	pmulhw 24(%esi), %mm4		# multiply by b1
	pmulhw 32(%esi), %mm5		# multiply by b2

	paddsw %mm4, %mm2		# accumulate
	paddsw %mm5, %mm2		# accumulate

					# mm2 is result 2

	# fourth vector
	movq %mm6, %mm4			# mm4 <- u[2]
	movq %mm7, %mm5			# mm5 <- u[1]

	pmulhw 0(%esi), %mm6		# multiply by a1
	pmulhw 8(%esi), %mm7		# multiply by a2

	paddsw %mm6, %mm3		# accumulate
	paddsw %mm7, %mm3		# accumulate
	paddsw %mm3, %mm3		# scale by 2

	
	movq %mm3, 0(%edi)		# store  u[3]
	movq %mm4, 8(%edi)		# store  u[2]
	pmulhw 16(%esi), %mm3		# multiply by b0
	pmulhw 24(%esi), %mm4		# multiply by b1
	pmulhw 32(%esi), %mm5		# multiply by b2

	paddsw %mm4, %mm3		# accumulate
	paddsw %mm5, %mm3		# accumulate

					# mm3 is result 3

	ret
	

	# in order to use the 4 line parallel biquad routine on horizontal
	# lines, we need to reorder (rotate or transpose) the matrix, since
	# images are scanline encoded, and we want to work in parallell
	# on 4 lines.
	#
	# since the 4 lines are independent, it doesnt matter in which order
	# the the vector elements are present. 
	#
	# this allows us to use the same routine for left->right and right->left
	# processing.
	#	
	# some comments on the non-abelean group of square isometries consisting of
	# (I) identity
	# (H) horizontal axis mirror 
	# (V) vertical axis mirror
	# (T) transpose (diagonal axis mirror)
	# (A) antitranspose (antidiagonal axis mirror)
	# (R1) 90deg anticlockwize rotation
	# (R2) 180deg rotation
	# (R3) 90deg clockwize rotation
	#
	#	
	# we basicly have two options: (R1,R3) or (T,A)
	# we opt for T and A because they are self inverting, which improves locality
	#
	# use antitranspose for right to left an transpose
	# for left to right (little endian)


	# antitranspose 4x4

	# input
	# %mm3 == {d0 d1 d2 d3}
	# %mm2 == {c0 c1 c2 c3}	
	# %mm1 == {b0 b1 b2 b3}	
	# %mm0 == {a0 a1 a2 a3}

	# output
	# %mm3 == {a3 b3 c3 d3}
	# %mm2 == {a2 b2 c2 d2}
	# %mm1 == {a1 b1 c1 d1}
	# %mm0 == {a0 b0 c0 d0}

	
	.antitranspose_4x4:	
	.align 16
	movq %mm3, %mm4
	punpcklwd %mm1, %mm4	# mm4 <- {b2 d2 b3 d3}
	movq %mm3, %mm5	
	punpckhwd %mm1, %mm5	# mm5 <- {b0 d0 b1 d1}
			
	movq %mm2, %mm6
	punpcklwd %mm0, %mm6	# mm6 <- {a2 c2 a3 c3}
	movq %mm2, %mm7	
	punpckhwd %mm0, %mm7	# mm7 <- {a0 c0 a1 c1}

	movq %mm4, %mm3
	punpcklwd %mm6, %mm3	# mm3 <- {a3 b3 c3 d3}
	movq %mm4, %mm2
	punpckhwd %mm6, %mm2	# mm2 <- {a2 b2 c2 d2}
		
	movq %mm5, %mm1
	punpcklwd %mm7, %mm1	# mm1 <- {a1 b1 c1 d1}
	movq %mm5, %mm0
	punpckhwd %mm7, %mm0	# mm0 <- {a0 b0 c0 d0}

	ret

	

	# transpose 4x4

	# input
	# %mm3 == {d3 d2 d1 d0}
	# %mm2 == {c3 c2 c1 c0}	
	# %mm1 == {b3 b2 b1 b0}	
	# %mm0 == {a3 a2 a1 a0}

	# output
	# %mm3 == {d3 c3 b3 a3}
	# %mm2 == {d2 c2 b2 a2}
	# %mm1 == {d1 c1 b1 a1}
	# %mm0 == {d0 c0 b0 a0}

	
	.transpose_4x4:	
	.align 16
	movq %mm0, %mm4
	punpcklwd %mm2, %mm4	# mm4 <- {c1 a1 c0 a0}
	movq %mm0, %mm5	
	punpckhwd %mm2, %mm5	# mm5 <- {c3 a3 c2 a2}
		
	movq %mm1, %mm6
	punpcklwd %mm3, %mm6	# mm6 <- {d1 b1 d0 b0}
	movq %mm1, %mm7	
	punpckhwd %mm3, %mm7	# mm7 <- {d3 b3 d2 b2}

	movq %mm4, %mm0
	punpcklwd %mm6, %mm0	# mm0 <- {d0 c0 b0 a0}
	movq %mm4, %mm1
	punpckhwd %mm6, %mm1	# mm1 <- {d1 c1 b1 a1}
		
	movq %mm5, %mm2
	punpcklwd %mm7, %mm2	# mm2 <- {d2 c2 b2 a2}
	movq %mm5, %mm3
	punpckhwd %mm7, %mm3	# mm3 <- {d3 c3 b3 a3}

	ret

	
.globl pixel_biquad_vertb_s16
.type  pixel_biquad_vertb_s16,@function


# pixel_biquad_vertbr_s16(char *pixel_array, int nb_rows, int linewidth, short int coef[20], short int state[8])

	
pixel_biquad_vertb_s16: 

		
	pushl %ebp
	movl %esp, %ebp
	push %ebx
	push %esi
	push %edi

	movl 8(%ebp),  %ebx	# pixel array offset
	movl 12(%ebp), %ecx	# nb of 4x4 pixblocks
	movl 16(%ebp), %edx	# line with

	movl 20(%ebp), %esi	# coefs
	movl 24(%ebp), %edi	# state

	shll $1, %edx		# short int addressing	
	movl %edx, %eax
	shll $1, %eax
	addl %edx, %eax		# eax = 3 * edx
	
	.align 16
	.biquad_vertb_line_loop:
	movq (%ebx), %mm0	
	movq (%ebx,%edx,1), %mm1	
	movq (%ebx,%edx,2), %mm2	
	movq (%ebx,%eax,1), %mm3
	call .biquad_4x4_pixels
	movq %mm0, (%ebx)	
	movq %mm1, (%ebx,%edx,1)	
	movq %mm2, (%ebx,%edx,2)	
	movq %mm3, (%ebx,%eax,1)
	addl %edx, %ebx
	addl %eax, %ebx
	decl %ecx
	jnz .biquad_vertb_line_loop
		
	emms
	
	pop %edi
	pop %esi
	pop %ebx
	leave
	ret

.globl pixel_biquad_horlr_s16
.type  pixel_biquad_horlr_s16,@function


# pixel_biquad_hor_s16(char *pixel_array, int nb_rows, int linewidth, short int coef[20], short int state[8])

	
pixel_biquad_horlr_s16: 

		
	pushl %ebp
	movl %esp, %ebp
	push %ebx
	push %esi
	push %edi

	movl 8(%ebp),  %ebx	# pixel array offset
	movl 12(%ebp), %ecx	# nb of 4x4 pixblocks
	movl 16(%ebp), %edx	# line with

	movl 20(%ebp), %esi	# coefs
	movl 24(%ebp), %edi	# state

	shll $1, %edx		# short int addressing
	movl %edx, %eax
	shll $1, %eax
	addl %edx, %eax		# eax = 3 * edx
	
	.align 16
	.biquad_horlr_line_loop:
	movq (%ebx), %mm0	
	movq (%ebx,%edx,1), %mm1	
	movq (%ebx,%edx,2), %mm2	
	movq (%ebx,%eax,1), %mm3
	call .transpose_4x4	
	call .biquad_4x4_pixels
	call .transpose_4x4	
	movq %mm0, (%ebx)	
	movq %mm1, (%ebx,%edx,1)	
	movq %mm2, (%ebx,%edx,2)	
	movq %mm3, (%ebx,%eax,1)
	addl $8, %ebx
	decl %ecx
	jnz .biquad_horlr_line_loop
		
	emms
	
	pop %edi
	pop %esi
	pop %ebx
	leave
	ret



