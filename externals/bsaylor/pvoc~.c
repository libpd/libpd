/* Copyright 2003 Benjamin R. Saylor <bensaylor@fastmail.fm>
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

#include <math.h>
#include <fftw3.h>
#include "m_pd.h"

// FIXME:
// set array when dsp is turned on
// get rid of shiftbuf, just save values that will be needed next before overwriting them
// cubic interp
// use float fftw?
// performance testing
// what if there are 2 transients less than fftsize apart?  second one might get smeared.
// compare sound with phaselockedvoc.pd
// detect transients
// peaks + noise
// other phase locking methods
// use floats?
// use in-place?
// if don't have an array, call setarray(x->arrayname)
// window size and fft size independent (what is gained by zero-padding?)
// error if parent blocksize is larger than hopsize
// slowly return window to true position after centering around a transient?
// use fewer fft arrays?

// DONE:
// use FFTW_MEASURE

static t_class *pvoc_class;

typedef struct _pvoc {
	t_object x_obj;
	t_symbol *arrayname;
	t_garray *arrayobj;
	t_float *array;
	int arraysize;
	double *window;
	int fftsize;
	int overlap;
	int hopsize;			// = fftsize / overlap
	int trans[256];			// sample indices of transients
	int ntrans;			// number of transients
	int wastrans;			// there was a transient in the left half of the window during the previous frame
	double phaselocking;
	fftw_plan fftplan;
	fftw_plan fft2plan;
	fftw_plan ifftplan;
	double *fftin;
	double *fft2in;
	double *ifftout;
	fftw_complex *fftout;
	fftw_complex *fft2out;
	fftw_complex *ifftin;
	fftw_complex *shiftbuf;
	double *outbuf;
	int outbufpos;
} t_pvoc;

// if there is a transient between samples a and b, return its position, else return -1
static inline int transient_between(t_pvoc *x, int a, int b)
{
	// linear search for now: FIXME
	int i;
	for (i = 0; i < x->ntrans; i++)
		if (a <= x->trans[i] && b >= x->trans[i])
			return x->trans[i];
	return -1;
}

#if 1
static inline double interpolate(t_pvoc *x, double t)
{
	// linear interpolation for now: FIXME
	if (t < 0 || t > (x->arraysize - 1))
		return 0.0;
	else {
		int x_1 = t;
		double y_1 = x->array[x_1];
		double y_2 = x->array[x_1 + 1];

		return (y_2 - y_1) * (t - x_1) + y_1;
	}
}
#else
static inline double interpolate(t_pvoc *x, double t)
{
	// FIXME check bounds (can't think now)
	int truncphase = (int) x->phase;
	double fr = x->phase - ((double) truncphase);
	double inm1 = x->ifftout[truncphase - 1];
	double in   = x->ifftout[truncphase + 0];
	double inp1 = x->ifftout[truncphase + 1];
	double inp2 = x->ifftout[truncphase + 2];

	// taken from swh-plugins-0.4.0/ladspa-util.h cube_interp, made to use doubles instead since doubles are what i'm using for some reason
	return in + 0.5 * fr * (inp1 - inm1 +
	 fr * (4.0 * inp1 + 2.0 * inm1 - 5.0 * in - inp2 +
	 fr * (3.0 * (in - inp1) - inm1 + inp2)));
}
#endif

static t_int *pvoc_perform(t_int *w)
{
	t_pvoc *x = (t_pvoc *)(w[1]);
	t_float *in1 = (t_float *)(w[2]);
	t_float *in2 = (t_float *)(w[3]);
	t_float *out = (t_float *)(w[4]);
	int n = (int)(w[5]);
	double t;
	double pitchshift;
	int transientpos;
	int desmear;
	double framestart;
	double frameend;
	int i;	// 0 to n -type iterator
	int j;	// start to end -type iterator
	int k;	// bin iterator
	double xlook;		// iterator for interpolated table lookup

	// if we are at the start of a new frame...
	if (x->outbufpos % x->hopsize == 0) {

		// don't desmear this frame by default
		desmear = 0;

		// sample the input signals (FIXME just sample these in the beginning..)
		t = in1[0];		// time position
		pitchshift = in2[0];	// pitch shift

		// set the frame boundaries with the desired time pos in the middle
		framestart = t - (pitchshift * x->fftsize / 2);
		frameend = framestart + pitchshift * x->fftsize;

		// prepare to de-smear transients
		transientpos = transient_between(x, (int) framestart, (int) frameend);
		if (transientpos != -1) {
			// there is a transient in this frame
#if 0
			if (transientpos > t) {
				// there is a transient in the right half of the window:
				// --> move the window left until the transient is outside it
				frameend = transientpos;
				framestart = frameend - x->fftsize;
				x->wastrans = 0;
			} else if ( ! x->wastrans) {
				// there is a transient in the left half of the window,
				// and there was no transient there during the previous frame:
				// --> center the window around the transient and remember to desmear this frame
				framestart = transientpos - (x->fftsize / 2);
				frameend = framestart + x->fftsize;
				desmear = 1;
				x->wastrans = 1;
			} else
				x->wastrans = 1;
#else
			// this simpler method turns out to sound better (timing sounds more accurate, no "frozen" sound preceding transients)
			if ( ! x->wastrans) {
				// there is a transient in the window,
				// and there wasn't during the previous frame:
				// --> center the window around the transient and remember to desmear this frame
				framestart = transientpos - (pitchshift * x->fftsize / 2);
				desmear = 1;
			}
			x->wastrans = 1;
#endif
		} else
			x->wastrans = 0;

		// interpolate-read the array from framestart to frameend into fftin, windowing it
		for (i = 0, xlook = framestart; i < x->fftsize; xlook += pitchshift, i++) {
			x->fftin[i] = interpolate(x, xlook) * x->window[i];
		}

		// hop forward and read the second frame into fft2in
		// FIXME merge the two loops?
		framestart += pitchshift * x->hopsize;
		for (i = 0, xlook = framestart; i < x->fftsize; xlook += pitchshift, i++) {
			x->fft2in[i] = interpolate(x, xlook) * x->window[i];
		}

		// do the ffts
		fftw_execute(x->fftplan);
		fftw_execute(x->fft2plan);

		if ( ! desmear) {
			// Miller Puckette's phase modification math (translation from 09.pvoc.pd and 10.phaselockedvoc.pd)

			double a, b, r, c, d;

			// propagate phase
			for (k = 0; k < (x->fftsize / 2 + 1); k++) {
				a = x->ifftin[k][0] * x->fftout[k][0] + x->ifftin[k][1] * x->fftout[k][1] + 0.00000000000000000001;
				b = x->ifftin[k][1] * x->fftout[k][0] - x->ifftin[k][0] * x->fftout[k][1];
				r = 1 / sqrt(a * a + b * b);
				c = a * r;
				d = b * r;
				x->shiftbuf[k][0] = c * x->fft2out[k][0] - d * x->fft2out[k][1];
				x->shiftbuf[k][1] = c * x->fft2out[k][1] + d * x->fft2out[k][0];
			}

			// don't phase-lock the first bin
			x->ifftin[0][0] = x->shiftbuf[0][0];
			x->ifftin[0][1] = x->shiftbuf[0][1];

			// phase-lock
			for (k = 1; k < (x->fftsize / 2); k++) {
				x->ifftin[k][0] = x->shiftbuf[k][0] - x->phaselocking * (x->shiftbuf[k - 1][0] + x->shiftbuf[k + 1][0]);
				x->ifftin[k][1] = x->shiftbuf[k][1] - x->phaselocking * (x->shiftbuf[k - 1][1] + x->shiftbuf[k + 1][1]);
			}

			// don't phase-lock the last bin
			x->ifftin[x->fftsize / 2][0] = x->shiftbuf[x->fftsize / 2][0];
			x->ifftin[x->fftsize / 2][1] = x->shiftbuf[x->fftsize / 2][1];

		} else {
			// this frame is to be de-smeared, which means don't modify the phases, just preserve the original phases
			for (k = 0; k < (x->fftsize / 2 + 1); k++) {
				x->ifftin[k][0] = x->fftout[k][0];
				x->ifftin[k][1] = x->fftout[k][1];
			}
		}

		// do the ifft
		fftw_execute(x->ifftplan);

		// add into output buffer, windowing and normalizing first (divide by blocksize)
		for (i = 0, j = x->outbufpos; i < x->fftsize; i++, j++) {
			x->outbuf[j % x->fftsize] += x->ifftout[i] / x->fftsize * x->window[i];
		}
	}

	// output one block of the output buffer
	for (i = 0, j = x->outbufpos; i < n; i++, j++) {
		out[i] = x->outbuf[j % x->fftsize];
		x->outbuf[j % x->fftsize] = 0;		// zero the part of the buffer that was just output
	}

	// move the output buffer pointer forward by one block
	x->outbufpos = (x->outbufpos + n) % x->fftsize;

	return (w+6);
}

static void pvoc_dsp(t_pvoc *x, t_signal **sp)
{
	    dsp_add(pvoc_perform, 5, x, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

// adapted from jsarlo's windowing library
// Hanning
static void makewindow(double *w, int n)
{
	int i;
	double xshift =  n / 2.0;
	double x;
	for (i = 0; i < n; i++) {
		x = (i - xshift) / xshift;
		w[i] = 0.5 * (1 + cos(M_PI * x));
	}
}

static void setarray(t_pvoc *x, t_symbol *s)
{
	x->arrayname = s;
	if ( ! (x->arrayobj = (t_garray *)pd_findbyclass(x->arrayname, garray_class))) {
 		if (*x->arrayname->s_name) pd_error(x, "pvoc~: %s: no such array", x->arrayname->s_name);
		x->array = NULL;
		x->arraysize = 0;
	} else if ( ! garray_getfloatarray(x->arrayobj, &x->arraysize, &x->array)) {
 		error("%s: bad template", x->arrayname->s_name);
		x->array = NULL;
		x->arraysize = 0;
	} else {
		garray_usedindsp(x->arrayobj);
	}
}

static void locking(t_pvoc *x, t_floatarg f)
{
	x->phaselocking = f;
}

// takes a list of sample positions of transients to be de-smeared
static void transients(t_pvoc *x, t_symbol *s, int argc, t_atom *argv)
{
	int i;

	x->ntrans = argc;
	for (i = 0; i < x->ntrans; i++)
		x->trans[i] = atom_getfloatarg(i, argc, argv);
}

// for clarity (same as "transients" with no args)
static void notransients(t_pvoc *x)
{
	x->ntrans = 0;
}

static void *pvoc_new(t_symbol *s, int argc, t_atom *argv)
{
	t_pvoc *x = (t_pvoc *)pd_new(pvoc_class);
	int i;

	inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);	// pitch-shift inlet
	outlet_new(&x->x_obj, gensym("signal"));

	if (argc != 3) {
		post("argc = %d", argc);
		error("pvoc~: usage: [pvoc~ <arrayname> <fftsize> <overlap>]");
		return NULL;
	}

	x->fftsize = atom_getfloatarg(1, argc, argv);
	x->overlap = atom_getfloatarg(2, argc, argv);
	x->hopsize = x->fftsize / x->overlap;
	x->ntrans = 0;
	x->wastrans = 0;
	x->phaselocking = 0;

	// get the source array
	setarray(x, atom_getsymbol(argv));

	// set up output ring buffer
	x->outbuf = getbytes(sizeof(double) * x->fftsize);
	x->outbufpos = 0;
	for (i = 0; i < x->fftsize; i++)
		x->outbuf[i] = 0;

	// make table for window function
	x->window = getbytes(sizeof(double) * x->fftsize);
	makewindow(x->window, x->fftsize);

	// set up fftw stuff
	x->fftin = fftw_malloc(sizeof(double) * x->fftsize);
	x->fft2in = fftw_malloc(sizeof(double) * x->fftsize);
	x->ifftout = fftw_malloc(sizeof(double) * x->fftsize);
	x->fftout = fftw_malloc(sizeof(fftw_complex) * (x->fftsize / 2 + 1));
	x->fft2out = fftw_malloc(sizeof(fftw_complex) * (x->fftsize / 2 + 1));
	x->ifftin = fftw_malloc(sizeof(fftw_complex) * (x->fftsize / 2 + 1));
	x->shiftbuf = fftw_malloc(sizeof(fftw_complex) * (x->fftsize / 2 + 1));
	for (i = 0; i < (x->fftsize / 2 + 1); i++) {
		x->ifftin[i][0] = 0;	// need to start the phases from zero
		x->ifftin[i][1] = 0;
	}
	x->fftplan = fftw_plan_dft_r2c_1d(x->fftsize, x->fftin, x->fftout, FFTW_MEASURE);
	x->fft2plan = fftw_plan_dft_r2c_1d(x->fftsize, x->fft2in, x->fft2out, FFTW_MEASURE);
	x->ifftplan = fftw_plan_dft_c2r_1d(x->fftsize, x->ifftin, x->ifftout, FFTW_MEASURE | FFTW_PRESERVE_INPUT);
	
	return (x);
}

static void pvoc_free(t_pvoc *x)
{
	freebytes(x->outbuf, sizeof(double) * x->fftsize);
	freebytes(x->window, sizeof(double) * x->fftsize);
	fftw_free(x->fftin);
	fftw_free(x->fft2in);
	fftw_free(x->ifftout);
	fftw_free(x->fftout);
	fftw_free(x->fft2out);
	fftw_free(x->ifftin);
	fftw_free(x->shiftbuf);
	fftw_destroy_plan(x->fftplan);
	fftw_destroy_plan(x->fft2plan);
	fftw_destroy_plan(x->ifftplan);
}

void pvoc_tilde_setup(void)
{
	pvoc_class = class_new(gensym("pvoc~"), (t_newmethod)pvoc_new, (t_method)pvoc_free, sizeof(t_pvoc), 0, A_GIMME, 0);
	class_addmethod(pvoc_class, nullfn, gensym("signal"), 0);
	class_addmethod(pvoc_class, (t_method) pvoc_dsp, gensym("dsp"), 0);
	class_addmethod(pvoc_class, (t_method) setarray, gensym("setarray"), A_DEFSYMBOL, 0);
	class_addmethod(pvoc_class, (t_method) locking, gensym("locking"), A_DEFFLOAT, 0);
	class_addmethod(pvoc_class, (t_method) transients, gensym("transients"), A_GIMME, 0);
	class_addmethod(pvoc_class, (t_method) notransients, gensym("notransients"), 0);
}
