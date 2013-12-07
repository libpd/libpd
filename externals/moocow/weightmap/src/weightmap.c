/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: weightmap.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: gui-less finer-grained probability-to-integer-map for PD
 *
 *   - inspired in part by Yves Degoyon's 'probalizer' object
 *
 * Copyright (c) 2002-2009 Bryan Jurish.
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See file LICENSE for further informations on licensing terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *=============================================================================*/


#include <m_pd.h>
#include <math.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include "mooPdUtils.h"

/*--------------------------------------------------------------------
 * DEBUG
 *--------------------------------------------------------------------*/
//#define WEIGHTMAP_DEBUG 1


/*=====================================================================
 * Constants
 *=====================================================================*/
#define DEFAULT_WEIGHT_VALUE 0
#define DEFAULT_WEIGHT_MAX 100

/*=====================================================================
 * Structures and Types
 *=====================================================================*/

static char *weightmap_banner =
"\n"
"weightmap: stochastic selection v" PACKAGE_VERSION " by Bryan Jurish\n"
"weightmap: compiled by " PACKAGE_BUILD_USER " on " PACKAGE_BUILD_DATE "";

static t_class *weightmap_class;

typedef struct _weightmap
{
  t_object x_obj;         /* black magic (probably inheritance-related) */
  t_float x_wmax;         /* maximum expected input weight              */
  t_int x_nvalues;        /* number of stored values                    */
  t_float *x_weights;     /* weight of each event (0..x_wmax)           */
  t_float x_wsum;         /* sum of all weights (internal use only)     */
  t_outlet *x_dumpoutlet; /* outlet for 'dump' messages                 */  
} t_weightmap;



/*--------------------------------------------------------------------
 * FLOAT : the guts : handle incoming weights
 *--------------------------------------------------------------------*/
void weightmap_float(t_weightmap *x, t_floatarg f) {
  int i;
  float wt;

  // scale weight from (0..x->x_wmax) to (0..x->x_wsum)
  wt = (f * x->x_wsum) / x->x_wmax;

#ifdef WEIGHTMAP_DEBUG
  post("weightmap_debug : float : rescale(%f : 0..%f) = %f : 0..%f", f, x->x_wmax, wt, x->x_wsum);
#endif

  // find match
  for (i = 0; i < x->x_nvalues; i++) {
    wt -= x->x_weights[i];
    if (wt < 0) {
      // end of search : outlet current index (counting from 1, for compatibility)
      outlet_float(x->x_obj.ob_outlet, i+1);
      return;
    }
  }
  // no matching value found: ouput 0
  outlet_float(x->x_obj.ob_outlet, 0);
}


/*--------------------------------------------------------------------
 * map : set selected values (no resize)
 *--------------------------------------------------------------------*/
void weightmap_map(t_weightmap *x, MOO_UNUSED t_symbol *s, int argc, t_atom *argv) {
  int i, idx;
  float wt;

#ifdef WEIGHTMAP_DEBUG
  post("weightmap_debug : map : argc=%d", argc);
#endif

  for (i = 0; i < argc; i += 2) {
    // get index
    idx = atom_getint(argv+i);
    if (idx <= 0 || idx > x->x_nvalues) {
      // sanity check: index-range
      pd_error(x,"weightmap : map : index out of range : %d", idx);
      continue;
    }
    idx--;

    // get weight-value
    wt = atom_getfloatarg(i+1, argc, argv);

    // adjust sum
    x->x_wsum += wt - x->x_weights[idx];

    // assign weight
    x->x_weights[idx] = wt;
  }
}

/*--------------------------------------------------------------------
 * zero : zero the weight-vector
 *--------------------------------------------------------------------*/
void weightmap_zero(t_weightmap *x) {
  int i;
  // zero all weights
  for (i = 0; i < x->x_nvalues; i++) {
    *(x->x_weights+i) = 0;
  }
  // reset sum
  x->x_wsum = 0;
}


/*--------------------------------------------------------------------
 * set : set the entire weight-vector in one go (with possible resize)
 *--------------------------------------------------------------------*/
void weightmap_set(t_weightmap *x, MOO_UNUSED t_symbol *s, int argc, t_atom *argv) {
  int i;
  
  if (x->x_nvalues != argc) {
    // set number of elements
    x->x_nvalues = argc;
    if ( x->x_nvalues < 1 ) x->x_nvalues = 1;
    
    // (re-)allocate weights
    if (x->x_weights) {
      freebytes(x->x_weights, x->x_nvalues*sizeof(t_float));
    }
    x->x_weights = (t_float *)getbytes(x->x_nvalues*sizeof(t_float));
    if (!x->x_weights) {
      pd_error(x,"weightmap : failed to allocate new weight vector");
      return;
    }
  }

  // zero sum
  x->x_wsum = 0;

  // assign new weight-vector
  for (i = 0; i < x->x_nvalues; i++) {
    if (i >= argc) {
      // sanity check: existence
      x->x_weights[i] = DEFAULT_WEIGHT_VALUE;
    }
    else {
      // normal case: set weight value
      x->x_weights[i] = atom_getfloat(argv);
    }
    x->x_wsum += x->x_weights[i];
    argv++;
  }
}

/*--------------------------------------------------------------------
 * resize : reset number of values & re-allocate
 *--------------------------------------------------------------------*/
void weightmap_resize(t_weightmap *x, t_floatarg f) {
  int i;
  t_int old_nvalues = x->x_nvalues;
  t_float *old_weights = x->x_weights;

#ifdef WEIGHTMAP_DEBUG
  post("weightmap_debug : resize : size=%d", (int)f);
#endif

  // reset number of elements
  x->x_nvalues = f;
  if ( x->x_nvalues < 1 ) x->x_nvalues = 1;

  // allocate new weight-vector
  x->x_weights = (t_float *)getbytes(x->x_nvalues*sizeof(t_float));
  if (!x->x_weights) {
    pd_error(x,"weightmap : resize : failed to allocate new weight vector");
    return;
  }

  // zero sum
  x->x_wsum = 0;

  // copy old values
  for (i = 0; i < x->x_nvalues; i++) {
    if (i >= old_nvalues) {
      x->x_weights[i] = DEFAULT_WEIGHT_VALUE;
    } else {
      x->x_weights[i] = old_weights[i];
      x->x_wsum += old_weights[i];
    }
  }

  // free old vector
  freebytes(old_weights, old_nvalues*sizeof(t_float));
}


/*--------------------------------------------------------------------
 * max : set maximum expected input-weight
 *--------------------------------------------------------------------*/
void weightmap_max(t_weightmap *x, t_floatarg f) {
#ifdef WEIGHTMAP_DEBUG
  post("weightmap_debug : max : max=%f", f);
#endif
  if (f > 0) {
    x->x_wmax = f;
  } else {
    pd_error(x,"weightmap : invalid maximum weight : %f", f);
  }
}


/*--------------------------------------------------------------------
 * dump : outputs weight-vector as a list
 *--------------------------------------------------------------------*/
void weightmap_dump(t_weightmap *x) {
  int i;
  float f;
  t_atom *dumpus;

#ifdef WEIGHTMAP_DEBUG
  post("weightmap_debug : dump : sum=%f, max=%f", x->x_wsum, x->x_wmax);
#endif

  // allocate dump-list
  dumpus = (t_atom *)getbytes(x->x_nvalues*sizeof(t_atom));
  if (!dumpus) {
    pd_error(x,"weightmap : failed to allocate dump list");
    return;
  }

  // populate dump-list
  for (i = 0; i < x->x_nvalues; i++) {
    f = x->x_weights[i];
    SETFLOAT(dumpus+i, f);
  }

  // output dump-list
  outlet_list(x->x_dumpoutlet,
	      &s_list,
	      x->x_nvalues,
	      dumpus);

}


/*--------------------------------------------------------------------
 * new SIZE
 *--------------------------------------------------------------------*/
static void *weightmap_new(t_floatarg f, t_floatarg max)
{
  t_weightmap *x;
  int i;

  x = (t_weightmap *)pd_new(weightmap_class);

  // create float (index) outlet
  outlet_new(&x->x_obj, &s_float);
  // create list (dump) outlet
  x->x_dumpoutlet = outlet_new(&x->x_obj, &s_list);

  // set number of elements
  x->x_nvalues = f;
  if ( x->x_nvalues < 1 ) x->x_nvalues = 1;

  // set maximum expected input probability
  x->x_wmax = max;
  if (!x->x_wmax) x->x_wmax = DEFAULT_WEIGHT_MAX;

  // allocate weight-vector
  x->x_weights = (t_float *)getbytes(x->x_nvalues*sizeof(t_float));
  if (!x->x_weights) {
    pd_error(x,"weightmap : failed to allocate weight vector");
    return NULL;
  }

  // initialize weights
  for (i = 0; i < x->x_nvalues; i++) {
    *(x->x_weights+i) = DEFAULT_WEIGHT_VALUE;
  }
  // initialize sum
  x->x_wsum = DEFAULT_WEIGHT_VALUE * x->x_nvalues;

#ifdef WEIGHTMAP_DEBUG
  post("weightmap_debug : create : nvalues=%d , wmax=%f", x->x_nvalues, x->x_wmax);
#endif

  return (void *)x;
}

/*--------------------------------------------------------------------
 * free
 *--------------------------------------------------------------------*/
static void weightmap_free(t_weightmap *x) {
#ifdef WEIGHTMAP_DEBUG
  post("weightmap_debug : free");
#endif
  if (x->x_weights) {
    freebytes(x->x_weights, x->x_nvalues*sizeof(t_float));
  }
}

/*--------------------------------------------------------------------
 * setup
 *--------------------------------------------------------------------*/
void weightmap_setup(void) {
  post(weightmap_banner);

  weightmap_class = class_new(gensym("weightmap"),          /* name */
			      (t_newmethod)weightmap_new,   /* newmethod */
			      (t_method)weightmap_free,     /* freemethod */
			      sizeof(t_weightmap),          /* size */
			      0,                            /* flags */
			      A_DEFFLOAT,                   /* ARGLIST: arg1 (size) */
			      A_DEFFLOAT,                   /* ARGLIST: arg2 (max) */
			      0);                           /* ARGLIST: END */

  class_addmethod(weightmap_class, (t_method)weightmap_float,  &s_float,         A_FLOAT, 0);
  class_addmethod(weightmap_class, (t_method)weightmap_map,    gensym("map"),    A_GIMME, 0);
  class_addmethod(weightmap_class, (t_method)weightmap_zero,   gensym("zero"),   0);
  class_addmethod(weightmap_class, (t_method)weightmap_set,    gensym("set"),    A_GIMME, 0);
  class_addmethod(weightmap_class, (t_method)weightmap_resize, gensym("resize"), A_DEFFLOAT, 0);
  class_addmethod(weightmap_class, (t_method)weightmap_max,    gensym("max"),    A_DEFFLOAT, 0);
  class_addmethod(weightmap_class, (t_method)weightmap_dump,   gensym("dump"),   0);

  class_sethelpsymbol(weightmap_class, gensym("weightmap-help.pd"));
}
