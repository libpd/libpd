/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

NLMS normalized least mean square (LMS) algorithm
lib iem_adaptfilt written by Markus Noisternig & Thomas Musil 
noisternig_AT_iem.at; musil_AT_iem.at
(c) Institute of Electronic Music and Acoustics, Graz Austria 2005 */


#include "m_pd.h"
#include "iemlib.h"
#include <math.h>
#include <stdio.h>
#include <string.h>


/* ----------------------- NLMSerr_in~ ------------------------------ */
/* -- Normalized Least Mean Square (linear adaptive FIR-filter) -- */
/* -- first input:  reference signal -- */
/* -- second input: desired signal -- */
/* -- the difference to NLMS~ is: we have only one ERROR input instead of desired in minus filter out  -- */
/* -- that means there is no feedback  -- */

/* for further information on adaptive filter design we refer to */
/* [1] Haykin, "Adaptive Filter Theory", 4th ed, Prentice Hall */
/* [2] Benesty, "Adaptive Signal Processing", Springer */


typedef struct NLMSerr_in_tilde
{
    t_object            x_obj;// common pd object structure
    t_symbol            *x_w_array_sym_name;
    t_float             *x_w_array_mem_beg;
    t_float             *x_ref_filt_in_hist;// history buffer for input signal reference for internal convolution = filter
    t_float             *x_ref_adapt_in_hist;// history buffer for input signal reference for internal adapting filter
    t_int               x_rw_index;// current read-write-index in circular buffer
    t_int               x_n_order;// order of filter or convolution
    t_int               x_update;// binary update parameter ON / OFF
    t_float             x_beta;// learn rate [0 .. 2]
    t_float             x_gamma;// regularization Parameter = minimum
    t_float             x_flt_sig_in1;
} t_NLMSerr_in_tilde;

t_class *NLMSerr_in_tilde_class;

static t_float *NLMSerr_in_tilde_check_array(t_symbol *array_sym_name, t_int length)
{
  int n_points;
  t_garray *a;
  t_float *vec;
  
  if(!(a = (t_garray *)pd_findbyclass(array_sym_name, garray_class)))
  {
    error("%s: no such array for NLMSerr_in~", array_sym_name->s_name);
    return((t_float *)0);
  }
  else if(!garray_getfloatarray(a, &n_points, &vec))
  {
    error("%s: bad template for NLMSerr_in~", array_sym_name->s_name);
    return((t_float *)0);
  }
  else if(n_points < length)
  {
    error("%s: bad array-size for NLMSerr_in~: %d", array_sym_name->s_name, n_points);
    return((t_float *)0);
  }
  else
  {
    return(vec);
  }
}

static void NLMSerr_in_tilde_beta(t_NLMSerr_in_tilde *x, t_floatarg f) // learn rate
{
  if(f < 0.0f)
    f = 0.0f;
  if(f > 2.0f)
    f = 2.0f;
  
  x->x_beta = f;
}

static void NLMSerr_in_tilde_gamma(t_NLMSerr_in_tilde *x, t_floatarg f) // regularization factor (dither)
{
  if(f < 0.0f)
    f = 0.0f;
  if(f > 1.0f)
    f = 1.0f;
  
  x->x_gamma = f;
}


static void NLMSerr_in_tilde_update(t_NLMSerr_in_tilde *x, t_floatarg f) // downsample learn-rate
{
  t_int i=1, u = (t_int)f;
  
  if(u != 0)
    u = 1;
  x->x_update = u;
}

/* ============== DSP ======================= */

static t_int *NLMSerr_in_tilde_perform_zero(t_int *w)
{
  t_NLMSerr_in_tilde *x = (t_NLMSerr_in_tilde *)(w[5]);
  t_int n = (t_int)(w[6]);
  t_float *filt_out = (t_float *)(w[4]);
  t_int i;
  
  for(i=0; i<n; i++)
  {
    *filt_out++ = 0.0f;
  }
  return (w+7);
}

static t_int *NLMSerr_in_tilde_perform(t_int *w)
{
  t_int n = (t_int)(w[6]);
  t_NLMSerr_in_tilde *x = (t_NLMSerr_in_tilde *)(w[5]);
  t_float *filt_out = (t_float *)(w[4]);// first sig out
  t_float *err_in = (t_float *)(w[3]);// third sig in
  t_float *ref_adapt_in = (t_float *)(w[2]);// second sig in
  t_float *ref_filt_in = (t_float *)(w[1]);// first sig in
  t_int n_order = x->x_n_order;   /* number of filter-order */
  t_int rw_index = x->x_rw_index;  /* current read write index in circular buffer */
  t_int update = x->x_update;
  t_float beta = x->x_beta;  /* learn rate */
  t_float gammax = x->x_gamma;  /* minimum energy */
  t_float my, my_err, sum, errin;
  t_int i, j, k;
  
  if(!x->x_w_array_mem_beg)
    goto NLMSerr_in_tildeperfzero;// this is quick&dirty Musil/Miller style
  
  for(i=0; i<n; i++)// store history and convolve
  {
    x->x_ref_filt_in_hist[rw_index] = ref_filt_in[i]; // inputs of ref_filt save to history
    x->x_ref_adapt_in_hist[rw_index] = ref_adapt_in[i]; // inputs of ref_adapt save to history
    errin = err_in[i];
    
		// begin convolution, filter : j++, k--, rw_index = aktueller index fuer lesen schreiben von history und convolution-beginn
    sum = 0.0f;
    k = rw_index;
    for(j=0; j<n_order; j++)
    {
      sum += x->x_w_array_mem_beg[j] * x->x_ref_filt_in_hist[k];
      k--;
      if(k < 0)
        k = n_order - 1;
    }
    filt_out[i] = sum;
    
    
    if(update)	// downsampling for learn rate
    {
      sum = 0.0f;// calculate energy for last n-order samples in filter
      k = rw_index;
      for(j=0; j<n_order; j++)	// unrolling quadrature calc
      {
        sum += x->x_ref_adapt_in_hist[k] * x->x_ref_adapt_in_hist[k]; // energie
        k--;
        if(k < 0)
          k = n_order - 1;
      }
      sum += gammax * gammax * (float)n_order; // convert gammax corresponding to filter order
      my = beta / sum;// calculate mue
      
      my_err = my * errin;
      
      k = rw_index;
      for(j=0; j<n_order; j++) // without unroll
      {
        x->x_w_array_mem_beg[j] += x->x_ref_adapt_in_hist[k] * my_err;
        k--;
        if(k < 0)
          k = n_order - 1;
      }
    }
    rw_index++;
    if(rw_index >= n_order)
      rw_index = 0;
  }
  x->x_rw_index = rw_index; // back to start
  return(w+7);
  
NLMSerr_in_tildeperfzero:
  
  while(n--)
  {
    *filt_out++ = 0.0f;
  }
  return(w+7);
}

static void NLMSerr_in_tilde_dsp(t_NLMSerr_in_tilde *x, t_signal **sp)
{
    x->x_w_array_mem_beg = NLMSerr_in_tilde_check_array(x->x_w_array_sym_name, x->x_n_order);

    if(!x->x_w_array_mem_beg)
        dsp_add(NLMSerr_in_tilde_perform_zero, 6, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, x, sp[0]->s_n);
    else
        dsp_add(NLMSerr_in_tilde_perform, 6, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, x, sp[0]->s_n);
}


/* setup/setdown things */

static void NLMSerr_in_tilde_free(t_NLMSerr_in_tilde *x)
{
    freebytes(x->x_ref_filt_in_hist, x->x_n_order*sizeof(t_float));
    freebytes(x->x_ref_adapt_in_hist, x->x_n_order*sizeof(t_float));
}

static void *NLMSerr_in_tilde_new(t_symbol *s, t_int argc, t_atom *argv)
{
    t_NLMSerr_in_tilde *x = (t_NLMSerr_in_tilde *)pd_new(NLMSerr_in_tilde_class);
    t_int i, n_order=39;
    t_symbol    *w_name;
    t_float beta=0.01f;
    t_float gammax=0.00001f;
    
    if((argc >= 4) &&
        IS_A_FLOAT(argv,0) &&   //IS_A_FLOAT/SYMBOL from iemlib.h
        IS_A_FLOAT(argv,1) &&
        IS_A_FLOAT(argv,2) &&
        IS_A_SYMBOL(argv,3))
    {
        n_order = (t_int)atom_getintarg(0, argc, argv);
        beta    = (t_float)atom_getfloatarg(1, argc, argv);
        gammax  = (t_float)atom_getfloatarg(2, argc, argv);
        w_name  = (t_symbol *)atom_getsymbolarg(3, argc, argv);
        
        if(beta < 0.0f)
            beta = 0.0f;
        if(beta > 2.0f)
            beta = 2.0f;
        
        if(gammax < 0.0f)
            gammax = 0.0f;
        if(gammax > 1.0f)
            gammax = 1.0f;
        
        if(n_order < 2)
            n_order = 2;
        if(n_order > 1111111)
            n_order = 1111111;
        
        inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
        inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
        outlet_new(&x->x_obj, &s_signal);
        
        x->x_flt_sig_in1 = 0;
        x->x_n_order = n_order;
        x->x_update = 0;
        x->x_beta = beta;
        x->x_gamma = gammax;
        // 2 times in and one time err_in memory allocation (history)
        x->x_ref_filt_in_hist = (t_float *)getbytes(x->x_n_order*sizeof(t_float));
        x->x_ref_adapt_in_hist = (t_float *)getbytes(x->x_n_order*sizeof(t_float));
        
        // table-symbols will be linked to their memory in future (dsp_routine)
        x->x_w_array_sym_name = gensym(w_name->s_name);
        x->x_w_array_mem_beg = (t_float *)0;
        
        x->x_rw_index = 0;
        
        return(x);
    }
    else
    {
        post("NLMSerr_in~-ERROR: need 3 float- + 1 symbol-arguments:");
        post("  order_of_filter + learnrate_beta + security_value + array_name_taps");
        return(0);
    }
}

void NLMSerr_in_tilde_setup(void)
{
    NLMSerr_in_tilde_class = class_new(gensym("NLMSerr_in~"), (t_newmethod)NLMSerr_in_tilde_new, (t_method)NLMSerr_in_tilde_free,
        sizeof(t_NLMSerr_in_tilde), 0, A_GIMME, 0);
    CLASS_MAINSIGNALIN(NLMSerr_in_tilde_class, t_NLMSerr_in_tilde, x_flt_sig_in1);
    class_addmethod(NLMSerr_in_tilde_class, (t_method)NLMSerr_in_tilde_dsp, gensym("dsp"), 0);
    class_addmethod(NLMSerr_in_tilde_class, (t_method)NLMSerr_in_tilde_update, gensym("update"), A_FLOAT, 0); // method: downsampling factor of learning (multiple of 2^N)
    class_addmethod(NLMSerr_in_tilde_class, (t_method)NLMSerr_in_tilde_beta, gensym("beta"), A_FLOAT, 0); //method: normalized learning rate
    class_addmethod(NLMSerr_in_tilde_class, (t_method)NLMSerr_in_tilde_gamma, gensym("gamma"), A_FLOAT, 0);   // method: dithering noise related to signal
}
