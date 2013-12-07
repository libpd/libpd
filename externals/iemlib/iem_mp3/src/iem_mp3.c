/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_mp3 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"

static t_class *iem_mp3_class;

static void *iem_mp3_new(void)
{
  t_object *x = (t_object *)pd_new(iem_mp3_class);
  
  return (x);
}

void mp3play_tilde_setup(void);

/* ------------------------ setup routine ------------------------- */

void iem_mp3_setup(void)
{
  iem_mp3_class = class_new(gensym("iem_mp3"), iem_mp3_new, 0,
    sizeof(t_object), CLASS_NOINLET, 0);
  
  mp3play_tilde_setup();
  
	post("iem_mp3 (R-1.17) library loaded!   (c) Thomas Musil 11.2006");
	post("   musil%ciem.at iem KUG Graz Austria", '@');
}
