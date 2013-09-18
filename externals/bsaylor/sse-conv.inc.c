// Ben Saylor 2005-08-10
// attempt at an SSE version of the convolution loop.
// Results sound weird, and CPU usage is about the same. :(

		cursumbuf_fd	= (v4sf *) x->sumbufs[(x->sumbuf->index + p) % x->nsumbufs].fd;
		input_fd	= (v4sf *) x->input_fd;
		irpart_fd	= (v4sf *) x->irpart_fd[p];
		nvecs = x->paddedsize/4;

		for (v1=0, v2=1; v2 < nvecs; v1+=2, v2+=2) {
			// v1 is the index of the first 4-float vector (v4sf) to process (v2 is the second)
			// input_fd, etc. are v4sf pointers.
			// pull in 4 bins (8 floats) (2 v4sfs) at a time.
			
			// (a + bi) * (c + di) = (ac - bd) + (ad + bc)i

			asm volatile (	
					// load inputs
					"movaps %[in1], %%xmm0\n\t"
					"movaps %[in2], %%xmm6\n\t"
					"movaps %[ir1], %%xmm2\n\t"
					"movaps %[ir2], %%xmm7\n\t"
					"movaps %%xmm0, %%xmm1\n\t"
					"movaps %%xmm2, %%xmm3\n\t"
					// xmm0 = xmm1 = [a1 b1 a2 b2]
					// xmm6 =        [a3 b3 a4 b4]
					// xmm2 = xmm3 = [c1 d1 c2 d2]
					// xmm7 =        [c3 d3 c4 d4]

					// de-interleave real and imaginary parts
					"shufps $0x88, %%xmm6, %%xmm0\n\t"	// xmm0 = [a1 a2 a3 a4]
					"shufps $0xDD, %%xmm6, %%xmm1\n\t"	// xmm1 = [b1 b2 b3 b4]
					"shufps $0x88, %%xmm7, %%xmm2\n\t"	// xmm2 = [c1 c2 c3 c4]
					"shufps $0xDD, %%xmm7, %%xmm3\n\t"	// xmm3 = [d1 d2 d3 d4]

					// load output (early, maybe it will help)
					"movaps %[out1], %%xmm6\n\t"
					"movaps %[out2], %%xmm7\n\t"

					// compute the real part of the complex product
					// (work on copies of xmm0 and xmm1, because we need to keep them)
					"movaps %%xmm0, %%xmm4\n\t"		// xmm4 = [a1 a2 a3 a4]
					"mulps %%xmm2, %%xmm4\n\t"		// xmm4 = [a1c1 a2c2 a3c3 a4c4]
					"movaps %%xmm1, %%xmm5\n\t"		// xmm5 = [b1 b2 b3 b4]
					"mulps %%xmm3, %%xmm5\n\t"		// xmm5 = [b1d1 b2d2 b3d3 b4d4]
					"subps %%xmm5, %%xmm4\n\t"		// xmm4 = (ac - bd) [r1 r2 r3 r4]

					// compute the imaginary part of the complex product
					"mulps %%xmm3, %%xmm0\n\t"		// xmm0 = [a1d1 a2d2 a3d3 a4d4]
					"mulps %%xmm2, %%xmm1\n\t"		// xmm1 = [b1c1 b2c2 b3c3 b4c4]
					"addps %%xmm1, %%xmm0\n\t"		// xmm0 = (ad + bc) [i1 i2 i3 i4]

					// re-interleave
					"movaps %%xmm4, %%xmm5\n\t"		// xmm5 = [r1 r2 r3 r4]
					"unpcklps %%xmm0, %%xmm4\n\t"		// xmm4 = [r1 i1 r2 i2]
					"unpckhps %%xmm5, %%xmm0\n\t"		// xmm0 = [r3 i3 r4 i4]

					// add into sumbuf
					"addps %%xmm4, %%xmm6\n\t"
					"addps %%xmm0, %%xmm7\n\t"
					"movaps %%xmm6, %[out1]\n\t"
					"movaps %%xmm7, %[out2]"

					// output/input operands
					: [out1] "+m" (cursumbuf_fd[v1]),
					  [out2] "+m" (cursumbuf_fd[v2])

					// input operands
					: [in1] "m" (input_fd[v1]),
					  [in2] "m" (input_fd[v2]),
					  [ir1] "m" (irpart_fd[v1]),
					  [ir2] "m" (irpart_fd[v2])

					// clobbered registers
					: "%xmm0", "%xmm1", "%xmm2", "%xmm3", "%xmm4", "%xmm5", "%xmm6", "%xmm7"
			    );
		}
