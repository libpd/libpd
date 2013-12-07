/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_ambi written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"
#include <math.h>

/* -------------------------- ambi_rot ------------------------------ */
/*
ambi_rot : 1. uebergabe-argument : ambisonic-ordnung 1 .. 4;

  input: <float> nur z-rotation;
  input: <list> mit 2 floats: nur z*y-rotation;
  input: <list> mit 3 floats: z*y*x-rotation;
  
    output: je nach ordnung: 1 .. 4 <list>-outputs mit selector "matrix" + row + col + ordnung*ordnung-coeff.;
*/

typedef struct _ambi_rot
{
  t_object  x_obj;
  t_atom    *x_at2;
  int       x_size2;
  t_atom    *x_at3;
  int       x_size3;
  void      *x_out3;
  t_atom    *x_at5;
  int       x_size5;
  void      *x_out5;
  t_atom    *x_at7;
  int       x_size7;
  void      *x_out7;
  t_atom    *x_at9;
  int       x_size9;
  void      *x_out9;
  t_atom    *x_at11;
  int       x_size11;
  void      *x_out11;
  void      *x_out13;
  void      *x_out15;
  void      *x_out17;
  void      *x_out19;
  void      *x_out21;
  void      *x_out23;
  void      *x_out25;
  t_float   x_sqrt2_16;
  t_float   x_sqrt3_2;
  t_float   x_sqrt5_32;
  t_float   x_sqrt6_4;
  t_float   x_sqrt7_8;
  t_float   x_sqrt10_4;
  t_float   x_sqrt14_16;
  t_float   x_sqrt15_8;
  t_float   x_sqrt35_64;
  t_float   x_sqrt70_32;
  t_float   x_pi_over_180;
  t_symbol  *x_s_matrix;
  int       x_order;
} t_ambi_rot;

static t_class *ambi_rot_class;

static void ambi_rot_float(t_ambi_rot *x, t_floatarg rho_z) /* = ambi_rot_z(); */
{
  t_float c, s, cc, ss, c2, s2, c3, s3, s4, c4, s5, c5, s6, c6, sx, cx;
  t_atom *at;
  
  rho_z *= x->x_pi_over_180;
  
  c = cos(rho_z);
  s = sin(rho_z);
  cc = c*c;
  ss = s*s;
  
  if(x->x_order >= 2)
  {
    c2 = cc - ss;
    s2 = 2.0f*s*c;
    if(x->x_order >= 3)
    {
      c3 = c*(4.0f*cc - 3.0f);
      s3 = s*(3.0f - 4.0f*ss);
      if(x->x_order >= 4)
      {
        c4 = 1.0f + 8.0f*cc*(cc - 1.0f);
        s4 = 2.0f*s2*c2;
        if(x->x_order >= 5)
        {
          c5 = c*(1.0f + 4.0f*ss*(ss - 3.0f*cc));
          s5 = s*(1.0f + 4.0f*cc*(cc - 3.0f*ss));
          if(x->x_order >= 6)
          {
            c6 = c3*c3 - s3*s3;
            s6 = 2.0f*s3*c3;
            if(x->x_order >= 7)
            {
              if(x->x_order >= 8)
              {
                if(x->x_order >= 9)
                {
                  if(x->x_order >= 10)
                  {
                    if(x->x_order >= 11)
                    {
                      if(x->x_order >= 12)
                      {
                        if(x->x_order >= 13)
                          post("ambi_rot-ERROR: do not support Ambisonic-Order greater than 12 in z-Rotation !!!");
                        
                        at = x->x_at2;
                        at += 2;
                        cx = c6*c6 - s6*s6;
                        sx = 2.0f*s6*c6;
                        SETFLOAT(at, cx);
                        at++;
                        SETFLOAT(at, -sx);
                        at++;
                        SETFLOAT(at, sx);
                        at++;
                        SETFLOAT(at, cx);
                        outlet_anything(x->x_out25, x->x_s_matrix, x->x_size2, x->x_at2);
                      }
                      at = x->x_at2;
                      at += 2;
                      cx = cos(11.0f*rho_z);
                      sx = sin(11.0f*rho_z);
                      SETFLOAT(at, cx);
                      at++;
                      SETFLOAT(at, -sx);
                      at++;
                      SETFLOAT(at, sx);
                      at++;
                      SETFLOAT(at, cx);
                      outlet_anything(x->x_out23, x->x_s_matrix, x->x_size2, x->x_at2);
                    }
                    at = x->x_at2;
                    at += 2;
                    cx = c5*c5 - s5*s5;
                    sx = 2.0f*s5*c5;
                    SETFLOAT(at, cx);
                    at++;
                    SETFLOAT(at, -sx);
                    at++;
                    SETFLOAT(at, sx);
                    at++;
                    SETFLOAT(at, cx);
                    outlet_anything(x->x_out21, x->x_s_matrix, x->x_size2, x->x_at2);
                  }
                  at = x->x_at2;
                  at += 2;
                  cx = cos(9.0f*rho_z);
                  sx = sin(9.0f*rho_z);
                  SETFLOAT(at, cx);
                  at++;
                  SETFLOAT(at, -sx);
                  at++;
                  SETFLOAT(at, sx);
                  at++;
                  SETFLOAT(at, cx);
                  outlet_anything(x->x_out19, x->x_s_matrix, x->x_size2, x->x_at2);
                }
                at = x->x_at2;
                at += 2;
                cx = c4*c4 - s4*s4;
                sx = 2.0f*s4*c4;
                SETFLOAT(at, cx);
                at++;
                SETFLOAT(at, -sx);
                at++;
                SETFLOAT(at, sx);
                at++;
                SETFLOAT(at, cx);
                outlet_anything(x->x_out17, x->x_s_matrix, x->x_size2, x->x_at2);
              }
              at = x->x_at2;
              at += 2;
              cx = cos(7.0f*rho_z);
              sx = sin(7.0f*rho_z);
              SETFLOAT(at, cx);
              at++;
              SETFLOAT(at, -sx);
              at++;
              SETFLOAT(at, sx);
              at++;
              SETFLOAT(at, cx);
              outlet_anything(x->x_out15, x->x_s_matrix, x->x_size2, x->x_at2);
            }
            at = x->x_at2;
            at += 2;
            SETFLOAT(at, c6);
            at++;
            SETFLOAT(at, -s6);
            at++;
            SETFLOAT(at, s6);
            at++;
            SETFLOAT(at, c6);
            outlet_anything(x->x_out13, x->x_s_matrix, x->x_size2, x->x_at2);
          }
          at = x->x_at2;
          at += 2;
          SETFLOAT(at, c5);
          at++;
          SETFLOAT(at, -s5);
          at++;
          SETFLOAT(at, s5);
          at++;
          SETFLOAT(at, c5);
          outlet_anything(x->x_out11, x->x_s_matrix, x->x_size2, x->x_at2);
        }
        at = x->x_at2;
        at += 2;
        SETFLOAT(at, c4);
        at++;
        SETFLOAT(at, -s4);
        at++;
        SETFLOAT(at, s4);
        at++;
        SETFLOAT(at, c4);
        outlet_anything(x->x_out9, x->x_s_matrix, x->x_size2, x->x_at2);
      }
      at = x->x_at2;
      at += 2;
      SETFLOAT(at, c3);
      at++;
      SETFLOAT(at, -s3);
      at++;
      SETFLOAT(at, s3);
      at++;
      SETFLOAT(at, c3);
      outlet_anything(x->x_out7, x->x_s_matrix, x->x_size2, x->x_at2);
    }
    at = x->x_at2;
    at += 2;
    SETFLOAT(at, c2);
    at++;
    SETFLOAT(at, -s2);
    at++;
    SETFLOAT(at, s2);
    at++;
    SETFLOAT(at, c2);
    outlet_anything(x->x_out5, x->x_s_matrix, x->x_size2, x->x_at2);
  }
  at = x->x_at2;
  at += 2;
  SETFLOAT(at, c);
  at++;
  SETFLOAT(at, -s);
  at++;
  SETFLOAT(at, s);
  at++;
  SETFLOAT(at, c);
  outlet_anything(x->x_out3, x->x_s_matrix, x->x_size2, x->x_at2);
}

static void ambi_rot_zy(t_ambi_rot *x, t_floatarg rho_z, t_floatarg rho_y)
{
  t_float cy, sy, ccy, ssy, c2y, s2y, c3y, s3y, c4y, s4y;
  t_float cz, sz, ccz, ssz, c2z, s2z, c3z, s3z, c4z, s4z;
  t_float r9_y[9][9];
  int i;
  t_atom *at;
  
  rho_z *= x->x_pi_over_180;
  rho_y *= x->x_pi_over_180;
  
  cz = cos(rho_z);
  sz = sin(rho_z);
  ccz = cz*cz;
  ssz = sz*sz;
  
  cy = cos(rho_y);
  sy = sin(rho_y);
  ccy = cy*cy;
  ssy = sy*sy;
  
  if(x->x_order >= 2)
  {
    c2z = ccz - ssz;
    s2z = 2.0f*sz*cz;
    c2y = ccy - ssy;
    s2y = 2.0f*sy*cy;
    if(x->x_order >= 3)
    {
      c3z = cz*(4.0f*ccz - 3.0f);
      s3z = sz*(3.0f - 4.0f*ssz);
      c3y = cy*(4.0f*ccy - 3.0f);
      s3y = sy*(3.0f - 4.0f*ssy);
      if(x->x_order >= 4)
      {
        if(x->x_order >= 5)
          post("ambi_rot-ERROR: do not support Ambisonic-Order greater than 5 in zy-Rotation !!!");
        
          /*y
          r9_11=(35 + 28*c2 + c4)/64;
          r9_22=(7c + c3)/8;
          r9_31 = x->x_sqrt2*(14*s2 + s4)/32;
          r9_33=(7*c2 + c4)/8;
          r9_42 = x->x_sqrt2*(7*s + 3*s3)/16;
          r9_44=(7*c + 9*c3)/16;
          r9_51 = x->x_sqrt7*(5 - 4*c2 - c4)/32;
          r9_53 = x->x_sqrt14*(2*s2 + s4)/16;
          r9_55=(5 + 4*c2 + 7*c4)/16;
          r9_62 = x->x_sqrt7*(c - c3)/8;
          r9_64 = x->x_sqrt14*(-s + 3*s3)/16;
          r9_66=(c + 7*c3)/8;
          r9_71 = x->x_sqrt14*(2*s2 - s4)/32;
          r9_73 = x->x_sqrt7*(c2 - c4)/8;
          r9_75 = x->x_sqrt2*(-2*s2 + 7*s4)/16;
          r9_77=(c2 + 7*c4)/8;
          r9_82 = x->x_sqrt14*(3*s - s3)/16;
          r9_84 = x->x_sqrt7*(3*c - 3*c3)/16;
          r9_86 = x->x_sqrt2*(3*s + 7*s3)/16;
          r9_88=(9*c + 7*c3)/16;
          r9_91 = x->x_sqrt35*(3 - 4*c2 + c4)/64;
          r9_93 = x->x_sqrt70*(2*s2 - s4)/32;
          r9_95 = x->x_sqrt5*(3 + 4*c2 - 7*c4)/32;
          r9_97 = x->x_sqrt10*(2*s2 + 7*s4)/32;
          r9_99=(9 + 20*c2 + 35*c4)/64;   
        */
        
        c4y = 1.0f + 8.0f*ccy*(ccy - 1.0f);
        s4y = 2.0f*s2y*c2y;
        
        c4z = 1.0f + 8.0f*ccz*(ccz - 1.0f);
        s4z = 2.0f*s2z*c2z;
        
        r9_y[0][0] = (35.0f + 28.0f*c2y + c4y)*0.015625f;/* -r9_31y, r9_51y, -r9_71y, r9_91y; */
        r9_y[1][1] = (7.0f*cy + c3y)*0.125f;/* -r9_42y, r9_62y, -r9_82y;*/
        r9_y[2][0] = x->x_sqrt2_16*(14.0f*s2y + s4y)*0.5f;
        r9_y[2][2] = (7.0f*c2y + c4y)*0.125f;/* -r9_53y, r9_73y, -r9_93y;*/
        r9_y[3][1] = x->x_sqrt2_16*(7.0f*sy + 3.0f*s3y);
        r9_y[3][3] = (7.0f*cy + 9.0f*c3y)*0.0625f;/* -r9_64y, r9_84y;*/
        r9_y[4][0] = x->x_sqrt7_8*(5.0f - 4.0f*c2y - c4y)*0.25f;
        r9_y[4][2] = x->x_sqrt14_16*(2.0f*s2y + s4y);
        r9_y[4][4] = (5.0f + 4.0f*c2y + 7.0f*c4y)*0.0625f;/* -r9_75y, r9_95y;*/
        r9_y[5][1] = x->x_sqrt7_8*(cy - c3y);
        r9_y[5][3] = x->x_sqrt14_16*(3.0f*s3y - sy);
        r9_y[5][5] = (cy + 7.0f*c3y)*0.125f;/* -r9_86y;*/
        r9_y[6][0] = x->x_sqrt14_16*(2.0f*s2y - s4y)*0.5f;
        r9_y[6][2] = x->x_sqrt7_8*(c2y - c4y);
        r9_y[6][4] = x->x_sqrt2_16*(7.0f*s4y - 2.0f*s2y);
        r9_y[6][6] = (c2y + 7.0f*c4y)*0.125f;/* -r9_97y;*/
        r9_y[7][1] = x->x_sqrt14_16*(3.0f*sy - s3y);
        r9_y[7][3] = x->x_sqrt7_8*(cy - c3y)*1.5f;
        r9_y[7][5] = x->x_sqrt2_16*(3.0f*sy + 7.0f*s3y);
        r9_y[7][7] = (9.0f*cy + 7.0f*c3y)*0.0625f;
        r9_y[8][0] = x->x_sqrt35_64*(3.0f - 4.0f*c2y + c4y);
        r9_y[8][2] = x->x_sqrt70_32*(2.0f*s2y - s4y);
        r9_y[8][4] = x->x_sqrt5_32*(3.0f + 4.0f*c2y - 7.0f*c4y);
        r9_y[8][6] = x->x_sqrt10_4*(2.0f*s2y + 7.0f*s4y)*0.125f;
        r9_y[8][8] = (9.0f + 20.0f*c2y + 35.0f*c4y)*0.015625f;
        
        r9_y[0][2] = -r9_y[2][0];
        r9_y[0][4] = r9_y[4][0];
        r9_y[0][6] = -r9_y[6][0];
        r9_y[0][8] = r9_y[8][0];
        r9_y[1][3] = -r9_y[3][1];
        r9_y[1][5] = r9_y[5][1];
        r9_y[1][7] = -r9_y[7][1];
        r9_y[2][4] = -r9_y[4][2];
        r9_y[2][6] = r9_y[6][2];
        r9_y[2][8] = -r9_y[8][2];
        r9_y[3][5] = -r9_y[5][3];
        r9_y[3][7] = r9_y[7][3];
        r9_y[4][6] = -r9_y[6][4];
        r9_y[4][8] = r9_y[8][4];
        r9_y[5][7] = -r9_y[7][5];
        r9_y[6][8] = -r9_y[8][6];
        
        at = x->x_at9;
        at += 2;
        
        for(i=0; i<8; i+=2)
        {
          SETFLOAT(at, c4z*r9_y[0][i]);
          at++;
          SETFLOAT(at, -s4z*r9_y[1][i+1]);
          at++;
        }
        SETFLOAT(at, c4z*r9_y[0][8]);
        at++;
        
        for(i=0; i<8; i+=2)
        {
          SETFLOAT(at, c4z*r9_y[0][i]);
          at++;
          SETFLOAT(at, s4z*r9_y[1][i+1]);
          at++;
        }
        SETFLOAT(at, s4z*r9_y[0][8]);
        at++;
        
        for(i=0; i<8; i+=2)
        {
          SETFLOAT(at, c3z*r9_y[2][i]);
          at++;
          SETFLOAT(at, -s3z*r9_y[3][i+1]);
          at++;
        }
        SETFLOAT(at, c3z*r9_y[2][8]);
        at++;
        
        for(i=0; i<8; i+=2)
        {
          SETFLOAT(at, c3z*r9_y[2][i]);
          at++;
          SETFLOAT(at, s3z*r9_y[3][i+1]);
          at++;
        }
        SETFLOAT(at, s3z*r9_y[2][8]);
        at++;
        
        for(i=0; i<8; i+=2)
        {
          SETFLOAT(at, c2z*r9_y[4][i]);
          at++;
          SETFLOAT(at, -s2z*r9_y[5][i+1]);
          at++;
        }
        SETFLOAT(at, c2z*r9_y[4][8]);
        at++;
        
        for(i=0; i<8; i+=2)
        {
          SETFLOAT(at, c2z*r9_y[4][i]);
          at++;
          SETFLOAT(at, s2z*r9_y[5][i+1]);
          at++;
        }
        SETFLOAT(at, s2z*r9_y[4][8]);
        at++;
        
        for(i=0; i<8; i+=2)
        {
          SETFLOAT(at, cz*r9_y[6][i]);
          at++;
          SETFLOAT(at, -sz*r9_y[7][i+1]);
          at++;
        }
        SETFLOAT(at, cz*r9_y[6][8]);
        at++;
        
        for(i=0; i<8; i+=2)
        {
          SETFLOAT(at, cz*r9_y[6][i]);
          at++;
          SETFLOAT(at, sz*r9_y[7][i+1]);
          at++;
        }
        SETFLOAT(at, sz*r9_y[6][8]);
        at++;
        
        for(i=0; i<8; i+=2)
        {
          SETFLOAT(at, r9_y[8][i]);
          at++;
          SETFLOAT(at, 0.0f);
          at++;
        }
        SETFLOAT(at, r9_y[8][8]);
        
        outlet_anything(x->x_out9, x->x_s_matrix, x->x_size9, x->x_at9);
      }
      
      /*y
      r7_11=(15*c + c3)/16;
      r7_22=(5 + 3*c2)/8;
      r7_31 = x->x_sqrt6*(5*s + s3)/16;
      r7_33=(5*c + 3*c3)/8;
      r7_42 = x->x_sqrt6*(s2)/4;
      r7_44=(c2);
      r7_51 = x->x_sqrt15*(c - c3)/16;
      r7_53 = x->x_sqrt10*(-s + 3*s3)/16;
      r7_55=(c + 15*c3)/16;
      r7_62 = x->x_sqrt15*(1 - c2)/8;
      r7_64 = x->x_sqrt10*(s2)/4;
      r7_66=(3 + 5*c2)/8;
      r7_71 = x->x_sqrt10*(3*s - s3)/16;
      r7_73 = x->x_sqrt15*(c - c3)/8 = 2*r7_51;
      r7_75 = x->x_sqrt6*(s + 5*s3)/16;
      r7_77=(3*c + 5*c3)/8;
      
        */
        r9_y[0][0] = (15.0f*cy + c3y)*0.0625f;/* -r7_31y, r7_51y, -r7_71y;*/
        r9_y[1][1] = (5.0f + 3.0f*c2y)*0.125f;/* -r7_42y, r7_62y;*/
        r9_y[2][0] = x->x_sqrt6_4*(5.0f*sy + s3y)*0.25f;
        r9_y[2][2] = (5.0f*cy + 3.0f*c3y)*0.125f;/* -r7_53y, r7_73y;*/
        r9_y[3][1] = x->x_sqrt6_4*s2y;
        r9_y[3][3] = c2y;/* -r7_64y;*/
        r9_y[6][2] = x->x_sqrt15_8*(cy - c3y);
        r9_y[4][0] = r9_y[6][2]*0.5f;
        r9_y[4][2] = x->x_sqrt10_4*(3.0f*s3y - sy)*0.25f;
        r9_y[4][4] = (cy + 15.0f*c3y)*0.0625f;/* -r7_75y;*/
        r9_y[5][1] = x->x_sqrt15_8*(1.0f - c2y);
        r9_y[5][3] = x->x_sqrt10_4*s2y;
        r9_y[5][5] = (3.0f + 5.0f*c2y)*0.125f;
        r9_y[6][0] = x->x_sqrt10_4*(3.0f*sy - s3y)*0.25f;
        
        r9_y[6][4] = x->x_sqrt6_4*(sy + 5.0f*s3y)*0.25f;
        r9_y[6][6] = (3.0f*cy + 5.0f*c3y)*0.125f;
        
        r9_y[0][2] = -r9_y[2][0];
        r9_y[0][4] = r9_y[4][0];
        r9_y[0][6] = -r9_y[6][0];
        r9_y[1][3] = -r9_y[3][1];
        r9_y[1][5] = r9_y[5][1];
        r9_y[2][4] = -r9_y[4][2];
        r9_y[2][6] = r9_y[6][2];
        r9_y[3][5] = -r9_y[5][3];
        r9_y[4][6] = -r9_y[6][4];
        
        at = x->x_at7;
        at += 2;
        
        for(i=0; i<6; i+=2)
        {
          SETFLOAT(at, c3z*r9_y[0][i]);
          at++;
          SETFLOAT(at, -s3z*r9_y[1][i+1]);
          at++;
        }
        SETFLOAT(at, c3z*r9_y[0][6]);
        at++;
        
        for(i=0; i<6; i+=2)
        {
          SETFLOAT(at, c3z*r9_y[0][i]);
          at++;
          SETFLOAT(at, s3z*r9_y[1][i+1]);
          at++;
        }
        SETFLOAT(at, s3z*r9_y[0][6]);
        at++;
        
        for(i=0; i<6; i+=2)
        {
          SETFLOAT(at, c2z*r9_y[2][i]);
          at++;
          SETFLOAT(at, -s2z*r9_y[3][i+1]);
          at++;
        }
        SETFLOAT(at, c2z*r9_y[2][6]);
        at++;
        
        for(i=0; i<6; i+=2)
        {
          SETFLOAT(at, c2z*r9_y[2][i]);
          at++;
          SETFLOAT(at, s2z*r9_y[3][i+1]);
          at++;
        }
        SETFLOAT(at, s2z*r9_y[2][6]);
        at++;
        
        for(i=0; i<6; i+=2)
        {
          SETFLOAT(at, cz*r9_y[4][i]);
          at++;
          SETFLOAT(at, -sz*r9_y[5][i+1]);
          at++;
        }
        SETFLOAT(at, cz*r9_y[4][6]);
        at++;
        
        for(i=0; i<6; i+=2)
        {
          SETFLOAT(at, cz*r9_y[4][i]);
          at++;
          SETFLOAT(at, sz*r9_y[5][i+1]);
          at++;
        }
        SETFLOAT(at, sz*r9_y[4][6]);
        at++;
        
        for(i=0; i<6; i+=2)
        {
          SETFLOAT(at, r9_y[6][i]);
          at++;
          SETFLOAT(at, 0.0f);
          at++;
        }
        SETFLOAT(at, r9_y[6][6]);
        
        outlet_anything(x->x_out7, x->x_s_matrix, x->x_size7, x->x_at7);
    }
    
    /*y
    r5_11=(3 + c2)/4;
    r5_22=(c);
    r5_31 = (s2)/2;
    r5_33=(c2);
    r5_42 = (s);
    r5_44=(c);
    r5_51 = x->x_sqrt3*(1 - c2)/4;
    r5_53 = x->x_sqrt3*(s2)/2;
    r5_55=(1 + 3*c2)/4; 
    */
    r9_y[0][0] = (3.0f + c2y)*0.25f;/* -r5_31y, r5_51y;*/
    r9_y[1][1] = cy;/* -r5_42y;*/
    r9_y[2][0] = s2y*0.5f;
    r9_y[2][2] = c2y;/* -r5_53y;*/
    r9_y[3][1] = sy;
    r9_y[3][3] = cy;
    r9_y[4][0] = x->x_sqrt3_2*(1.0f - c2y)*0.5f;
    r9_y[4][2] = x->x_sqrt3_2*s2y;
    r9_y[4][4] = (1.0f + 3.0f*c2y)*0.25f;
    
    r9_y[0][2] = -r9_y[2][0];
    r9_y[0][4] = r9_y[4][0];
    r9_y[1][3] = -r9_y[3][1];
    r9_y[2][4] = -r9_y[4][2];
    
    at = x->x_at5;
    at += 2;
    
    for(i=0; i<4; i+=2)
    {
      SETFLOAT(at, c2z*r9_y[0][i]);
      at++;
      SETFLOAT(at, -s2z*r9_y[1][i+1]);
      at++;
    }
    SETFLOAT(at, c2z*r9_y[0][4]);
    at++;
    
    for(i=0; i<4; i+=2)
    {
      SETFLOAT(at, c2z*r9_y[0][i]);
      at++;
      SETFLOAT(at, s2z*r9_y[1][i+1]);
      at++;
    }
    SETFLOAT(at, s2z*r9_y[0][4]);
    at++;
    
    for(i=0; i<4; i+=2)
    {
      SETFLOAT(at, cz*r9_y[2][i]);
      at++;
      SETFLOAT(at, -sz*r9_y[3][i+1]);
      at++;
    }
    SETFLOAT(at, cz*r9_y[2][4]);
    at++;
    
    for(i=0; i<4; i+=2)
    {
      SETFLOAT(at, cz*r9_y[2][i]);
      at++;
      SETFLOAT(at, sz*r9_y[3][i+1]);
      at++;
    }
    SETFLOAT(at, sz*r9_y[2][4]);
    at++;
    
    for(i=0; i<4; i+=2)
    {
      SETFLOAT(at, r9_y[4][i]);
      at++;
      SETFLOAT(at, 0.0f);
      at++;
    }
    SETFLOAT(at, r9_y[4][4]);
    
    outlet_anything(x->x_out5, x->x_s_matrix, x->x_size5, x->x_at5);
  }
  
  /*y
  r3_11=(c);
  r3_22=(1);
  r3_31 = (s);
  r3_33=(c);
  */
  r9_y[0][0] = cy;/* -r3_31y;*/
  r9_y[1][1] = 1.0f;
  r9_y[2][0] = sy;
  r9_y[2][2] = cy;
  r9_y[0][2] = -r9_y[2][0];
  
  at = x->x_at3;
  at += 2;
  
  SETFLOAT(at, cz*r9_y[0][0]);
  at++;
  SETFLOAT(at, -sz*r9_y[1][1]);
  at++;
  SETFLOAT(at, cz*r9_y[0][2]);
  at++;
  
  SETFLOAT(at, sz*r9_y[0][0]);
  at++;
  SETFLOAT(at, cz*r9_y[1][1]);
  at++;
  SETFLOAT(at, sz*r9_y[0][2]);
  at++;
  
  SETFLOAT(at, r9_y[2][0]);
  at++;
  SETFLOAT(at, 0.0f);
  at++;
  SETFLOAT(at, r9_y[2][2]);
  
  outlet_anything(x->x_out3, x->x_s_matrix, x->x_size3, x->x_at3);
}

static void ambi_rot_zyx(t_ambi_rot *x, t_floatarg rho_z, t_floatarg rho_y, t_floatarg rho_x)
{
  t_float cx, sx, ccx, ssx, c2x, s2x, c3x, s3x, c4x, s4x;
  t_float cy, sy, ccy, ssy, c2y, s2y, c3y, s3y, c4y, s4y;
  t_float cz, sz, ccz, ssz, c2z, s2z, c3z, s3z, c4z, s4z;
  t_float r9_zy[9][9];
  t_float r9_z[9][9];
  t_float r9_y[9][9];
  t_float r9_x[9][9];
  int i, j;
  t_atom *at;
  
  rho_z *= x->x_pi_over_180;
  rho_y *= x->x_pi_over_180;
  rho_x *= x->x_pi_over_180;
  
  cz = cos(rho_z);
  sz = sin(rho_z);
  ccz = cz*cz;
  ssz = sz*sz;
  
  cy = cos(rho_y);
  sy = sin(rho_y);
  ccy = cy*cy;
  ssy = sy*sy;
  
  cx = cos(rho_x);
  sx = sin(rho_x);
  ccx = cx*cx;
  ssx = sx*sx;
  
  if(x->x_order >= 2)
  {
    c2z = ccz - ssz;
    s2z = 2.0f*sz*cz;
    c2y = ccy - ssy;
    s2y = 2.0f*sy*cy;
    c2x = ccx - ssx;
    s2x = 2.0f*sx*cx;
    
    if(x->x_order >= 3)
    {
      c3z = cz*(4.0f*ccz - 3.0f);
      s3z = sz*(3.0f - 4.0f*ssz);
      c3y = cy*(4.0f*ccy - 3.0f);
      s3y = sy*(3.0f - 4.0f*ssy);
      c3x = cx*(4.0f*ccx - 3.0f);
      s3x = sx*(3.0f - 4.0f*ssx);
      
      if(x->x_order >= 4)
      {
        if(x->x_order >= 5)
          post("ambi_rot-ERROR: do not support Ambisonic-Order greater than 5 in zyx-Rotation !!!");
        
        c4z = 1.0f + 8.0f*ccz*(ccz - 1.0f);
        s4z = 2.0f*s2z*c2z;
        
        c4y = 1.0f + 8.0f*ccy*(ccy - 1.0f);
        s4y = 2.0f*s2y*c2y;
        
        c4x = 1.0f + 8.0f*ccx*(ccx - 1.0f);
        s4x = 2.0f*s2x*c2x;
        
        r9_z[0][0] = c4z;
        r9_z[0][1] = -s4z;
        r9_z[1][0] = s4z;
        r9_z[1][1] = c4z;
        r9_z[2][2] = c3z;
        r9_z[2][3] = -s3z;
        r9_z[3][2] = s3z;
        r9_z[3][3] = c3z;
        r9_z[4][4] = c2z;
        r9_z[4][5] = -s2z;
        r9_z[5][4] = s2z;
        r9_z[5][5] = c2z;
        r9_z[6][6] = cz;
        r9_z[6][7] = -sz;
        r9_z[7][6] = sz;
        r9_z[7][7] = cz;
        r9_z[8][8] = 1.0f;
        /*y
        r9_11=(35 + 28*c2 + c4)/64;
        r9_22=(7c + c3)/8;
        r9_31 = x->x_sqrt2*(14*s2 + s4)/32;
        r9_33=(7*c2 + c4)/8;
        r9_42 = x->x_sqrt2*(7*s + 3*s3)/16;
        r9_44=(7*c + 9*c3)/16;
        r9_51 = x->x_sqrt7*(5 - 4*c2 - c4)/32;
        r9_53 = x->x_sqrt14*(2*s2 + s4)/16;
        r9_55=(5 + 4*c2 + 7*c4)/16;
        r9_62 = x->x_sqrt7*(c - c3)/8;
        r9_64 = x->x_sqrt14*(-s + 3*s3)/16;
        r9_66=(c + 7*c3)/8;
        r9_71 = x->x_sqrt14*(2*s2 - s4)/32;
        r9_73 = x->x_sqrt7*(c2 - c4)/8;
        r9_75 = x->x_sqrt2*(-2*s2 + 7*s4)/16;
        r9_77=(c2 + 7*c4)/8;
        r9_82 = x->x_sqrt14*(3*s - s3)/16;
        r9_84 = x->x_sqrt7*(3*c - 3*c3)/16;
        r9_86 = x->x_sqrt2*(3*s + 7*s3)/16;
        r9_88=(9*c + 7*c3)/16;
        r9_91 = x->x_sqrt35*(3 - 4*c2 + c4)/64;
        r9_93 = x->x_sqrt70*(2*s2 - s4)/32;
        r9_95 = x->x_sqrt5*(3 + 4*c2 - 7*c4)/32;
        r9_97 = x->x_sqrt10*(2*s2 + 7*s4)/32;
        r9_99=(9 + 20*c2 + 35*c4)/64;   
        */
        r9_y[0][0] = (35.0f + 28.0f*c2y + c4y)*0.015625f;/* -r9_31y, r9_51y, -r9_71y, r9_91y; */
        r9_y[1][1] = (7.0f*cy + c3y)*0.125f;/* -r9_42y, r9_62y, -r9_82y;*/
        r9_y[2][0] = x->x_sqrt2_16*(14.0f*s2y + s4y)*0.5f;
        r9_y[2][2] = (7.0f*c2y + c4y)*0.125f;/* -r9_53y, r9_73y, -r9_93y;*/
        r9_y[3][1] = x->x_sqrt2_16*(7.0f*sy + 3.0f*s3y);
        r9_y[3][3] = (7.0f*cy + 9.0f*c3y)*0.0625f;/* -r9_64y, r9_84y;*/
        r9_y[4][0] = x->x_sqrt7_8*(5.0f - 4.0f*c2y - c4y)*0.25f;
        r9_y[4][2] = x->x_sqrt14_16*(2.0f*s2y + s4y);
        r9_y[4][4] = (5.0f + 4.0f*c2y + 7.0f*c4y)*0.0625f;/* -r9_75y, r9_95y;*/
        r9_y[5][1] = x->x_sqrt7_8*(cy - c3y);
        r9_y[5][3] = x->x_sqrt14_16*(3.0f*s3y - sy);
        r9_y[5][5] = (cy + 7.0f*c3y)*0.125f;/* -r9_86y;*/
        r9_y[6][0] = x->x_sqrt14_16*(2.0f*s2y - s4y)*0.5f;
        r9_y[6][2] = x->x_sqrt7_8*(c2y - c4y);
        r9_y[6][4] = x->x_sqrt2_16*(7.0f*s4y - 2.0f*s2y);
        r9_y[6][6] = (c2y + 7.0f*c4y)*0.125f;/* -r9_97y;*/
        r9_y[7][1] = x->x_sqrt14_16*(3.0f*sy - s3y);
        r9_y[7][3] = x->x_sqrt7_8*(cy - c3y)*1.5f;
        r9_y[7][5] = x->x_sqrt2_16*(3.0f*sy + 7.0f*s3y);
        r9_y[7][7] = (9.0f*cy + 7.0f*c3y)*0.0625f;
        r9_y[8][0] = x->x_sqrt35_64*(3.0f - 4.0f*c2y + c4y);
        r9_y[8][2] = x->x_sqrt70_32*(2.0f*s2y - s4y);
        r9_y[8][4] = x->x_sqrt5_32*(3.0f + 4.0f*c2y - 7.0f*c4y);
        r9_y[8][6] = x->x_sqrt10_4*(2.0f*s2y + 7.0f*s4y)*0.125f;
        r9_y[8][8] = (9.0f + 20.0f*c2y + 35.0f*c4y)*0.015625f;
        
        r9_y[0][2] = -r9_y[2][0];
        r9_y[0][4] = r9_y[4][0];
        r9_y[0][6] = -r9_y[6][0];
        r9_y[0][8] = r9_y[8][0];
        r9_y[1][3] = -r9_y[3][1];
        r9_y[1][5] = r9_y[5][1];
        r9_y[1][7] = -r9_y[7][1];
        r9_y[2][4] = -r9_y[4][2];
        r9_y[2][6] = r9_y[6][2];
        r9_y[2][8] = -r9_y[8][2];
        r9_y[3][5] = -r9_y[5][3];
        r9_y[3][7] = r9_y[7][3];
        r9_y[4][6] = -r9_y[6][4];
        r9_y[4][8] = r9_y[8][4];
        r9_y[5][7] = -r9_y[7][5];
        r9_y[6][8] = -r9_y[8][6];
        
        /*x 
        r9_11=(35 + 28*c2 + c4)/64;
        r9_22=(7c + c3)/8;
        r9_32 = x->x_sqrt2*(7*s + 3*s3)/16;
        r9_33=(7*c + 9*c3)/16;
        r9_41 = x->x_sqrt2*(-14*s2 - s4)/32;
        r9_44=(7*c2 + c4)/8;
        r9_51 = x->x_sqrt7*(-5 + 4*c2 + c4)/32;
        r9_54 = x->x_sqrt14*(2*s2 + s4)/16;
        r9_55=(5 + 4*c2 + 7*c4)/16;
        r9_62 = x->x_sqrt7*(-c + c3)/8;
        r9_63 = x->x_sqrt14*(s - 3*s3)/16;
        r9_66=(c + 7*c3)/8;
        r9_72 = x->x_sqrt14*(-3s + s3)/16;
        r9_73 = x->x_sqrt7*(-3*c + 3*c3)/16;
        r9_76 = x->x_sqrt2*(3*s + 7*s3)/16;
        r9_77=(9*c + 7*c3)/16;
        r9_81 = x->x_sqrt14*(2*s2 - s4)/32;
        r9_84 = x->x_sqrt7*(-c2 + c4)/8;
        r9_85 = x->x_sqrt2*(2*s2 - 7*s4)/16;
        r9_88=(c2 + 7*c4)/8;
        r9_91 = x->x_sqrt35*(3 - 4*c2 + c4)/64;
        r9_94 = x->x_sqrt70*(-2*s2 + s4)/32;
        r9_95 = x->x_sqrt5*(-3 - 4*c2 + 7*c4)/32;
        r9_98 = x->x_sqrt10*(2*s2 + 7*s4)/32;
        r9_99=(9 + 20*c2 + 35*c4)/64;
        */
        
        r9_x[0][0] = (35.0f + 28.0f*c2x + c4x)*0.015625f;/* -r9_41x, r9_51x, -r9_81x, r9_91x;*/
        r9_x[1][1] = (7.0f*cx + c3x)*0.125f;/* -r9_32x, r9_62x, -r9_72x;*/
        r9_x[2][1] = x->x_sqrt2_16*(7.0f*sx + 3.0f*s3x);
        r9_x[2][2] = (7.0f*cx + 9.0f*c3x)*0.0625f;/* -r9_63x, r9_73x;*/
        r9_x[3][0] = -x->x_sqrt2_16*(14.0f*s2x + s4x)*0.5f;
        r9_x[3][3] = (7.0f*c2x + c4x)*0.125f;/* -r9_54x, r9_84x, -r9_94x;*/
        r9_x[4][0] = x->x_sqrt7_8*(4.0f*c2x + c4x - 5.0f)*0.25f;
        r9_x[4][3] = x->x_sqrt14_16*(2.0f*s2x + s4x);
        r9_x[4][4] = (5.0f + 4.0f*c2x + 7.0f*c4x)*0.0625f;/* -r9_85x, r9_95x;*/
        r9_x[5][1] = x->x_sqrt7_8*(c3x - cx);
        r9_x[5][2] = x->x_sqrt14_16*(sx - 3.0f*s3x);
        r9_x[5][5] = (cx + 7.0f*c3x)*0.125f;/* -r9_76x;*/
        r9_x[6][1] = x->x_sqrt14_16*(s3x - 3.0f*sx);
        r9_x[6][2] = x->x_sqrt7_8*(c3x - cx)*1.5f;
        r9_x[6][5] = x->x_sqrt2_16*(3.0f*sx + 7.0f*s3x);
        r9_x[6][6] = (9.0f*cx + 7.0f*c3x)*0.0625f;
        r9_x[7][0] = x->x_sqrt14_16*(2.0f*s2x - s4x)*0.5f;
        r9_x[7][3] = x->x_sqrt7_8*(c4x - c2x);
        r9_x[7][4] = x->x_sqrt2_16*(2.0f*s2x - 7.0f*s4x);
        r9_x[7][7] = (c2x + 7.0f*c4x)*0.125f;/* -r9_98x*/
        r9_x[8][0] = x->x_sqrt35_64*(3.0f - 4.0f*c2x + c4x);
        r9_x[8][3] = x->x_sqrt70_32*(s4x - 2.0f*s2x);
        r9_x[8][4] = x->x_sqrt5_32*(7.0f*c4x - 3.0f - 4.0f*c2x);
        r9_x[8][7] = x->x_sqrt10_4*(2.0f*s2x + 7.0f*s4x)*0.125f;
        r9_x[8][8] = (9.0f + 20.0f*c2x + 35.0f*c4x)*0.015625f;
        
        r9_x[0][3] = -r9_x[3][0];
        r9_x[0][4] = r9_x[4][0];
        r9_x[0][7] = -r9_x[7][0];
        r9_x[0][8] = r9_x[8][0];
        r9_x[1][2] = -r9_x[2][1];
        r9_x[1][5] = r9_x[5][1];
        r9_x[1][6] = -r9_x[6][1];
        r9_x[2][5] = -r9_x[5][2];
        r9_x[2][6] = r9_x[6][2];
        r9_x[3][4] = -r9_x[4][3];
        r9_x[3][7] = r9_x[7][3];
        r9_x[3][8] = -r9_x[8][3];
        r9_x[4][7] = -r9_x[7][4];
        r9_x[4][8] = r9_x[8][4];
        r9_x[5][6] = -r9_x[6][5];
        r9_x[7][8] = -r9_x[8][7];
        
        for(j=0; j<8; j+=2)
        {
          for(i=0; i<8; i+=2)
          {
            r9_zy[j][i] = r9_z[j][j]*r9_y[j][i];
            r9_zy[j][i+1] = r9_z[j][j+1]*r9_y[j+1][i+1];
            
            r9_zy[j+1][i] = r9_z[j+1][j]*r9_y[j][i];
            r9_zy[j+1][i+1] = r9_z[j+1][j+1]*r9_y[j+1][i+1];
          }
          r9_zy[j][8] = r9_z[j][j]*r9_y[j][8];
          r9_zy[j+1][8] = r9_z[j+1][j]*r9_y[j][8];
        }
        for(i=0; i<=8; i+=2)
        {
          r9_zy[8][i] = r9_y[8][i];
        }
        
        at = x->x_at9;
        at += 2;
        
        for(i=0; i<8; i++)
        {
          SETFLOAT(at, (r9_zy[i][0]*r9_x[0][0] + r9_zy[i][3]*r9_x[3][0] + r9_zy[i][4]*r9_x[4][0] + r9_zy[i][7]*r9_x[7][0] + r9_zy[i][8]*r9_x[8][0]));
          at++;
          SETFLOAT(at, (r9_zy[i][1]*r9_x[1][1] + r9_zy[i][2]*r9_x[2][1] + r9_zy[i][5]*r9_x[5][1] + r9_zy[i][6]*r9_x[6][1]));
          at++;
          SETFLOAT(at, (r9_zy[i][1]*r9_x[1][2] + r9_zy[i][2]*r9_x[2][2] + r9_zy[i][5]*r9_x[5][2] + r9_zy[i][6]*r9_x[6][2]));
          at++;
          SETFLOAT(at, (r9_zy[i][0]*r9_x[0][3] + r9_zy[i][3]*r9_x[3][3] + r9_zy[i][4]*r9_x[4][3] + r9_zy[i][7]*r9_x[7][3] + r9_zy[i][8]*r9_x[8][3]));
          at++;
          SETFLOAT(at, (r9_zy[i][0]*r9_x[0][4] + r9_zy[i][3]*r9_x[3][4] + r9_zy[i][4]*r9_x[4][4] + r9_zy[i][7]*r9_x[7][4] + r9_zy[i][8]*r9_x[8][4]));
          at++;
          SETFLOAT(at, (r9_zy[i][1]*r9_x[1][5] + r9_zy[i][2]*r9_x[2][5] + r9_zy[i][5]*r9_x[5][5] + r9_zy[i][6]*r9_x[6][5]));
          at++;
          SETFLOAT(at, (r9_zy[i][1]*r9_x[1][6] + r9_zy[i][2]*r9_x[2][6] + r9_zy[i][5]*r9_x[5][6] + r9_zy[i][6]*r9_x[6][6]));
          at++;
          SETFLOAT(at, (r9_zy[i][0]*r9_x[0][7] + r9_zy[i][3]*r9_x[3][7] + r9_zy[i][4]*r9_x[4][7] + r9_zy[i][7]*r9_x[7][7] + r9_zy[i][8]*r9_x[8][7]));
          at++;
          SETFLOAT(at, (r9_zy[i][0]*r9_x[0][8] + r9_zy[i][3]*r9_x[3][8] + r9_zy[i][4]*r9_x[4][8] + r9_zy[i][7]*r9_x[7][8] + r9_zy[i][8]*r9_x[8][8]));
          at++;
        }
        
        SETFLOAT(at, (r9_zy[8][0]*r9_x[0][0] + r9_zy[8][4]*r9_x[4][0] + r9_zy[8][8]*r9_x[8][0]));
        at++;
        SETFLOAT(at, (r9_zy[8][2]*r9_x[2][1] + r9_zy[8][6]*r9_x[6][1]));
        at++;
        SETFLOAT(at, (r9_zy[8][2]*r9_x[2][2] + r9_zy[8][6]*r9_x[6][2]));
        at++;
        SETFLOAT(at, (r9_zy[8][0]*r9_x[0][3] + r9_zy[8][4]*r9_x[4][3] + r9_zy[8][8]*r9_x[8][3]));
        at++;
        SETFLOAT(at, (r9_zy[8][0]*r9_x[0][4] + r9_zy[8][4]*r9_x[4][4] + r9_zy[8][8]*r9_x[8][4]));
        at++;
        SETFLOAT(at, (r9_zy[8][2]*r9_x[2][5] + r9_zy[8][6]*r9_x[6][5]));
        at++;
        SETFLOAT(at, (r9_zy[8][2]*r9_x[2][6] + r9_zy[8][6]*r9_x[6][6]));
        at++;
        SETFLOAT(at, (r9_zy[8][0]*r9_x[0][7] + r9_zy[8][4]*r9_x[4][7] + r9_zy[8][8]*r9_x[8][7]));
        at++;
        SETFLOAT(at, (r9_zy[8][0]*r9_x[0][8] + r9_zy[8][4]*r9_x[4][8] + r9_zy[8][8]*r9_x[8][8]));
        
        outlet_anything(x->x_out9, x->x_s_matrix, x->x_size9, x->x_at9);
      }
      
      r9_z[0][0] = c3z;
      r9_z[0][1] = -s3z;
      r9_z[1][0] = s3z;
      r9_z[1][1] = c3z;
      r9_z[2][2] = c2z;
      r9_z[2][3] = -s2z;
      r9_z[3][2] = s2z;
      r9_z[3][3] = c2z;
      r9_z[4][4] = cz;
      r9_z[4][5] = -sz;
      r9_z[5][4] = sz;
      r9_z[5][5] = cz;
      r9_z[6][6] = 1.0f;
      /*y
      r7_11=(15*c + c3)/16;
      r7_22=(5 + 3*c2)/8;
      r7_31 = x->x_sqrt6*(5*s + s3)/16;
      r7_33=(5*c + 3*c3)/8;
      r7_42 = x->x_sqrt6*(s2)/4;
      r7_44=(c2);
      r7_51 = x->x_sqrt15*(c - c3)/16;
      r7_53 = x->x_sqrt10*(-s + 3*s3)/16;
      r7_55=(c + 15*c3)/16;
      r7_62 = x->x_sqrt15*(1 - c2)/8;
      r7_64 = x->x_sqrt10*(s2)/4;
      r7_66=(3 + 5*c2)/8;
      r7_71 = x->x_sqrt10*(3*s - s3)/16;
      r7_73 = x->x_sqrt15*(c - c3)/8 = 2*r7_51;
      r7_75 = x->x_sqrt6*(s + 5*s3)/16;
      r7_77=(3*c + 5*c3)/8;
      
      */
      r9_y[0][0] = (15.0f*cy + c3y)*0.0625f;/* -r7_31y, r7_51y, -r7_71y;*/
      r9_y[1][1] = (5.0f + 3.0f*c2y)*0.125f;/* -r7_42y, r7_62y;*/
      r9_y[2][0] = x->x_sqrt6_4*(5.0f*sy + s3y)*0.25f;
      r9_y[2][2] = (5.0f*cy + 3.0f*c3y)*0.125f;/* -r7_53y, r7_73y;*/
      r9_y[3][1] = x->x_sqrt6_4*s2y;
      r9_y[3][3] = c2y;/* -r7_64y;*/
      r9_y[6][2] = x->x_sqrt15_8*(cy - c3y);
      r9_y[4][0] = r9_y[6][2]*0.5f;
      r9_y[4][2] = x->x_sqrt10_4*(3.0f*s3y - sy)*0.25f;
      r9_y[4][4] = (cy + 15.0f*c3y)*0.0625f;/* -r7_75y;*/
      r9_y[5][1] = x->x_sqrt15_8*(1.0f - c2y);
      r9_y[5][3] = x->x_sqrt10_4*s2y;
      r9_y[5][5] = (3.0f + 5.0f*c2y)*0.125f;
      r9_y[6][0] = x->x_sqrt10_4*(3.0f*sy - s3y)*0.25f;
      
      r9_y[6][4] = x->x_sqrt6_4*(sy + 5.0f*s3y)*0.25f;
      r9_y[6][6] = (3.0f*cy + 5.0f*c3y)*0.125f;
      
      r9_y[0][2] = -r9_y[2][0];
      r9_y[0][4] = r9_y[4][0];
      r9_y[0][6] = -r9_y[6][0];
      r9_y[1][3] = -r9_y[3][1];
      r9_y[1][5] = r9_y[5][1];
      r9_y[2][4] = -r9_y[4][2];
      r9_y[2][6] = r9_y[6][2];
      r9_y[3][5] = -r9_y[5][3];
      r9_y[4][6] = -r9_y[6][4];
      
      /*x
      r7_11=(5 + 3*c2)/8;
      r7_22=(15*c + c3)/16;
      r7_32 = x->x_sqrt6*(5*s + s3)/16;
      r7_33=(5*c + 3*c3)/8;
      r7_41 = x->x_sqrt6*(-s2)/4;
      r7_44=(c2);
      r7_51 = x->x_sqrt15*(-1 + c2)/8;
      r7_54 = x->x_sqrt10*(s2)/4;
      r7_55=(3 + 5*c2)/8;
      r7_62 = x->x_sqrt15*(-c + c3)/16;
      r7_63 = x->x_sqrt10*(s - 3*s3)/16;
      r7_66=(c + 15*c3)/16;
      r7_72 = x->x_sqrt10*(-3*s + s3)/16;
      r7_73 = x->x_sqrt15*(-c + c3)/8 = 2*r7_62;
      r7_76 = x->x_sqrt6*(s + 5*s3)/16;
      r7_77=(3*c + 5*c3)/8;
      */
      
      r9_x[0][0] = (5.0f + 3.0f*c2x)*0.125f;/* -r7_41, r7_51;*/
      r9_x[1][1] = (15.0f*cx + c3x)*0.0625f;/* -r7_32, r7_62, -r7_72;*/
      r9_x[2][1] = x->x_sqrt6_4*(5.0f*sx + s3x)*0.25f;
      r9_x[2][2] = (5.0f*cx + 3.0f*c3x)*0.125f;/* */
      r9_x[3][0] = -x->x_sqrt6_4*s2x;
      r9_x[3][3] = c2x;
      r9_x[4][0] = x->x_sqrt15_8*(c2x - 1.0f);
      r9_x[4][3] = x->x_sqrt10_4*s2x;
      r9_x[4][4] = (3.0f + 5.0f*c2x)*0.125f;
      r9_x[6][2] = x->x_sqrt15_8*(c3x - cx);
      r9_x[5][1] = r9_x[6][2]*0.5f;
      r9_x[5][2] = x->x_sqrt10_4*(sx - 3.0f*s3x)*0.25f;
      r9_x[5][5] = (cx + 15.0f*c3x)*0.0625f;
      r9_x[6][1] = x->x_sqrt10_4*(s3x - 3.0f*sx)*0.25f;
      
      r9_x[6][5] = x->x_sqrt6_4*(sx + 5.0f*s3x)*0.25f;
      r9_x[6][6] = (3.0f*cx + 5.0f*c3x)*0.125f;
      
      r9_x[0][3] = -r9_x[3][0];
      r9_x[0][4] = r9_x[4][0];
      r9_x[1][2] = -r9_x[2][1];
      r9_x[1][5] = r9_x[5][1];
      r9_x[1][6] = -r9_x[6][1];
      r9_x[2][5] = -r9_x[5][2];
      r9_x[2][6] = r9_x[6][2];
      r9_x[3][4] = -r9_x[4][3];
      r9_x[5][6] = -r9_x[6][5];
      
      for(j=0; j<6; j+=2)
      {
        for(i=0; i<6; i+=2)
        {
          r9_zy[j][i] = r9_z[j][j]*r9_y[j][i];
          r9_zy[j][i+1] = r9_z[j][j+1]*r9_y[j+1][i+1];
          
          r9_zy[j+1][i] = r9_z[j+1][j]*r9_y[j][i];
          r9_zy[j+1][i+1] = r9_z[j+1][j+1]*r9_y[j+1][i+1];
        }
        r9_zy[j][6] = r9_z[j][j]*r9_y[j][6];
        r9_zy[j+1][6] = r9_z[j+1][j]*r9_y[j][6];
      }
      for(i=0; i<=6; i+=2)
      {
        r9_zy[6][i] = r9_y[6][i];
      }
      
      at = x->x_at7;
      at += 2;
      
      for(i=0; i<6; i++)
      {
        SETFLOAT(at, (r9_zy[i][0]*r9_x[0][0] + r9_zy[i][3]*r9_x[3][0] + r9_zy[i][4]*r9_x[4][0]));
        at++;
        SETFLOAT(at, (r9_zy[i][1]*r9_x[1][1] + r9_zy[i][2]*r9_x[2][1] + r9_zy[i][5]*r9_x[5][1] + r9_zy[i][6]*r9_x[6][1]));
        at++;
        SETFLOAT(at, (r9_zy[i][1]*r9_x[1][2] + r9_zy[i][2]*r9_x[2][2] + r9_zy[i][5]*r9_x[5][2] + r9_zy[i][6]*r9_x[6][2]));
        at++;
        SETFLOAT(at, (r9_zy[i][0]*r9_x[0][3] + r9_zy[i][3]*r9_x[3][3] + r9_zy[i][4]*r9_x[4][3]));
        at++;
        SETFLOAT(at, (r9_zy[i][0]*r9_x[0][4] + r9_zy[i][3]*r9_x[3][4] + r9_zy[i][4]*r9_x[4][4]));
        at++;
        SETFLOAT(at, (r9_zy[i][1]*r9_x[1][5] + r9_zy[i][2]*r9_x[2][5] + r9_zy[i][5]*r9_x[5][5] + r9_zy[i][6]*r9_x[6][5]));
        at++;
        SETFLOAT(at, (r9_zy[i][1]*r9_x[1][6] + r9_zy[i][2]*r9_x[2][6] + r9_zy[i][5]*r9_x[5][6] + r9_zy[i][6]*r9_x[6][6]));
        at++;
      }
      
      SETFLOAT(at, (r9_zy[6][0]*r9_x[0][0] + r9_zy[6][4]*r9_x[4][0]));
      at++;
      SETFLOAT(at, (r9_zy[6][2]*r9_x[2][1] + r9_zy[6][6]*r9_x[6][1]));
      at++;
      SETFLOAT(at, (r9_zy[6][2]*r9_x[2][2] + r9_zy[6][6]*r9_x[6][2]));
      at++;
      SETFLOAT(at, (r9_zy[6][0]*r9_x[0][3] + r9_zy[6][4]*r9_x[4][3]));
      at++;
      SETFLOAT(at, (r9_zy[6][0]*r9_x[0][4] + r9_zy[6][4]*r9_x[4][4]));
      at++;
      SETFLOAT(at, (r9_zy[6][2]*r9_x[2][5] + r9_zy[6][6]*r9_x[6][5]));
      at++;
      SETFLOAT(at, (r9_zy[6][2]*r9_x[2][6] + r9_zy[6][6]*r9_x[6][6]));
      
      outlet_anything(x->x_out7, x->x_s_matrix, x->x_size7, x->x_at7);
    }
    
    r9_z[0][0] = c2z;
    r9_z[0][1] = -s2z;
    r9_z[1][0] = s2z;
    r9_z[1][1] = c2z;
    r9_z[2][2] = cz;
    r9_z[2][3] = -sz;
    r9_z[3][2] = sz;
    r9_z[3][3] = cz;
    r9_z[4][4] = 1.0f;
    /*y
    r5_11=(3 + c2)/4;
    r5_22=(c);
    r5_31 = (s2)/2;
    r5_33=(c2);
    r5_42 = (s);
    r5_44=(c);
    r5_51 = x->x_sqrt3*(1 - c2)/4;
    r5_53 = x->x_sqrt3*(s2)/2;
    r5_55=(1 + 3*c2)/4; 
    */
    r9_y[0][0] = (3.0f + c2y)*0.25f;/* -r5_31y, r5_51y;*/
    r9_y[1][1] = cy;/* -r5_42y;*/
    r9_y[2][0] = s2y*0.5f;
    r9_y[2][2] = c2y;/* -r5_53y;*/
    r9_y[3][1] = sy;
    r9_y[3][3] = cy;
    r9_y[4][0] = x->x_sqrt3_2*(1.0f - c2y)*0.5f;
    r9_y[4][2] = x->x_sqrt3_2*s2y;
    r9_y[4][4] = (1.0f + 3.0f*c2y)*0.25f;
    
    r9_y[0][2] = -r9_y[2][0];
    r9_y[0][4] = r9_y[4][0];
    r9_y[1][3] = -r9_y[3][1];
    r9_y[2][4] = -r9_y[4][2];
    
    /*x
    r5_11=(3 + c2)/4;
    r5_22=(c);
    r5_32 = (s);
    r5_33=(c);
    r5_41 = (-s2)/2;
    r5_44=(c2);
    r5_51 = x->x_sqrt3*(-1 + c2)/4;
    r5_54 = x->x_sqrt3*(s2)/2;
    r5_55=(1 + 3*c2)/4;
    */
    
    r9_x[0][0] = (3.0f + c2x)*0.25f;
    r9_x[1][1] = cx;
    r9_x[2][1] = sx;
    r9_x[2][2] = cx;
    r9_x[3][0] = -0.5f*s2x;
    r9_x[3][3] = c2x;
    r9_x[4][0] = x->x_sqrt3_2*(c2x - 1.0f)*0.5f;
    r9_x[4][3] = x->x_sqrt3_2*s2x;
    r9_x[4][4] = (1.0f + 3.0f*c2x)*0.25f;
    
    r9_x[0][3] = -r9_x[3][0];
    r9_x[0][4] = r9_x[4][0];
    r9_x[1][2] = -r9_x[2][1];
    r9_x[3][4] = -r9_x[4][3];
    
    for(j=0; j<4; j+=2)
    {
      for(i=0; i<4; i+=2)
      {
        r9_zy[j][i] = r9_z[j][j]*r9_y[j][i];
        r9_zy[j][i+1] = r9_z[j][j+1]*r9_y[j+1][i+1];
        
        r9_zy[j+1][i] = r9_z[j+1][j]*r9_y[j][i];
        r9_zy[j+1][i+1] = r9_z[j+1][j+1]*r9_y[j+1][i+1];
      }
      r9_zy[j][4] = r9_z[j][j]*r9_y[j][4];
      r9_zy[j+1][4] = r9_z[j+1][j]*r9_y[j][4];
    }
    for(i=0; i<=4; i+=2)
    {
      r9_zy[4][i] = r9_y[4][i];
    }
    
    at = x->x_at5;
    at += 2;
    
    for(i=0; i<4; i++)
    {
      SETFLOAT(at, (r9_zy[i][0]*r9_x[0][0] + r9_zy[i][3]*r9_x[3][0] + r9_zy[i][4]*r9_x[4][0]));
      at++;
      SETFLOAT(at, (r9_zy[i][1]*r9_x[1][1] + r9_zy[i][2]*r9_x[2][1]));
      at++;
      SETFLOAT(at, (r9_zy[i][1]*r9_x[1][2] + r9_zy[i][2]*r9_x[2][2]));
      at++;
      SETFLOAT(at, (r9_zy[i][0]*r9_x[0][3] + r9_zy[i][3]*r9_x[3][3] + r9_zy[i][4]*r9_x[4][3]));
      at++;
      SETFLOAT(at, (r9_zy[i][0]*r9_x[0][4] + r9_zy[i][3]*r9_x[3][4] + r9_zy[i][4]*r9_x[4][4]));
      at++;
    }
    
    SETFLOAT(at, (r9_zy[4][0]*r9_x[0][0] + r9_zy[4][4]*r9_x[4][0]));
    at++;
    SETFLOAT(at, (r9_zy[4][2]*r9_x[2][1]));
    at++;
    SETFLOAT(at, (r9_zy[4][2]*r9_x[2][2]));
    at++;
    SETFLOAT(at, (r9_zy[4][0]*r9_x[0][3] + r9_zy[4][4]*r9_x[4][3]));
    at++;
    SETFLOAT(at, (r9_zy[4][0]*r9_x[0][4] + r9_zy[4][4]*r9_x[4][4]));
    
    outlet_anything(x->x_out5, x->x_s_matrix, x->x_size5, x->x_at5);
  }
  
  r9_z[0][0] = cz;
  r9_z[0][1] = -sz;
  r9_z[1][0] = sz;
  r9_z[1][1] = cz;
  r9_z[2][2] = 1.0f;
  /*y
  r3_11=(c);
  r3_22=(1);
  r3_31 = (s);
  r3_33=(c);
  */
  r9_y[0][0] = cy;/* -r3_31y;*/
  r9_y[1][1] = 1.0f;
  r9_y[2][0] = sy;
  r9_y[2][2] = cy;
  
  r9_y[0][2] = -r9_y[2][0];
  
  /*x
  r3_11=(1);
  r3_22=(c);
  r3_32 = (s);
  r3_33=(c);
  */
  r9_x[0][0] = 1.0f;
  r9_x[1][1] = cx;/* -r3_32x;*/
  r9_x[2][1] = sx;
  r9_x[2][2] = cx;
  
  r9_x[1][2] = -r9_x[2][1];
  
  r9_zy[0][0] = cz*r9_y[0][0];
  r9_zy[0][1] = -sz*r9_y[1][1];
  r9_zy[0][2] = cz*r9_y[0][2];
  
  r9_zy[1][0] = sz*r9_y[0][0];
  r9_zy[1][1] = cz*r9_y[1][1];
  r9_zy[1][2] = sz*r9_y[0][2];
  
  r9_zy[2][0] = r9_y[2][0];
  r9_zy[2][1] = 0.0f;
  r9_zy[2][2] = r9_y[2][2];
  
  at = x->x_at3;
  at += 2;
  
  for(i=0; i<2; i++)
  {
    SETFLOAT(at, (r9_zy[i][0]*r9_x[0][0]));
    at++;
    SETFLOAT(at, (r9_zy[i][1]*r9_x[1][1] + r9_zy[i][2]*r9_x[2][1]));
    at++;
    SETFLOAT(at, (r9_zy[i][1]*r9_x[1][2] + r9_zy[i][2]*r9_x[2][2]));
    at++;
  }
  
  SETFLOAT(at, (r9_zy[2][0]*r9_x[0][0]));
  at++;
  SETFLOAT(at, (r9_zy[2][2]*r9_x[2][1]));
  at++;
  SETFLOAT(at, (r9_zy[2][2]*r9_x[2][2]));
  
  outlet_anything(x->x_out3, x->x_s_matrix, x->x_size3, x->x_at3);
}

static void ambi_rot_list(t_ambi_rot *x, t_symbol *s, int argc, t_atom *argv)
{
  if(argc == 1)
    ambi_rot_float(x, atom_getfloatarg(0, argc, argv));/* = ambi_rot_z(); */
  else if(argc == 2)
    ambi_rot_zy(x, atom_getfloatarg(0, argc, argv), atom_getfloatarg(1, argc, argv));
  else if(argc >= 3)
    ambi_rot_zyx(x, atom_getfloatarg(0, argc, argv), atom_getfloatarg(1, argc, argv), atom_getfloatarg(2, argc, argv));
}

static void ambi_rot_free(t_ambi_rot *x)
{
  if(x->x_size11)
    freebytes(x->x_at11, x->x_size11 * sizeof(t_atom));
  if(x->x_size9)
    freebytes(x->x_at9, x->x_size9 * sizeof(t_atom));
  if(x->x_size7)
    freebytes(x->x_at7, x->x_size7 * sizeof(t_atom));
  if(x->x_size5)
    freebytes(x->x_at5, x->x_size5 * sizeof(t_atom));
  if(x->x_size3)
    freebytes(x->x_at3, x->x_size3 * sizeof(t_atom));
  if(x->x_size2)
    freebytes(x->x_at2, x->x_size2 * sizeof(t_atom));
}

static void *ambi_rot_new(t_floatarg forder)
{
  t_ambi_rot *x = (t_ambi_rot *)pd_new(ambi_rot_class);
  t_atom *at;
  int i=(int)forder;
  
  if(i < 1)
    i = 1;
  if(i > 12)
    i = 12;
  x->x_order = i;
  x->x_size2 = 2*2 + 2;
  x->x_at2 = (t_atom *)getbytes(x->x_size2 * sizeof(t_atom));
  at = x->x_at2;
  SETFLOAT(at, 2.0f);
  at++;
  SETFLOAT(at, 2.0f);
  at++;
  for(i=0; i<4; i++)
  {
    SETFLOAT(at, 0.0f);
    at++;
  }
  x->x_size3 = 3*3 + 2;
  x->x_at3 = (t_atom *)getbytes(x->x_size3 * sizeof(t_atom));
  at = x->x_at3;
  SETFLOAT(at, 3.0f);
  at++;
  SETFLOAT(at, 3.0f);
  at++;
  for(i=0; i<9; i++)
  {
    SETFLOAT(at, 0.0f);
    at++;
  }
  x->x_out3 = outlet_new(&x->x_obj, &s_list);
  if(x->x_order >= 2)
  {
    x->x_size5 = 5*5 + 2;
    x->x_at5 = (t_atom *)getbytes(x->x_size5 * sizeof(t_atom));
    at = x->x_at5;
    SETFLOAT(at, 5.0f);
    at++;
    SETFLOAT(at, 5.0f);
    at++;
    for(i=0; i<25; i++)
    {
      SETFLOAT(at, 0.0f);
      at++;
    }
    x->x_out5 = outlet_new(&x->x_obj, &s_list);
  }
  else
  {
    x->x_size5 = 0;
    x->x_at5 = (t_atom *)0;
  }
  if(x->x_order >= 3)
  {
    x->x_size7 = 7*7 + 2;
    x->x_at7 = (t_atom *)getbytes(x->x_size7 * sizeof(t_atom));
    at = x->x_at7;
    SETFLOAT(at, 7.0f);
    at++;
    SETFLOAT(at, 7.0f);
    at++;
    for(i=0; i<49; i++)
    {
      SETFLOAT(at, 0.0f);
      at++;
    }
    x->x_out7 = outlet_new(&x->x_obj, &s_list);
  }
  else
  {
    x->x_size7 = 0;
    x->x_at7 = (t_atom *)0;
  }
  if(x->x_order >= 4)
  {
    x->x_size9 = 9*9 + 2;
    x->x_at9 = (t_atom *)getbytes(x->x_size9 * sizeof(t_atom));
    at = x->x_at9;
    SETFLOAT(at, 9.0f);
    at++;
    SETFLOAT(at, 9.0f);
    at++;
    for(i=0; i<81; i++)
    {
      SETFLOAT(at, 0.0f);
      at++;
    }
    x->x_out9 = outlet_new(&x->x_obj, &s_list);
  }
  else
  {
    x->x_size9 = 0;
    x->x_at9 = (t_atom *)0;
  }
  
  if(x->x_order >= 5)
  {
    x->x_size11 = 11*11 + 2;
    x->x_at11 = (t_atom *)getbytes(x->x_size11 * sizeof(t_atom));
    at = x->x_at11;
    SETFLOAT(at, 11.0f);
    at++;
    SETFLOAT(at, 11.0f);
    at++;
    for(i=0; i<121; i++)
    {
      SETFLOAT(at, 0.0f);
      at++;
    }
    x->x_out11 = outlet_new(&x->x_obj, &s_list);
  }
  else
  {
    x->x_size11 = 0;
    x->x_at11 = (t_atom *)0;
  }
  
  if(x->x_order >= 6)
    x->x_out13 = outlet_new(&x->x_obj, &s_list);
  
  if(x->x_order >= 7)
    x->x_out15 = outlet_new(&x->x_obj, &s_list);
  
  if(x->x_order >= 8)
    x->x_out17 = outlet_new(&x->x_obj, &s_list);
  
  if(x->x_order >= 9)
    x->x_out19 = outlet_new(&x->x_obj, &s_list);
  
  if(x->x_order >= 10)
    x->x_out21 = outlet_new(&x->x_obj, &s_list);
  
  if(x->x_order >= 11)
    x->x_out23 = outlet_new(&x->x_obj, &s_list);
  
  if(x->x_order >= 12)
    x->x_out25 = outlet_new(&x->x_obj, &s_list);
  
  x->x_sqrt2_16   = (t_float)(sqrt(2.0) / 16.0);
  x->x_sqrt3_2    = (t_float)(sqrt(3.0) / 2.0);
  x->x_sqrt5_32   = (t_float)(sqrt(5.0) / 32.0);
  x->x_sqrt6_4    = (t_float)(sqrt(6.0) / 4.0);
  x->x_sqrt7_8    = (t_float)(sqrt(7.0) / 8.0);
  x->x_sqrt10_4   = (t_float)(sqrt(10.0) / 4.0);
  x->x_sqrt14_16  = (t_float)(sqrt(14.0) / 16.0);
  x->x_sqrt15_8   = (t_float)(sqrt(15.0) / 8.0);
  x->x_sqrt35_64  = (t_float)(sqrt(35.0) / 64.0);
  x->x_sqrt70_32  = (t_float)(sqrt(70.0) / 32.0);
  
  x->x_pi_over_180  = (t_float)(4.0 * atan(1.0) / 180.0);
  
  x->x_s_matrix = gensym("matrix");
  return (x);
}

void ambi_rot_setup(void)
{
  ambi_rot_class = class_new(gensym("ambi_rot"), (t_newmethod)ambi_rot_new, (t_method)ambi_rot_free,
    sizeof(t_ambi_rot), 0, A_DEFFLOAT, 0);
  class_addfloat(ambi_rot_class, (t_method)ambi_rot_float);
  class_addlist(ambi_rot_class, (t_method)ambi_rot_list);
//  class_sethelpsymbol(ambi_rot_class, gensym("iemhelp2/ambi_rot-help"));
}
