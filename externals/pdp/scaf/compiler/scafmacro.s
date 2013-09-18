	#    Pure Data Packet - scaf assembler macros.
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


	# this file contains pure asm macros. it is to be included before assembly
	# after scaforth.pl has processed the .scaf file

	# *************************** JMP CALL RET **************************************
	# j c r
	
	.macro j address
	jmp \address
	.endm

	.macro c address
	call \address
	.endm

	.macro r
	ret
	.endm
		

	# *************************** CA CELL ACCESS MACROS *****************************
	# dropldTL - dropldBR

	# shift / load rectangle macros:

	# shift rectangle horizontal	
	# result is in reg1
	.macro shift reg1 reg2 count
	psllq $(16-\count), \reg1
	psrlq $(16+\count), \reg2
	psrlq $32, \reg1
	psllq $32, \reg2
	por \reg2, \reg1
	.endm

	.macro ldtop reg1 reg2
	movq (%edi), \reg1
	movq 8(%edi), \reg2
	.endm

	.macro ldcenter reg1 reg2
	movq 8(%edi), \reg1
	movq 16(%edi), \reg2
	.endm

	.macro ldbottom reg1 reg2
	movq 16(%edi), \reg1
	movq 24(%edi), \reg2
	.endm
	

	# dropld from top row

	# dropld the top left square
	.macro dropldTL
	ldtop %mm0, %mm1
	shift %mm0, %mm1, -1
	.endm

	# dropld the top mid square
	.macro dropldTM
	ldtop %mm0, %mm1
	shift %mm0, %mm1, 0
	.endm

	# dropld the top right square
	.macro dropldTR
	ldtop %mm0, %mm1
	shift %mm0, %mm1, 1
	.endm


	
	# dropld from center row

	# dropld the mid left square
	.macro dropldML
	ldcenter %mm0, %mm1
	shift %mm0, %mm1, -1
	.endm

	# dropld the mid mid square
	.macro dropldMM
	ldcenter %mm0, %mm1
	shift %mm0, %mm1, 0
	.endm

	# dropld the mid right square
	.macro dropldMR
	ldcenter %mm0, %mm1
	shift %mm0, %mm1, 1
	.endm


	

			
	# dropld from bottom row

	# dropld the bottom left square
	.macro dropldBL
	ldbottom %mm0, %mm1
	shift %mm0, %mm1, -1
	.endm

	# dropld the bottom mid square
	.macro dropldBM
	ldbottom %mm0, %mm1
	shift %mm0, %mm1, 0
	.endm

	# dropld the bottom right square
	.macro dropldBR
	ldbottom %mm0, %mm1
	shift %mm0, %mm1, 1
	.endm



	# *************************** CA STACK MANIP MACROS *****************************
	# these are the only asm macros that have a stack effect other than
	# just replacing the TOS
	#
	# dup drop dropdup swap nip dropover

	.macro dup
	lea -8(%esi), %esi
	movq %mm0, (%esi)	
	.endm

	.macro drop
	movq (%esi), %mm0
	lea 8(%esi), %esi
	.endm

	.macro dropdup
	movq (%esi), %mm0
	.endm

	.macro nipdup
	movq %mm0, (%esi)
	.endm

	.macro swap
	movq (%esi), %mm1
	movq %mm0, (%esi)
	movq %mm1, %mm0
	.endm

	.macro nip
	lea 8(%esi), %esi
	.endm

	.macro dropover
	movq 8(%esi), %mm0
	.endm


	# *************************** CA BOOLEAN LOGIC MACROS *****************************
	# overxor overand overor not
	
	.macro overxor
	pxor (%esi), %mm0
	.endm	

	.macro overand
	pand (%esi), %mm0
	.endm	

	.macro overor
	por (%esi), %mm0
	.endm	

	.macro not
	pxor %mm3, %mm0
	.endm
	


	# *************************** CONSTANTS  *****************************
	# dropzero dropone 

	.macro dropzero
	pxor %mm0, %mm0
	.endm

	.macro dropone
	pcmpeqw %mm0, %mm0
	.endm
	
	
	# *************************** 4 BIT REG ACCESS  ******************************
	# dupsta0 - dupsta4   droplda0 - droplda4
	# store bit in accumulator

	# bit store
	
	.macro dupsta0
	movq %mm0, %mm4
	.endm

	.macro dupsta1
	movq %mm0, %mm5
	.endm
	
	.macro dupsta2
	movq %mm0, %mm6
	.endm
	
	.macro dupsta3
	movq %mm0, %mm7
	.endm

	# load bit from accumulator

	.macro droplda0
	movq %mm4, %mm0
	.endm

	.macro droplda1
	movq %mm5, %mm0
	.endm
	
	.macro droplda2
	movq %mm6, %mm0
	.endm

	.macro droplda3
	movq %mm7, %mm0
	.endm


	# *************************** LOAD 4 BIT CONSTANT IN REG  ******************************
	# a0000 - a1111
	
	.macro ldbit0 value
	.ifeq \value
	movq %mm1, %mm4
	.else
	movq %mm3, %mm4
	.endif
	.endm

	.macro ldbit1 value
	.ifeq \value
	movq %mm1, %mm5
	.else
	movq %mm3, %mm5
	.endif
	.endm

	.macro ldbit2 value
	.ifeq \value
	movq %mm1, %mm6
	.else
	movq %mm3, %mm6
	.endif
	.endm

	.macro ldbit3 value
	.ifeq \value
	movq %mm1, %mm7
	.else
	movq %mm3, %mm7
	.endif
	.endm

	.macro ldbin b3 b2 b1 b0
	pxor %mm1, %mm1
	ldbit0 \b0
	ldbit1 \b1
	ldbit2 \b2
	ldbit3 \b3
	.endm
	
	.macro a0000
	ldbin 0 0 0 0
	.endm

	.macro a0001
	ldbin 0 0 0 1
	.endm

	.macro a0010
	ldbin 0 0 1 0
	.endm

	.macro a0011
	ldbin 0 0 1 1
	.endm

	.macro a0100
	ldbin 0 1 0 0
	.endm

	.macro a0101
	ldbin 0 1 0 1
	.endm

	.macro a0110
	ldbin 0 1 1 0
	.endm

	.macro a0111
	ldbin 0 1 1 1
	.endm

	.macro a1000
	ldbin 1 0 0 0
	.endm

	.macro a1001
	ldbin 1 0 0 1
	.endm

	.macro a1010
	ldbin 1 0 1 0
	.endm

	.macro a1011
	ldbin 1 0 1 1
	.endm

	.macro a1100
	ldbin 1 1 0 0
	.endm

	.macro a1101
	ldbin 1 1 0 1
	.endm

	.macro a1110
	ldbin 1 1 1 0
	.endm

	.macro a1111
	ldbin 1 1 1 1
	.endm

	
	
		
	# *************************** 4 BIT COUNTER  ******************************
	# adds TOS to bit of counter and returns carry in TOS
	#
	# adb0 - adb3


	.macro adb0
	movq %mm4, %mm2
	pxor %mm0, %mm4
	pand %mm2, %mm0
	.endm

	.macro adb1
	movq %mm5, %mm2
	pxor %mm0, %mm5
	pand %mm2, %mm0
	.endm

	.macro adb2
	movq %mm6, %mm2
	pxor %mm0, %mm6
	pand %mm2, %mm0
	.endm

	.macro adb3
	movq %mm7, %mm2
	pxor %mm0, %mm7
	pand %mm2, %mm0
	.endm


	# *************************** ACCUMULATOR TESTS ***************************
	# dropisnonzero4 - dropisnonzero1

	.macro dropisnonzero4
	movq %mm4, %mm0
	por %mm5, %mm0
	por %mm6, %mm0
	por %mm7, %mm0
	.endm
	
	.macro dropisnonzero3
	movq %mm4, %mm0
	por %mm5, %mm0
	por %mm6, %mm0
	.endm
	
	.macro dropisnonzero2
	movq %mm4, %mm0
	por %mm5, %mm0
	.endm
	
	.macro dropisnonzero1
	movq %mm4, %mm0
	.endm
	

	# *************************** REGISTER SHIFT OPERATIONS **********************
	# shift and leave shifted out byte on stack
	# rotate trough top of stack

	.macro dropshiftright
	movq %mm4, %mm0
	movq %mm5, %mm4	
	movq %mm6, %mm5	
	movq %mm7, %mm6
	pxor %mm7, %mm7
	.endm

	.macro dropshiftleft
	movq %mm7, %mm0
	movq %mm6, %mm7	
	movq %mm5, %mm6	
	movq %mm4, %mm5
	pxor %mm4, %mm4
	.endm

	.macro dropshiftrighta
	movq %mm4, %mm0
	movq %mm5, %mm4	
	movq %mm6, %mm5	
	movq %mm7, %mm6
	.endm

	.macro rotateright
	movq %mm4, %mm1
	movq %mm5, %mm4	
	movq %mm6, %mm5	
	movq %mm7, %mm6
	movq %mm1, %mm7
	.endm

	.macro rotateleft
	movq %mm7, %mm1
	movq %mm6, %mm7	
	movq %mm5, %mm6	
	movq %mm4, %mm5
	movq %mm1, %mm4
	.endm

	.macro rotaterightstack
	movq %mm0, %mm1
	movq %mm4, %mm0
	movq %mm5, %mm4	
	movq %mm6, %mm5	
	movq %mm7, %mm6
	movq %mm1, %mm7
	.endm

	.macro rotateleftstack
	movq %mm0, %mm1
	movq %mm7, %mm0
	movq %mm6, %mm7	
	movq %mm5, %mm6	
	movq %mm4, %mm5
	movq %mm1, %mm4
	.endm

	# *************************** OTHER REGISTER OPERATIONS **********************
	# anot :	 complement reg (can be used to implement subtraction)

	.macro anot
	pxor %mm3, %mm4
	pxor %mm3, %mm5
	pxor %mm3, %mm6
	pxor %mm3, %mm7
	.endm
