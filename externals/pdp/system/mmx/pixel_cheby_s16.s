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
.globl pixel_cheby_s16_3plus
.type  pixel_cheby_s16_3plus,@function

# void pixel_cheby_s16(int *buf, int nb_8pixel_vectors, int order+1, short int *coefs)


# coefs are s2.13 fixed point (-4->4)
pixel_cheby_s16_3plus:
	pushl %ebp
	movl %esp, %ebp
	push %esi
	push %edi
	push %edx

	movl 8(%ebp),  %esi	# input array
	movl 12(%ebp), %ecx	# vector count
	movl 16(%ebp), %eax	# get order+1

	shll $3, %eax
	movl 20(%ebp), %edx
	addl %eax, %edx		# edx = coef endx address
	
#	jmp skip
	
	.align 16
	.loop_cheby:	

	movl 20(%ebp), %edi	# get coefs
	movq (%esi), %mm0	# load 4 pixels from memory (mm0 = x)
	pcmpeqw %mm2, %mm2
	movq %mm0, %mm1		# mm1 (T_n-1) <- x
	psrlw $1, %mm2		# mm2 (T_n-2) <- 1
	

	movq (%edi), %mm4	# mm4 (acc) == a0
	psraw $1, %mm4		# mm4 == a0/2
	movq %mm0, %mm5		# mm5 (intermediate)
	pmulhw 8(%edi), %mm5	# mm5 == (x * a1)/2
	paddsw %mm5, %mm4	# acc = c0 + c1 x
	addl $16, %edi

	.loop_cheby_inner:	
	movq %mm1, %mm3		# mm3 == T_n-1
	psraw $2, %mm2		# mm2 == T_n-2 / 4
	pmulhw %mm0, %mm3	# mm3 == (2 x T_n-1) / 4
	psubsw %mm2, %mm3	# mm3 == (2 x T_n-1 - T_n-2) / 4
	paddsw %mm3, %mm3
	paddsw %mm3, %mm3	# mm3 == T_n
	movq %mm1, %mm2		# mm2 == new T_n-1
	movq %mm3, %mm1		# mm3 == new T_n-2
	pmulhw (%edi), %mm3	# mm3 = a_n * T_n / 2
	paddsw %mm3, %mm4	# accumulate
	addl $8, %edi
	cmpl %edx, %edi
	jne .loop_cheby_inner
	
	paddsw %mm4, %mm4	# compensate for 0.125 factor
	paddsw %mm4, %mm4
	paddsw %mm4, %mm4
	movq %mm4, (%esi)	# store result in memory
	addl $8, %esi		# increment source/dest pointer
	decl %ecx
	jnz .loop_cheby		# loop

skip:	
	emms

	pop %edx
	pop %edi
	pop %esi
	leave
	ret
	
