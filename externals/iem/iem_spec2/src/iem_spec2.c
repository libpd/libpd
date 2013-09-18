/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_spec2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2009 */


#include "m_pd.h"
#include "iemlib.h"

static t_class *iem_spec2_class;

static void *iem_spec2_new(void)
{
  t_object *x = (t_object *)pd_new(iem_spec2_class);
  
  return (x);
}

void spec2_1p1z_freq_tilde_setup(void);
void spec2_1p1z_time_tilde_setup(void);
void spec2_abs_tilde_setup(void);
void spec2_add_scalar_tilde_setup(void);
void spec2_add_tilde_setup(void);
void spec2_block_delay_tilde_setup(void);
void spec2_clip_max_tilde_setup(void);
void spec2_clip_min_tilde_setup(void);
void spec2_dbtopow_tilde_setup(void);
void spec2_dbtorms_tilde_setup(void);
void spec2_matrix_bundle_stat_tilde_setup(void);
void spec2_mul_scalar_tilde_setup(void);
void spec2_mul_tilde_setup(void);
void spec2_powtodb_tilde_setup(void);
void spec2_rmstodb_tilde_setup(void);
void spec2_shift_tilde_setup(void);
void spec2_sqrt_tilde_setup(void);
void spec2_stretch_tilde_setup(void);
void spec2_sub_tilde_setup(void);
void spec2_sum_tilde_setup(void);
void spec2_tab_conv_tilde_setup(void);
void spec2_tabreceive_enable_tilde_setup(void);
void spec2_tabreceive_tilde_setup(void);

/* ------------------------ setup routine ------------------------- */

void iem_spec2_setup(void)
{
  iem_spec2_class = class_new(gensym("iem_spec2"), iem_spec2_new, 0,
    sizeof(t_object), CLASS_NOINLET, 0);
  
  spec2_1p1z_freq_tilde_setup();
  spec2_1p1z_time_tilde_setup();
  spec2_abs_tilde_setup();
  spec2_add_scalar_tilde_setup();
  spec2_add_tilde_setup();
  spec2_block_delay_tilde_setup();
  spec2_clip_max_tilde_setup();
  spec2_clip_min_tilde_setup();
  spec2_dbtopow_tilde_setup();
  spec2_dbtorms_tilde_setup();
  spec2_matrix_bundle_stat_tilde_setup();
  spec2_mul_scalar_tilde_setup();
  spec2_mul_tilde_setup();
  spec2_powtodb_tilde_setup();
  spec2_rmstodb_tilde_setup();
  spec2_shift_tilde_setup();
  spec2_sqrt_tilde_setup();
  spec2_stretch_tilde_setup();
  spec2_sub_tilde_setup();
  spec2_sum_tilde_setup();
  spec2_tab_conv_tilde_setup();
  spec2_tabreceive_enable_tilde_setup();
  spec2_tabreceive_tilde_setup();
  
  post("iem_spec2 (R-1.18) library loaded!   (c) Thomas Musil 01.2009");
  post("   musil%ciem.at iem KUG Graz Austria", '@');
}
