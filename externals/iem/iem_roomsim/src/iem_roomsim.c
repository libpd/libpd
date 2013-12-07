/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_roomsim written by Thomas Musil (c) IEM KUG Graz Austria 2002 - 2006 */

#include "m_pd.h"
#include "iemlib.h"

static t_class *iem_roomsim_class;

static void *iem_roomsim_new(void)
{
  t_object *x = (t_object *)pd_new(iem_roomsim_class);
  
  return (x);
}

void early_reflections_3d_setup(void);
void early_reflections_2d_setup(void);
void cart2del_damp_2d_setup(void);
void cart2del_damp_3d_setup(void);

/* ------------------------ setup routine ------------------------- */

void iem_roomsim_setup(void)
{
  early_reflections_3d_setup();
  early_reflections_2d_setup();
  cart2del_damp_2d_setup();
  cart2del_damp_3d_setup();
  
  post("iem_roomsim (R-1.17) library loaded!   (c) Thomas Musil 11.2006");
  post("   musil%ciem.at iem KUG Graz Austria", '@');
}
