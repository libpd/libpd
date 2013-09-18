/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_roomsim written by Thomas Musil (c) IEM KUG Graz Austria 2002 - 2006 */

#include "m_pd.h"
#include "iemlib.h"
#include <math.h>


/* -------------------------- early_reflections_3d ------------------------------ */
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

typedef struct _early_reflections_3d
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
  t_symbol  *x_s_index_delta_phi;
  t_symbol  *x_s_bundle;
  t_float   x_room_x;
  t_float   x_room_y;
  t_float   x_room_z;
  t_float   x_head_x;
  t_float   x_head_y;
  t_float   x_head_z;
  int       x_n_src;
  int       x_bundle;
  t_float   x_src_x[30];
  t_float   x_src_y[30];
  t_float   x_src_z[30];
  t_float   x_r_ambi;
  t_float   x_speed;
  t_float   x_180_over_pi;
} t_early_reflections_3d;

static t_class *early_reflections_3d_class;

static t_float early_reflections_3d_calc_radius(t_floatarg r_ambi, t_floatarg dx, t_floatarg dy, t_floatarg dz)
{
  t_float r = (t_float)sqrt(dx*dx + dy*dy + dz*dz);
  
  if(r < r_ambi)
    return(r_ambi);
  else
    return(r);
}

static t_float early_reflections_3d_calc_azimuth(t_floatarg x_180_over_pi, t_floatarg dx, t_floatarg dy, t_floatarg dz)
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

static t_float early_reflections_3d_calc_elevation(t_floatarg x_180_over_pi, t_floatarg dx, t_floatarg dy, t_floatarg dz)/*changes*/
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

static t_float early_reflections_3d_calc_bundle_index(t_floatarg delta, t_floatarg phi)/*changes*/
{
  if(delta > 67.5f)
    return(1.0f);
  else if(delta > 22.5f)
  {
    if(phi <= 180.0f)
    {
      if(phi <= 90.0f)
        return(2.0f);
      else
        return(3.0f);
    }
    else
    {
      if(phi <= 270.0f)
        return(4.0f);
      else
        return(5.0f);
    }
  }
  else if(delta > -22.5f)
  {
    phi += 22.5f;
    if(phi >= 360.0f)
      phi -= 360.0f;
    
    if(phi <= 180.0f)
    {
      if(phi <= 90.0f)
      {
        if(phi <= 45.0f)/* 0 .. 45 */
          return(6.0f);
        else
          return(7.0f);
      }
      else
      {
        if(phi <= 135.0f)
          return(8.0f);
        else
          return(9.0f);
      }
    }
    else
    {
      if(phi <= 270.0f)
      {
        if(phi <= 225.0f)
          return(10.0f);
        else
          return(11.0f);
      }
      else
      {
        if(phi <= 315.0f)/* 270 .. 315 */
          return(12.0f);
        else
          return(13.0f);/* 315 .. 360 */
      }
    }
  }
  else
  {
    if(phi <= 180.0f)
    {
      if(phi <= 90.0f)
        return(14.0f);
      else
        return(15.0f);
    }
    else
    {
      if(phi <= 270.0f)
        return(16.0f);
      else
        return(17.0f);
    }
  }
}

static void early_reflections_3d_doit(t_early_reflections_3d *x)
{
  t_atom *at;
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
  t_float rad[50], delta[50], phi[50];
  int n_src=x->x_n_src;
  int i;
  /*t_float hz2 = 0.5f*x->x_room_z;
  
    diff_x = x->x_src_x - x->x_head_x;
    diff_y = x->x_src_y - x->x_head_y;
    diff_z = (x->x_src_z - hz2) - (x->x_head_z - hz2);
    sum_x = x->x_src_x + x->x_head_x;
    sum_y = x->x_src_y + x->x_head_y;
  sum_z = (x->x_src_z - hz2) + (x->x_head_z - hz2);*/
  
  lx = x->x_room_x;
  wy = x->x_room_y;
  hz = x->x_room_z;
  
  SETFLOAT(x->x_para_at, early_reflections_3d_calc_radius(r_ambi, lx, wy, hz)*m2ms);
  outlet_anything(x->x_rev_out, x->x_s_del0, 1, x->x_para_at);
  
  for(i=0; i<n_src; i++)
  {
    diff_x = x->x_src_x[i] - x->x_head_x;
    diff_y = x->x_src_y[i] - x->x_head_y;
    diff_z = x->x_src_z[i] - x->x_head_z;
    sum_x = x->x_src_x[i] + x->x_head_x;
    sum_y = x->x_src_y[i] + x->x_head_y;
    sum_z = x->x_src_z[i] + x->x_head_z - hz;
    
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
    
    rad[0] = early_reflections_3d_calc_radius(r_ambi, x0, y0_, z0);
    rad[1] = early_reflections_3d_calc_radius(r_ambi, xp1, y0_, z0);
    rad[2] = early_reflections_3d_calc_radius(r_ambi, x0, yp1, z0);
    rad[3] = early_reflections_3d_calc_radius(r_ambi, x0, y0_, zp1);
    rad[4] = early_reflections_3d_calc_radius(r_ambi, xn1, y0_, z0);
    rad[5] = early_reflections_3d_calc_radius(r_ambi, x0, yn1, z0);
    rad[6] = early_reflections_3d_calc_radius(r_ambi, x0, y0_, zn1);
    rad[7] = early_reflections_3d_calc_radius(r_ambi, xp2, y0_, z0);
    rad[8] = early_reflections_3d_calc_radius(r_ambi, x0, yp2, z0);
    rad[9] = early_reflections_3d_calc_radius(r_ambi, x0, y0_, zp2);
    rad[10] = early_reflections_3d_calc_radius(r_ambi, xn2, y0_, z0);
    rad[11] = early_reflections_3d_calc_radius(r_ambi, x0, yn2, z0);
    rad[12] = early_reflections_3d_calc_radius(r_ambi, x0, y0_, zn2);
    rad[13] = early_reflections_3d_calc_radius(r_ambi, xp1, yp1, z0);
    rad[14] = early_reflections_3d_calc_radius(r_ambi, xp1, y0_, zp1);
    rad[15] = early_reflections_3d_calc_radius(r_ambi, x0, yp1, zp1);
    rad[16] = early_reflections_3d_calc_radius(r_ambi, xn1, yn1, z0);
    rad[17] = early_reflections_3d_calc_radius(r_ambi, xn1, y0_, zn1);
    rad[18] = early_reflections_3d_calc_radius(r_ambi, x0, yn1, zn1);
    rad[19] = early_reflections_3d_calc_radius(r_ambi, xp1, yn1, z0);
    rad[20] = early_reflections_3d_calc_radius(r_ambi, xp1, y0_, zn1);
    rad[21] = early_reflections_3d_calc_radius(r_ambi, x0, yp1, zn1);
    rad[22] = early_reflections_3d_calc_radius(r_ambi, xn1, yp1, z0);
    rad[23] = early_reflections_3d_calc_radius(r_ambi, xn1, y0_, zp1);
    rad[24] = early_reflections_3d_calc_radius(r_ambi, x0, yn1, zp1);
    
    /* delay-reihenfolge: 0 auslassen,
    +1x, +1y, +1z, -1x, -1y, -1z
    +2x, +2y, +2z, -2x, -2y, -2z
    +1x+1y, +1x+1z, +1y+1z
    -1x-1y, -1x-1z, -1y-1z
    +1x-1y, +1x-1z, +1y-1z
    -1x+1y, -1x+1z, -1y+1z
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
    at++;
    SETFLOAT(at, rad[5] *m2ms);
    at++;
    SETFLOAT(at, rad[6] *m2ms);
    outlet_anything(x->x_early_out, x->x_s_del1, 7, x->x_para_at);
    
    at = x->x_para_at+1;
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
    at++;
    SETFLOAT(at, rad[13] *m2ms);
    at++;
    SETFLOAT(at, rad[14] *m2ms);
    at++;
    SETFLOAT(at, rad[15] *m2ms);
    at++;
    SETFLOAT(at, rad[16] *m2ms);
    at++;
    SETFLOAT(at, rad[17] *m2ms);
    at++;
    SETFLOAT(at, rad[18] *m2ms);
    at++;
    SETFLOAT(at, rad[19] *m2ms);
    at++;
    SETFLOAT(at, rad[20] *m2ms);
    at++;
    SETFLOAT(at, rad[21] *m2ms);
    at++;
    SETFLOAT(at, rad[22] *m2ms);
    at++;
    SETFLOAT(at, rad[23] *m2ms);
    at++;
    SETFLOAT(at, rad[24] *m2ms);
    outlet_anything(x->x_early_out, x->x_s_del2, 19, x->x_para_at);
    
    
    /* daempfungs-reihenfolge:
    0,
    +1x, +1y, +1z, -1x, -1y, -1z
    
      +2x, +2y, +2z, -2x, -2y, -2z
      +1x+1y, +1x+1z, +1y+1z
      -1x-1y, -1x-1z, -1y-1z
      +1x-1y, +1x-1z, +1y-1z
      -1x+1y, -1x+1z, -1y+1z
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
    at++;
    
    SETFLOAT(at, r_ambi / rad[13]);
    at++;
    SETFLOAT(at, r_ambi / rad[14]);
    at++;
    SETFLOAT(at, r_ambi / rad[15]);
    at++;
    
    SETFLOAT(at, r_ambi / rad[16]);
    at++;
    SETFLOAT(at, r_ambi / rad[17]);
    at++;
    SETFLOAT(at, r_ambi / rad[18]);
    at++;
    
    SETFLOAT(at, r_ambi / rad[19]);
    at++;
    SETFLOAT(at, r_ambi / rad[20]);
    at++;
    SETFLOAT(at, r_ambi / rad[21]);
    at++;
    
    SETFLOAT(at, r_ambi / rad[22]);
    at++;
    SETFLOAT(at, r_ambi / rad[23]);
    at++;
    SETFLOAT(at, r_ambi / rad[24]);
    
    outlet_anything(x->x_early_out, x->x_s_damp, 25, x->x_para_at);
    
    
    /* encoder-winkel-reihenfolge: index delta phi
    0,
    +1x, +1y, +1z, -1x, -1y, -1z
    +2x, +2y, +2z, -2x, -2y, -2z
    +1x+1y, +1x+1z, +1y+1z
    -1x-1y, -1x-1z, -1y-1z
    +1x-1y, +1x-1z, +1y-1z
    -1x+1y, -1x+1z, -1y+1z
    */
    
    at = x->x_para_at+1;
    SETFLOAT(at, early_reflections_3d_calc_elevation(x_180_over_pi, x0, y0_, z0));
    at++;
    SETFLOAT(at, early_reflections_3d_calc_azimuth(x_180_over_pi, x0, y0_, z0));
    
    outlet_anything(x->x_direct_out, x->x_s_index_delta_phi, 3, x->x_para_at);
    
    /* encoder-winkel-reihenfolge: bundle
    0,
    +1x, +1y, +1z, -1x, -1y, -1z
    +2x, +2y, +2z, -2x, -2y, -2z
    +1x+1y, +1x+1z, +1y+1z
    -1x-1y, -1x-1z, -1y-1z
    +1x-1y, +1x-1z, +1y-1z
    -1x+1y, -1x+1z, -1y+1z
    */
    
    delta[0] = early_reflections_3d_calc_elevation(x_180_over_pi, xp1, y0_, z0);
    phi[0] = early_reflections_3d_calc_azimuth(x_180_over_pi, xp1, y0_, z0);
    delta[1] = early_reflections_3d_calc_elevation(x_180_over_pi, x0, yp1, z0);
    phi[1] = early_reflections_3d_calc_azimuth(x_180_over_pi, x0, yp1, z0);
    delta[2] = early_reflections_3d_calc_elevation(x_180_over_pi, x0, y0_, zp1);
    phi[2] = early_reflections_3d_calc_azimuth(x_180_over_pi, x0, y0_, zp1);
    delta[3] = early_reflections_3d_calc_elevation(x_180_over_pi, xn1, y0_, z0);
    phi[3] = early_reflections_3d_calc_azimuth(x_180_over_pi, xn1, y0_, z0);
    delta[4] = early_reflections_3d_calc_elevation(x_180_over_pi, x0, yn1, z0);
    phi[4] = early_reflections_3d_calc_azimuth(x_180_over_pi, x0, yn1, z0);
    delta[5] = early_reflections_3d_calc_elevation(x_180_over_pi, x0, y0_, zn1);
    phi[5] = early_reflections_3d_calc_azimuth(x_180_over_pi, x0, y0_, zn1);
    delta[6] = early_reflections_3d_calc_elevation(x_180_over_pi, xp2, y0_, z0);
    phi[6] = early_reflections_3d_calc_azimuth(x_180_over_pi, xp2, y0_, z0);
    delta[7] = early_reflections_3d_calc_elevation(x_180_over_pi, x0, yp2, z0);
    phi[7] = early_reflections_3d_calc_azimuth(x_180_over_pi, x0, yp2, z0);
    delta[8] = early_reflections_3d_calc_elevation(x_180_over_pi, x0, y0_, zp2);
    phi[8] = early_reflections_3d_calc_azimuth(x_180_over_pi, x0, y0_, zp2);
    delta[9] = early_reflections_3d_calc_elevation(x_180_over_pi, xn2, y0_, z0);
    phi[9] = early_reflections_3d_calc_azimuth(x_180_over_pi, xn2, y0_, z0);
    delta[10] = early_reflections_3d_calc_elevation(x_180_over_pi, x0, yn2, z0);
    phi[10] = early_reflections_3d_calc_azimuth(x_180_over_pi, x0, yn2, z0);
    delta[11] = early_reflections_3d_calc_elevation(x_180_over_pi, x0, y0_, zn2);
    phi[11] = early_reflections_3d_calc_azimuth(x_180_over_pi, x0, y0_, zn2);
    delta[12] = early_reflections_3d_calc_elevation(x_180_over_pi, xp1, yp1, z0);
    phi[12] = early_reflections_3d_calc_azimuth(x_180_over_pi, xp1, yp1, z0);
    delta[13] = early_reflections_3d_calc_elevation(x_180_over_pi, xp1, y0_, zp1);
    phi[13] = early_reflections_3d_calc_azimuth(x_180_over_pi, xp1, y0_, zp1);
    delta[14] = early_reflections_3d_calc_elevation(x_180_over_pi, x0, yp1, zp1);
    phi[14] = early_reflections_3d_calc_azimuth(x_180_over_pi, x0, yp1, zp1);
    delta[15] = early_reflections_3d_calc_elevation(x_180_over_pi, xn1, yn1, z0);
    phi[15] = early_reflections_3d_calc_azimuth(x_180_over_pi, xn1, yn1, z0);
    delta[16] = early_reflections_3d_calc_elevation(x_180_over_pi, xn1, y0_, zn1);
    phi[16] = early_reflections_3d_calc_azimuth(x_180_over_pi, xn1, y0_, zn1);
    delta[17] = early_reflections_3d_calc_elevation(x_180_over_pi, x0, yn1, zn1);
    phi[17] = early_reflections_3d_calc_azimuth(x_180_over_pi, x0, yn1, zn1);
    delta[18] = early_reflections_3d_calc_elevation(x_180_over_pi, xp1, yn1, z0);
    phi[18] = early_reflections_3d_calc_azimuth(x_180_over_pi, xp1, yn1, z0);
    delta[19] = early_reflections_3d_calc_elevation(x_180_over_pi, xp1, y0_, zn1);
    phi[19] = early_reflections_3d_calc_azimuth(x_180_over_pi, xp1, y0_, zn1);
    delta[20] = early_reflections_3d_calc_elevation(x_180_over_pi, x0, yp1, zn1);
    phi[20] = early_reflections_3d_calc_azimuth(x_180_over_pi, x0, yp1, zn1);
    delta[21] = early_reflections_3d_calc_elevation(x_180_over_pi, xn1, yp1, z0);
    phi[21] = early_reflections_3d_calc_azimuth(x_180_over_pi, xn1, yp1, z0);
    delta[22] = early_reflections_3d_calc_elevation(x_180_over_pi, xn1, y0_, zp1);
    phi[22] = early_reflections_3d_calc_azimuth(x_180_over_pi, xn1, y0_, zp1);
    delta[23] = early_reflections_3d_calc_elevation(x_180_over_pi, x0, yn1, zp1);
    phi[23] = early_reflections_3d_calc_azimuth(x_180_over_pi, x0, yn1, zp1);
    
    if(x->x_bundle)
    {
      at = x->x_para_at+1;
      SETFLOAT(at, early_reflections_3d_calc_bundle_index(delta[0], phi[0]));
      at++;
      SETFLOAT(at, early_reflections_3d_calc_bundle_index(delta[1], phi[1]));
      at++;
      SETFLOAT(at, early_reflections_3d_calc_bundle_index(delta[2], phi[2]));
      at++;
      SETFLOAT(at, early_reflections_3d_calc_bundle_index(delta[3], phi[3]));
      at++;
      SETFLOAT(at, early_reflections_3d_calc_bundle_index(delta[4], phi[4]));
      at++;
      SETFLOAT(at, early_reflections_3d_calc_bundle_index(delta[5], phi[5]));
      at++;
      SETFLOAT(at, early_reflections_3d_calc_bundle_index(delta[6], phi[6]));
      at++;
      SETFLOAT(at, early_reflections_3d_calc_bundle_index(delta[7], phi[7]));
      at++;
      SETFLOAT(at, early_reflections_3d_calc_bundle_index(delta[8], phi[8]));
      at++;
      SETFLOAT(at, early_reflections_3d_calc_bundle_index(delta[9], phi[9]));
      at++;
      SETFLOAT(at, early_reflections_3d_calc_bundle_index(delta[10], phi[10]));
      at++;
      SETFLOAT(at, early_reflections_3d_calc_bundle_index(delta[11], phi[11]));
      at++;
      SETFLOAT(at, early_reflections_3d_calc_bundle_index(delta[12], phi[12]));
      at++;
      SETFLOAT(at, early_reflections_3d_calc_bundle_index(delta[13], phi[13]));
      at++;
      SETFLOAT(at, early_reflections_3d_calc_bundle_index(delta[14], phi[14]));
      at++;
      SETFLOAT(at, early_reflections_3d_calc_bundle_index(delta[15], phi[15]));
      at++;
      SETFLOAT(at, early_reflections_3d_calc_bundle_index(delta[16], phi[16]));
      at++;
      SETFLOAT(at, early_reflections_3d_calc_bundle_index(delta[17], phi[17]));
      at++;
      SETFLOAT(at, early_reflections_3d_calc_bundle_index(delta[18], phi[18]));
      at++;
      SETFLOAT(at, early_reflections_3d_calc_bundle_index(delta[19], phi[19]));
      at++;
      SETFLOAT(at, early_reflections_3d_calc_bundle_index(delta[20], phi[20]));
      at++;
      SETFLOAT(at, early_reflections_3d_calc_bundle_index(delta[21], phi[21]));
      at++;
      SETFLOAT(at, early_reflections_3d_calc_bundle_index(delta[22], phi[22]));
      at++;
      SETFLOAT(at, early_reflections_3d_calc_bundle_index(delta[23], phi[23]));
      outlet_anything(x->x_early_out, x->x_s_bundle, 25, x->x_para_at);
    }
    
    at = x->x_para_at+1;
    SETFLOAT(at, 1.0f);
    at++;
    SETFLOAT(at, delta[0]);
    at++;
    SETFLOAT(at, phi[0]);
    outlet_anything(x->x_early_out, x->x_s_index_delta_phi, 4, x->x_para_at);
    
    at = x->x_para_at+1;
    SETFLOAT(at, 2.0f);
    at++;
    SETFLOAT(at, delta[1]);
    at++;
    SETFLOAT(at, phi[1]);
    outlet_anything(x->x_early_out, x->x_s_index_delta_phi, 4, x->x_para_at);
    
    at = x->x_para_at+1;
    SETFLOAT(at, 3.0f);
    at++;
    SETFLOAT(at, delta[2]);
    at++;
    SETFLOAT(at, phi[2]);
    outlet_anything(x->x_early_out, x->x_s_index_delta_phi, 4, x->x_para_at);
    
    at = x->x_para_at+1;
    SETFLOAT(at, 4.0f);
    at++;
    SETFLOAT(at, delta[3]);
    at++;
    SETFLOAT(at, phi[3]);
    outlet_anything(x->x_early_out, x->x_s_index_delta_phi, 4, x->x_para_at);
    
    at = x->x_para_at+1;
    SETFLOAT(at, 5.0f);
    at++;
    SETFLOAT(at, delta[4]);
    at++;
    SETFLOAT(at, phi[4]);
    outlet_anything(x->x_early_out, x->x_s_index_delta_phi, 4, x->x_para_at);
    
    at = x->x_para_at+1;
    SETFLOAT(at, 6.0f);
    at++;
    SETFLOAT(at, delta[5]);
    at++;
    SETFLOAT(at, phi[5]);
    outlet_anything(x->x_early_out, x->x_s_index_delta_phi, 4, x->x_para_at);
    
    at = x->x_para_at+1;
    SETFLOAT(at, 7.0f);
    at++;
    SETFLOAT(at, delta[6]);
    at++;
    SETFLOAT(at, phi[6]);
    outlet_anything(x->x_early_out, x->x_s_index_delta_phi, 4, x->x_para_at);
    
    at = x->x_para_at+1;
    SETFLOAT(at, 8.0f);
    at++;
    SETFLOAT(at, delta[7]);
    at++;
    SETFLOAT(at, phi[7]);
    outlet_anything(x->x_early_out, x->x_s_index_delta_phi, 4, x->x_para_at);
    
    at = x->x_para_at+1;
    SETFLOAT(at, 9.0f);
    at++;
    SETFLOAT(at, delta[8]);
    at++;
    SETFLOAT(at, phi[8]);
    outlet_anything(x->x_early_out, x->x_s_index_delta_phi, 4, x->x_para_at);
    
    at = x->x_para_at+1;
    SETFLOAT(at, 10.0f);
    at++;
    SETFLOAT(at, delta[9]);
    at++;
    SETFLOAT(at, phi[9]);
    outlet_anything(x->x_early_out, x->x_s_index_delta_phi, 4, x->x_para_at);
    
    at = x->x_para_at+1;
    SETFLOAT(at, 11.0f);
    at++;
    SETFLOAT(at, delta[10]);
    at++;
    SETFLOAT(at, phi[10]);
    outlet_anything(x->x_early_out, x->x_s_index_delta_phi, 4, x->x_para_at);
    
    at = x->x_para_at+1;
    SETFLOAT(at, 12.0f);
    at++;
    SETFLOAT(at, delta[11]);
    at++;
    SETFLOAT(at, phi[11]);
    outlet_anything(x->x_early_out, x->x_s_index_delta_phi, 4, x->x_para_at);
    
    at = x->x_para_at+1;
    SETFLOAT(at, 13.0f);
    at++;
    SETFLOAT(at, delta[12]);
    at++;
    SETFLOAT(at, phi[12]);
    outlet_anything(x->x_early_out, x->x_s_index_delta_phi, 4, x->x_para_at);
    
    at = x->x_para_at+1;
    SETFLOAT(at, 14.0f);
    at++;
    SETFLOAT(at, delta[13]);
    at++;
    SETFLOAT(at, phi[13]);
    outlet_anything(x->x_early_out, x->x_s_index_delta_phi, 4, x->x_para_at);
    
    at = x->x_para_at+1;
    SETFLOAT(at, 15.0f);
    at++;
    SETFLOAT(at, delta[14]);
    at++;
    SETFLOAT(at, phi[14]);
    outlet_anything(x->x_early_out, x->x_s_index_delta_phi, 4, x->x_para_at);
    
    at = x->x_para_at+1;
    SETFLOAT(at, 16.0f);
    at++;
    SETFLOAT(at, delta[15]);
    at++;
    SETFLOAT(at, phi[15]);
    outlet_anything(x->x_early_out, x->x_s_index_delta_phi, 4, x->x_para_at);
    
    at = x->x_para_at+1;
    SETFLOAT(at, 17.0f);
    at++;
    SETFLOAT(at, delta[16]);
    at++;
    SETFLOAT(at, phi[16]);
    outlet_anything(x->x_early_out, x->x_s_index_delta_phi, 4, x->x_para_at);
    
    at = x->x_para_at+1;
    SETFLOAT(at, 18.0f);
    at++;
    SETFLOAT(at, delta[17]);
    at++;
    SETFLOAT(at, phi[17]);
    outlet_anything(x->x_early_out, x->x_s_index_delta_phi, 4, x->x_para_at);
    
    at = x->x_para_at+1;
    SETFLOAT(at, 19.0f);
    at++;
    SETFLOAT(at, delta[18]);
    at++;
    SETFLOAT(at, phi[18]);
    outlet_anything(x->x_early_out, x->x_s_index_delta_phi, 4, x->x_para_at);
    
    at = x->x_para_at+1;
    SETFLOAT(at, 20.0f);
    at++;
    SETFLOAT(at, delta[19]);
    at++;
    SETFLOAT(at, phi[19]);
    outlet_anything(x->x_early_out, x->x_s_index_delta_phi, 4, x->x_para_at);
    
    at = x->x_para_at+1;
    SETFLOAT(at, 21.0f);
    at++;
    SETFLOAT(at, delta[20]);
    at++;
    SETFLOAT(at, phi[20]);
    outlet_anything(x->x_early_out, x->x_s_index_delta_phi, 4, x->x_para_at);
    
    at = x->x_para_at+1;
    SETFLOAT(at, 22.0f);
    at++;
    SETFLOAT(at, delta[21]);
    at++;
    SETFLOAT(at, phi[21]);
    outlet_anything(x->x_early_out, x->x_s_index_delta_phi, 4, x->x_para_at);
    
    at = x->x_para_at+1;
    SETFLOAT(at, 23.0f);
    at++;
    SETFLOAT(at, delta[22]);
    at++;
    SETFLOAT(at, phi[22]);
    outlet_anything(x->x_early_out, x->x_s_index_delta_phi, 4, x->x_para_at);
    
    at = x->x_para_at+1;
    SETFLOAT(at, 24.0f);
    at++;
    SETFLOAT(at, delta[23]);
    at++;
    SETFLOAT(at, phi[23]);
    outlet_anything(x->x_early_out, x->x_s_index_delta_phi, 4, x->x_para_at);
  }
}

static void early_reflections_3d_dump_para(t_early_reflections_3d *x)
{
  int i, n=x->x_n_src;
  
  post("*******************************************************************************");
  post("room-dimensions: L_x = %.3f, W_y = %.3f, H_z = %.3f", x->x_room_x, x->x_room_y, x->x_room_z);
  post("hear-position: x_hear = %.3f, y_hear = %.3f, z_hear = %.3f", x->x_head_x, x->x_head_y, x->x_head_z);
  for(i=0; i<n; i++)
    post("source-coordinates: x_src%d = %.3f, y_src%d = %.3f, z_src%d = %.3f",
    x->x_src_x[i], i+1, x->x_src_y[i], i+1, x->x_src_z[i], i+1);
  post("ambisonic-radius: %f", x->x_r_ambi);
  post("sonic-speed: %.3f", x->x_speed);
  post("order of outputs: direct early rev");
  post("*******************************************************************************");
}

static void early_reflections_3d_para(t_early_reflections_3d *x, t_symbol *s, int argc, t_atom *argv)
{
  int i, n=x->x_n_src*3 + 7;/* r_ambi + 3*room + 3*head */
  
  if(argc != n)
  {
    post("early_reflections_3d ERROR: para needs 1 r_ambi + 3*room + 3*head +n*3*src");
    return;
  }
  
  x->x_r_ambi = atom_getfloat(argv++);
  x->x_room_x = atom_getfloat(argv++);
  x->x_room_y = atom_getfloat(argv++);
  x->x_room_z = atom_getfloat(argv++);
  x->x_head_x = atom_getfloat(argv++);
  x->x_head_y = atom_getfloat(argv++);
  x->x_head_z = atom_getfloat(argv++);
  n = x->x_n_src;
  for(i=0; i<n; i++)
  {
    x->x_src_x[i] = atom_getfloat(argv++);
    x->x_src_y[i] = atom_getfloat(argv++);
    x->x_src_z[i] = atom_getfloat(argv++);
  }
  early_reflections_3d_doit(x);
}

static void early_reflections_3d_sonic_speed(t_early_reflections_3d *x, t_floatarg speed)
{
  if(speed < 300.0f)
    speed = 300.0f;
  if(speed > 400.0f)
    speed = 400.0f;
  x->x_speed = speed;
}

static void early_reflections_3d_bundle(t_early_reflections_3d *x, t_floatarg bundle)
{
  if(bundle == 0.0f)
    x->x_bundle = 0;
  else
    x->x_bundle = 1;
}

static void early_reflections_3d_free(t_early_reflections_3d *x)
{
}

static void *early_reflections_3d_new(t_floatarg fn_src)
{
  int i, n;
  t_early_reflections_3d *x = (t_early_reflections_3d *)pd_new(early_reflections_3d_class);
  
  n = (int)fn_src;
  if(n < 1)
    n = 1;
  if(n > 30)
    n = 30;
  x->x_n_src = n;
  x->x_room_x = 12.0f;
  x->x_room_y = 8.0f;
  x->x_room_z = 4.0f;
  x->x_head_x = 0.0f;
  x->x_head_y = 0.0f;
  x->x_head_z = 1.7f;
  for(i=0; i<n; i++)
  {
    x->x_src_x[i] = 3.0f;
    x->x_src_y[i] = 0.5f;
    x->x_src_z[i] = 2.5f;
  }
  x->x_r_ambi = 1.4f;
  x->x_speed = 340.0f;
  
  x->x_s_del0 = gensym("del0");
  x->x_s_del1 = gensym("del1");
  x->x_s_del2 = gensym("del2");
  x->x_s_damp = gensym("damp");
  x->x_s_index_delta_phi = gensym("index_delta_phi");
  x->x_s_bundle = gensym("bundle");
  x->x_direct_out = outlet_new(&x->x_obj, &s_list);
  x->x_early_out = outlet_new(&x->x_obj, &s_list);
  x->x_rev_out = outlet_new(&x->x_obj, &s_list);
  x->x_180_over_pi  = (t_float)(180.0 / (4.0 * atan(1.0)));
  x->x_bundle = 0;
  return (x);
}

void early_reflections_3d_setup(void)
{
  early_reflections_3d_class = class_new(gensym("early_reflections_3d"), (t_newmethod)early_reflections_3d_new, (t_method)early_reflections_3d_free,
    sizeof(t_early_reflections_3d), 0, A_DEFFLOAT, 0);
  class_addmethod(early_reflections_3d_class, (t_method)early_reflections_3d_para, gensym("para"), A_GIMME, 0);
  class_addmethod(early_reflections_3d_class, (t_method)early_reflections_3d_sonic_speed, gensym("sonic_speed"), A_FLOAT, 0);
  class_addmethod(early_reflections_3d_class, (t_method)early_reflections_3d_bundle, gensym("bundle"), A_FLOAT, 0);
  class_addmethod(early_reflections_3d_class, (t_method)early_reflections_3d_dump_para, gensym("dump_para"), 0);
//  class_sethelpsymbol(early_reflections_3d_class, gensym("iemhelp2/early_reflections_3d-help"));
}
