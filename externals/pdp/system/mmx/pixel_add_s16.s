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
.globl pixel_add_s16
.type  pixel_add_s16,@function

# simple add
# void pixel_add_s16(int *left, int *right, int nb_4pixel_vectors)

pixel_add_s16:
	pushl %ebp
	movl %esp, %ebp
	push %esi
	push %edi

	movl 8(%ebp),  %edi	# left array
	movl 12(%ebp), %esi	# right array
	movl 16(%ebp), %ecx	# pixel count

	
	.align 16
	.loop_mix:	

#	prefetch 128(%esi)	
	movq (%esi), %mm1	# load right 4 pixels from memory
	movq (%edi), %mm0	# load 4 left pixels from memory
	paddsw %mm1, %mm0	# mix
	movq %mm0, (%edi)
	addl $8, %esi
	addl $8, %edi
	decl %ecx
	jnz .loop_mix		# loop

	emms

	
	pop %edi
	pop %esi
	leave
	ret
	
