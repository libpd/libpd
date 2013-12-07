/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_roomsim written by Thomas Musil (c) IEM KUG Graz Austria 2002 - 2006 */

#include "m_pd.h"
#include "iemlib.h"
#include <math.h>

/* -------------------------- early_reflections_2d ------------------------------ */
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



typedef struct _early_reflections_2d
{
  t_object  x_obj;
  t_atom    x_para_at[27];
  void      *x_direct_out;
  void      *x_early_out;
  void      *x_rev_out;
  t_symbol  *x_s_del0;
  t_symbol  *x_s_del1;
  t_symbol  *x_s_del2;
  t_symbol  *x_s_damp;
  t_symbol  *x_s_index_phi;
  t_symbol  *x_s_bundle;
  t_float   x_azimuth_denominator;
  t_float   x_azimuth_offset;
  t_float   x_room_x;
  t_float   x_room_y;
  t_float   x_head_x;
  t_float   x_head_y;
  int       x_n_src;
  int       x_bundle;
  t_float   x_src_x[30];
  t_float   x_src_y[30];
  t_float   x_r_ambi;
  t_float   x_speed;
  t_float   x_180_over_pi;
} t_early_reflections_2d;

static t_class *early_reflections_2d_class;

static t_float early_reflections_2d_calc_radius(t_floatarg r_ambi, t_floatarg dx, t_floatarg dy)
{
  t_float r = (t_float)sqrt(dx*dx + dy*dy);
  
  if(r < r_ambi)
    return(r_ambi);
  else
    return(r);
}

static t_float early_reflections_2d_calc_azimuth(t_floatarg x_180_over_pi, t_floatarg dx, t_floatarg dy)/*changes*/
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

static t_float early_reflections_2d_calc_bundle_index(t_floatarg phi)/*changes*/
{
  phi += 22.5f;
  if(phi >= 360.0f)
    phi -= 360.0f;
  
  if(phi <= 180.0f)
  {
    if(phi <= 90.0f)
    {
      if(phi <= 45.0f)/* 0 .. 45 */
        return(1.0f);
      else
        return(2.0f);
    }
    else
    {
      if(phi <= 135.0f)
        return(3.0f);
      else
        return(4.0f);
    }
  }
  else
  {
    if(phi <= 270.0f)
    {
      if(phi <= 225.0f)
        return(5.0f);
      else
        return(6.0f);
    }
    else
    {
      if(phi <= 315.0f)/* 270 .. 315 */
        return(7.0f);
      else
        return(8.0f);/* 315 .. 360 */
    }
  }
}

static void early_reflections_2d_doit(t_early_reflections_2d *x)
{
  t_atom *at;
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
  t_float phi[50];
  t_float rad[50];
  int n_src=x->x_n_src;
  int i;
  
  lx = x->x_room_x;
  wy = x->x_room_y;
  
  SETFLOAT(x->x_para_at, early_reflections_2d_calc_radius(r_ambi, lx, wy)*m2ms);
  outlet_anything(x->x_rev_out, x->x_s_del0, 1, x->x_para_at);
  
  for(i=0; i<n_src; i++)
  {
    diff_x = x->x_src_x[i] - x->x_head_x;
    diff_y = x->x_src_y[i] - x->x_head_y;
    sum_x = x->x_src_x[i] + x->x_head_x;
    sum_y = x->x_src_y[i] + x->x_head_y;
    
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
    
    rad[0] = early_reflections_2d_calc_radius(r_ambi, x0, y0_);
    rad[1] = early_reflections_2d_calc_radius(r_ambi, xp1, y0_);
    rad[2] = early_reflections_2d_calc_radius(r_ambi, x0, yp1);
    rad[3] = early_reflections_2d_calc_radius(r_ambi, xn1, y0_);
    rad[4] = early_reflections_2d_calc_radius(r_ambi, x0, yn1);
    rad[5] = early_reflections_2d_calc_radius(r_ambi, xp2, y0_);
    rad[6] = early_reflections_2d_calc_radius(r_ambi, x0, yp2);
    rad[7] = early_reflections_2d_calc_radius(r_ambi, xn2, y0_);
    rad[8] = early_reflections_2d_calc_radius(r_ambi, x0, yn2);
    rad[9] = early_reflections_2d_calc_radius(r_ambi, xp1, yp1);
    rad[10] = early_reflections_2d_calc_radius(r_ambi, xn1, yn1);
    rad[11] = early_reflections_2d_calc_radius(r_ambi, xp1, yn1);
    rad[12] = early_reflections_2d_calc_radius(r_ambi, xn1, yp1);
    
    /* delay-reihenfolge: 0,
    +1x, +1y, -1x, -1y
    +2x, +2y, -2x, -2y
    +1x+1y, -1x-1y
    +1x-1y, -1x+1y
    */
    
    at = x->x_para_at;
    SETFLOAT(at, (t_float)(i+1));/*changes*/
    at++;
    SETFLOAT(at, rad[0] * m2ms);
    outlet_anything(x->x_direct_out, x->x_s_del0, 2, x->x_para_at);
    
    at = x->x_para_at+1;
    SETFLOAT(at, rad[1] *m2ms);
    at++;
    SETFLOAT(at, rad[2] *m2ms);
    at++;
    SETFLOAT(at, rad[3] *m2ms);
    at++;
    SETFLOAT(at, rad[4] *m2ms);
    outlet_anything(x->x_early_out, x->x_s_del1, 5, x->x_para_at);
    
    at = x->x_para_at+1;
    SETFLOAT(at, rad[5] *m2ms);
    at++;
    SETFLOAT(at, rad[6] *m2ms);
    at++;
    SETFLOAT(at, rad[7] *m2ms);
    at++;
    SETFLOAT(at, rad[8] *m2ms);
    at++;
    SETFLOAT(at, rad[9] *m2ms);
    at++;
    SETFLOAT(at, rad[10] *m2ms);
    at++;
    SETFLOAT(at, rad[11] *m2ms);
    at++;
    SETFLOAT(at, rad[12] *m2ms);
    outlet_anything(x->x_early_out, x->x_s_del2, 9, x->x_para_at);
    
    
    /* daempfungs-reihenfolge:
    0,
    +1x, +1y, -1x, -1y
    +2x, +2y, -2x, -2y
    +1x+1y, -1x-1y
    +1x-1y, -1x+1y
    */
    
    at = x->x_para_at+1;
    SETFLOAT(at, r_ambi / rad[0]);
    outlet_anything(x->x_direct_out, x->x_s_damp, 2, x->x_para_at);
    
    at = x->x_para_at+1;
    SETFLOAT(at, r_ambi / rad[1]);
    at++;
    SETFLOAT(at, r_ambi / rad[2]);
    at++;
    SETFLOAT(at, r_ambi / rad[3]);
    at++;
    SETFLOAT(at, r_ambi / rad[4]);
    at++;
    SETFLOAT(at, r_ambi / rad[5]);
    at++;
    SETFLOAT(at, r_ambi / rad[6]);
    at++;
    SETFLOAT(at, r_ambi / rad[7]);
    at++;
    SETFLOAT(at, r_ambi / rad[8]);
    at++;
    SETFLOAT(at, r_ambi / rad[9]);
    at++;
    SETFLOAT(at, r_ambi / rad[10]);
    at++;
    SETFLOAT(at, r_ambi / rad[11]);
    at++;
    SETFLOAT(at, r_ambi / rad[12]);
    
    outlet_anything(x->x_early_out, x->x_s_damp, 13, x->x_para_at);
    
    
    /* encoder-winkel-reihenfolge: index delta phi
    0,
    +1x, +1y, -1x, -1y
    +2x, +2y, -2x, -2y
    +1x+1y, -1x-1y
    +1x-1y, -1x+1y
    */
    
    at = x->x_para_at+1;
    SETFLOAT(at, early_reflections_2d_calc_azimuth(x_180_over_pi, x0, y0_));
    
    outlet_anything(x->x_direct_out, x->x_s_index_phi, 2, x->x_para_at);
    
    /* encoder-winkel-reihenfolge: bundle
    0,
    +1x, +1y, -1x, -1y
    +2x, +2y, -2x, -2y
    +1x+1y, -1x-1y
    +1x-1y, -1x+1y
    */
    
    phi[0] = early_reflections_2d_calc_azimuth(x_180_over_pi, xp1, y0_);
    phi[1] = early_reflections_2d_calc_azimuth(x_180_over_pi, x0, yp1);
    phi[2] = early_reflections_2d_calc_azimuth(x_180_over_pi, xn1, y0_);
    phi[3] = early_reflections_2d_calc_azimuth(x_180_over_pi, x0, yn1);
    phi[4] = early_reflections_2d_calc_azimuth(x_180_over_pi, xp2, y0_);
    phi[5] = early_reflections_2d_calc_azimuth(x_180_over_pi, x0, yp2);
    phi[6] = early_reflections_2d_calc_azimuth(x_180_over_pi, xn2, y0_);
    phi[7] = early_reflections_2d_calc_azimuth(x_180_over_pi, x0, yn2);
    phi[8] = early_reflections_2d_calc_azimuth(x_180_over_pi, xp1, yp1);
    phi[9] = early_reflections_2d_calc_azimuth(x_180_over_pi, xn1, yn1);
    phi[10] = early_reflections_2d_calc_azimuth(x_180_over_pi, xp1, yn1);
    phi[11] = early_reflections_2d_calc_azimuth(x_180_over_pi, xn1, yp1);
    
    if(x->x_bundle)
    {
      at = x->x_para_at+1;
      SETFLOAT(at, early_reflections_2d_calc_bundle_index(phi[0]));
      at++;
      SETFLOAT(at, early_reflections_2d_calc_bundle_index(phi[1]));
      at++;
      SETFLOAT(at, early_reflections_2d_calc_bundle_index(phi[2]));
      at++;
      SETFLOAT(at, early_reflections_2d_calc_bundle_index(phi[3]));
      at++;
      SETFLOAT(at, early_reflections_2d_calc_bundle_index(phi[4]));
      at++;
      SETFLOAT(at, early_reflections_2d_calc_bundle_index(phi[5]));
      at++;
      SETFLOAT(at, early_reflections_2d_calc_bundle_index(phi[6]));
      at++;
      SETFLOAT(at, early_reflections_2d_calc_bundle_index(phi[7]));
      at++;
      SETFLOAT(at, early_reflections_2d_calc_bundle_index(phi[8]));
      at++;
      SETFLOAT(at, early_reflections_2d_calc_bundle_index(phi[9]));
      at++;
      SETFLOAT(at, early_reflections_2d_calc_bundle_index(phi[10]));
      at++;
      SETFLOAT(at, early_reflections_2d_calc_bundle_index(phi[11]));
      
      outlet_anything(x->x_early_out, x->x_s_bundle, 13, x->x_para_at);
    }
    
    at = x->x_para_at+1;
    SETFLOAT(at, 1.0f);
    at++;
    SETFLOAT(at, phi[0]);
    outlet_anything(x->x_early_out, x->x_s_index_phi, 3, x->x_para_at);
    
    at = x->x_para_at+1;
    SETFLOAT(at, 2.0f);
    at++;
    SETFLOAT(at, phi[1]);
    outlet_anything(x->x_early_out, x->x_s_index_phi, 3, x->x_para_at);
    
    at = x->x_para_at+1;
    SETFLOAT(at, 3.0f);
    at++;
    SETFLOAT(at, phi[2]);
    outlet_anything(x->x_early_out, x->x_s_index_phi, 3, x->x_para_at);
    
    at = x->x_para_at+1;
    SETFLOAT(at, 4.0f);
    at++;
    SETFLOAT(at, phi[3]);
    outlet_anything(x->x_early_out, x->x_s_index_phi, 3, x->x_para_at);
    
    at = x->x_para_at+1;
    SETFLOAT(at, 5.0f);
    at++;
    SETFLOAT(at, phi[4]);
    outlet_anything(x->x_early_out, x->x_s_index_phi, 3, x->x_para_at);
    
    at = x->x_para_at+1;
    SETFLOAT(at, 6.0f);
    at++;
    SETFLOAT(at, phi[5]);
    outlet_anything(x->x_early_out, x->x_s_index_phi, 3, x->x_para_at);
    
    at = x->x_para_at+1;
    SETFLOAT(at, 7.0f);
    at++;
    SETFLOAT(at, phi[6]);
    outlet_anything(x->x_early_out, x->x_s_index_phi, 3, x->x_para_at);
    
    at = x->x_para_at+1;
    SETFLOAT(at, 8.0f);
    at++;
    SETFLOAT(at, phi[7]);
    outlet_anything(x->x_early_out, x->x_s_index_phi, 3, x->x_para_at);
    
    at = x->x_para_at+1;
    SETFLOAT(at, 9.0f);
    at++;
    SETFLOAT(at, phi[8]);
    outlet_anything(x->x_early_out, x->x_s_index_phi, 3, x->x_para_at);
    
    at = x->x_para_at+1;
    SETFLOAT(at, 10.0f);
    at++;
    SETFLOAT(at, phi[9]);
    outlet_anything(x->x_early_out, x->x_s_index_phi, 3, x->x_para_at);
    
    at = x->x_para_at+1;
    SETFLOAT(at, 11.0f);
    at++;
    SETFLOAT(at, phi[10]);
    outlet_anything(x->x_early_out, x->x_s_index_phi, 3, x->x_para_at);
    
    at = x->x_para_at+1;
    SETFLOAT(at, 12.0f);
    at++;
    SETFLOAT(at, phi[11]);
    outlet_anything(x->x_early_out, x->x_s_index_phi, 3, x->x_para_at);
  }
}

static void early_reflections_2d_dump_para(t_early_reflections_2d *x)
{
  int i, n=x->x_n_src;
  
  post("*******************************************************************************");
  post("room-dimensions: L_x = %.3f, W_y = %.3f", x->x_room_x, x->x_room_y);
  post("hear-position: x_hear = %.3f, y_hear = %.3f", x->x_head_x, x->x_head_y);
  for(i=0; i<n; i++)
    post("source-coordinates: x_src%d = %.3f, y_src%d = %.3f",
    i+1, x->x_src_x[i], i+1, x->x_src_y[i]);
  post("ambisonic-radius: %f", x->x_r_ambi);
  post("sonic-speed: %.3f", x->x_speed);
  post("order of outputs: direct early rev");
  post("*******************************************************************************");
}

static void early_reflections_2d_para(t_early_reflections_2d *x, t_symbol *s, int argc, t_atom *argv)
{
  int i, n=x->x_n_src*2 + 5;/* r_ambi + 2*room + 2*head */
  
  if(argc != n)
  {
    post("early_reflections_2d ERROR: para needs 1 r_ambi + 2*room + 2*head +n*2*src");
    return;
  }
  
  x->x_r_ambi = atom_getfloat(argv++);
  x->x_room_x = atom_getfloat(argv++);
  x->x_room_y = atom_getfloat(argv++);
  x->x_head_x = atom_getfloat(argv++);
  x->x_head_y = atom_getfloat(argv++);
  n = x->x_n_src;
  for(i=0; i<n; i++)
  {
    x->x_src_x[i] = atom_getfloat(argv++);
    x->x_src_y[i] = atom_getfloat(argv++);
  }
  early_reflections_2d_doit(x);
}

static void early_reflections_2d_sonic_speed(t_early_reflections_2d *x, t_floatarg speed)
{
  if(speed < 300.0f)
    speed = 300.0f;
  if(speed > 400.0f)
    speed = 400.0f;
  x->x_speed = speed;
}

static void early_reflections_2d_bundle(t_early_reflections_2d *x, t_floatarg bundle)
{
  if(bundle == 0.0f)
    x->x_bundle = 0;
  else
    x->x_bundle = 1;
}

static void early_reflections_2d_free(t_early_reflections_2d *x)
{
}

static void *early_reflections_2d_new(t_floatarg fn_src)
{
  int i, n;
  t_early_reflections_2d *x = (t_early_reflections_2d *)pd_new(early_reflections_2d_class);
  
  n = (int)fn_src;
  if(n < 1)
    n = 1;
  if(n > 30)
    n = 30;
  x->x_n_src = n;
  x->x_room_x = 12.0f;
  x->x_room_y = 8.0f;
  x->x_head_x = 0.0f;
  x->x_head_y = 0.0f;
  for(i=0; i<n; i++)
  {
    x->x_src_x[i] = 3.0f;
    x->x_src_y[i] = 0.5f;
  }
  x->x_r_ambi = 1.4f;
  x->x_speed = 340.0f;
  
  x->x_s_del0 = gensym("del0");
  x->x_s_del1 = gensym("del1");
  x->x_s_del2 = gensym("del2");
  x->x_s_damp = gensym("damp");
  x->x_s_index_phi = gensym("index_phi");
  x->x_s_bundle = gensym("bundle");
  x->x_direct_out = outlet_new(&x->x_obj, &s_list);
  x->x_early_out = outlet_new(&x->x_obj, &s_list);
  x->x_rev_out = outlet_new(&x->x_obj, &s_list);
  x->x_180_over_pi  = (t_float)(180.0 / (4.0 * atan(1.0)));
  x->x_bundle = 0;
  return (x);
}

void early_reflections_2d_setup(void)
{
  early_reflections_2d_class = class_new(gensym("early_reflections_2d"), (t_newmethod)early_reflections_2d_new, (t_method)early_reflections_2d_free,
    sizeof(t_early_reflections_2d), 0, A_DEFFLOAT, 0);
  class_addmethod(early_reflections_2d_class, (t_method)early_reflections_2d_para, gensym("para"), A_GIMME, 0);
  class_addmethod(early_reflections_2d_class, (t_method)early_reflections_2d_sonic_speed, gensym("sonic_speed"), A_FLOAT, 0);
  class_addmethod(early_reflections_2d_class, (t_method)early_reflections_2d_bundle, gensym("bundle"), A_FLOAT, 0);
  class_addmethod(early_reflections_2d_class, (t_method)early_reflections_2d_dump_para, gensym("dump_para"), 0);
//  class_sethelpsymbol(early_reflections_2d_class, gensym("iemhelp2/early_reflections_2d-help"));
}
