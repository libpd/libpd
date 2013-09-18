/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_roomsim written by Thomas Musil (c) IEM KUG Graz Austria 2002 - 2006 */


#include "m_pd.h"
#include "iemlib.h"
#include <math.h>


/* -------------------------- cart2del_damp_3d ------------------------------ */
/*
**  pos. x-Richtung Nase
**  pos. y-Richtung Linke Hand
**  pos. z-Richtung Scheitel
**  Kartesischer Koordinaten-Ursprung liegt in der Mitte des Raums am Boden
*/

/*
Reihenfolge der bundle-sektoren: index delta phi:

  1 90 0
  2 45 45
  3 45 135
  4 45 225
  5 45 315
  6 0 0
  7 0 45
  8 0 90
  9 0 135
  10 0 180
  11 0 225
  12 0 270
  13 0 315
  14 -45 45
  15 -45 135
  16 -45 225
  17 -45 315
  
    
          top
      
        
           +x
          
            
  +y        9
              
                
                 
           +x
                    
           15
  +y    23  3 14
           24 
                      
                        
           +x
            8
        22  2 13
  +y 10  4  0  1  7
        16  5 19
           11
                          
           +x
                            
           21
  +y    17  6 20
           18 
                              
           +x
                                
                                  
  +y       12
                                    
                                      
                                        
*/



typedef struct _cart2del_damp_3d
{
  t_object  x_obj;
  t_symbol  *x_s_direct;
  t_symbol  *x_s_early1;
  t_symbol  *x_s_early2;
  t_symbol  *x_s_del;
  t_symbol  *x_s_damp;
  t_symbol  *x_s_index_theta_phi;
  t_float   x_room_x;
  t_float   x_room_y;
  t_float   x_room_z;
  t_float   x_head_x;
  t_float   x_head_y;
  t_float   x_head_z;
  t_float   x_src_x;
  t_float   x_src_y;
  t_float   x_src_z;
  t_float   x_r_ambi;
  t_float   x_speed;
  t_float   x_180_over_pi;
  void      *x_clock;
} t_cart2del_damp_3d;

static t_class *cart2del_damp_3d_class;

static t_float cart2del_damp_3d_calc_radius(t_floatarg r_ambi, t_floatarg dx, t_floatarg dy, t_floatarg dz)
{
  t_float r = (t_float)sqrt(dx*dx + dy*dy + dz*dz);
  
  if(r < r_ambi)
    return(r_ambi);
  else
    return(r);
}

static t_float cart2del_damp_3d_calc_azimuth(t_floatarg x_180_over_pi, t_floatarg dx, t_floatarg dy, t_floatarg dz)
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

static t_float cart2del_damp_3d_calc_elevation(t_floatarg x_180_over_pi, t_floatarg dx, t_floatarg dy, t_floatarg dz)
{
  t_float dxy = sqrt(dx*dx + dy*dy);
  
  if(dxy == 0.0f)
  {
    if(dz < 0.0f)
      return(-90.0f);
    else
      return(90.0f);
  }
  else
  {
    return(x_180_over_pi * (t_float)atan(dz / dxy));
  }
}

static void cart2del_damp_3d_doit(t_cart2del_damp_3d *x)
{
  t_float diff_x, diff_y, diff_z;
  t_float sum_x, sum_y, sum_z;
  t_float lx, wy, hz;
  t_float x0, y0_, z0;
  t_float xp1, yp1, zp1;
  t_float xn1, yn1, zn1;
  t_float xp2, yp2, zp2;
  t_float xn2, yn2, zn2;
  t_float m2ms = 1000.0f / x->x_speed;
  t_float x_180_over_pi=x->x_180_over_pi;
  t_float r_ambi = x->x_r_ambi;
  t_float rad[30];
  t_atom at[30];
  
  lx = x->x_room_x;
  wy = x->x_room_y;
  hz = x->x_room_z;
  
  diff_x = x->x_src_x - x->x_head_x;
  diff_y = x->x_src_y - x->x_head_y;
  diff_z = x->x_src_z - x->x_head_z;
  sum_x = x->x_src_x + x->x_head_x;
  sum_y = x->x_src_y + x->x_head_y;
  sum_z = x->x_src_z + x->x_head_z - hz;
  
  x0 = diff_x;
  y0_ = diff_y;
  z0 = diff_z;
  xp1 = lx - sum_x;
  yp1 = wy - sum_y;
  zp1 = hz - sum_z;
  xn1 = -lx - sum_x;
  yn1 = -wy - sum_y;
  zn1 = -hz - sum_z;
  xp2 = 2.0f*lx + diff_x;
  yp2 = 2.0f*wy + diff_y;
  zp2 = 2.0f*hz + diff_z;
  xn2 = -2.0f*lx + diff_x;
  yn2 = -2.0f*wy + diff_y;
  zn2 = -2.0f*hz + diff_z;
  
  rad[0] = cart2del_damp_3d_calc_radius(r_ambi, x0, y0_, z0);
  rad[1] = cart2del_damp_3d_calc_radius(r_ambi, xp1, y0_, z0);
  rad[2] = cart2del_damp_3d_calc_radius(r_ambi, x0, yp1, z0);
  rad[3] = cart2del_damp_3d_calc_radius(r_ambi, x0, y0_, zp1);
  rad[4] = cart2del_damp_3d_calc_radius(r_ambi, xn1, y0_, z0);
  rad[5] = cart2del_damp_3d_calc_radius(r_ambi, x0, yn1, z0);
  rad[6] = cart2del_damp_3d_calc_radius(r_ambi, x0, y0_, zn1);
  rad[7] = cart2del_damp_3d_calc_radius(r_ambi, xp2, y0_, z0);
  rad[8] = cart2del_damp_3d_calc_radius(r_ambi, x0, yp2, z0);
  rad[9] = cart2del_damp_3d_calc_radius(r_ambi, x0, y0_, zp2);
  rad[10] = cart2del_damp_3d_calc_radius(r_ambi, xn2, y0_, z0);
  rad[11] = cart2del_damp_3d_calc_radius(r_ambi, x0, yn2, z0);
  rad[12] = cart2del_damp_3d_calc_radius(r_ambi, x0, y0_, zn2);
  rad[13] = cart2del_damp_3d_calc_radius(r_ambi, xp1, yp1, z0);
  rad[14] = cart2del_damp_3d_calc_radius(r_ambi, xp1, y0_, zp1);
  rad[15] = cart2del_damp_3d_calc_radius(r_ambi, x0, yp1, zp1);
  rad[16] = cart2del_damp_3d_calc_radius(r_ambi, xn1, yn1, z0);
  rad[17] = cart2del_damp_3d_calc_radius(r_ambi, xn1, y0_, zn1);
  rad[18] = cart2del_damp_3d_calc_radius(r_ambi, x0, yn1, zn1);
  rad[19] = cart2del_damp_3d_calc_radius(r_ambi, xp1, yn1, z0);
  rad[20] = cart2del_damp_3d_calc_radius(r_ambi, xp1, y0_, zn1);
  rad[21] = cart2del_damp_3d_calc_radius(r_ambi, x0, yp1, zn1);
  rad[22] = cart2del_damp_3d_calc_radius(r_ambi, xn1, yp1, z0);
  rad[23] = cart2del_damp_3d_calc_radius(r_ambi, xn1, y0_, zp1);
  rad[24] = cart2del_damp_3d_calc_radius(r_ambi, x0, yn1, zp1);
  
  /* delay-reihenfolge: 0 auslassen,
  +1x, +1y, +1z, -1x, -1y, -1z
  +2x, +2y, +2z, -2x, -2y, -2z
  +1x+1y, +1x+1z, +1y+1z
  -1x-1y, -1x-1z, -1y-1z
  +1x-1y, +1x-1z, +1y-1z
  -1x+1y, -1x+1z, -1y+1z
  */
  
  SETSYMBOL(at, x->x_s_del);
  
  SETFLOAT(at+1, rad[0] * m2ms);
  outlet_anything(x->x_obj.ob_outlet, x->x_s_direct, 2, at);
  
  SETFLOAT(at+1, rad[1] *m2ms);
  SETFLOAT(at+2, rad[2] *m2ms);
  SETFLOAT(at+3, rad[3] *m2ms);
  SETFLOAT(at+4, rad[4] *m2ms);
  SETFLOAT(at+5, rad[5] *m2ms);
  SETFLOAT(at+6, rad[6] *m2ms);
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early1, 7, at);
  
  SETFLOAT(at+1, rad[7] *m2ms);
  SETFLOAT(at+2, rad[8] *m2ms);
  SETFLOAT(at+3, rad[9] *m2ms);
  SETFLOAT(at+4, rad[10] *m2ms);
  SETFLOAT(at+5, rad[11] *m2ms);
  SETFLOAT(at+6, rad[12] *m2ms);
  SETFLOAT(at+7, rad[13] *m2ms);
  SETFLOAT(at+8, rad[14] *m2ms);
  SETFLOAT(at+9, rad[15] *m2ms);
  SETFLOAT(at+10, rad[16] *m2ms);
  SETFLOAT(at+11, rad[17] *m2ms);
  SETFLOAT(at+12, rad[18] *m2ms);
  SETFLOAT(at+13, rad[19] *m2ms);
  SETFLOAT(at+14, rad[20] *m2ms);
  SETFLOAT(at+15, rad[21] *m2ms);
  SETFLOAT(at+16, rad[22] *m2ms);
  SETFLOAT(at+17, rad[23] *m2ms);
  SETFLOAT(at+18, rad[24] *m2ms);
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early2, 19, at);
  
  
  /* daempfungs-reihenfolge:
  0,
  +1x, +1y, +1z, -1x, -1y, -1z
  
    +2x, +2y, +2z, -2x, -2y, -2z
    +1x+1y, +1x+1z, +1y+1z
    -1x-1y, -1x-1z, -1y-1z
    +1x-1y, +1x-1z, +1y-1z
    -1x+1y, -1x+1z, -1y+1z
  */
  
  SETSYMBOL(at, x->x_s_damp);
  
  SETFLOAT(at+1, r_ambi / rad[0]);
  outlet_anything(x->x_obj.ob_outlet, x->x_s_direct, 2, at);
  
  SETFLOAT(at+1, r_ambi / rad[1]);
  SETFLOAT(at+2, r_ambi / rad[2]);
  SETFLOAT(at+3, r_ambi / rad[3]);
  SETFLOAT(at+4, r_ambi / rad[4]);
  SETFLOAT(at+5, r_ambi / rad[5]);
  SETFLOAT(at+6, r_ambi / rad[6]);
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early1, 7, at);
  
  SETFLOAT(at+1, r_ambi / rad[7]);
  SETFLOAT(at+2, r_ambi / rad[8]);
  SETFLOAT(at+3, r_ambi / rad[9]);
  SETFLOAT(at+4, r_ambi / rad[10]);
  SETFLOAT(at+5, r_ambi / rad[11]);
  SETFLOAT(at+6, r_ambi / rad[12]);
  SETFLOAT(at+7, r_ambi / rad[13]);
  SETFLOAT(at+8, r_ambi / rad[14]);
  SETFLOAT(at+9, r_ambi / rad[15]);
  SETFLOAT(at+10, r_ambi / rad[16]);
  SETFLOAT(at+11, r_ambi / rad[17]);
  SETFLOAT(at+12, r_ambi / rad[18]);
  SETFLOAT(at+13, r_ambi / rad[19]);
  SETFLOAT(at+14, r_ambi / rad[20]);
  SETFLOAT(at+15, r_ambi / rad[21]);
  SETFLOAT(at+16, r_ambi / rad[22]);
  SETFLOAT(at+17, r_ambi / rad[23]);
  SETFLOAT(at+18, r_ambi / rad[24]);
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early2, 19, at);
  
  
  /* encoder-winkel-reihenfolge: index delta phi
  0,
  +1x, +1y, +1z, -1x, -1y, -1z
  +2x, +2y, +2z, -2x, -2y, -2z
  +1x+1y, +1x+1z, +1y+1z
  -1x-1y, -1x-1z, -1y-1z
  +1x-1y, +1x-1z, +1y-1z
  -1x+1y, -1x+1z, -1y+1z
  */
  
  SETSYMBOL(at, x->x_s_index_theta_phi);
  
  SETFLOAT(at+1, 1.0f);
  SETFLOAT(at+2, cart2del_damp_3d_calc_elevation(x_180_over_pi, x0, y0_, z0));
  SETFLOAT(at+3, cart2del_damp_3d_calc_azimuth(x_180_over_pi, x0, y0_, z0));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_direct, 4, at);
  
  
  SETFLOAT(at+1, 1.0f);
  SETFLOAT(at+2, cart2del_damp_3d_calc_elevation(x_180_over_pi, xp1, y0_, z0));
  SETFLOAT(at+3, cart2del_damp_3d_calc_azimuth(x_180_over_pi, xp1, y0_, z0));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early1, 4, at);
  
  SETFLOAT(at+1, 2.0f);
  SETFLOAT(at+2, cart2del_damp_3d_calc_elevation(x_180_over_pi, x0, yp1, z0));
  SETFLOAT(at+3, cart2del_damp_3d_calc_azimuth(x_180_over_pi, x0, yp1, z0));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early1, 4, at);
  
  SETFLOAT(at+1, 3.0f);
  SETFLOAT(at+2, cart2del_damp_3d_calc_elevation(x_180_over_pi, x0, y0_, zp1));
  SETFLOAT(at+3, cart2del_damp_3d_calc_azimuth(x_180_over_pi, x0, y0_, zp1));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early1, 4, at);
  
  SETFLOAT(at+1, 4.0f);
  SETFLOAT(at+2, cart2del_damp_3d_calc_elevation(x_180_over_pi, xn1, y0_, z0));
  SETFLOAT(at+3, cart2del_damp_3d_calc_azimuth(x_180_over_pi, xn1, y0_, z0));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early1, 4, at);
  
  SETFLOAT(at+1, 5.0f);
  SETFLOAT(at+2, cart2del_damp_3d_calc_elevation(x_180_over_pi, x0, yn1, z0));
  SETFLOAT(at+3, cart2del_damp_3d_calc_azimuth(x_180_over_pi, x0, yn1, z0));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early1, 4, at);
  
  SETFLOAT(at+1, 6.0f);
  SETFLOAT(at+2, cart2del_damp_3d_calc_elevation(x_180_over_pi, x0, y0_, zn1));
  SETFLOAT(at+3, cart2del_damp_3d_calc_azimuth(x_180_over_pi, x0, y0_, zn1));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early1, 4, at);
  
  
  SETFLOAT(at+1, 1.0f);
  SETFLOAT(at+2, cart2del_damp_3d_calc_elevation(x_180_over_pi, xp2, y0_, z0));
  SETFLOAT(at+3, cart2del_damp_3d_calc_azimuth(x_180_over_pi, xp2, y0_, z0));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early2, 4, at);
  
  SETFLOAT(at+1, 2.0f);
  SETFLOAT(at+2, cart2del_damp_3d_calc_elevation(x_180_over_pi, x0, yp2, z0));
  SETFLOAT(at+3, cart2del_damp_3d_calc_azimuth(x_180_over_pi, x0, yp2, z0));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early2, 4, at);
  
  SETFLOAT(at+1, 3.0f);
  SETFLOAT(at+2, cart2del_damp_3d_calc_elevation(x_180_over_pi, x0, y0_, zp2));
  SETFLOAT(at+3, cart2del_damp_3d_calc_azimuth(x_180_over_pi, x0, y0_, zp2));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early2, 4, at);
  
  SETFLOAT(at+1, 4.0f);
  SETFLOAT(at+2, cart2del_damp_3d_calc_elevation(x_180_over_pi, xn2, y0_, z0));
  SETFLOAT(at+3, cart2del_damp_3d_calc_azimuth(x_180_over_pi, xn2, y0_, z0));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early2, 4, at);
  
  SETFLOAT(at+1, 5.0f);
  SETFLOAT(at+2, cart2del_damp_3d_calc_elevation(x_180_over_pi, x0, yn2, z0));
  SETFLOAT(at+3, cart2del_damp_3d_calc_azimuth(x_180_over_pi, x0, yn2, z0));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early2, 4, at);
  
  SETFLOAT(at+1, 6.0f);
  SETFLOAT(at+2, cart2del_damp_3d_calc_elevation(x_180_over_pi, x0, y0_, zn2));
  SETFLOAT(at+3, cart2del_damp_3d_calc_azimuth(x_180_over_pi, x0, y0_, zn2));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early2, 4, at);
  
  SETFLOAT(at+1, 7.0f);
  SETFLOAT(at+2, cart2del_damp_3d_calc_elevation(x_180_over_pi, xp1, yp1, z0));
  SETFLOAT(at+3, cart2del_damp_3d_calc_azimuth(x_180_over_pi, xp1, yp1, z0));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early2, 4, at);
  
  SETFLOAT(at+1, 8.0f);
  SETFLOAT(at+2, cart2del_damp_3d_calc_elevation(x_180_over_pi, xp1, y0_, zp1));
  SETFLOAT(at+3, cart2del_damp_3d_calc_azimuth(x_180_over_pi, xp1, y0_, zp1));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early2, 4, at);
  
  SETFLOAT(at+1, 9.0f);
  SETFLOAT(at+2, cart2del_damp_3d_calc_elevation(x_180_over_pi, x0, yp1, zp1));
  SETFLOAT(at+3, cart2del_damp_3d_calc_azimuth(x_180_over_pi, x0, yp1, zp1));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early2, 4, at);
  
  SETFLOAT(at+1, 10.0f);
  SETFLOAT(at+2, cart2del_damp_3d_calc_elevation(x_180_over_pi, xn1, yn1, z0));
  SETFLOAT(at+3, cart2del_damp_3d_calc_azimuth(x_180_over_pi, xn1, yn1, z0));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early2, 4, at);
  
  SETFLOAT(at+1, 11.0f);
  SETFLOAT(at+2, cart2del_damp_3d_calc_elevation(x_180_over_pi, xn1, y0_, zn1));
  SETFLOAT(at+3, cart2del_damp_3d_calc_azimuth(x_180_over_pi, xn1, y0_, zn1));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early2, 4, at);
  
  SETFLOAT(at+1, 12.0f);
  SETFLOAT(at+2, cart2del_damp_3d_calc_elevation(x_180_over_pi, x0, yn1, zn1));
  SETFLOAT(at+3, cart2del_damp_3d_calc_azimuth(x_180_over_pi, x0, yn1, zn1));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early2, 4, at);
  
  SETFLOAT(at+1, 13.0f);
  SETFLOAT(at+2, cart2del_damp_3d_calc_elevation(x_180_over_pi, xp1, yn1, z0));
  SETFLOAT(at+3, cart2del_damp_3d_calc_azimuth(x_180_over_pi, xp1, yn1, z0));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early2, 4, at);
  
  SETFLOAT(at+1, 14.0f);
  SETFLOAT(at+2, cart2del_damp_3d_calc_elevation(x_180_over_pi, xp1, y0_, zn1));
  SETFLOAT(at+3, cart2del_damp_3d_calc_azimuth(x_180_over_pi, xp1, y0_, zn1));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early2, 4, at);
  
  SETFLOAT(at+1, 15.0f);
  SETFLOAT(at+2, cart2del_damp_3d_calc_elevation(x_180_over_pi, x0, yp1, zn1));
  SETFLOAT(at+3, cart2del_damp_3d_calc_azimuth(x_180_over_pi, x0, yp1, zn1));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early2, 4, at);
  
  SETFLOAT(at+1, 16.0f);
  SETFLOAT(at+2, cart2del_damp_3d_calc_elevation(x_180_over_pi, xn1, yp1, z0));
  SETFLOAT(at+3, cart2del_damp_3d_calc_azimuth(x_180_over_pi, xn1, yp1, z0));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early2, 4, at);
  
  SETFLOAT(at+1, 17.0f);
  SETFLOAT(at+2, cart2del_damp_3d_calc_elevation(x_180_over_pi, xn1, y0_, zp1));
  SETFLOAT(at+3, cart2del_damp_3d_calc_azimuth(x_180_over_pi, xn1, y0_, zp1));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early2, 4, at);
  
  SETFLOAT(at+1, 18.0f);
  SETFLOAT(at+2, cart2del_damp_3d_calc_elevation(x_180_over_pi, x0, yn1, zp1));
  SETFLOAT(at+3, cart2del_damp_3d_calc_azimuth(x_180_over_pi, x0, yn1, zp1));
  outlet_anything(x->x_obj.ob_outlet, x->x_s_early2, 4, at);
}

static void cart2del_damp_3d_src_xyz(t_cart2del_damp_3d *x, t_symbol *s, int argc, t_atom *argv)
{
  if((argc >= 3)&&IS_A_FLOAT(argv, 0)&&IS_A_FLOAT(argv, 1)&&IS_A_FLOAT(argv, 2))
  {
    t_float xr2=0.5f*x->x_room_x, yr2=0.5f*x->x_room_y;
    
    x->x_src_x = atom_getfloat(argv++);
    x->x_src_y = atom_getfloat(argv++);
    x->x_src_z = atom_getfloat(argv);
    if(x->x_src_x > xr2)
      x->x_src_x = xr2;
    if(x->x_src_x < -xr2)
      x->x_src_x = -xr2;
    if(x->x_src_y > yr2)
      x->x_src_y = yr2;
    if(x->x_src_y < -yr2)
      x->x_src_y = -yr2;
    if(x->x_src_z > x->x_room_z)
      x->x_src_z = x->x_room_z;
    if(x->x_src_z < 0.0f)
      x->x_src_z = 0.0f;
    clock_delay(x->x_clock, 0.0f);
  }
}

static void cart2del_damp_3d_head_xyz(t_cart2del_damp_3d *x, t_symbol *s, int argc, t_atom *argv)
{
  if((argc >= 3)&&IS_A_FLOAT(argv, 0)&&IS_A_FLOAT(argv, 1)&&IS_A_FLOAT(argv, 2))
  {
    t_float xr2=0.5f*x->x_room_x, yr2=0.5f*x->x_room_y;
    
    x->x_head_x = atom_getfloat(argv++);
    x->x_head_y = atom_getfloat(argv++);
    x->x_head_z = atom_getfloat(argv);
    if(x->x_head_x > xr2)
      x->x_head_x = xr2;
    if(x->x_head_x < -xr2)
      x->x_head_x = -xr2;
    if(x->x_head_y > yr2)
      x->x_head_y = yr2;
    if(x->x_head_y < -yr2)
      x->x_head_y = -yr2;
    if(x->x_head_z > x->x_room_z)
      x->x_head_z = x->x_room_z;
    if(x->x_head_z < 0.0f)
      x->x_head_z = 0.0f;
    clock_delay(x->x_clock, 0.0f);
  }
}

static void cart2del_damp_3d_room_dim(t_cart2del_damp_3d *x, t_symbol *s, int argc, t_atom *argv)
{
  if((argc >= 3)&&IS_A_FLOAT(argv, 0)&&IS_A_FLOAT(argv, 1)&&IS_A_FLOAT(argv, 2))
  {
    x->x_room_x = atom_getfloat(argv++);
    x->x_room_y = atom_getfloat(argv++);
    x->x_room_z = atom_getfloat(argv);
    if(x->x_room_x < 0.5f)
      x->x_room_x = 0.5f;
    if(x->x_room_y < 0.5f)
      x->x_room_y = 0.5f;
    if(x->x_room_z < 0.5f)
      x->x_room_z = 0.5f;
    clock_delay(x->x_clock, 0.0f);
  }
}

static void cart2del_damp_3d_r_ambi(t_cart2del_damp_3d *x, t_floatarg r_ambi)
{
  if(r_ambi < 0.1f)
    r_ambi = 0.1f;
  x->x_r_ambi = r_ambi;
  clock_delay(x->x_clock, 0.0f);
}

static void cart2del_damp_3d_sonic_speed(t_cart2del_damp_3d *x, t_floatarg speed)
{
  if(speed < 10.0f)
    speed = 10.0f;
  if(speed > 2000.0f)
    speed = 2000.0f;
  x->x_speed = speed;
  clock_delay(x->x_clock, 0.0f);
}

static void cart2del_damp_3d_free(t_cart2del_damp_3d *x)
{
  clock_free(x->x_clock);
}

static void *cart2del_damp_3d_new(t_symbol *s, int argc, t_atom *argv)
{
  t_cart2del_damp_3d *x = (t_cart2del_damp_3d *)pd_new(cart2del_damp_3d_class);
  
  x->x_room_x = 12.0f;
  x->x_room_y = 8.0f;
  x->x_room_z = 4.0f;
  x->x_head_x = 0.0f;
  x->x_head_y = 0.0f;
  x->x_head_z = 1.7f;
  x->x_src_x = 3.0f;
  x->x_src_y = 0.5f;
  x->x_src_z = 2.5f;
  x->x_r_ambi = 1.4f;
  x->x_speed = 340.0f;
  x->x_s_direct = gensym("direct");
  x->x_s_early1 = gensym("early1");
  x->x_s_early2 = gensym("early2");
  x->x_s_del = gensym("del");
  x->x_s_damp = gensym("damp");
  x->x_s_index_theta_phi = gensym("index_theta_phi");
  outlet_new(&x->x_obj, &s_list);
  x->x_clock = clock_new(x, (t_method)cart2del_damp_3d_doit);
  x->x_180_over_pi  = (t_float)(180.0 / (4.0 * atan(1.0)));
  return (x);
}

void cart2del_damp_3d_setup(void)
{
  cart2del_damp_3d_class = class_new(gensym("cart2del_damp_3d"), (t_newmethod)cart2del_damp_3d_new, (t_method)cart2del_damp_3d_free,
    sizeof(t_cart2del_damp_3d), 0, A_GIMME, 0);
  class_addmethod(cart2del_damp_3d_class, (t_method)cart2del_damp_3d_src_xyz, gensym("src_xyz"), A_GIMME, 0);
  class_addmethod(cart2del_damp_3d_class, (t_method)cart2del_damp_3d_head_xyz, gensym("head_xyz"), A_GIMME, 0);
  class_addmethod(cart2del_damp_3d_class, (t_method)cart2del_damp_3d_room_dim, gensym("room_dim"), A_GIMME, 0);
  class_addmethod(cart2del_damp_3d_class, (t_method)cart2del_damp_3d_sonic_speed, gensym("sonic_speed"), A_FLOAT, 0);
  class_addmethod(cart2del_damp_3d_class, (t_method)cart2del_damp_3d_r_ambi, gensym("r_ambi"), A_FLOAT, 0);
//  class_sethelpsymbol(cart2del_damp_3d_class, gensym("iemhelp2/cart2del_damp_3d-help"));
}
