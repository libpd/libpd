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

	
	# DIRECT FORM II BIQUAD
	#
	# y[k]  = b0 * x[k] + u1[k-1]
	# u1[k] = b1 * x[k] + u2[k-1] - a1 * y[k]
	# u2[k] = b2 * x[k]           - a2 * y[k]
	# MACRO:	df2 <reg>
	#
	# computes a direct form 2 biquad
	# does not use {mm0-mm3}\<inreg>
	#
	# input:	<reg>   == input
	#		%mm4    == state 1
	#		%mm5    == state 2
	#		(%esi)  == biquad coefs (-a1 -a2 b0 b1 b2) in s1.14
	# output:	<reg>   == output
	#		%mm4    == state 1
	#		%mm5    == state 2

	.macro df2 reg 
	movq \reg, %mm6			# mm6 == x[k]
	movq \reg, %mm7			# mm7 == x[k]
	pmulhw 16(%esi), %mm6		# mm6 == x[k] * b0
	pmulhw 24(%esi), %mm7		# mm7 == x[k] * b1
	paddw %mm4, %mm6		# mm6 == x[k] * b0 + u1[k-1] == y[k]
	paddw %mm5, %mm7		# mm7 == x[k] * b1 + u2[k-1]
	paddsw %mm6, %mm6		# compensate for mul = x*y/4 (coefs are s1.14 fixed point)
	paddsw %mm6, %mm6		# paddsw ensures saturation
	movq \reg, %mm5			# mm5 == x[k]
	movq %mm6, %mm4			# mm4 == y[k]
	movq %mm6, \reg			# reg == y[k]	--------------------
	pmulhw 0(%esi), %mm4		# mm4 == y[k] * (-a1)
	pmulhw 8(%esi), %mm6		# mm6 == y[k] * (-a2)
	pmulhw 32(%esi), %mm5		# mm5 == x[k] * b2
	paddw %mm7, %mm4		# mm4 == u1[k]	--------------------
	paddw %mm6, %mm5		# mm5 == u2[k]	--------------------
	.endm

		
	# input in register:	
	# %mm0-mm3:	input 4x4 pixels {x0 x1 x2 x3}
	# %esi:		coef memory  (-a1, -a2, b0, b1, b2) in s1.14
	# %edi:		state memory (u1, u2)
	
	# return in register:	 
	# %mm0-mm4:	4x4 pixels result



	
	.macro biquad_4x4_pixels	
	.align 16
	movq 0(%edi), %mm4		# get state
	movq 8(%edi), %mm5
	df2 %mm0			# compute 4 biquads
	df2 %mm1
	df2 %mm2
	df2 %mm3
	movq %mm4, 0(%edi)		# store state
	movq %mm5, 8(%edi)
	.endm

	

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

	
	.macro antitranspose_4x4
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
	
	.endm
	

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

	
	.macro transpose_4x4
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

	.endm
	
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
	biquad_4x4_pixels
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
.globl pixel_biquad_verbt_s16
.type  pixel_biquad_verbt_s16,@function


# pixel_biquad_vertbt_s16(char *pixel_array, int nb_rows, int linewidth, short int coef[20], short int state[8])

	
pixel_biquad_verbt_s16: 

		
	pushl %ebp
	movl %esp, %ebp
	push %ebx
	push %esi
	push %edi

	movl 8(%ebp),  %ebx	# pixel array offset
	movl 12(%ebp), %ecx	# nb of 4x4 pixblocks
	movl 16(%ebp), %eax	# line with

	shll $3, %eax		# 4 line byte spacing
	decl %ecx
	mul %ecx
	incl %ecx
	addl %eax, %ebx		# ebx points to last pixblock

	movl 16(%ebp), %edx	# line with

	movl 20(%ebp), %esi	# coefs
	movl 24(%ebp), %edi	# state

	shll $1, %edx		# short int addressing	
	movl %edx, %eax
	shll $1, %eax
	addl %edx, %eax		# eax = 3 * edx
	
	.align 16
	.biquad_verbt_line_loop:
	movq (%ebx), %mm3	
	movq (%ebx,%edx,1), %mm2	
	movq (%ebx,%edx,2), %mm1	
	movq (%ebx,%eax,1), %mm0
	biquad_4x4_pixels
	movq %mm3, (%ebx)	
	movq %mm2, (%ebx,%edx,1)	
	movq %mm1, (%ebx,%edx,2)	
	movq %mm0, (%ebx,%eax,1)
	subl %edx, %ebx
	subl %eax, %ebx
	decl %ecx
	jnz .biquad_verbt_line_loop
		
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
	transpose_4x4	
	biquad_4x4_pixels
	transpose_4x4	
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


.globl pixel_biquad_horrl_s16
.type  pixel_biquad_horrl_s16,@function
# pixel_biquad_horrl_s16(char *pixel_array, int nb_rows, int linewidth, short int coef[20], short int state[8])

pixel_biquad_horrl_s16: 

	pushl %ebp
	movl %esp, %ebp
	push %ebx
	push %esi
	push %edi

	movl 8(%ebp),  %ebx	# pixel array offset
	movl 12(%ebp), %ecx	# nb of 4x4 pixblocks
	movl 16(%ebp), %edx	# line with


	movl %ecx, %eax
	decl %eax
	shll $3, %eax
	addl %eax, %ebx		# ebx points to last pixblock

	
	movl 20(%ebp), %esi	# coefs
	movl 24(%ebp), %edi	# state

	shll $1, %edx		# short int addressing
	movl %edx, %eax
	shll $1, %eax
	addl %edx, %eax		# eax = 3 * edx
	
	.align 16
	.biquad_horrl_line_loop:
	movq (%ebx), %mm0	
	movq (%ebx,%edx,1), %mm1	
	movq (%ebx,%edx,2), %mm2	
	movq (%ebx,%eax,1), %mm3
	antitranspose_4x4	
	biquad_4x4_pixels
	antitranspose_4x4	
	movq %mm0, (%ebx)	
	movq %mm1, (%ebx,%edx,1)	
	movq %mm2, (%ebx,%edx,2)	
	movq %mm3, (%ebx,%eax,1)
	subl $8, %ebx
	decl %ecx
	jnz .biquad_horrl_line_loop
		
	emms
	
	pop %edi
	pop %esi
	pop %ebx
	leave
	ret


.globl pixel_biquad_time_s16
.type  pixel_biquad_time_s16,@function
# pixel_biquad_time_s16(short int *pixel_array, short int *s1, short int *s2, short int *coefs, int nb_4_pix_vectors)

pixel_biquad_time_s16: 

	pushl %ebp
	movl %esp, %ebp
	push %ebx
	push %esi
	push %edi

	movl 8(%ebp),  %ebx	# pixel array offset
	movl 12(%ebp), %edx	# state 1 array
	movl 16(%ebp), %edi	# state 2 array

	movl 20(%ebp), %esi	# coefs
	movl 24(%ebp), %ecx	# nb of 4 pixel vectors


	.align 16
	.biquad_time_loop:
	movq (%ebx), %mm0	# get input
	movq (%edx), %mm4	# get state 1
	movq (%edi), %mm5	# get state 2
	df2 %mm0		# compute direct form 2
	movq %mm0, (%ebx)	# write output
	movq %mm5, (%edi)	# write state 2
	movq %mm4, (%edx)	# write state 1
	addl $8, %ebx
	addl $8, %edi
	addl $8, %edx
	decl %ecx
	jnz .biquad_time_loop
		
	emms
	
	pop %edi
	pop %esi
	pop %ebx
	leave
	ret


