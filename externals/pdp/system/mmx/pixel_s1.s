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

	# this file contains ops for binary image processing
	# 8x8 bit tile encoded
	# low byte = bottom row
	# low bit = right column
	# %mm7 = scratch reg for all macros


	# ************ load mask *******************
	# compute bit masks for rows and columns
	# %mm7:	 scratch reg

	# load mask top
	.macro ldmt count reg
	pcmpeqb \reg, \reg
	psllq $(64-(\count<<3)), \reg
	.endm

	# load mask bottom
	.macro ldmb count reg
	pcmpeqb \reg, \reg
	psrlq $(64-(\count<<3)), \reg
	.endm

	# load mask top and bottom
	.macro ldmtb count regt regb
	ldmb \count, \regb
	ldmt \count, \regt
	.endm

	# load mask right
	.macro ldmr count reg
	pcmpeqb %mm7, %mm7
	psrlw $(16-\count), %mm7
	movq %mm7, \reg
	psllq $8, %mm7
	por %mm7, \reg
	.endm

	# load mask left	
	.macro ldml count reg
	pcmpeqb %mm7, %mm7
	psllw $(16-\count), %mm7
	movq %mm7, \reg
	psrlq $8, %mm7
	por %mm7, \reg
	.endm

	# load mask left and right
	.macro ldmlr count regl regr
	pcmpeqb %mm7, %mm7
	psllw $(16-\count), %mm7
	movq %mm7, \regl
	psrlq $8, %mm7
	por %mm7, \regl
	movq \regl, \regr
	psrlq $(8-\count), \regr
	.endm

	# ************* shift square **********
	# shifts a square in reg, fills with zeros

	# shift square top
	.macro sst count reg
	psllq $(\count<<3), \reg
	.endm

	# shift square bottom
	.macro ssb count reg
	psrlq $(\count<<3), \reg
	.endm

	# not tested
	# shift square left
	.macro ssl count reg
	movq \reg, %mm7
	pcmpeqb \reg, \reg
	psllw $(16-\count), \reg
	psrlw $8, \reg
	pandn %mm7, \reg
	psllw $(\count), \reg
	.endm

	# shift square right
	.macro ssr count reg
	movq \reg, %mm7
	pcmpeqb \reg, \reg
	psrlw $(16-\count), \reg
	psllw $8, \reg
	pandn %mm7, \reg
	psrlw $(\count), \reg
	.endm


	# ********** combine square *************
	# combines 2 squares

	# combine right
	.macro csr count regr reg
	ssl \count, \reg
	ssr (8-\count), \regr
	por \regr, \reg
	.endm

	# combine left
	.macro csl count regl reg
	ssr \count, \reg
	ssl (8-\count), \regl
	por \regl, \reg
	.endm

	# combine top
	.macro cst count regt reg
	ssb \count, \reg
	sst (8-\count), \regt
	por \regt, \reg
	.endm

	
	# combine bottom
	.macro csb count regb reg
	sst \count, \reg
	ssb (8-\count), \regb
	por \regb, \reg
	.endm


	# ********** load combine square *************
	# loads combined square using mask

	# load combined square left
	# mask should be count bits set right (i.e. 0x01)
	.macro lcsml count mask source sourcel dstreg
	movq \mask, \dstreg
	movq \mask, %mm7
	pandn \source, \dstreg
	pand \sourcel, %mm7
	psrlq $(\count), \dstreg
	psllq $(8-\count), %mm7
	por %mm7, \dstreg
	.endm
	
	
			
.globl pixel_test_s1
.type  pixel_test_s1,@function

# simple add
# void pixel_add_s16(void *dest, void *source, int nb_squares, int spacing)



	#
	

pixel_test_s1:
	pushl %ebp
	movl %esp, %ebp
	push %esi
	push %edi

	movl 8(%ebp),  %edi	# dest
	movl 12(%ebp), %esi	# source
	movl 16(%ebp), %ecx	# count
	movl 20(%ebp), %edx	# row distance

	ldmr 1, %mm6
	lcsml 1, %mm6, (%esi), 8(%esi), %mm0
	movq %mm0, (%edi)


#	movq (%esi), %mm0
#	movq 8(%esi), %mm1
#	csl 4, %mm1, %mm0
#	movq %mm0, (%edi)

	emms

	
	pop %edi
	pop %esi
	leave
	ret
	
