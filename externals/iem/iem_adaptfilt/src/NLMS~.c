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


/* ----------------------- NLMS~ ------------------------------ */
/* -- Normalized Least Mean Square (linear adaptive FIR-filter) -- */
/* -- first input:  reference signal -- */
/* -- second input: desired signal -- */
/* --  -- */

/* for further information on adaptive filter design we refer to */
/* [1] Haykin, "Adaptive Filter Theory", 4th ed, Prentice Hall */
/* [2] Benesty, "Adaptive Signal Processing", Springer */


typedef struct NLMS_tilde
{
    t_object            x_obj;
    t_symbol            *x_w_array_sym_name;
    t_float             *x_w_array_mem_beg;
    t_float             *x_io_ptr_beg[4];// memory: 2 sig-in and 2 sig-out vectors
    t_float             *x_in_hist;// start point double buffer for sig-in history
    t_int               x_rw_index;// read-write-index
    t_int               x_n_order;// order of filter
    t_int               x_update;// 2^n rounded value, downsampling of update speed
    t_float             x_beta;// learn rate [0 .. 2]
    t_float             x_gamma;// regularization
    t_float             x_msi;
} t_NLMS_tilde;

t_class *NLMS_tilde_class;

static t_float *NLMS_tilde_check_array(t_symbol *array_sym_name, t_int length)
{
    int n_points;
    t_garray *a;
    t_float *vec;

    if(!(a = (t_garray *)pd_findbyclass(array_sym_name, garray_class)))
    {
        error("%s: no such array for NLMS~", array_sym_name->s_name);
        return((t_float *)0);
    }
    else if(!garray_getfloatarray(a, &n_points, &vec))
    {
        error("%s: bad template for NLMS~", array_sym_name->s_name);
        return((t_float *)0);
    }
    else if(n_points < length)
    {
        error("%s: bad array-size for NLMS~: %d", array_sym_name->s_name, n_points);
        return((t_float *)0);
    }
    else
    {
        return(vec);
    }
}

static void NLMS_tilde_beta(t_NLMS_tilde *x, t_floatarg f) // learn rate
{
    if(f < 0.0f)
        f = 0.0f;
    if(f > 2.0f)
        f = 2.0f;
    
    x->x_beta = f;
}

static void NLMS_tilde_gamma(t_NLMS_tilde *x, t_floatarg f) // regularization factor (dither)
{
    if(f < 0.0f)
        f = 0.0f;
    if(f > 1.0f)
        f = 1.0f;
    
    x->x_gamma = f;
}


static void NLMS_tilde_update(t_NLMS_tilde *x, t_floatarg f) // downsample learn-rate
{
    t_int i=1, u = (t_int)f;
    
    if(u < 0)
        u = 0;
    else
    {
        while(i <= u)   // convert u for 2^N
            i *= 2;     // round downwards
        i /= 2;
        u = i;
    }
    x->x_update = u;
}

/* ============== DSP ======================= */

static t_int *NLMS_tilde_perform_zero(t_int *w)
{
    t_NLMS_tilde *x = (t_NLMS_tilde *)(w[1]);
    t_int n = (t_int)(w[2]);
    
    t_float **io = x->x_io_ptr_beg;
    t_float *out;
    t_int i, j;
    
    for(j=0; j<2; j++)/* output-vector-row */
    {
        out = io[j+2];
        for(i=0; i<n; i++)
        {
            *out++ = 0.0f;
        }
    }
    return (w+3);
}

static t_int *NLMS_tilde_perform(t_int *w)
{
    t_NLMS_tilde *x = (t_NLMS_tilde *)(w[1]);
    t_int n = (t_int)(w[2]);
    t_int n_order = x->x_n_order;   /* number of filter-order */
    t_int rw_index = x->x_rw_index;
    t_float *in = x->x_io_ptr_beg[0];// first sig in
    t_float *desired_in = x->x_io_ptr_beg[1], din;// second sig in
    t_float *filt_out = x->x_io_ptr_beg[2];// first sig out
    t_float *err_out = x->x_io_ptr_beg[3], eout;// second sig out
    t_float *write_in_hist1 = x->x_in_hist;
    t_float *write_in_hist2 = write_in_hist1+n_order;
    t_float *read_in_hist = write_in_hist2;
    t_float *w_filt_coeff = x->x_w_array_mem_beg;
    t_float my, my_err, sum;
    t_float beta = x->x_beta;
    t_float gammax = x->x_gamma;
    t_int i, j, update_counter;
    t_int update = x->x_update;
    t_int ord8=n_order&0xfffffff8;
    t_int ord_residual=n_order&0x7;
    
    if(!w_filt_coeff)
        goto NLMS_tildeperfzero;// this is quick&dirty Musil/Miller style
    
    for(i=0, update_counter=0; i<n; i++)// store history and convolve
    {
        write_in_hist1[rw_index] = in[i]; // save inputs to variable & history
        write_in_hist2[rw_index] = in[i];
        din = desired_in[i];
        
		// begin convolution
        sum = 0.0f;
        w_filt_coeff = x->x_w_array_mem_beg; // Musil's special convolution buffer struct
        read_in_hist = &write_in_hist2[rw_index];
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
        
        filt_out[i] = sum;
        eout = din - filt_out[i];	// buffer-struct for further use
        err_out[i] = eout;
        
        if(update)	// downsampling for learn rate
        {
            update_counter++;
            if(update_counter >= update)
            {
                update_counter = 0;
                
                sum = 0.0f;// calculate energy for last n-order samples in filter
                read_in_hist = &write_in_hist2[rw_index];
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
                sum += gammax * gammax * (float)n_order; // convert gammax corresponding to filter order
                my = beta / sum;// calculate mue
                
                
                my_err = my * eout;
                w_filt_coeff = x->x_w_array_mem_beg; // coefficient constraints
                read_in_hist = &write_in_hist2[rw_index];
                for(j=0; j<n_order; j++) // without unroll
                    w_filt_coeff[j] += read_in_hist[-j] * my_err;
            }
        }
        rw_index++;
        if(rw_index >= n_order)
            rw_index -= n_order;
    }

    x->x_rw_index = rw_index; // back to start

    return(w+3);
    
NLMS_tildeperfzero:
    
    while(n--)
    {
        *filt_out++ = 0.0f;
        *err_out++ = 0.0f;
    }
    return(w+3);
}

static void NLMS_tilde_dsp(t_NLMS_tilde *x, t_signal **sp)
{
    t_int i, n = sp[0]->s_n;
    
    for(i=0; i<4; i++) // store io_vec
        x->x_io_ptr_beg[i] = sp[i]->s_vec;
    
    x->x_w_array_mem_beg = NLMS_tilde_check_array(x->x_w_array_sym_name, x->x_n_order);

    if(!x->x_w_array_mem_beg)
        dsp_add(NLMS_tilde_perform_zero, 2, x, n);
    else
        dsp_add(NLMS_tilde_perform, 2, x, n);
}


/* setup/setdown things */

static void NLMS_tilde_free(t_NLMS_tilde *x)
{
    freebytes(x->x_in_hist, 2*x->x_n_order*sizeof(t_float));
}

static void *NLMS_tilde_new(t_symbol *s, t_int argc, t_atom *argv)
{
    t_NLMS_tilde *x = (t_NLMS_tilde *)pd_new(NLMS_tilde_class);
    t_int i, n_order=39;
    t_symbol    *w_name;
    t_float beta=0.1f;
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
        if(n_order > 11111)
            n_order = 11111;
        
        inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
        outlet_new(&x->x_obj, &s_signal);
        outlet_new(&x->x_obj, &s_signal);
        
        x->x_msi = 0;
        x->x_n_order = n_order;
        x->x_update = 0;
        x->x_beta = beta;
        x->x_gamma = gammax;
        // 2 times in and one time desired_in memory allocation (history)
        x->x_in_hist = (t_float *)getbytes(2*x->x_n_order*sizeof(t_float));
        
        // table-symbols will be linked to their memory in future (dsp_routine)
        x->x_w_array_sym_name = gensym(w_name->s_name);
        x->x_w_array_mem_beg = (t_float *)0;
        
        return(x);
    }
    else
    {
        post("NLMS~-ERROR: need 3 float- + 1 symbol-arguments:");
        post("  order_of_filter + learnrate_beta + security_value + array_name_taps");
        return(0);
    }
}

void NLMS_tilde_setup(void)
{
    NLMS_tilde_class = class_new(gensym("NLMS~"), (t_newmethod)NLMS_tilde_new, (t_method)NLMS_tilde_free,
        sizeof(t_NLMS_tilde), 0, A_GIMME, 0);
    CLASS_MAINSIGNALIN(NLMS_tilde_class, t_NLMS_tilde, x_msi);
    class_addmethod(NLMS_tilde_class, (t_method)NLMS_tilde_dsp, gensym("dsp"), 0);
    class_addmethod(NLMS_tilde_class, (t_method)NLMS_tilde_update, gensym("update"), A_FLOAT, 0); // method: downsampling factor of learning (multiple of 2^N)
    class_addmethod(NLMS_tilde_class, (t_method)NLMS_tilde_beta, gensym("beta"), A_FLOAT, 0); //method: normalized learning rate
    class_addmethod(NLMS_tilde_class, (t_method)NLMS_tilde_gamma, gensym("gamma"), A_FLOAT, 0);   // method: dithering noise related to signal

    //class_sethelpsymbol(NLMS_tilde_class, gensym("iemhelp2/NLMS~"));
}
