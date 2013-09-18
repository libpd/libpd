/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_roomsim written by Thomas Musil (c) IEM KUG Graz Austria 2002 - 2006 */

#include "m_pd.h"
#include "iemlib.h"
#include <math.h>


/* -------------------------- cart2del_damp_2d ------------------------------ */
/*
**  pos. x-Richtung Nase
**  pos. y-Richtung Linke Hand
**  pos. z-Richtung Scheitel
**  Kartesischer Koordinaten-Ursprung liegt in der Mitte des Raums am Boden

  aenderungen: src-index von 1 .. n auf 0 .. (n-1)
  aenderungen: azimuth von rad auf degree
*/

/*
Reihenfolge der bundle-sektoren: index phi:

  
    1 0
    2 45
    3 90
    4 135
    5 180
    6 225
    7 270
    8 315
    
    1. und 2. reflexionen:
      
        
          +x
          5
       9  1 11
+y  6  2  0  4  8
      12  3 10
          7
          
*/



typedef struct _cart2del_damp_2d
{
  t_object  x_obj;
  t_symbol  *x_s_direct;
  t_symbol  *x_s_early1;
  t_symbol  *x_s_early2;
  t_symbol  *x_s_del;
  t_symbol  *x_s_damp;
  t_symbol  *x_s_index_phi;
  t_float   x_room_x;
  t_float   x_room_y;
  t_float   x_head_x;
  t_float   x_head_y;
  t_float   x_src_x;
  t_float   x_src_y;
  t_float   x_r_ambi;
  t_float   x_speed;
  t_float   x_180_over_pi;
  void      *x_clock;
} t_cart2del_damp_2d;

static t_class *cart2del_damp_2d_class;

static t_float cart2del_damp_2d_calc_radius(t_floatarg r_ambi, t_floatarg dx, t_floatarg dy)
{
  t_float r = (t_float)sqrt(dx*dx + dy*dy);
  
  if(r < r_ambi)
    return(r_ambi);
  else
    return(r);
}

static t_float cart2del_damp_2d_calc_azimuth(t_floatarg x_180_over_pi, t_floatarg dx, t_floatarg dy)/*changes*/
{
  if(dx == 0.0f)
  {
    if(dy < 0.0f)
      return(270.0f);
    else
      return(90.0f);
  }
  else if(dx < 0.0f)
  {
    return(180.0f + x_180_over_pi * (t_float)atan(dy / dx));
  }
  else
  {
    if(dy < 0.0f)
      return(360.0f + x_180_over_pi * (t_float)atan(dy / dx));
    else
      return(x_180_over_pi * (t_float)atan(dy / dx));
  }
}

static void cart2del_damp_2d_doit(t_cart2del_damp_2d *x)
{
  t_float diff_x, diff_y;
  t_float sum_x, sum_y;
  t_float lx, wy;
  t_float x0, y0_;
  t_float xp1, yp1;
  t_float xn1, yn1;
  t_float xp2, yp2;
  t_float xn2, yn2;
  t_float m2ms = 1000.0f / x->x_speed;
  t_float x_180_over_pi=x->x_180_over_pi;
  t_float r_ambi = x->x_r_ambi;
  t_float rad[20];
  t_atom at[20];
  
  lx = x->x_room_x;
  wy = x->x_room_y;
  
  diff_x = x->x_src_x - x->x_head_x;
  diff_y = x->x_src_y - x->x_head_y;
  sum_x = x->x_src_x + x->x_head_x;
  sum_y = x->x_src_y + x->x_head_y;
  
  x0 = diff_x;
  y0_ = diff_y;
  xp1 = lx - sum_x;
  yp1 = wy - sum_y;
  xn1 = -lx - sum_x;
  yn1 = -wy - sum_y;
  xp2 = 2.0f*lx + diff_x;
  yp2 = 2.0f*wy + diff_y;
  xn2 = -2.0f*lx + diff_x;
  yn2 = -2.0f*wy + diff_y;
  
  rad[0] = cart2del_damp_2d_calc_radius(r_ambi, x0, y0_);
  rad[1] = cart2del_damp_2d_calc_radius(r_ambi, xp1, y0_);
  rad[2] = cart2del_damp_2d_calc_radius(r_ambi, x0, yp1);
  rad[3] = cart2del_damp_2d_calc_radius(r_ambi, xn1, y0_);
  rad[4] = cart2del_damp_2d_calc_radius(r_ambi, x0, yn1);
  rad[5] = cart2del_damp_2d_calc_radius(r_ambi, xp2, y0_);
  rad[6] = cart2del_damp_2d_calc_radius(r_ambi, x0, yp2);
  rad[7] = cart2del_damp_2d_calc_radius(r_ambi, xn2, y0_);
  rad[8] = cart2del_damp_2d_calc_radius(r_ambi, x0, yn2);
  rad[9] = cart2del_damp_2d_calc_radius(r_ambi, xp1, yp1);
  rad[10] = cart2del_damp_2d_calc_radius(r_ambi, xn1, yn1);
  rad[11] = cart2del_damp_2d_calc_radius(r_ambi, xp1, yn1);
  rad[12] = cart2del_damp_2d_calc_radius(r_ambi, xn1, yp1);
  
  /* delay-reihenfolge: 0,
  +1x, +1y, -1x, -1y
  +2x, +2y, -2x, -2y
  +1x+1y, -1x-1y
  +1x-1y, -1x+1y
  */
  
  SETSYMBOL(at, x->x_s_del);
  
  SETFLOAT(at+1, rad[0] * m2ms);
  outlet_anything(x->x_obj.ob_outlet, x->x_s_direct, 2, at);
  
  SETFLOAT(at+1, rad[1] *m2ms);
  SETFLOAT(at+2, rad[2] *m2ms);
  SETFLOAT(at+3, rad[3] *m2ms);
  SETFLOAT(at+4, rad[4] *m2ms);
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early1, 5, at);
  
  SETFLOAT(at+1, rad[5] *m2ms);
  SETFLOAT(at+2, rad[6] *m2ms);
  SETFLOAT(at+3, rad[7] *m2ms);
  SETFLOAT(at+4, rad[8] *m2ms);
  SETFLOAT(at+5, rad[9] *m2ms);
  SETFLOAT(at+6, rad[10] *m2ms);
  SETFLOAT(at+7, rad[11] *m2ms);
  SETFLOAT(at+8, rad[12] *m2ms);
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early2, 9, at);
  
  
  /* daempfungs-reihenfolge:
  0,
  +1x, +1y, -1x, -1y
  +2x, +2y, -2x, -2y
  +1x+1y, -1x-1y
  +1x-1y, -1x+1y
  */
  
  SETSYMBOL(at, x->x_s_damp);
  
  SETFLOAT(at+1, r_ambi / rad[0]);
  outlet_anything(x->x_obj.ob_outlet, x->x_s_direct, 2, at);
  
  SETFLOAT(at+1, r_ambi / rad[1]);
  SETFLOAT(at+2, r_ambi / rad[2]);
  SETFLOAT(at+3, r_ambi / rad[3]);
  SETFLOAT(at+4, r_ambi / rad[4]);
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early1, 5, at);
  
  SETFLOAT(at+1, r_ambi / rad[5]);
  SETFLOAT(at+2, r_ambi / rad[6]);
  SETFLOAT(at+3, r_ambi / rad[7]);
  SETFLOAT(at+4, r_ambi / rad[8]);
  SETFLOAT(at+5, r_ambi / rad[9]);
  SETFLOAT(at+6, r_ambi / rad[10]);
  SETFLOAT(at+7, r_ambi / rad[11]);
  SETFLOAT(at+8, r_ambi / rad[12]);
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early2, 9, at);
  
  
  /* encoder-winkel-reihenfolge: index delta phi
  0,
  +1x, +1y, -1x, -1y
  +2x, +2y, -2x, -2y
  +1x+1y, -1x-1y
  +1x-1y, -1x+1y
  */
  
  SETSYMBOL(at, x->x_s_index_phi);
  
  SETFLOAT(at+1, 1.0f);
  SETFLOAT(at+2, cart2del_damp_2d_calc_azimuth(x_180_over_pi, x0, y0_));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_direct, 3, at);
  
  
  SETFLOAT(at+1, 1.0f);
  SETFLOAT(at+2, cart2del_damp_2d_calc_azimuth(x_180_over_pi, xp1, y0_));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early1, 3, at);
  
  SETFLOAT(at+1, 2.0f);
  SETFLOAT(at+2, cart2del_damp_2d_calc_azimuth(x_180_over_pi, x0, yp1));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early1, 3, at);
  
  SETFLOAT(at+1, 3.0f);
  SETFLOAT(at+2, cart2del_damp_2d_calc_azimuth(x_180_over_pi, xn1, y0_));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early1, 3, at);
  
  SETFLOAT(at+1, 4.0f);
  SETFLOAT(at+2, cart2del_damp_2d_calc_azimuth(x_180_over_pi, x0, yn1));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early1, 3, at);
  
  
  SETFLOAT(at+1, 1.0f);
  SETFLOAT(at+2, cart2del_damp_2d_calc_azimuth(x_180_over_pi, xp2, y0_));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early2, 3, at);
  
  SETFLOAT(at+1, 2.0f);
  SETFLOAT(at+2, cart2del_damp_2d_calc_azimuth(x_180_over_pi, x0, yp2));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early2, 3, at);
  
  SETFLOAT(at+1, 3.0f);
  SETFLOAT(at+2, cart2del_damp_2d_calc_azimuth(x_180_over_pi, xn2, y0_));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early2, 3, at);
  
  SETFLOAT(at+1, 4.0f);
  SETFLOAT(at+2, cart2del_damp_2d_calc_azimuth(x_180_over_pi, x0, yn2));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early2, 3, at);
  
  SETFLOAT(at+1, 5.0f);
  SETFLOAT(at+2, cart2del_damp_2d_calc_azimuth(x_180_over_pi, xp1, yp1));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early2, 3, at);
  
  SETFLOAT(at+1, 6.0f);
  SETFLOAT(at+2, cart2del_damp_2d_calc_azimuth(x_180_over_pi, xn1, yn1));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early2, 3, at);
  
  SETFLOAT(at+1, 7.0f);
  SETFLOAT(at+2, cart2del_damp_2d_calc_azimuth(x_180_over_pi, xp1, yn1));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early2, 3, at);
  
  SETFLOAT(at+1, 8.0f);
  SETFLOAT(at+2, cart2del_damp_2d_calc_azimuth(x_180_over_pi, xn1, yp1));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early2, 3, at);
}

static void cart2del_damp_2d_src_xy(t_cart2del_damp_2d *x, t_symbol *s, int argc, t_atom *argv)
{
  if((argc >= 2)&&IS_A_FLOAT(argv, 0)&&IS_A_FLOAT(argv, 1))
  {
    t_float xr2=0.5f*x->x_room_x, yr2=0.5f*x->x_room_y;
    
    x->x_src_x = atom_getfloat(argv++);
    x->x_src_y = atom_getfloat(argv);
    if(x->x_src_x > xr2)
      x->x_src_x = xr2;
    if(x->x_src_x < -xr2)
      x->x_src_x = -xr2;
    if(x->x_src_y > yr2)
      x->x_src_y = yr2;
    if(x->x_src_y < -yr2)
      x->x_src_y = -yr2;
    clock_delay(x->x_clock, 0.0f);
  }
}

static void cart2del_damp_2d_head_xy(t_cart2del_damp_2d *x, t_symbol *s, int argc, t_atom *argv)
{
  if((argc >= 2)&&IS_A_FLOAT(argv, 0)&&IS_A_FLOAT(argv, 1))
  {
    t_float xr2=0.5f*x->x_room_x, yr2=0.5f*x->x_room_y;
    
    x->x_head_x = atom_getfloat(argv++);
    x->x_head_y = atom_getfloat(argv);
    if(x->x_head_x > xr2)
      x->x_head_x = xr2;
    if(x->x_head_x < -xr2)
      x->x_head_x = -xr2;
    if(x->x_head_y > yr2)
      x->x_head_y = yr2;
    if(x->x_head_y < -yr2)
      x->x_head_y = -yr2;
    clock_delay(x->x_clock, 0.0f);
  }
}

static void cart2del_damp_2d_room_dim(t_cart2del_damp_2d *x, t_symbol *s, int argc, t_atom *argv)
{
  if((argc >= 2)&&IS_A_FLOAT(argv, 0)&&IS_A_FLOAT(argv, 1))
  {
    x->x_room_x = atom_getfloat(argv++);
    x->x_room_y = atom_getfloat(argv);
    if(x->x_room_x < 0.5f)
      x->x_room_x = 0.5f;
    if(x->x_room_y < 0.5f)
      x->x_room_y = 0.5f;
    clock_delay(x->x_clock, 0.0f);
  }
}

static void cart2del_damp_2d_r_ambi(t_cart2del_damp_2d *x, t_floatarg r_ambi)
{
  if(r_ambi < 0.1f)
    r_ambi = 0.1f;
  x->x_r_ambi = r_ambi;
  clock_delay(x->x_clock, 0.0f);
}

static void cart2del_damp_2d_sonic_speed(t_cart2del_damp_2d *x, t_floatarg speed)
{
  if(speed < 10.0f)
    speed = 10.0f;
  if(speed > 2000.0f)
    speed = 2000.0f;
  x->x_speed = speed;
  clock_delay(x->x_clock, 0.0f);
}

static void cart2del_damp_2d_free(t_cart2del_damp_2d *x)
{
  clock_free(x->x_clock);
}

static void *cart2del_damp_2d_new(t_symbol *s, int argc, t_atom *argv)
{
  t_cart2del_damp_2d *x = (t_cart2del_damp_2d *)pd_new(cart2del_damp_2d_class);
  
  x->x_room_x = 12.0f;
  x->x_room_y = 8.0f;
  x->x_head_x = 0.0f;
  x->x_head_y = 0.0f;
  x->x_src_x = 3.0f;
  x->x_src_y = 0.5f;
  x->x_r_ambi = 1.4f;
  x->x_speed = 340.0f;
  x->x_s_direct = gensym("direct");
  x->x_s_early1 = gensym("early1");
  x->x_s_early2 = gensym("early2");
  x->x_s_damp = gensym("damp");
  x->x_s_del = gensym("del");
  x->x_s_index_phi = gensym("index_phi");
  outlet_new(&x->x_obj, &s_list);
  x->x_clock = clock_new(x, (t_method)cart2del_damp_2d_doit);
  x->x_180_over_pi  = (t_float)(180.0 / (4.0 * atan(1.0)));
  return (x);
}

void cart2del_damp_2d_setup(void)
{
  cart2del_damp_2d_class = class_new(gensym("cart2del_damp_2d"), (t_newmethod)cart2del_damp_2d_new, (t_method)cart2del_damp_2d_free,
    sizeof(t_cart2del_damp_2d), 0, A_GIMME, 0);
  class_addmethod(cart2del_damp_2d_class, (t_method)cart2del_damp_2d_src_xy, gensym("src_xy"), A_GIMME, 0);
  class_addmethod(cart2del_damp_2d_class, (t_method)cart2del_damp_2d_head_xy, gensym("head_xy"), A_GIMME, 0);
  class_addmethod(cart2del_damp_2d_class, (t_method)cart2del_damp_2d_room_dim, gensym("room_dim"), A_GIMME, 0);
  class_addmethod(cart2del_damp_2d_class, (t_method)cart2del_damp_2d_sonic_speed, gensym("sonic_speed"), A_FLOAT, 0);
  class_addmethod(cart2del_damp_2d_class, (t_method)cart2del_damp_2d_r_ambi, gensym("r_ambi"), A_FLOAT, 0);
//  class_sethelpsymbol(cart2del_damp_2d_class, gensym("iemhelp2/cart2del_damp_2d-help"));
}
