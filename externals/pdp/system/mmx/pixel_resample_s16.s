	

#interpolation data:
#* 4 vectors: neighbourhood for samples (TL, TR, BL, BR)
#* 2 vectors: fractional part (unsigned)
#* 2 vectors: addresses of pixel blocks

#coord conversion data:
#1 vector: 32bit splatted address	
#1 vector: 16bit splatted w-1
#1 vector: 16bit splatted h-1
#1 vector: 16bit splatted w (reuse w-1 with add?)
#1 dword:  32 bit line offset

#coord generation data:	several vectors for parameter update stuff..

#coordinate systems: 16 bit virtual coordinates (signed, center relative)
#* 2 vectors: virtual coordinates
#(evt tussenstap + conversie naar 16 bit virtual)


#step 1:	generate virtual coords

		
#step 2:	virtual coords -> block adresses + fractional adresses
#* mulhigh: real coords (x,y) (center relative)
#* add center -> unsigned (top left relative)
#* mullow: fractional part (x_frac, y_frac)
#* mulhigh, mullow, pack 32bit: y_offset
#* pack 32bit: x_offset
#* add, shift, add start address: real addresses
	

#step3:		data fetch using generated addresses: 
#		this step would be much simpler in 4x16bit rgba. life's a bitch..

#step4:		billinear interpolation

#stat5:		store



		# this can be simplified by doing 32 bit unaligned moves
		# and vector unpacking on the data

	

		# cooked image data structure
		# pixel environment temp storage
		TL1 = 0x00
		TL2 = 0x02
		TL3 = 0x04
		TL4 = 0x06
		TR1 = 0x08
		TR2 = 0x0A
		TR3 = 0x0C
		TR4 = 0x0E
		BL1 = 0x10
		BL2 = 0x12
		BL3 = 0x14
		BL4 = 0x16
		BR1 = 0x18
		BR2 = 0x1A
		BR3 = 0x1C
		BR4 = 0x1E
		# addresses of pixel blocks
		ADDRESS1  = 0x20
		ADDRESS2  = 0x24
		ADDRESS3  = 0x28
		ADDRESS4  = 0x2C

		# second env + address buffer (testing:	 not used)
		SECONDBUFFER = 0x30
	
		# 32bit splatted bitmap address
		V2PLANEADDRESS = 0x60
		# 16bit splatted image constants
		V4TWOWIDTHM1 = 0x68
		V4TWOHEIGHTM1 = 0x70
		V4LINEOFFSET = 0x78
		# data struct size
		RESAMPLEDATASIZE = 0x80
	
	

		# interpolation routine
		# input:	%mm0, %mm1 4 x 16bit unsigned top left relative virtual x and y coordinates
		#		%esi: temp & algo data structure

getpixelsbilin:	psrlw $1, %mm0			# convert to range 0->0x7fff [0,0.5[
		psrlw $1, %mm1
		movq %mm0, %mm2
		movq %mm1, %mm3
		movq V4TWOWIDTHM1(%esi), %mm4	# 2 * (width - 1)
		movq V4TWOHEIGHTM1(%esi), %mm5	# 2 * (height - 1)
		pmulhw %mm5, %mm3		# mm3 == y coord (topleft relative)
		pmulhw %mm4, %mm2		# mm2 == x coord (topleft relative)
		pmullw %mm5, %mm1		# mm1 == y frac (unsigned)
		pmullw %mm4, %mm0		# mm0 == x frac (unsigned)

		movq %mm3, %mm5			# copy y coord 
		pmullw V4LINEOFFSET(%esi), %mm3	# low part of line offset
		pmulhw V4LINEOFFSET(%esi), %mm5	# high part of line offset

		movq %mm2, %mm7			# copy x coord vector
		pxor %mm4, %mm4
		punpcklwd %mm4, %mm2		# low part in %mm2
		punpckhwd %mm4, %mm7		# hight part in %mm7
	
		movq %mm3, %mm6			# copy
		punpcklwd %mm5, %mm3		# unpack low part in %mm3
		punpckhwd %mm5, %mm6		# high part int %mm6

		paddd %mm2, %mm3
		paddd %mm7, %mm6
		pslld $1, %mm3			# convert to word adresses
		pslld $1, %mm6

		paddd V2PLANEADDRESS(%esi), %mm3	# add pixel plane address
		paddd V2PLANEADDRESS(%esi), %mm6

		movq %mm3, ADDRESS1(%esi)	# store adresses
		movq %mm6, ADDRESS3(%esi)

		pcmpeqw %mm2, %mm2		# all ones
		movq %mm0, %mm4			# copy x frac
		movq %mm1, %mm5			# copy y frac
		pxor %mm2, %mm4			# compute compliment (approx negative)
		pxor %mm2, %mm5

		psrlw $1, %mm0			# shift right (0.5 * (frac x)
		psrlw $1, %mm1			# shift right (0.5 * (frac y)
		psrlw $1, %mm4			# shift right (0.5 * (1 - frac x)
		psrlw $1, %mm5			# shift right (0.5 * (1 - frac y)

		movq %mm0, %mm2			# copy of frac x
		movq %mm4, %mm3			# copy of (1-frac x)
						# fetch data

		#jmp skipfetch			# seems the fetch is the real killer. try to optimize this
						# using 32 bit accesses & shifts

						# the src image data struct is padded to the cooked data struct
		movl RESAMPLEDATASIZE(%esi), %edi
		shll $1, %edi

		movl ADDRESS1(%esi), %ecx 
		movl ADDRESS2(%esi), %edx
	
		movw (%ecx), %ax
		movw (%edx), %bx
		movw %ax, TL1(%esi)
		movw %bx, TL2(%esi)
		movw 2(%ecx), %ax
		movw 2(%edx), %bx
		movw %ax, TR1(%esi)
		movw %bx, TR2(%esi)

		addl %edi, %ecx
		addl %edi, %edx

		movw (%ecx), %ax
		movw (%edx), %bx
		movw %ax, BL1(%esi)
		movw %bx, BL2(%esi)
		movw 2(%ecx), %ax
		movw 2(%edx), %bx
		movw %ax, BR1(%esi)
		movw %bx, BR2(%esi)

		
		movl ADDRESS3(%esi), %ecx 
		movl ADDRESS4(%esi), %edx


		movw (%ecx), %ax
		movw (%edx), %bx
		movw %ax, TL3(%esi)
		movw %bx, TL4(%esi)
		movw 2(%ecx), %ax
		movw 2(%edx), %bx
		movw %ax, TR3(%esi)
		movw %bx, TR4(%esi)
	
		addl %edi, %ecx
		addl %edi, %edx

		movw (%ecx), %ax
		movw (%edx), %bx
		movw %ax, BL3(%esi)
		movw %bx, BL4(%esi)
		movw 2(%ecx), %ax
		movw 2(%edx), %bx
		movw %ax, BR3(%esi)
		movw %bx, BR4(%esi)

	
skipfetch:	
		pmulhw TL1(%esi), %mm4		# bilin interpolation
		pmulhw TR1(%esi), %mm0
		pmulhw BL1(%esi), %mm3
		pmulhw BR1(%esi), %mm2


		paddw %mm4, %mm0
		paddw %mm3, %mm2

		pmulhw %mm5, %mm0
		pmulhw %mm1, %mm2

		paddw %mm2, %mm0
		psllw $2, %mm0			# compensate for gain reduction

		ret


		// linear mapping data struct
		ROWSTATEX = 0x0
		ROWSTATEY = 0x8
		COLSTATEX = 0x10
		COLSTATEY = 0x18
		ROWINCX = 0x20		
		ROWINCY = 0x28
		COLINCX = 0x30		
		COLINCY = 0x38

		// image data struct
		LINEOFFSET = 0x0
		IMAGEADDRESS = 0x4
		WIDTH = 0x8
		HEIGHT = 0xC
		IMAGEDATASIZE = 0x10
		


# pixel_resample_linmap_s16(void *x)		
.globl pixel_resample_linmap_s16
.type  pixel_resample_linmap_s16,@function

		SOURCEIMAGE = RESAMPLEDATASIZE
		DESTIMAGE = SOURCEIMAGE + IMAGEDATASIZE
		LINMAPDATA = DESTIMAGE + IMAGEDATASIZE
	
pixel_resample_linmap_s16:	
		pushl %ebp
		movl %esp, %ebp
		pushl %esi
		pushl %edi
		pushl %ebx


		movl 8(%ebp),  %esi			# get data struct
		movl DESTIMAGE+HEIGHT(%esi), %edx	# image height
		movl DESTIMAGE+IMAGEADDRESS(%esi), %edi # dest image address
		movl DESTIMAGE+WIDTH(%esi), %ecx	# image width
		shrl $2, %ecx				# vector count
		.align 16
	
linmap_looprow:
		movq LINMAPDATA+ROWSTATEX(%esi), %mm0	# get current coordinates
		movq LINMAPDATA+ROWSTATEY(%esi), %mm1

linmap_loopcol:		
		movq %mm0, %mm4				# copy
		movq %mm1, %mm5
		paddd LINMAPDATA+ROWINCX(%esi), %mm4	# increment
		paddd LINMAPDATA+ROWINCY(%esi), %mm5
		movq %mm4, %mm6				# copy
		movq %mm5, %mm7	
		paddd LINMAPDATA+ROWINCX(%esi), %mm6	# increment
		paddd LINMAPDATA+ROWINCY(%esi), %mm7
		movq %mm6, LINMAPDATA+ROWSTATEX(%esi)	# store next state
		movq %mm7, LINMAPDATA+ROWSTATEY(%esi) 

		psrad $16, %mm0				# round to 16 bit
		psrad $16, %mm1
		psrad $16, %mm4
		psrad $16, %mm5
		packssdw %mm4, %mm0			# pack new coordinates
		packssdw %mm5, %mm1
	
		push %ecx
		push %edx
		push %edi
	
		call getpixelsbilin			# do interpolation

		pop %edi
		pop %edx
		pop %ecx
		movq %mm0, (%edi)			# store 4 pixels
		addl $0x8, %edi				# point to next 4 pixels
		decl %ecx				# dec row counter
		jnz linmap_looprow

		movq LINMAPDATA+COLSTATEX(%esi), %mm0	# get column state vector
		movq LINMAPDATA+COLSTATEY(%esi), %mm1
		movl DESTIMAGE+WIDTH(%esi), %ecx	# image width
		shrl $2, %ecx				# vector count
		paddd LINMAPDATA+COLINCX(%esi), %mm0	# increment
		paddd LINMAPDATA+COLINCY(%esi), %mm1
		movq %mm0, LINMAPDATA+COLSTATEX(%esi)	# store
		movq %mm1, LINMAPDATA+COLSTATEY(%esi)
		decl %edx				# dec column counter
		jnz linmap_loopcol
		
		emms
		popl %ebx
		popl %edi
		popl %esi
		leave
		ret


