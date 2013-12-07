/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

n_CLNLMS multichannel-constrained leaky normalized LMS algorithm
lib iem_adaptfilt written by Markus Noisternig & Thomas Musil 
noisternig_AT_iem.at; musil_AT_iem.at
(c) Institute of Electronic Music and Acoustics, Graz Austria 2005 */

#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif


#include "m_pd.h"
#include "iemlib.h"
#include <math.h>
#include <stdio.h>
#include <string.h>


/* ----------------------- n_CLNLMS~ ------------------------------ */
/* -- multiple Constraint LEAKY Normalized Least Mean Square (linear adaptive FIR-filter) -- */

//* -- first input:  reference signal -- */
/* -- second input: desired signal -- */
/* --  -- */

/* for further information on adaptive filter design we refer to */
/* [1] Haykin, "Adaptive Filter Theory", 4th ed, Prentice Hall */
/* [2] Benesty, "Adaptive Signal Processing", Springer */

typedef struct n_CLNLMS_tilde_kern
{
  t_symbol            *x_w_array_sym_name;
  t_float             *x_w_array_mem_beg;
  t_float             *x_in_ptr_beg;// memory: sig-in vector
  t_float             *x_out_ptr_beg;// memory: sig-out vector
  t_float             *x_in_hist;// start point double buffer for sig-in history
} t_n_CLNLMS_tilde_kern;


typedef struct n_CLNLMS_tilde
{
  t_object            x_obj;
  t_n_CLNLMS_tilde_kern   *x_my_kern;
  t_float             *x_des_in_ptr_beg;// memory: desired-in vector
  t_float             *x_err_out_ptr_beg;// memory: error-out vector
  t_int               x_n_io;// number of in-channels and filtered out-channels
  t_int               x_rw_index;// read-write-index
  t_int               x_n_order;// filter order 
  t_int               x_update;// rounded by 2^n, yields downsampling of learn-rate
  t_float             x_beta;// learn rate [0 .. 2]
  t_float             x_gamma;// normalization
  t_float             x_kappa;// constreint: treshold of energy (clipping)
  t_float             x_leakage;// leakage-Faktor for NLMS
  t_outlet            *x_out_compressing_bang;
  t_clock             *x_clock;
  t_float             x_msi;
} t_n_CLNLMS_tilde;

t_class *n_CLNLMS_tilde_class;

static void n_CLNLMS_tilde_tick(t_n_CLNLMS_tilde *x)
{
  outlet_bang(x->x_out_compressing_bang);
}

static t_float *n_CLNLMS_tilde_check_array(t_symbol *array_sym_name, t_int length)
{
  int n_points;
  t_garray *a;
  t_float *vec;
  
  if(!(a = (t_garray *)pd_findbyclass(array_sym_name, garray_class)))
  {
    error("%s: no such array for n_CLNLMS~", array_sym_name->s_name);
    return((t_float *)0);
  }
  else if(!garray_getfloatarray(a, &n_points, &vec))
  {
    error("%s: bad template for n_CLNLMS~", array_sym_name->s_name);
    return((t_float *)0);
  }
  else if(n_points < length)
  {
    error("%s: bad array-size for n_CLNLMS~: %d", array_sym_name->s_name, n_points);
    return((t_float *)0);
  }
  else
  {
    return(vec);
  }
}

static void n_CLNLMS_tilde_beta(t_n_CLNLMS_tilde *x, t_floatarg f) // learn rate
{
  if(f < 0.0f)
    f = 0.0f;
  if(f > 2.0f)
    f = 2.0f;
  
  x->x_beta = f;
}

static void n_CLNLMS_tilde_gamma(t_n_CLNLMS_tilde *x, t_floatarg f) // regularization (dither)
{
  if(f < 0.0f)
    f = 0.0f;
  if(f > 1.0f)
    f = 1.0f;
  
  x->x_gamma = f;
}

static void n_CLNLMS_tilde_kappa(t_n_CLNLMS_tilde *x, t_floatarg f) // threshold for w_coeff
{
  if(f < 0.0001f)
    f = 0.0001f;
  if(f > 10000.0f)
    f = 10000.0f;
  
  x->x_kappa = f;
}

static void n_CLNLMS_tilde_leakage(t_n_CLNLMS_tilde *x, t_floatarg f) // leakage of NLMS
{
  if(f < 0.0001f)
    f = 0.0001f;
  if(f > 1.0f)
    f = 1.0f;
  
  x->x_leakage = f;
}

static void n_CLNLMS_tilde_update(t_n_CLNLMS_tilde *x, t_floatarg f) // downsample learn rate
{
  t_int i=1, u = (t_int)f;
  
  if(u < 0)
    u = 0;
  else
  {
    while(i <= u)   // convert u for 2^N
      i *= 2;     // round down
    i /= 2;
    u = i;
  }
  x->x_update = u;
}

/* ============== DSP ======================= */

static t_int *n_CLNLMS_tilde_perform_zero(t_int *w)
{
  t_n_CLNLMS_tilde *x = (t_n_CLNLMS_tilde *)(w[1]);
  t_int n = (t_int)(w[2]);
  
  t_int n_io = x->x_n_io;
  t_float *out;
  t_int i, j;
  
  out = x->x_err_out_ptr_beg;
  for(i=0; i<n; i++)
    *out++ = 0.0f;
  for(j=0; j<n_io; j++)
  {
    out = x->x_my_kern[j].x_out_ptr_beg;
    for(i=0; i<n; i++)
      *out++ = 0.0f;
  }
  return (w+3);
}

static t_int *n_CLNLMS_tilde_perform(t_int *w)
{
  t_n_CLNLMS_tilde *x = (t_n_CLNLMS_tilde *)(w[1]);
  t_int n = (t_int)(w[2]);
  t_int n_order = x->x_n_order;   /* number of filter-order */
  t_int rw_index2, rw_index = x->x_rw_index;
  t_int n_io = x->x_n_io;
  t_float *in;// first sig in
  t_float din;// second sig in
  t_float *filt_out;// first sig out
  t_float *err_out, err_sum;// second sig out
  t_float *read_in_hist;
  t_float *w_filt_coeff;
  t_float my, my_err, sum;
  t_float beta = x->x_beta;
  t_float hgamma, gammax = x->x_gamma;
  t_float hkappa, kappa = x->x_kappa;
  t_float hleakage, leakage = x->x_leakage;
  t_int i, j, k, update_counter;
  t_int update = x->x_update;
  t_int ord8=n_order&0xfffffff8;
  t_int ord_residual=n_order&0x7;
  t_int compressed = 0;
  
  for(k=0; k<n_io; k++)
  {
    if(!x->x_my_kern[k].x_w_array_mem_beg)
      goto n_CLNLMS_tildeperfzero;// this is Musil/Miller style
  }

  hgamma = gammax * gammax * (float)n_order;
  //hkappa = kappa * kappa * (float)n_order;
  hkappa = kappa; // kappa regards to energy value, else use line above
  
  for(i=0, update_counter=0; i<n; i++)// history and (block-)convolution
  {
    rw_index2 = rw_index + n_order;

    for(k=0; k<n_io; k++)// times n_io
    {
      x->x_my_kern[k].x_in_hist[rw_index] = x->x_my_kern[k].x_in_ptr_beg[i]; // save inputs into variabel & history
      x->x_my_kern[k].x_in_hist[rw_index+n_order] = x->x_my_kern[k].x_in_ptr_beg[i];
    }
    din = x->x_des_in_ptr_beg[i];

// begin convolution
    err_sum = din;
    for(k=0; k<n_io; k++)// times n_io
    {
      sum = 0.0f;
      w_filt_coeff = x->x_my_kern[k].x_w_array_mem_beg; // Musil's special convolution buffer struct
      read_in_hist = &x->x_my_kern[k].x_in_hist[rw_index2];
      for(j=0; j<ord8; j+=8)	// loop unroll 8 taps
      {
        sum += w_filt_coeff[0] * read_in_hist[0];
        sum += w_filt_coeff[1] * read_in_hist[-1];
        sum += w_filt_coeff[2] * read_in_hist[-2];
        sum += w_filt_coeff[3] * read_in_hist[-3];
        sum += w_filt_coeff[4] * read_in_hist[-4];
        sum += w_filt_coeff[5] * read_in_hist[-5];
        sum += w_filt_coeff[6] * read_in_hist[-6];
        sum += w_filt_coeff[7] * read_in_hist[-7];
        w_filt_coeff += 8;
        read_in_hist -= 8;
      }
      for(j=0; j<ord_residual; j++)	// for filter order < 2^N
        sum += w_filt_coeff[j] * read_in_hist[-j];
        
      x->x_my_kern[k].x_out_ptr_beg[i] = sum;
      err_sum -= sum;
    }
    x->x_err_out_ptr_beg[i] = err_sum;
// end convolution

    if(update)	// downsampling of learn rate
    {
      update_counter++;
      if(update_counter >= update)
      {
        update_counter = 0;
        
        for(k=0; k<n_io; k++)// times n_io
        {
          sum = 0.0f;// calculate energy for last n-order samples in filter
          read_in_hist = &x->x_my_kern[k].x_in_hist[rw_index2];
          for(j=0; j<ord8; j+=8)	// unrolling quadrature calc
          {
            sum += read_in_hist[0] * read_in_hist[0];
            sum += read_in_hist[-1] * read_in_hist[-1];
            sum += read_in_hist[-2] * read_in_hist[-2];
            sum += read_in_hist[-3] * read_in_hist[-3];
            sum += read_in_hist[-4] * read_in_hist[-4];
            sum += read_in_hist[-5] * read_in_hist[-5];
            sum += read_in_hist[-6] * read_in_hist[-6];
            sum += read_in_hist[-7] * read_in_hist[-7];
            read_in_hist -= 8;
          }
          for(j=0; j<ord_residual; j++)	// residual
            sum += read_in_hist[-j] * read_in_hist[-j]; // [-j] only valid for Musil's double buffer structure
          sum += hgamma; // convert gammax corresponding to filter order
          my = beta / sum;// calculate mue

          my_err = my * err_sum;
          w_filt_coeff = x->x_my_kern[k].x_w_array_mem_beg;
          read_in_hist = &x->x_my_kern[k].x_in_hist[rw_index2];
          sum = 0.0f;
          for(j=0; j<ord8; j+=8)	// unrolling quadrature calc
          {
            w_filt_coeff[0] = leakage * w_filt_coeff[0] + read_in_hist[0] * my_err;
            sum += w_filt_coeff[0] * w_filt_coeff[0];
	    w_filt_coeff[1] = leakage * w_filt_coeff[1] + read_in_hist[-1] * my_err;
            sum += w_filt_coeff[1] * w_filt_coeff[1];
            w_filt_coeff[2] = leakage * w_filt_coeff[2] + read_in_hist[-2] * my_err;
            sum += w_filt_coeff[2] * w_filt_coeff[2];
	    w_filt_coeff[3] = leakage * w_filt_coeff[3] + read_in_hist[-3] * my_err;
            sum += w_filt_coeff[3] * w_filt_coeff[3];
            w_filt_coeff[4] = leakage * w_filt_coeff[4] + read_in_hist[-4] * my_err;
            sum += w_filt_coeff[4] * w_filt_coeff[4];
	    w_filt_coeff[5] = leakage * w_filt_coeff[5] + read_in_hist[-5] * my_err;
            sum += w_filt_coeff[5] * w_filt_coeff[5];
            w_filt_coeff[6] = leakage * w_filt_coeff[6] + read_in_hist[-6] * my_err;
            sum += w_filt_coeff[6] * w_filt_coeff[6];
	    w_filt_coeff[7] = leakage * w_filt_coeff[7] + read_in_hist[-7] * my_err;
            sum += w_filt_coeff[7] * w_filt_coeff[7];
            w_filt_coeff += 8;
            read_in_hist -= 8;
          }
          for(j=0; j<ord_residual; j++)	// residual
          {
            w_filt_coeff[j] = leakage * w_filt_coeff[j] + read_in_hist[-j] * my_err;
            sum += w_filt_coeff[j] * w_filt_coeff[j];
          }
          if(sum > hkappa)
          {
            compressed = 1;
            my = sqrt(hkappa/sum);
            w_filt_coeff = x->x_my_kern[k].x_w_array_mem_beg;
            for(j=0; j<ord8; j+=8)	// unrolling quadrature calc
            {
              w_filt_coeff[0] *= my;
              w_filt_coeff[1] *= my;
              w_filt_coeff[2] *= my;
              w_filt_coeff[3] *= my;
              w_filt_coeff[4] *= my;
              w_filt_coeff[5] *= my;
              w_filt_coeff[6] *= my;
              w_filt_coeff[7] *= my;
              w_filt_coeff += 8;
            }
            for(j=0; j<ord_residual; j++)	// residual
              w_filt_coeff[j] *= my;
          }
        }
      }
    }
    rw_index++;
    if(rw_index >= n_order)
      rw_index -= n_order;
  }


  x->x_rw_index = rw_index; // wieder in die garage stellen
  
  if(compressed)
    clock_delay(x->x_clock, 0);

  return(w+3);
  
n_CLNLMS_tildeperfzero:

  err_out = x->x_err_out_ptr_beg;
  for(i=0; i<n; i++)
    *err_out++ = 0.0f;
  for(j=0; j<n_io; j++)
  {
    filt_out = x->x_my_kern[j].x_out_ptr_beg;
    for(i=0; i<n; i++)
      *filt_out++ = 0.0f;
  }

  return(w+3);
}

static void n_CLNLMS_tilde_dsp(t_n_CLNLMS_tilde *x, t_signal **sp)
{
  t_int i, n = sp[0]->s_n;
  t_int ok_w = 1;
  t_int m = x->x_n_io;

  for(i=0; i<m; i++)
    x->x_my_kern[i].x_in_ptr_beg = sp[i]->s_vec;
  x->x_des_in_ptr_beg = sp[m]->s_vec;
  for(i=0; i<m; i++)
    x->x_my_kern[i].x_out_ptr_beg = sp[i+m+1]->s_vec;
  x->x_err_out_ptr_beg = sp[2*m+1]->s_vec;

  for(i=0; i<m; i++)
  {
    x->x_my_kern[i].x_w_array_mem_beg = n_CLNLMS_tilde_check_array(x->x_my_kern[i].x_w_array_sym_name, x->x_n_order);
    if(!x->x_my_kern[i].x_w_array_mem_beg)
      ok_w = 0;
  }
  
  if(!ok_w)
    dsp_add(n_CLNLMS_tilde_perform_zero, 2, x, n);
  else
    dsp_add(n_CLNLMS_tilde_perform, 2, x, n);
}


/* setup/setdown things */

static void n_CLNLMS_tilde_free(t_n_CLNLMS_tilde *x)
{
  t_int i, n_io=x->x_n_io, n_order=x->x_n_order;

  for(i=0; i<n_io; i++)
    freebytes(x->x_my_kern[i].x_in_hist, 2*x->x_n_order*sizeof(t_float));
  freebytes(x->x_my_kern, n_io*sizeof(t_n_CLNLMS_tilde_kern));
  
  clock_free(x->x_clock);
}

static void *n_CLNLMS_tilde_new(t_symbol *s, t_int argc, t_atom *argv)
{
  t_n_CLNLMS_tilde *x = (t_n_CLNLMS_tilde *)pd_new(n_CLNLMS_tilde_class);
  char buffer[400];
  int i;
  t_int n_order=39, n_io=1;
  t_symbol    *w_name;
  t_float beta=0.1f;
  t_float gammax=0.00001f;
  t_float kappa = 1.0f;
  t_float leakage = 0.99f;
  
  if((argc >= 7) &&
    IS_A_FLOAT(argv,0) &&   //IS_A_FLOAT/SYMBOL from iemlib.h
    IS_A_FLOAT(argv,1) &&
    IS_A_FLOAT(argv,2) &&
    IS_A_FLOAT(argv,3) &&
    IS_A_FLOAT(argv,4) &&
    IS_A_FLOAT(argv,5) &&
    IS_A_SYMBOL(argv,6))
  {
    n_io = (t_int)atom_getintarg(0, argc, argv);
    n_order = (t_int)atom_getintarg(1, argc, argv);
    beta    = (t_float)atom_getfloatarg(2, argc, argv);
    gammax  = (t_float)atom_getfloatarg(3, argc, argv);
    kappa   = (t_float)atom_getfloatarg(4, argc, argv);
    leakage   = (t_float)atom_getfloatarg(5, argc, argv);
    w_name  = (t_symbol *)atom_getsymbolarg(6, argc, argv);
    
    if(beta < 0.0f)
      beta = 0.0f;
    if(beta > 2.0f)
      beta = 2.0f;
    
    if(gammax < 0.0f)
      gammax = 0.0f;
    if(gammax > 1.0f)
      gammax = 1.0f;
    
    if(kappa < 0.0001f)
      kappa = 0.0001f;
    if(kappa > 10000.0f)
      kappa = 10000.0f;
    
    if(leakage < 0.0001f)
      leakage = 0.0001f;
    if(leakage > 1.0f)
      leakage = 1.0f;
      
    if(n_order < 2)
      n_order = 2;
    if(n_order > 11111)
      n_order = 11111;
    
    if(n_io < 1)
      n_io = 1;
    if(n_io > 60)
      n_io = 60;
    
    for(i=0; i<n_io; i++)
      inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    for(i=0; i<=n_io; i++)
      outlet_new(&x->x_obj, &s_signal);
    
    x->x_out_compressing_bang = outlet_new(&x->x_obj, &s_bang);
    
    x->x_msi = 0;
    x->x_n_io = n_io;
    x->x_n_order = n_order;
    x->x_update = 0;
    x->x_beta = beta;
    x->x_gamma = gammax;
    x->x_kappa = kappa;
    x->x_leakage = leakage;
    x->x_my_kern = (t_n_CLNLMS_tilde_kern *)getbytes(x->x_n_io*sizeof(t_n_CLNLMS_tilde_kern));
    for(i=0; i<n_io; i++)
    {
      sprintf(buffer, "%d_%s", i+1, w_name->s_name);
      x->x_my_kern[i].x_w_array_sym_name = gensym(buffer);
      x->x_my_kern[i].x_w_array_mem_beg = (t_float *)0;
      x->x_my_kern[i].x_in_hist = (t_float *)getbytes(2*x->x_n_order*sizeof(t_float));
    }
    x->x_clock = clock_new(x, (t_method)n_CLNLMS_tilde_tick);
    
    return(x);
  }
  else
  {
    post("n_CLNLMSC~-ERROR: need 6 float- + 1 symbol-arguments:");
    post("  number_of_filters + order_of_filters + learnrate_beta + security_value_gamma + threshold_kappa + leakage_factor_lambda + array_name_taps");
    return(0);
  }
}

void n_CLNLMS_tilde_setup(void)
{
  n_CLNLMS_tilde_class = class_new(gensym("n_CLNLMS~"), (t_newmethod)n_CLNLMS_tilde_new, (t_method)n_CLNLMS_tilde_free,
    sizeof(t_n_CLNLMS_tilde), 0, A_GIMME, 0);
  CLASS_MAINSIGNALIN(n_CLNLMS_tilde_class, t_n_CLNLMS_tilde, x_msi);
  class_addmethod(n_CLNLMS_tilde_class, (t_method)n_CLNLMS_tilde_dsp, gensym("dsp"), 0);
  class_addmethod(n_CLNLMS_tilde_class, (t_method)n_CLNLMS_tilde_update, gensym("update"), A_FLOAT, 0); // method: downsampling factor of learning (multiple of 2^N)
  class_addmethod(n_CLNLMS_tilde_class, (t_method)n_CLNLMS_tilde_beta, gensym("beta"), A_FLOAT, 0); //method: normalized learning rate
  class_addmethod(n_CLNLMS_tilde_class, (t_method)n_CLNLMS_tilde_gamma, gensym("gamma"), A_FLOAT, 0);   // method: dithering noise related to signal
  class_addmethod(n_CLNLMS_tilde_class, (t_method)n_CLNLMS_tilde_kappa, gensym("kappa"), A_FLOAT, 0);   // method: threshold for compressing w_coeff
  class_addmethod(n_CLNLMS_tilde_class, (t_method)n_CLNLMS_tilde_leakage, gensym("leakage"), A_FLOAT, 0);   // method: leakage factor [0 1] for w update

  //class_sethelpsymbol(n_CLNLMS_tilde_class, gensym("iemhelp2/n_CLNLMS~"));
}
