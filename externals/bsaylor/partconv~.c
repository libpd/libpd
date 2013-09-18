/* Copyright 2003-2005 Benjamin R. Saylor <bensaylor@fastmail.fm>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// Thu Feb 17 22:29:27 CST 2005 - Changed outbuf loops to use comparison instead of modulo (suggested by c. clepper) - faster
// Sat Jul 30 19:59:08 AKDT 2005 - Completed modification for re-blocking and dividing up the work between calls.
// 					This has eliminated dropouts due to lots of work on large block boundaries.
// Fri Aug  5 10:46:33 AKDT 2005 - accumulate in the frequency domain, so only 1 IFFT is needed per input block,
// 					rather than <nparts> IFFTs.  Big performance boost.
// Fri Aug  5 20:27:05 AKDT 2005 - should work properly with arbitrary (2^n) blocksize <= partsize
// Fri Aug 12 00:32:29 AKDT 2005 - added altivec code by Chris Clepper

// TODO
// SSE version
// multichannel (multiple IRs)?  probably wouldn't gain much from this
// divide work more evenly? (0, 15, 7, 23, 3, 11, 19, 27, ...)
// someday, an SSE3 version (supposed to make complex math fast)

#include <math.h>
#include <string.h>
#include <fftw3.h>
#include "m_pd.h"

#ifdef __VEC__
#include <altivec.h>
#endif

#define MAXPARTS 256	// max number of partitions

#ifdef USE_SSE
typedef float v4sf __attribute__ ((vector_size (16)));
#endif

static t_class *partconv_class;

struct sumbuffer {
	int index;
	fftwf_complex *fd;
	float *td;
	fftwf_plan plan;
	int readpos;
	struct sumbuffer *next, *prev;
};

typedef struct _partconv {
	t_object x_obj;
	t_symbol *arrayname;
	int partsize;
	int fftsize;
	float scale;
	int paddedsize;
	int nbins;
	int nparts;
	int ir_prepared;
	int pd_blocksize;

	// partitions of impulse response
	fftwf_plan irpart_plan;
	float *irpart_td[MAXPARTS];
	fftwf_complex *irpart_fd[MAXPARTS];

	// input
	fftwf_plan input_plan;
	float *inbuf;
	int inbufpos;
	float *input_td;
	fftwf_complex *input_fd;

	// circular array/list of buffers for accumulating results of convolution
	struct sumbuffer sumbufs[MAXPARTS+2];
	int nsumbufs;  // number of sumbufs
	struct sumbuffer *sumbuf;  // the current sumbuf corresponding to the first partition of the IR

	// dividing up the work between calls to perform()
	int parts_per_call[MAXPARTS];	// parts_per_call[c] is the number of partitions to convolve during perform() call c
	int curcall;			// current call, counted from the beginning of the current cycle (input buffer full)
	int curpart;			// current partition to convolve
} t_partconv;

// Determine how to divide the work as evenly as possible between calls to perform().
static void partconv_deal_work(t_partconv *x)
{
	int calls_per_cycle;
	int parts_to_distribute;
	int i;

	// Like dealing cards.
	// One cycle is defined as the time it takes to fill the input buffer (whose size is the user-given partition size)
	calls_per_cycle = x->partsize / x->pd_blocksize;
	for (i = 0; i < calls_per_cycle; i++) {
		x->parts_per_call[i] = 0;
	}
	i = 0;
	parts_to_distribute = x->nparts;
	while (parts_to_distribute) {
		x->parts_per_call[i]++;
		parts_to_distribute--;
		i = (i + 1) % calls_per_cycle;
	}
	/*
	for (i = 0; i < calls_per_cycle; i++) {
		printf("parts_per_call[%d] = %d\n", i, x->parts_per_call[i]);
	}
	*/
}

#ifdef __VEC__
#include "altivec-perform.inc.c"
#else

static t_int *partconv_perform(t_int *w)
{
	t_partconv *x = (t_partconv *)(w[1]);
	t_float *in = (t_float *)(w[2]);
	t_float *out = (t_float *)(w[3]);
	int n = (int)(w[4]);
	int i;
	int j;
	int k;	// bin
	int p;	// partition
	int endpart;

#ifdef USE_SSE
	int v1;
	int v2;
	int nvecs;
	v4sf *cursumbuf_fd;
	v4sf *input_fd;
	v4sf *irpart_fd;
#else
	fftwf_complex *cursumbuf_fd;
	fftwf_complex *input_fd;
	fftwf_complex *irpart_fd;
#endif

	float *sumbuf1ptr;
	float *sumbuf2ptr;

	memcpy(&(x->inbuf[x->inbufpos]), in, n*sizeof(float));  // gather a block of input into input buffer
	x->inbufpos += n;
	if (x->inbufpos >= x->partsize) {
		// input buffer is full, so we begin a new cycle
		
		if (x->pd_blocksize != n) {
			// the patch's blocksize has change since we last dealt the work
			x->pd_blocksize = n;
			partconv_deal_work(x);
		}
		
		x->inbufpos = 0;
		x->curcall = 0;
		x->curpart = 0;
		memcpy(x->input_td, x->inbuf, x->partsize * sizeof(float));  // copy 'gathering' input buffer into 'transform' buffer
		memset(&(x->input_td[x->partsize]), 0, (x->paddedsize - x->partsize) * sizeof(float));  // pad

		fftwf_execute(x->input_plan);  // transform the input

		// everything has been read out of prev sumbuf, so clear it
		memset(x->sumbuf->prev->td, 0,  x->paddedsize * sizeof(float));

		// advance sumbuf pointers
		x->sumbuf = x->sumbuf->next;
		x->sumbuf->readpos = 0;
		x->sumbuf->prev->readpos = x->partsize;
	}

	// convolve this call's portion of partitions
	endpart = x->curpart + x->parts_per_call[x->curcall];
	if (endpart > x->nparts)  // FIXME does this ever happen?
		endpart = x->nparts;
	for (p = x->curpart; p < endpart; p++) {
		// multiply the input block by the partition, accumulating the result in the appropriate sumbuf
#ifdef USE_SSE
#include "sse-conv.inc.c"
#else
		cursumbuf_fd = x->sumbufs[(x->sumbuf->index + p) % x->nsumbufs].fd;
		input_fd = x->input_fd;
		irpart_fd = x->irpart_fd[p];

		for (k = 0; k < x->nbins; k++) {

			cursumbuf_fd[k][0]
				+=
				(  input_fd[k][0] * irpart_fd[k][0]
				 - input_fd[k][1] * irpart_fd[k][1]);

			cursumbuf_fd[k][1]
				+=
				(  input_fd[k][0] * irpart_fd[k][1]
				 + input_fd[k][1] * irpart_fd[k][0]);
		}
#endif
	}
	x->curpart = p;

	// The convolution of the fresh block of input with the first partition of the IR
	// is the last thing that gets summed into the current sumbuf before it gets IFFTed and starts being output.
	// This happens during the first call of every cycle.
	if (x->curcall == 0) {
		// current sumbuf has been filled, so transform it
		// Output loop will begin to read it and sum it with the last one
		fftwf_execute(x->sumbuf->plan);
	}

	// we're summing and outputting the first half of the most recently IFFTed sumbuf
	// and the second half of the previous one
	sumbuf1ptr = &(x->sumbuf->td[x->sumbuf->readpos]);
	sumbuf2ptr = &(x->sumbuf->prev->td[x->sumbuf->prev->readpos]);
	for (i = 0; i < n; i++) {
		out[i] = (sumbuf1ptr[i] + sumbuf2ptr[i]) * x->scale;
	}
	x->sumbuf->readpos += n;
	x->sumbuf->prev->readpos += n;

	x->curcall++;

	return (w+5);
}

#endif // __VEC__

static void partconv_free(t_partconv *x)
{
	int i;

	fftwf_free(x->inbuf);
	for (i = 0; i < x->nparts; i++)
		fftwf_free(x->irpart_td[i]);
	fftwf_free(x->input_td);
	fftwf_destroy_plan(x->input_plan);
	for (i  = 0; i < x->nsumbufs; i++) {
		fftwf_free(x->sumbufs[i].fd);
		fftwf_destroy_plan(x->sumbufs[i].plan);
	}
}

static void partconv_set(t_partconv *x, t_symbol *s)
{
	int i;
	int j;
	t_garray *arrayobj;
	t_word *array;
	int arraysize;
	int arraypos;

	// get the array from pd
	x->arrayname = s;
	if ( ! (arrayobj = (t_garray *)pd_findbyclass(x->arrayname, garray_class))) {
		if (*x->arrayname->s_name) {
			pd_error(x, "partconv~: %s: no such array", x->arrayname->s_name);
			return;
		}
	} else if ( ! garray_getfloatwords(arrayobj, &arraysize, &array)) {
		pd_error(x, "%s: bad template", x->arrayname->s_name);
		return;
	}

	// if the IR has already been prepared, free everything first
	if (x->ir_prepared == 1) {
		partconv_free(x);
	}

	// caculate number of partitions
	x->nparts = arraysize / x->partsize;
	if (arraysize % x->partsize != 0)
		x->nparts++;
	if (x->nparts > MAXPARTS)
		x->nparts = MAXPARTS;

	// allocate, fill, pad, and transform each IR partition
	for (arraypos = 0, i = 0; i < x->nparts; i++) {
		x->irpart_td[i] = fftwf_malloc(sizeof(float) * x->paddedsize);
		x->irpart_fd[i] = (fftwf_complex *) x->irpart_td[i];
		x->irpart_plan = fftwf_plan_dft_r2c_1d(x->fftsize, x->irpart_td[i], x->irpart_fd[i], FFTW_MEASURE);
		for (j = 0; j < x->partsize && arraypos < arraysize; j++, arraypos++) {
			x->irpart_td[i][j] = array[arraypos].w_float;
		}
		for ( ; j < x->paddedsize; j++) {
			x->irpart_td[i][j] = 0;
		}
		fftwf_execute(x->irpart_plan);
		fftwf_destroy_plan(x->irpart_plan);
		// now, x->irpart[i] contains the DFT of the ith partition of the impulse response.
	}

	x->inbuf = fftwf_malloc(sizeof(float) * x->partsize);
	x->inbufpos = 0;
	
	// allocate buffer for DFT of padded input
	x->input_td = fftwf_malloc(sizeof(float) * x->paddedsize);	// float array into which input block is copied and padded
	x->input_fd = (fftwf_complex *) x->input_td;			// fftwf_complex pointer to the same array to facilitate in-place fft
	x->input_plan =  fftwf_plan_dft_r2c_1d(x->fftsize, x->input_td, x->input_fd, FFTW_MEASURE);

	// set up circular list/array of buffers for accumulating results of convolution
	x->nsumbufs = x->nparts + 2;
	x->sumbuf = &(x->sumbufs[0]);
	for (i  = 0; i < x->nsumbufs; i++) {
		x->sumbufs[i].index = i;
		x->sumbufs[i].fd = fftwf_malloc(sizeof(float) * x->paddedsize);
		memset(x->sumbufs[i].fd, 0, sizeof(float) * x->paddedsize);
		x->sumbufs[i].td = (float *) x->sumbufs[i].fd;
		x->sumbufs[i].plan = fftwf_plan_dft_c2r_1d(x->fftsize, x->sumbufs[i].fd, x->sumbufs[i].td, FFTW_MEASURE);
		x->sumbufs[i].readpos = 0;
	}
	x->sumbufs[0].next = &(x->sumbufs[1]);
	x->sumbufs[0].prev = &(x->sumbufs[x->nsumbufs - 1]);
	for (i = 1; i < x->nsumbufs; i++) {
		x->sumbufs[i].next = &(x->sumbufs[(i + 1) % x->nsumbufs]);
		x->sumbufs[i].prev = &(x->sumbufs[i - 1]);
	}

	partconv_deal_work(x);
	x->curcall = 0;
	x->curpart = 0;

	post("partconv~: using %s in %d partitions with FFT-size %d", x->arrayname->s_name, x->nparts, x->fftsize);
	x->ir_prepared = 1;
}

static void partconv_dsp(t_partconv *x, t_signal **sp)
{
	// if the ir array has not been prepared, prepare it
	if (x->ir_prepared == 0) {
		partconv_set(x, x->arrayname);
	}

	dsp_add(partconv_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void *partconv_new(t_symbol *s, int argc, t_atom *argv)
{
	t_partconv *x = (t_partconv *)pd_new(partconv_class);

	outlet_new(&x->x_obj, gensym("signal"));

	if (argc != 2) {
		post("argc = %d", argc);
		error("partconv~: usage: [partconv~ <arrayname> <partsize>]\n\t- partition size must be a power of 2 >= blocksize");
		return NULL;
	}

	x->arrayname = atom_getsymbol(argv);
	x->partsize = atom_getfloatarg(1, argc, argv);
	if (x->partsize <= 0 || x->partsize != (1 << ilog2(x->partsize)))
	{
		error("partconv~: partition size not a power of 2");
		return NULL;
	}
	x->fftsize = 2 * x->partsize;
	x->scale = 1 / ((float) x->fftsize);

	// need 2*(n/2+1) float array for in-place transform, where n is fftsize.
#ifdef USE_SSE
	// for sse, make it a multiple of 8, because we pull in 4 bins at a time and don't want to get a segfault
	x->paddedsize = 2 * (x->fftsize / 2 + 4);
#else
	x->paddedsize = 2 * (x->fftsize / 2 + 1);
#endif
	x->nbins = x->fftsize / 2 + 1;
	x->ir_prepared = 0;
	x->pd_blocksize = sys_getblksize();

	return (x);
}

void partconv_tilde_setup(void)
{
	partconv_class = class_new(gensym("partconv~"), (t_newmethod)partconv_new,
			(t_method)partconv_free, sizeof(t_partconv), 0, A_GIMME, 0);
	class_addmethod(partconv_class, nullfn, gensym("signal"), 0);
	class_addmethod(partconv_class, (t_method) partconv_dsp, gensym("dsp"), 0);
	class_addmethod(partconv_class, (t_method) partconv_set, gensym("set"), A_DEFSYMBOL, 0);
}
