/*
 *  iemmatrix
 *
 *  objects for manipulating simple matrices
 *  mostly refering to matlab/octave matrix functions
 *
 * Copyright (c) Thomas Musil, IEM KUG Graz Austria 2000-2003
 * Copyright (c) IOhannes m zmölnig, forum::für::umläute, IEM, Graz, Austria
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 */

#include "iemmatrix.h"


/* ---------- mtx_*~ - signal matrix multiplication object with message matrix-coeff. ----------- */


/* usage:
 *   [mtx_*~ <#out> <#in>]: multiply #in signals with a <#out,#in>-matrix to get #out-signals
 *   [mtx_*~ <#io>]: multiply #io signals with a <#io,#io>-matrix to get #io-signals
 *
 *  1st inlet: (message only): the matrix to multiply with (defaults to zero(#out, #in)
 *  2nd-(n-1)th inlet: the input signals
 *  last inlet: the interpolation time in [ms]
 *
 * this also implements a compatibility layer with old objects from tm and jmz
 * that provide basically the same functionality
 *
 * further note:
 *  pd does not like signal-objects that don't listen to signals on the very first inlet
 *  however, the first signal-inlet of [mtx_*~] is the 2nd(!) inlet
 *  to achieve this, we silently ignore any signal that comes in at the first inlet
 */

typedef struct matrix_multilde
{
  t_object	x_obj;
  t_float       *x_matcur;
  t_float	*x_matend;
  t_float	*x_inc;
  t_float	*x_biginc;
  t_float	**x_io;
  t_float	*x_outsumbuf;
  int		x_outsumbufsize;
  int		x_n_in;	/* columns */
  int		x_n_out; /* rows	*/
  t_float	x_msi; /* for sending floats to signal~ins */
  int		x_retarget;
  t_float	x_time_ms;
  int		x_remaining_ticks;
  t_float	x_ms2tick;
  t_float	x_1overn;
  int           x_compat; /* 0=mtx_*~; 1=matrix_mul_line~; 2=matrix~ */
} t_matrix_multilde;

t_class *matrix_multilde_class;

static void matrix_multilde_time(t_matrix_multilde *x, t_floatarg time_ms)
{
  if(time_ms <= 0.0f)
    time_ms = 0.0f;
  x->x_time_ms = time_ms;
}

static void matrix_multilde_matrix_set(t_matrix_multilde *x, int argc, t_atom *argv, int transpose)
{
  int col, row, i, length;
  t_float *matcur = x->x_matcur;
  t_float *matend = x->x_matend;

  if(argc<2)
    {
      pd_error(x, "mtx_*~ : bad matrix: <int> out_rows <int> in_cols !");
      return;
    }

  row = atom_getint(argv);
  argv++;
  col = atom_getint(argv);
  argv++;
  argc-=2;

  if(transpose){
    int dummy=row;
    row=col;
    col=dummy;
  }

  if((col!=x->x_n_in)||(row!=x->x_n_out))
    {
      pd_error(x,"mtx_*~ : matrix dimensions do not match (%dx%d != %dx%d)!!", col, row, x->x_n_in, x->x_n_out);
      return;
    }
  if(argc<row*col)
    {
      pd_error(x,"mtx_*~ : reduced matrices not yet supported");
      return;
    }
  
  length = col * row;
  
  if(transpose){
    /* we need to transpose the matrix */
    for(i=0; i<row; i++){
      int j=0;
      for(j=0; j<col; j++){
	*matend++=atom_getfloat(argv+i+j*row);
      }
    }
  } else
    for(i=0; i<length; i++)
      *matend++=atom_getfloat(argv++);

  if(x->x_time_ms <= 0.0f){
    matend = x->x_matend;
    for(i=0; i<length; i++)
      *matcur++=*matend++;
    x->x_remaining_ticks = x->x_retarget = 0;
  } else x->x_retarget = 1;
}
static void matrix_multilde_matrix(t_matrix_multilde *x, t_symbol *s, int argc, t_atom *argv){
  matrix_multilde_matrix_set(x,argc, argv, 0);
}
static void matrix_multilde_matrixT(t_matrix_multilde *x, t_symbol *s, int argc, t_atom *argv){
  /* transpose the matrix before setting it */
  matrix_multilde_matrix_set(x,argc, argv, 1);
}
static void matrix_multilde_element(t_matrix_multilde *x, t_symbol *s, int argc, t_atom *argv)
{
  int col, row, n_in_cols=x->x_n_in;
  t_float element; 
  t_float *matcur = x->x_matcur;
  t_float *matend = x->x_matend;

  if(argc != 3)
    {
      pd_error(x,"mtx_*~ : bad element: 3 floats: <int> out_row <int> in_col <float> element !");
      return;
    }

  row = atom_getint(argv) - 1;
  col = atom_getint(argv+1) - 1;
  element = atom_getfloat(argv+2);

  if((row >= x->x_n_out) || (row < 0))
    {
      pd_error(x,"mtx_*~ : row dimensions do not match !!");
      return;
    }
  if((col >= n_in_cols) || (col < 0))
    {
      pd_error(x,"mtx_*~ : col dimensions do not match !!");
      return;
    }

  matend += row * n_in_cols + col;
  matcur += row * n_in_cols + col;

  if(x->x_time_ms <= 0.0f)
    {
      *matend = *matcur = element;
      x->x_remaining_ticks = x->x_retarget = 0;
    }
  else
    {
      *matend = element;
      x->x_retarget = 1;
    }
}

static void matrix_multilde_row(t_matrix_multilde *x, t_symbol *s, int argc, t_atom *argv)
{
  int col, nth_row, i;
  t_float *matcur = x->x_matcur;
  t_float *matend = x->x_matend;

  if(argc<1)
    {
      pd_error(x,"mtx_*~ : bad row: <int> in_row !");
      return;
    }

  nth_row = atom_getint(argv) - 1;
  argv++;
  argc--;

  if((nth_row >= x->x_n_out) || (nth_row < 0))
    {
      pd_error(x,"mtx_*~ : row dimensions do not match !!");
      return;
    }
  col = x->x_n_in;
  if(argc < col)
    {
      pd_error(x,"mtx_*~ : col dimensions do not match !!");
      return;
    }

  matend += nth_row * col;
  matcur += nth_row * col;
  if(x->x_time_ms <= 0.0f)
    {
      for(i=0; i<col; i++)
	{
	  *matend++ = *matcur++ = atom_getfloat(argv);
	  argv++;
	}
      x->x_remaining_ticks = x->x_retarget = 0;
    }
  else
    {
      for(i=0; i<col; i++)
	{
	  *matend++ = atom_getfloat(argv);
	  argv++;
	}
      x->x_retarget = 1;
    }
}

static void matrix_multilde_col(t_matrix_multilde *x, t_symbol *s, int argc, t_atom *argv)
{
  int row, col, nth_col, i;
  t_float *matcur = x->x_matcur;
  t_float *matend = x->x_matend;

  if(argc<1)
    {
      pd_error(x,"mtx_*~ : bad col: <int> in_cols !");
      return;
    }

  nth_col = atom_getint(argv) - 1;
  argv++;
  argc--;

  col = x->x_n_in;
  if((nth_col < 0)||(nth_col >= col))
    {
      pd_error(x,"mtx_*~ : col dimensions do not match !!");
      return;
    }
  row = x->x_n_out;
  if(argc < row)
    {
      pd_error(x,"mtx_*~ : row dimensions do not match !!");
      return;
    }

  matend += nth_col;
  matcur += nth_col;
  if(x->x_time_ms <= 0.0f)
    {
      for(i=0; i<row; i++)
	{
	  *matend = *matcur = atom_getfloat(argv);
	  argv++;
	  matend += col;
	  matcur += col;
	}
      x->x_remaining_ticks = x->x_retarget = 0;
    }
  else
    {
      for(i=0; i<row; i++)
	{
	  *matend = atom_getfloat(argv);
	  argv++;
	  matend += col;
	  matcur += col;
	}
      x->x_retarget = 1;
    }
}

static void matrix_multilde_stop(t_matrix_multilde *x)
{
  int i = x->x_n_out*x->x_n_in;
  t_float *matend=x->x_matend;
  t_float *matcur=x->x_matcur;

  while(i--)
    {
      *matend++ = *matcur++;
    }
  x->x_remaining_ticks = x->x_retarget = 0;
}

/* the dsp thing */

static t_int *matrix_multilde_perf8(t_int *w)
{
  t_matrix_multilde *x = (t_matrix_multilde *)(w[1]);
  int n = (int)(w[2]);
  t_float **io = x->x_io;
  t_float *outsum, *houtsum;
  t_float *matcur, *matend;
  t_float *inc1 ,*biginc, inc;
  int n_in = x->x_n_in;   /* columns */
  int n_out = x->x_n_out; /* rows	*/
  t_float *in, *out, mul, bigmul;
  int r, c, i;

  if(x->x_retarget)
    {
      int nticks = (int)(x->x_time_ms * x->x_ms2tick);

      if(!nticks)
	nticks = 1;
      x->x_remaining_ticks = nticks;
      inc1 = x->x_inc;
      biginc = x->x_biginc;
      matcur = x->x_matcur;
      matend = x->x_matend;
      mul = x->x_1overn / (float)nticks;
      bigmul = 1.0f / (float)nticks;
      i = n_out*n_in;
      while(i--)
	{
	  inc = *matend++ - *matcur++;
	  *inc1++ = inc * mul;
	  *biginc++ = inc * bigmul;
	}
      x->x_retarget = 0;
    }

  if(x->x_remaining_ticks)
    {
      inc1 = x->x_inc;
      biginc = x->x_biginc;
      matcur = x->x_matcur;
      /* 1. output-vector-row */
      in = io[0];
      houtsum = x->x_outsumbuf;
      outsum = houtsum;
      inc = *inc1++;
      mul = *matcur;
      for(i=n; i; i -= 8, outsum += 8, in += 8)
	{
	  outsum[0] = in[0] * mul;
	  mul += inc;
	  outsum[1] = in[1] * mul;
	  mul += inc;
	  outsum[2] = in[2] * mul;
	  mul += inc;
	  outsum[3] = in[3] * mul;
	  mul += inc;
	  outsum[4] = in[4] * mul;
	  mul += inc;
	  outsum[5] = in[5] * mul;
	  mul += inc;
	  outsum[6] = in[6] * mul;
	  mul += inc;
	  outsum[7] = in[7] * mul;
	  mul += inc;
	}
      *matcur++ += *biginc++;
      for(c=1; c<n_in; c++)/* c+1. element of 1. row */
	{
	  in = io[c];
	  outsum = houtsum;
	  inc = *inc1++;
	  mul = *matcur;
	  for(i=n; i; i -= 8, outsum += 8, in += 8)
	    {
	      outsum[0] += in[0] * mul;
	      mul += inc;
	      outsum[1] += in[1] * mul;
	      mul += inc;
	      outsum[2] += in[2] * mul;
	      mul += inc;
	      outsum[3] += in[3] * mul;
	      mul += inc;
	      outsum[4] += in[4] * mul;
	      mul += inc;
	      outsum[5] += in[5] * mul;
	      mul += inc;
	      outsum[6] += in[6] * mul;
	      mul += inc;
	      outsum[7] += in[7] * mul;
	      mul += inc;
	    }
	  *matcur++ += *biginc++;
	}
      for(r=1; r<n_out; r++)/* 2. .. n_out. output-vector-row */
	{
	  in = io[0];
	  houtsum += n;
	  outsum = houtsum;
	  inc = *inc1++;
	  mul = *matcur;
	  for(i=n; i; i -= 8, outsum += 8, in += 8)
	    {
	      outsum[0] = in[0] * mul;
	      mul += inc;
	      outsum[1] = in[1] * mul;
	      mul += inc;
	      outsum[2] = in[2] * mul;
	      mul += inc;
	      outsum[3] = in[3] * mul;
	      mul += inc;
	      outsum[4] = in[4] * mul;
	      mul += inc;
	      outsum[5] = in[5] * mul;
	      mul += inc;
	      outsum[6] = in[6] * mul;
	      mul += inc;
	      outsum[7] = in[7] * mul;
	      mul += inc;
	    }
	  *matcur++ += *biginc++;
	  for(c=1; c<n_in; c++)/* c+1. element of r+1. row */
	    {
	      in = io[c];
	      outsum = houtsum;
	      inc = *inc1++;
	      mul = *matcur;
	      for(i=n; i; i -= 8, outsum += 8, in += 8)
		{
		  outsum[0] += in[0] * mul;
		  mul += inc;
		  outsum[1] += in[1] * mul;
		  mul += inc;
		  outsum[2] += in[2] * mul;
		  mul += inc;
		  outsum[3] += in[3] * mul;
		  mul += inc;
		  outsum[4] += in[4] * mul;
		  mul += inc;
		  outsum[5] += in[5] * mul;
		  mul += inc;
		  outsum[6] += in[6] * mul;
		  mul += inc;
		  outsum[7] += in[7] * mul;
		  mul += inc;
		}
	      *matcur++ += *biginc++;
	    }
	}

      if(!--x->x_remaining_ticks)
	{
	  matcur = x->x_matcur;
	  matend = x->x_matend;
	  i = n_in * n_out;
	  while(i--)
	    *matcur++ = *matend++;
	}
    }
  else
    {
      matend = x->x_matend;
      /* 1. output-vector-row */
      in = io[0];
      houtsum = x->x_outsumbuf;
      outsum = houtsum;
      mul = *matend++;
      if(mul == 0.0f)
	{
	  for(i=n; i; i -= 8, outsum += 8, in += 8)
	    {
	      outsum[0] = 0.0f;
	      outsum[1] = 0.0f;
	      outsum[2] = 0.0f;
	      outsum[3] = 0.0f;
	      outsum[4] = 0.0f;
	      outsum[5] = 0.0f;
	      outsum[6] = 0.0f;
	      outsum[7] = 0.0f;
	    }
	}
      else
	{
	  for(i=n; i; i -= 8, outsum += 8, in += 8)
	    {
	      outsum[0] = in[0] * mul;
	      outsum[1] = in[1] * mul;
	      outsum[2] = in[2] * mul;
	      outsum[3] = in[3] * mul;
	      outsum[4] = in[4] * mul;
	      outsum[5] = in[5] * mul;
	      outsum[6] = in[6] * mul;
	      outsum[7] = in[7] * mul;
	    }
	}
      for(c=1; c<n_in; c++)/* c+1. element of 1. row */
	{
	  in = io[c];
	  outsum = houtsum;
	  mul = *matend++;
	  if(mul != 0.0f)
	    {
	      for(i=n; i; i -= 8, outsum += 8, in += 8)
		{
		  outsum[0] += in[0] * mul;
		  outsum[1] += in[1] * mul;
		  outsum[2] += in[2] * mul;
		  outsum[3] += in[3] * mul;
		  outsum[4] += in[4] * mul;
		  outsum[5] += in[5] * mul;
		  outsum[6] += in[6] * mul;
		  outsum[7] += in[7] * mul;
		}
	    }
	}
      for(r=1; r<n_out; r++)/* 2. .. n_out. output-vector-row */
	{
	  in = io[0];
	  houtsum += n;
	  outsum = houtsum;
	  mul = *matend++;
	  if(mul == 0.0f)
	    {
	      for(i=n; i; i -= 8, outsum += 8, in += 8)
		{
		  outsum[0] = 0.0f;
		  outsum[1] = 0.0f;
		  outsum[2] = 0.0f;
		  outsum[3] = 0.0f;
		  outsum[4] = 0.0f;
		  outsum[5] = 0.0f;
		  outsum[6] = 0.0f;
		  outsum[7] = 0.0f;
		}
	    }
	  else
	    {
	      for(i=n; i; i -= 8, outsum += 8, in += 8)
		{
		  outsum[0] = in[0] * mul;
		  outsum[1] = in[1] * mul;
		  outsum[2] = in[2] * mul;
		  outsum[3] = in[3] * mul;
		  outsum[4] = in[4] * mul;
		  outsum[5] = in[5] * mul;
		  outsum[6] = in[6] * mul;
		  outsum[7] = in[7] * mul;
		}
	    }
	  for(c=1; c<n_in; c++)/* c+1. element of r+1. row */
	    {
	      in = io[c];
	      outsum = houtsum;
	      mul = *matend++;
	      if(mul != 0.0f)
		{
		  for(i=n; i; i -= 8, outsum += 8, in += 8)
		    {
		      outsum[0] += in[0] * mul;
		      outsum[1] += in[1] * mul;
		      outsum[2] += in[2] * mul;
		      outsum[3] += in[3] * mul;
		      outsum[4] += in[4] * mul;
		      outsum[5] += in[5] * mul;
		      outsum[6] += in[6] * mul;
		      outsum[7] += in[7] * mul;
		    }
		}
	    }
	}
    }
  outsum = x->x_outsumbuf;
  for(r=0; r<n_out; r++)/* output-vector-row */
    {
      out = io[n_in+r];
      for (i=n; i; i -= 8, out += 8, outsum += 8)
	{
	  out[0] = outsum[0];
	  out[1] = outsum[1];
	  out[2] = outsum[2];
	  out[3] = outsum[3];
	  out[4] = outsum[4];
	  out[5] = outsum[5];
	  out[6] = outsum[6];
	  out[7] = outsum[7];
	}
    }
  return (w+3);
}
static t_int *matrix_multilde_perform(t_int *w)
{
  t_matrix_multilde *x = (t_matrix_multilde *)(w[1]);
  int n = (int)(w[2]);
  t_float **io = x->x_io;
  t_float *outsum, *houtsum;
  t_float *matcur, *matend;
  t_float *inc1 ,*biginc, inc;
  int n_in = x->x_n_in;   /* columns */
  int n_out = x->x_n_out; /* rows	*/
  t_float *in, *out, mul, bigmul;
  int r, c, i;

  if(x->x_retarget)
    {
      int nticks = (int)(x->x_time_ms * x->x_ms2tick);

      if(!nticks)
	nticks = 1;
      x->x_remaining_ticks = nticks;
      inc1 = x->x_inc;
      biginc = x->x_biginc;
      matcur = x->x_matcur;
      matend = x->x_matend;
      mul = x->x_1overn / (float)nticks;
      bigmul = 1.0f / (float)nticks;
      i = n_out*n_in;
      while(i--)
	{
	  inc = *matend++ - *matcur++;
	  *inc1++ = inc * mul;
	  *biginc++ = inc * bigmul;
	}
      x->x_retarget = 0;
    }

  if(x->x_remaining_ticks)
    {
      inc1 = x->x_inc;
      biginc = x->x_biginc;
      matcur = x->x_matcur;
      /* 1. output-vector-row */
      in = io[0];
      houtsum = x->x_outsumbuf;
      outsum = houtsum;
      inc = *inc1++;
      mul = *matcur;
      i=n;
      while(i--)
	{
	  *outsum++ = *in++ * mul;
	  mul += inc;
	}
      *matcur++ += *biginc++;
      for(c=1; c<n_in; c++)/* c+1. element of 1. row */
	{
	  in = io[c];
	  outsum = houtsum;
	  inc = *inc1++;
	  mul = *matcur;
	  i=n;
	  while(i--)
	    {
	      *outsum++ += *in++ * mul;
	      mul += inc;
	    }
	  *matcur++ += *biginc++;
	}
      for(r=1; r<n_out; r++)/* 2. .. n_out. output-vector-row */
	{
	  in = io[0];
	  houtsum += n;
	  outsum = houtsum;
	  inc = *inc1++;
	  mul = *matcur;
	  i=n;
	  while(i--)
	    {
	      *outsum++ = *in++ * mul;
	      mul += inc;
	    }
	  *matcur++ += *biginc++;
	  for(c=1; c<n_in; c++)/* c+1. element of r+1. row */
	    {
	      in = io[c];
	      outsum = houtsum;
	      inc = *inc1++;
	      mul = *matcur;
	      i=n;
	      while(i--)
		{
		  *outsum++ += *in++ * mul;
		  mul += inc;
		}
	      *matcur++ += *biginc++;
	    }
	}

      if(!--x->x_remaining_ticks)
	{
	  matcur = x->x_matcur;
	  matend = x->x_matend;
	  i = n_in * n_out;
	  while(i--)
	    *matcur++ = *matend++;
	}
    }
  else
    {
      matend = x->x_matend;
      /* 1. output-vector-row */
      in = io[0];
      houtsum = x->x_outsumbuf;
      outsum = houtsum;
      mul = *matend++;
      i=n;
      if(mul == 0.0f)
	while(i--)*outsum++ = 0.0f;
      else
	while(i--)*outsum++ = *in++ * mul;

      for(c=1; c<n_in; c++)/* c+1. element of 1. row */
	{
	  in = io[c];
	  outsum = houtsum;
	  mul = *matend++;
	  if(mul != 0.0f)
	    {
	      i=n;
	      while(i--) *outsum++ += *in++ * mul;
	    }
	}
      for(r=1; r<n_out; r++)/* 2. .. n_out. output-vector-row */
	{
	  in = io[0];
	  houtsum += n;
	  outsum = houtsum;
	  mul = *matend++;
	  i=n;
	  if(mul == 0.0f)
	    while(i--)*outsum++ = 0.0f;
	  else
	    while(i--)*outsum++ = *in++ * mul;

	  for(c=1; c<n_in; c++)/* c+1. element of r+1. row */
	    {
	      in = io[c];
	      outsum = houtsum;
	      mul = *matend++;
	      i=n;
	      if(mul != 0.0f)
		  while(i--)*outsum++ += *in++ * mul;
	    }
	}
    }
  outsum = x->x_outsumbuf;

  for(r=0; r<n_out; r++)/* output-vector-row */
    {
      out = io[n_in+r];
      i=n;
      while(i--)*out++ = *outsum++;
    }
  
  return (w+3);
}
static void matrix_multilde_dsp(t_matrix_multilde *x, t_signal **sp)
{
  int i, n=sp[0]->s_n * x->x_n_out;
  /* [mtx_*~] ignores the signal on the very 1st inlet */
  int compat_offset=(x->x_compat)?0:1;

  if(!x->x_outsumbuf)
    {
      x->x_outsumbufsize = n;
      x->x_outsumbuf = (t_float *)getbytes(x->x_outsumbufsize * sizeof(t_float));
    }
  else if(x->x_outsumbufsize != n)
    {
      x->x_outsumbuf = (t_float *)resizebytes(x->x_outsumbuf, 
					      x->x_outsumbufsize*sizeof(t_float), 
					      n*sizeof(t_float));
      x->x_outsumbufsize = n;
    }

  n = x->x_n_in + x->x_n_out;
  for(i=0; i<n; i++)
    x->x_io[i] = sp[i+compat_offset]->s_vec;

  n = sp[0]->s_n;
  x->x_ms2tick = 0.001f * (float)(sp[0]->s_sr) / (float)n;
  x->x_1overn = 1.0f / (float)n;


  if(n&7)
    {
      dsp_add(matrix_multilde_perform, 2, x, n);
    }
  else {
    dsp_add(matrix_multilde_perf8, 2, x, n);
  }
}


/* setup/setdown things */

static void matrix_multilde_free(t_matrix_multilde *x)
{
  freebytes(x->x_matcur, x->x_n_in * x->x_n_out * sizeof(t_float));
  freebytes(x->x_matend, x->x_n_in * x->x_n_out * sizeof(t_float));
  freebytes(x->x_inc, x->x_n_in * x->x_n_out * sizeof(t_float));
  freebytes(x->x_biginc, x->x_n_in * x->x_n_out * sizeof(t_float));
  freebytes(x->x_io, (x->x_n_in + x->x_n_out) * sizeof(t_float *));
  if(x->x_outsumbuf)
    freebytes(x->x_outsumbuf, x->x_outsumbufsize * sizeof(t_float));
}

static void *matrix_multilde_new(t_symbol *s, int argc, t_atom *argv)
{
  t_matrix_multilde *x = (t_matrix_multilde *)pd_new(matrix_multilde_class);
  int i, n;


  /* arguments parsing:
   *  this might depend on whether we are creating an object 
   *  [mtx_*~], [matrix~] or [matrix_mul_line~]
   *
   * [mtx_*~  [#out [#in [time]]]]:  in1=matrix; in2-in(#in+1)=signals; in(#in+2)=time
   * [matrix~ [#in [#out [time]]]]:  in1-in(#in)=signals; in(#in+1)=matrix; in(#in+2)=time
   * [matrix_mul_line~ [#in [#out [time]]]]: in1=matrix; in1=time; in1-in(#in):=signals
   *
   * furthermore:
   *  [mtx_*~] and [matrix_mul_line~] : O^=A*I^
   *  [matrix~]                       : O^'=I^'*B
   *
   *  with "matrix=(A or B)" and "A=B'"
   */

  t_atom*ap_in  =argv+1;
  t_atom*ap_out =argv+0;
  t_atom*ap_time=argv+2;

  x->x_compat=0;
  
  if(s==gensym("matrix~")){
    pd_error(x,"[matrix~] is deprecated! use [mtx_*~] instead!!");
    x->x_compat=2;
  }
  else if (s==gensym("matrix_mul_line~")){
    pd_error(x,"[matrix_mul_line~] is deprecated! use [mtx_*~] instead!!");
    x->x_compat=1;
  }

  if(x->x_compat){
    ap_in=argv+0;
    ap_out=argv+1;
  }

  switch(argc)
    {
    case 0:
      x->x_n_in = x->x_n_out = 1;
      x->x_time_ms = (x->x_compat==2)?0.f:50.0f;
      break;
    case 1:
      x->x_n_in = x->x_n_out = (int)atom_getint(argv);
      x->x_time_ms = (x->x_compat==2)?0.f:50.0f;
      break;
    case 2:
      x->x_n_in = (int)atom_getint(ap_in);
      x->x_n_out = (int)atom_getint(ap_out);
      x->x_time_ms = (x->x_compat==2)?0.f:50.0f;
      break;
    default:
      x->x_n_in = (int)atom_getint(ap_in);
      x->x_n_out = (int)atom_getint(ap_out);
      x->x_time_ms = atom_getfloat(ap_time);
      break;
    }


  /* sanity check */
  if(x->x_time_ms < 0.0f)
    x->x_time_ms = (x->x_compat==1)?50.f:0.0f;
  if(x->x_n_in < 1)
    x->x_n_in = 1;
  if(x->x_n_out < 1)
    x->x_n_out = 1;

  /* creating signal ins & outs */
  i = x->x_n_in;
  if(x->x_compat)i--;
  while(i--)
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  i = x->x_n_out;
  while(i--)
    outlet_new(&x->x_obj, &s_signal);

  /* creating the matrix-inlet for [matrix~] */
  if(x->x_compat==2)
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("matrix"), gensym(""));

  /* creating time-inlet (not for [matrix_mul_linie~]) */
  if(x->x_compat!=1)
    inlet_new(&x->x_obj,  &x->x_obj.ob_pd, &s_float, gensym("time"));


  /* setting up internal values */
  x->x_msi = 0;
  x->x_outsumbuf = (t_float *)0;
  x->x_outsumbufsize = 0;
  x->x_matcur = (t_float *)getbytes(x->x_n_in * x->x_n_out * sizeof(t_float));
  x->x_matend = (t_float *)getbytes(x->x_n_in * x->x_n_out * sizeof(t_float));
  x->x_inc = (t_float *)getbytes(x->x_n_in * x->x_n_out * sizeof(t_float));
  x->x_biginc = (t_float *)getbytes(x->x_n_in * x->x_n_out * sizeof(t_float));
  x->x_io = (t_float **)getbytes((x->x_n_in + x->x_n_out) * sizeof(t_float *));
  x->x_remaining_ticks = 0;
  x->x_retarget = 0;
  x->x_ms2tick = 0.001f * 44100.0f / 64.0f; /* will be set in the dsp-routine */
  x->x_1overn = 1.0f / 64.0f;               /* will be set in the dsp-routine */

  /* setting up internal matrices */
  n = x->x_n_in * x->x_n_out;
  for(i=0; i<n; i++)
    {
      x->x_matcur[i] = 0.0f;
      x->x_matend[i] = 0.0f;
      x->x_inc[i] = 0.0f;
      x->x_biginc[i] = 0.0f;
    }
  return (x);
}

void mtx_mul_tilde_setup(void)
{
  matrix_multilde_class = class_new(gensym("mtx_mul~"),
				       (t_newmethod)matrix_multilde_new, 
				       (t_method)matrix_multilde_free,
				       sizeof(t_matrix_multilde), 0, A_GIMME, 0);

  class_addcreator((t_newmethod)matrix_multilde_new, gensym("matrix_mul~"), A_GIMME, 0);
  class_addcreator((t_newmethod)matrix_multilde_new, gensym("mtx_*~"), A_GIMME, 0);
  /* compatibility with tm's iem_matrix */
  class_addcreator((t_newmethod)matrix_multilde_new, gensym("matrix_mul_line~"), A_GIMME, 0);
  /* compatibility with jmz's zexy */
  class_addcreator((t_newmethod)matrix_multilde_new, gensym("matrix~"), A_GIMME, 0);


  class_addmethod(matrix_multilde_class, (t_method)matrix_multilde_dsp, gensym("dsp"), 0);
  CLASS_MAINSIGNALIN(matrix_multilde_class, t_matrix_multilde, x_msi);

  class_addmethod(matrix_multilde_class, (t_method)matrix_multilde_matrix, gensym("matrix"), A_GIMME, 0);
  class_addmethod(matrix_multilde_class, (t_method)matrix_multilde_element, gensym("element"), A_GIMME, 0);
  class_addmethod(matrix_multilde_class, (t_method)matrix_multilde_row, gensym("row"), A_GIMME, 0);
  class_addmethod(matrix_multilde_class, (t_method)matrix_multilde_col, gensym("col"), A_GIMME, 0);
  class_addmethod(matrix_multilde_class, (t_method)matrix_multilde_stop, gensym("stop"), 0);
  class_addmethod(matrix_multilde_class, (t_method)matrix_multilde_time, gensym("time"), A_FLOAT, 0);

  class_addmethod(matrix_multilde_class, (t_method)matrix_multilde_matrixT, gensym(""), A_GIMME, 0);
}

void iemtx_mul__setup(void)
{
  mtx_mul_tilde_setup();
}
