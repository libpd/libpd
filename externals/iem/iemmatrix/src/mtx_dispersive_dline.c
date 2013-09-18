/******************************************************
 *
 * warped delay line
 *
 *   written by Franz Zotter
 *
 *   2007
 *
 *   institute of electronic music and acoustics (iem)
 *
 ******************************************************
 *
 * license: GNU General Public License v.2
 *
 ******************************************************/


/* ------------------------ mtx_dispersive_dline~ ----------------------------- */

/* builds a tap vector with first order all-passes A instead of unit delays.
 * 'ha *' denotes the convolution with the impulse response of A. The chain is of
 * length L
   {x[n]} -> {... ha * ha * x[n], ha * x[n], x[n]} 

   especially useful for frequency warped ffts

   All-pass z-Transform A(z)=(1+lambda*z)/(z-lambda), lambda ... input parameter
   
inlet:  is a signal matrix with C rows (number of channels) and N columns (samples)
outlet: C rows and L colums matrix with current state of the dispersive delay 
   line after computing the N imput samples. It is possible to capture 
   the list outlet every sample when using a column (sample) vector as input matrix.
 
   creation input parameters are:

1: L ...        length of allpass chain
2: lambda ...   alpass/warping parameter


*/

#include "iemmatrix.h"

static t_class *mtx_dispersive_dline_class;

typedef struct _mtx_dispersive_dline
{
  t_object x_obj;
  t_float lambda;
  int length;
  int channels;
  int size;
  t_float *z;
  t_float *tap;
  t_atom *list_out;
  t_outlet *list_outlet;
} t_mtx_dispersive_dline;

static void mtx_dispersive_dline_bang (t_mtx_dispersive_dline *x)
{
   int count;
   t_atom *list = x->list_out;
   SETFLOAT(list, (t_float) x->channels);
   SETFLOAT(list+1, (t_float) x->length);

   list+=2;
   for (count=0; count < x->size; count++)
      SETFLOAT(&list[count],x->tap[count]);
   outlet_anything (x->list_outlet, gensym("matrix"), x->size+2, x->list_out);

}

static void mtx_dispersive_dline_set_lambda(t_mtx_dispersive_dline *x, t_floatarg f)
{
  if ((f<1.0f)&&(f>-1.0f))
	  x->lambda = f;
  else
	  post("mtx_dispersive_dline: stable allpass coefficient must be -1<lambda<1");
}

static void mtx_dispersive_dline_reset(t_mtx_dispersive_dline *x)
{
   int count;
   for (count=0;count<x->size;count++) {
      x->tap[count]=0;
      x->z[count]=0;
   }
}

static void mtx_dispersive_dline_delete(t_mtx_dispersive_dline *x)
{
      if(x->list_out)freebytes(x->list_out, sizeof(t_atom)*(x->size+2));
      if(x->tap)freebytes(x->tap, sizeof(t_float)*x->size);
      if(x->z)freebytes(x->z, sizeof(t_float)*x->size);
      x->z=0;
      x->tap=0;
      x->list_out=0;
}


static void mtx_dispersive_dline_resize(t_mtx_dispersive_dline *x, t_symbol *s,
      int argc, t_atom *argv)
{
   int length=(int)atom_getfloat(argv);
   int channels=x->channels;
   int size=length*channels;

   if (argc>1) {
      channels=(int)atom_getfloat(argv+1);
      size=length*channels;
      if ((channels<1)||(channels>1000)) {
	 post("mtx_dispersive_dline: number of channels (input rows) must lie between 1 and 1000!");
	 return; 
      }
   }

   if ((length<1)||(length>10000)) {
	  post("mtx_dispersive_dline: length not between 1 and 10000!");
	  return;
   }
   if ((x->size!=size)) {
      mtx_dispersive_dline_delete(x);
      if(!(x->list_out=(t_atom*) getbytes(sizeof(t_atom)*(size+2)))) {
	 post("mtx_dispersive_dline: out of memory");
	 mtx_dispersive_dline_delete(x);
	 return;
      }
      if(!(x->tap=(t_float*) getbytes(sizeof(t_float)*size))) {
	 post("mtx_dispersive_dline: out of memory");
	 mtx_dispersive_dline_delete(x);
	 return;
      }
      if(!(x->z = (t_float*) getbytes(sizeof(t_float)*size))) {
	 post("mtx_dispersive_dline: out of memory");
	 mtx_dispersive_dline_delete(x);
	 return;
      }
      x->length=length;
      x->channels=channels;
      x->size=size;
   }
}

static allpass_chain_cycle (t_float x, t_float *y, t_float *z, int n, t_float a) {
	t_float w, in;
        int c;
	in = y[0] = x;
        // z[0] unused here
	for (c=1; c<n; c++) {
		w  = in + a * z[c]; 
		in = y[c] = z[c] - a * w;
		z[c] = w;
	}
}

static mtx_dispersive_dline_matrix(t_mtx_dispersive_dline *x, t_symbol *s,
      int argc, t_atom *argv)
{
  int channels=(int)atom_getfloat(argv);
  int samples=(int)atom_getfloat(argv+1);
  int c,n,n2;
  t_atom resize_msg[2];
  
  if (channels*samples>argc) {
     post("mtx_dispersive_dline: corrupt matrix passed");
     return;
  }
  post("%d samples, %d channels",samples,channels);

  SETFLOAT(resize_msg,(t_float)x->length);
  SETFLOAT(resize_msg+1,(t_float)channels);
  mtx_dispersive_dline_resize(x,gensym("resize"),2,resize_msg);

  post("%d new size",x->size);

  argv+=2;
  for (c=0, n2=0; c<x->size; c+=x->length) {
     for (n=0; n<samples; n++, n2++) {
         allpass_chain_cycle (atom_getfloat(argv+n2),x->tap+c,x->z+c,x->length,x->lambda); 
     }
  }

  mtx_dispersive_dline_bang(x);
}

static void mtx_dispersive_dline_helper(void)
{
  post("\n%c mtx_dispersive_dline~-object for warping a signal");
  post("'help' : view this\n"
       "signal~");
  post("outlet : signal~");
}

static void *mtx_dispersive_dline_new(t_symbol *s, int argc, t_atom *argv)
{
  t_mtx_dispersive_dline *x = (t_mtx_dispersive_dline *)pd_new(mtx_dispersive_dline_class);
  x->list_outlet = outlet_new(&x->x_obj, &s_list);
  t_float length=1;
  t_float lambda=0;
  t_atom resize_msg[2];
  switch ((argc>2)?2:argc)
  {
     case 2:
	lambda=atom_getfloat(argv+1);
     case 1:
	length=atom_getfloat(argv);
  }
   
  x->length=0;
  x->channels=0;
  x->size=0;
  x->z=0;
  x->tap=0;
  x->list_out=0;
  mtx_dispersive_dline_set_lambda (x,lambda);
  SETFLOAT(resize_msg,(t_float)length);
  SETFLOAT(resize_msg+1,(t_float)1);
  mtx_dispersive_dline_resize (x,gensym("resize"),2,resize_msg);
  mtx_dispersive_dline_reset (x);
  return (x);
}

void mtx_dispersive_dline_setup(void)
{
  mtx_dispersive_dline_class = class_new(
	gensym("mtx_dispersive_dline"), 
	(t_newmethod)mtx_dispersive_dline_new, 
	(t_method)mtx_dispersive_dline_delete,
	sizeof(t_mtx_dispersive_dline),
	CLASS_DEFAULT,
	A_GIMME,0);
  class_addmethod (mtx_dispersive_dline_class, (t_method) mtx_dispersive_dline_matrix, 
	gensym("matrix"),A_GIMME,0);
  class_addmethod (mtx_dispersive_dline_class, (t_method) mtx_dispersive_dline_reset, 
	gensym("reset"), 0); 
  class_addmethod (mtx_dispersive_dline_class, (t_method) mtx_dispersive_dline_resize, 
	gensym("resize"), A_GIMME,0); 
  class_addmethod (mtx_dispersive_dline_class, (t_method) mtx_dispersive_dline_set_lambda, 
	gensym("lambda"), A_DEFFLOAT,0); 
 
  class_addmethod(mtx_dispersive_dline_class, (t_method)mtx_dispersive_dline_helper, gensym("help"), 0);
}

void iemtx_dispersive_dline_setup(void)
{
    mtx_dispersive_dline_setup();
}
