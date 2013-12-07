/*  Lattice IIR filter from PARCOR coefficients
 *  Copyright (C) 2005 Nicolas Chetry <okin@altern.org>
 *  and Edward Kelly <morph_2016@yahoo.co.uk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "m_pd.h"

static t_class *lpreson_tilde_class;

typedef struct _lpreson_control
{
  t_float *c_residual, *c_output;
  t_atom *x_parcors;
  /*  t_atom x_parc_interp_one[MAXPOLES];
  t_int x_parc_ord_one;
  t_atom x_parc_interp_two[MAXPOLES];
  t_int x_parc_ord_two; */
} t_lpreson_control;

typedef struct _lpreson_tilde
{
  t_object x_obj;
  t_float f_dummy;
  t_int x_order;
  t_lpreson_control x_ctl;
} t_lpreson_tilde;

t_int *lpreson_tilde_perform(t_int *w)
{
  t_object x_obj;
  t_lpreson_tilde     *x =     (t_lpreson_tilde *)(w[1]);
  t_lpreson_control *ctl =   (t_lpreson_control *)(w[2]);
  int                  n =                   (int)(w[3]);
  t_float            *in = ctl->c_residual;
  t_float           *out = ctl->c_output;
  t_int              ord = x->x_order;
  float mem[ord+1];
/* 
 *  Do the inverse filtering 
 *  
 *  'data_in'     : residual signal (length 'len')
 *  'data_out'    : output frame (length 'len')
 *  'coeff'       : the parcor coefficients (length 'tap')
 *  'lattice_iir' : filter memory - Previously initialised using init_lattice_iir_filter()
 *
 */  
  int k, i;
  float sri;

  for (i=0;i<=ord;i++)
  {
    //    SETFLOAT (&schur->x_parcors[i],0);
    mem[i] = 0.0;
  }
  
  for (k = 0; k < n; k++ ) 
  {
    /* Synthesis filter - Lattice structure */
    sri  = in[k];
    for (i=0; i<ord; i++)
    {
      t_float parcor = atom_getfloatarg ((ord-1-i),ord,ctl->x_parcors);          
      sri =  sri -  parcor * mem[ord-1-i];
      mem[ord-i] = mem[ord-1-i] + parcor*sri;
    }
    out[k] = sri;
    mem[0] = sri;
            
  } /* next k */
  return(w+4);          
}

static void lpreson_tilde_list(t_lpreson_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
  if (argc) 
  {
    //    x->x_ctl.parc_interp_two = copybytes(x->x_ctl.parc_interp_one, x->x_ctl.parc_ord_one * sizeof(t_atom));
    //    x->x_ctl.parc_ord_two = x->x_ctl.parc_ord_one;
    //    x->x_ctl.parc_interp_one = copybytes(x->x_ctl.parcors, x->x_order * sizeof(t_atom));
    //    x->x_ctl.parc_ord_one = x->x_order;
    freebytes(x->x_ctl.x_parcors, x->x_order * sizeof(t_atom));
  }

  x->x_ctl.x_parcors = copybytes(argv, argc * sizeof(t_atom));
  x->x_order = argc;
}

void *lpreson_tilde_dsp(t_lpreson_tilde *x, t_signal **sp)
{
  x->x_ctl.c_residual = sp[0]->s_vec;
  x->x_ctl.c_output = sp[1]->s_vec;
  dsp_add(lpreson_tilde_perform, 3, x, &x->x_ctl, sp[0]->s_n);
  return (void *)x;
}

void *lpreson_tilde_new(t_floatarg f)
{
  t_lpreson_tilde *x = (t_lpreson_tilde *)pd_new(lpreson_tilde_class);
  x->x_order = f >= 1 ? (int)f : 5;
  
  outlet_new(&x->x_obj, &s_signal);
  return (void *)x;
}

void lpreson_tilde_setup(void)
{
  lpreson_tilde_class = class_new(gensym("lpreson~"), (t_newmethod)lpreson_tilde_new, 0, sizeof(t_lpreson_tilde), CLASS_DEFAULT, A_DEFFLOAT, 0);

  post(". . Lattice IIR filter for lpc. . . . . . .");
  post(". . by Nicolas Chetry <okin@altern.org> . .");
  post(". & Edward Kelly <morph_2016@yahoo.co.uk> .");
  
  class_addmethod(lpreson_tilde_class, (t_method)lpreson_tilde_dsp, gensym("dsp"), 0);

  class_addlist(lpreson_tilde_class, lpreson_tilde_list);

  CLASS_MAINSIGNALIN(lpreson_tilde_class, t_lpreson_tilde, f_dummy);
}
