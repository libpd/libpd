/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_t3_lib written by Gerhard Eckel, Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"


static t_class *iem_t3_lib_class;

static void *iem_t3_lib_new(void)
{
  t_object *x = (t_object *)pd_new(iem_t3_lib_class);
  
  return (x);
}

void t3_bpe_setup(void);
void t3_delay_setup(void);
void t3_line_tilde_setup(void);
void t3_metro_setup(void);
void t3_sig_tilde_setup(void);
void t3_timer_setup(void);

/* ------------------------ setup routine ------------------------- */

void iem_t3_lib_setup(void)
{
  iem_t3_lib_class = class_new(gensym("iem_t3_lib"), iem_t3_lib_new, 0,
    sizeof(t_object), CLASS_NOINLET, 0);
  
  t3_bpe_setup();
  t3_delay_setup();
  t3_line_tilde_setup();
  t3_metro_setup();
  t3_sig_tilde_setup();
  t3_timer_setup();
  
	post("iem_t3_lib (R-1.17) library loaded!   (c) Gerhard Eckel, Thomas Musil 11.2006");
	post("   musil%ciem.at iem KUG Graz Austria", '@');
}
