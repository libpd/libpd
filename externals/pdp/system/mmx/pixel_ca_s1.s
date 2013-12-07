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

	# this file contains assembler routines for 2D 1 bit cellular automata
	# processing. it is organized around a feeder kernel and a
	# stack based bit processor (virtual forth machine)
	#
	# the feeder kernel is responsable for loading/storing CA cells
	# from/to memory. data in memory is organized as a scanline
	# encoded toroidial bitplane (lsb = left). to simplify the kernel, the top
	# left corner of the rectangular grid of pixels will shift down
	# every processing step.
	#
	# the stack machine has the following architecture:
	# CA stack:	%esi, TOS: %mm0 (32x2 pixels. lsw = top row)
	# CA horizon:	%mm4-%mm7 (64x4 pixels. %mm4 = top row)
	#
	# the stack size / organization is not known to the stack machine. 
	# it can be thought of as operating on a 3x3 cell neightbourhood.
	# the only purpose of forth program is to determine the CA local update rule.
	#
	# the machine is supposed to be very minimal. no looping control.
	# no adressing modes. no conditional code (hey, this is an experiment!)
	# so recursion is not allowed (no way to stop it)
	# there are 9 words to load the cell neigbourhood on the stack.
	# the rest is just logic and stack manips.


	# this file contains pure asm macros. it is to be included before assembly
	# after scaforth.pl has processed the .scaf file
	

	# *************************** CA CELL ACCESS MACROS *****************************
	# fetchTL - fetchBR

	# shift / load rectangle macros:

	# shift rectangle horizontal	
	# result is in reg1
	.macro shift reg1 reg2 count
	psllq $(32+\count), \reg1
	psrlq $(32-\count), \reg2
	psrlq $32, \reg1
	psllq $32, \reg2
	por \reg2, \reg1
	.endm

	.macro ldtop reg1 reg2
	movq %mm4, \reg1
	movq %mm5, \reg2
	.endm

	.macro ldcenter reg1 reg2
	movq %mm5, \reg1
	movq %mm6, \reg2
	.endm

	.macro ldbottom reg1 reg2
	movq %mm6, \reg1
	movq %mm7, \reg2
	.endm
	

	# fetch from top row

	# fetch the top left square
	.macro fetchTL
	ldtop %mm0, %mm1
	shift %mm0, %mm1, -1
	.endm

	# fetch the top mid square
	.macro fetchTM
	ldtop %mm0, %mm1
	shift %mm0, %mm1, 0
	.endm

	# fetch the top right square
	.macro fetchTR
	ldtop %mm0, %mm1
	shift %mm0, %mm1, 1
	.endm


	
	# fetch from center row

	# fetch the mid left square
	.macro fetchML
	ldcenter %mm0, %mm1
	shift %mm0, %mm1, -1
	.endm

	# fetch the mid mid square
	.macro fetchMM
	ldcenter %mm0, %mm1
	shift %mm0, %mm1, 0
	.endm

	# fetch the mid right square
	.macro fetchMR
	ldcenter %mm0, %mm1
	shift %mm0, %mm1, 1
	.endm


	

			
	# fetch from bottom row

	# fetch the bottom left square
	.macro fetchBL
	ldbottom %mm0, %mm1
	shift %mm0, %mm1, -1
	.endm

	# fetch the bottom mid square
	.macro fetchBM
	ldbottom %mm0, %mm1
	shift %mm0, %mm1, 0
	.endm

	# fetch the bottom right square
	.macro fetchBR
	ldbottom %mm0, %mm1
	shift %mm0, %mm1, 1
	.endm



	# *************************** CA STACK MANIP MACROS *****************************
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
	# overxor 
	
	.macro overxor
	pxor (%esi), %mm0
	.endm	
	
	
	
	

