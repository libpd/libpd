/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_ambi written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"
#include <math.h>

/* -------------------------- ambi_encode ------------------------------ */

typedef struct _ambi_encode
{
  t_object  x_obj;
  t_atom    *x_at;
  int       x_size;
  int       x_size2d;
  int       x_size3d;
  t_float   x_sqrt3;
  t_float   x_sqrt10_4;
  t_float   x_sqrt15;
  t_float   x_sqrt6_4;
  t_float   x_sqrt35_2;
  t_float   x_sqrt70_4;
  t_float   x_sqrt5_2;
  t_float   x_sqrt126_16;
  t_float   x_sqrt315_2;
  t_float   x_sqrt105_2;
  t_float   x_pi_over_180;
  t_float   *x_ambi_order_weight;
  int       x_colrow;
  int       x_n_order;
} t_ambi_encode;

static t_class *ambi_encode_class;

static void ambi_encode_ambi_weight(t_ambi_encode *x, t_symbol *s, int argc, t_atom *argv)
{
  if(argc > x->x_n_order)
  {
    int i, n=x->x_n_order;
    
    for(i=0; i<=n; i++)
    {
      x->x_ambi_order_weight[i] = atom_getfloat(argv++);
    }
  }
  else
    post("ambi_encode-ERROR: ambi_weight needs %d float weights", x->x_n_order+1);
}

static void ambi_encode_do_2d(t_ambi_encode *x, t_floatarg phi)
{
  t_float c, s, cc, ss, c2, s2, c3, s3, s4, c4, s5, c5, s6, c6;
  t_float *awght = x->x_ambi_order_weight;
  t_atom *at=x->x_at;
  
  phi *= x->x_pi_over_180;
  c = cos(phi);
  s = sin(phi);
  cc = c*c;
  ss = s*s;
  
  SETFLOAT(at, (t_float)x->x_colrow);
  at++;
  
  SETFLOAT(at, awght[0]);
  at++;
  
  SETFLOAT(at, c*awght[1]);
  at++;
  SETFLOAT(at, s*awght[1]);
  at++;
  
  if(x->x_n_order >= 2)
  {
    c2 = cc - ss;
    s2 = 2.0f*s*c;
    SETFLOAT(at, c2*awght[2]);
    at++;
    SETFLOAT(at, s2*awght[2]);
    at++;
    
    if(x->x_n_order >= 3)
    {
      c3 = c*(4.0f*cc - 3.0f);
      s3 = s*(3.0f - 4.0f*ss);
      SETFLOAT(at, c3*awght[3]);
      at++;
      SETFLOAT(at, s3*awght[3]);
      at++;
      
      if(x->x_n_order >= 4)
      {
        c4 = 1.0f + 8.0f*cc*(cc - 1.0f);
        s4 = 2.0f*s2*c2;
        SETFLOAT(at, c4*awght[4]);
        at++;
        SETFLOAT(at, s4*awght[4]);
        at++;
        
        if(x->x_n_order >= 5)
        {
          c5 = c*(1.0f + 4.0f*ss*(ss - 3.0f*cc));
          s5 = s*(1.0f + 4.0f*cc*(cc - 3.0f*ss));
          SETFLOAT(at, c5*awght[5]);
          at++;
          SETFLOAT(at, s5*awght[5]);
          at++;
          
          if(x->x_n_order >= 6)
          {
            c6 = c3*c3 - s3*s3;
            s6 = 2.0f*s3*c3;
            SETFLOAT(at, c6*awght[6]);
            at++;
            SETFLOAT(at, s6*awght[6]);
            at++;
            
            if(x->x_n_order >= 7)
            {
              SETFLOAT(at, cos(7.0f*phi)*awght[7]);
              at++;
              SETFLOAT(at, sin(7.0f*phi)*awght[7]);
              at++;
              
              if(x->x_n_order >= 8)
              {
                SETFLOAT(at, (c4*c4 - s4*s4)*awght[8]);
                at++;
                SETFLOAT(at, 2.0f*s4*c4*awght[8]);
                at++;
                
                if(x->x_n_order >= 9)
                {
                  SETFLOAT(at, cos(9.0f*phi)*awght[9]);
                  at++;
                  SETFLOAT(at, sin(9.0f*phi)*awght[9]);
                  at++;
                  
                  if(x->x_n_order >= 10)
                  {
                    SETFLOAT(at, (c5*c5 - s5*s5)*awght[10]);
                    at++;
                    SETFLOAT(at, 2.0f*s5*c5*awght[10]);
                    at++;
                    
                    if(x->x_n_order >= 11)
                    {
                      SETFLOAT(at, cos(11.0f*phi)*awght[11]);
                      at++;
                      SETFLOAT(at, sin(11.0f*phi)*awght[11]);
                      at++;
                      
                      if(x->x_n_order >= 12)
                      {
                        SETFLOAT(at, (c6*c6 - s6*s6)*awght[12]);
                        at++;
                        SETFLOAT(at, 2.0f*s6*c6*awght[12]);
                      }
                      
                      if(x->x_n_order >= 13)
                        post("ambi_encode-ERROR: do not support Ambisonic-Order greater than 12 in 2d !!!");
                      
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}

static void ambi_encode_do_3d(t_ambi_encode *x, t_symbol *s, int argc, t_atom *argv)
{
  t_float delta, phi;
  t_float cd, x1, y, z, x2, y2, z2, x2my2, x2m3y2, p3x2my2, xy, xz, yz, m1p5z2, m1p7z2, m3p7z2;
  t_float *awght = x->x_ambi_order_weight;
  t_atom *at=x->x_at;
  
  delta = atom_getfloat(argv++)*x->x_pi_over_180;
  phi = atom_getfloat(argv)*x->x_pi_over_180;
  
  cd = cos(delta);
  x1 = cd * cos(phi);
  y = cd * sin(phi);
  z = sin(delta);
  
  xy = x1*y;
  xz = x1*z;
  yz = y*z;
  x2 = x1*x1;
  y2 = y*y;
  z2 = z*z;
  
  x2my2 = x2 - y2;
  x2m3y2 = x2my2 - 2.0f*y2;
  p3x2my2 = 2.0f*x2 + x2my2;
  m1p5z2 = 5.0f*z2 - 1.0f;
  m1p7z2 = 2.0f*z2 + m1p5z2;
  m3p7z2 = m1p7z2 - 2.0f;
  
  SETFLOAT(at, (t_float)x->x_colrow);
  at++;
  
  SETFLOAT(at, awght[0]);
  at++;
  
  SETFLOAT(at, x1*awght[1]);
  at++;
  SETFLOAT(at, y*awght[1]);
  at++;
  SETFLOAT(at, z*awght[1]);
  at++;
  
  if(x->x_n_order >= 2)
  {
    SETFLOAT(at, 0.5f*x->x_sqrt3*x2my2*awght[2]);
    at++;
    SETFLOAT(at, x->x_sqrt3*xy*awght[2]);
    at++;
    SETFLOAT(at, x->x_sqrt3*xz*awght[2]);
    at++;
    SETFLOAT(at, x->x_sqrt3*yz*awght[2]);
    at++;
    SETFLOAT(at, 0.5f*(3.0f*z2 - 1.0f)*awght[2]);
    at++;
    
    if(x->x_n_order >= 3)
    {
      SETFLOAT(at, x->x_sqrt10_4*x1*x2m3y2*awght[3]);
      at++;
      SETFLOAT(at, x->x_sqrt10_4*y*p3x2my2*awght[3]);
      at++;
      SETFLOAT(at, 0.5f*x->x_sqrt15*z*x2my2*awght[3]);
      at++;
      SETFLOAT(at, x->x_sqrt15*xy*z*awght[3]);
      at++;
      SETFLOAT(at, x->x_sqrt6_4*x1*m1p5z2*awght[3]);
      at++;
      SETFLOAT(at, x->x_sqrt6_4*y*m1p5z2*awght[3]);
      at++;
      SETFLOAT(at, 0.5f*z*(m1p5z2 - 2.0f)*awght[3]);
      at++;
      
      if(x->x_n_order >= 4)
      {
        SETFLOAT(at, 0.25f*x->x_sqrt35_2*(x2my2*x2my2 - 4.0f*x2*y2)*awght[4]);
        at++;
        SETFLOAT(at, x->x_sqrt35_2*xy*x2my2*awght[4]);
        at++;
        SETFLOAT(at, x->x_sqrt70_4*xz*x2m3y2*awght[4]);
        at++;
        SETFLOAT(at, x->x_sqrt70_4*yz*p3x2my2*awght[4]);
        at++;
        SETFLOAT(at, 0.5f*x->x_sqrt5_2*x2my2*m1p7z2*awght[4]);
        at++;
        SETFLOAT(at, x->x_sqrt5_2*xy*m1p7z2*awght[4]);
        at++;
        SETFLOAT(at, x->x_sqrt10_4*xz*m3p7z2*awght[4]);
        at++;
        SETFLOAT(at, x->x_sqrt10_4*yz*m3p7z2*awght[4]);
        at++;
        SETFLOAT(at, 0.125f*(5.0f*(z2 - 1.0f)*(m1p7z2 + 2.0f) + 8.0f)*awght[4]);
        at++;
        
        if(x->x_n_order >= 5)
        {
          SETFLOAT(at, x->x_sqrt126_16*x1*(x2*(x2 - 10.0f*y2) + 5.0f*y2*y2)*awght[5]);
          at++;
          SETFLOAT(at, x->x_sqrt126_16*y*(y2*(y2 - 10.0f*x2) + 5.0f*x2*x2)*awght[5]);
          at++;
          SETFLOAT(at, 0.25f*x->x_sqrt315_2*z*(y2*(y2 - 6.0f*x2) + x2*x2)*awght[5]);
          at++;
          SETFLOAT(at, x->x_sqrt315_2*xy*z*x2my2*awght[5]);
          at++;
          SETFLOAT(at, 0.25f*x->x_sqrt70_4*x1*(9.0f*z2 - 1.0f)*x2m3y2*awght[5]);
          at++;
          SETFLOAT(at, 0.25f*x->x_sqrt70_4*y*(9.0f*z2 - 1.0f)*p3x2my2*awght[5]);
          at++;
          SETFLOAT(at, 0.5f*x->x_sqrt105_2*x2my2*z*(3.0f*z2 - 1.0f)*awght[5]);
          at++;
          SETFLOAT(at, x->x_sqrt105_2*xy*z*(3.0f*z2 - 1.0f)*awght[5]);
          at++;
          SETFLOAT(at, 0.125f*x->x_sqrt15*x1*(z2*(21.0f*z2 - 14.0f) + 1.0f)*awght[5]);
          at++;
          SETFLOAT(at, 0.125f*x->x_sqrt15*y*(z2*(21.0f*z2 - 14.0f) + 1.0f)*awght[5]);
          at++;
          SETFLOAT(at, 0.125f*z*(z2*(63.0f*z2 - 70.0f) + 15.0f)*awght[5]);
        }
        
        if(x->x_n_order > 5)
          post("ambi_encode-ERROR: do not support Ambisonic-Order greater than 5 in 3d !!!");
      }
    }
  }
}

static void ambi_encode_float(t_ambi_encode *x, t_floatarg phi)
{
  x->x_colrow = -1;
  ambi_encode_do_2d(x, phi);
  outlet_list(x->x_obj.ob_outlet, &s_list, x->x_size2d, x->x_at+1);
}

static void ambi_encode_list(t_ambi_encode *x, t_symbol *s, int argc, t_atom *argv)
{
  if(argc <= 0)
  {
    post("ambi_encode ERROR: list-input needs 2 angles: delta [rad] and phi [rad]");
    return;
  }
  else if(argc == 1)
  {
    ambi_encode_float(x, atom_getfloat(argv));
  }
  else
  {
    x->x_colrow = -1;
    ambi_encode_do_3d(x, &s_list, 2, argv);
    outlet_list(x->x_obj.ob_outlet, &s_list, x->x_size3d, x->x_at+1);
  }
}

static void ambi_encode_row(t_ambi_encode *x, t_symbol *s, int argc, t_atom *argv)
{
  if(argc == 2)
  {
    x->x_colrow = (int)atom_getint(argv++);
    ambi_encode_do_2d(x, atom_getfloat(argv));
    outlet_anything(x->x_obj.ob_outlet, s, x->x_size2d+1, x->x_at);
  }
  else if(argc >= 3)
  {
    x->x_colrow = (int)atom_getint(argv++);
    ambi_encode_do_3d(x, &s_list, 2, argv);
    outlet_anything(x->x_obj.ob_outlet, s, x->x_size3d+1, x->x_at);
  }
  else
  {
    post("ambi_encode-ERROR: row needs <float> row-index + <float> angle ( + <float> angle)");
  }
}

static void ambi_encode_col(t_ambi_encode *x, t_symbol *s, int argc, t_atom *argv)
{
  if(argc == 2)
  {
    x->x_colrow = (int)atom_getint(argv++);
    ambi_encode_do_2d(x, atom_getfloat(argv));
    outlet_anything(x->x_obj.ob_outlet, s, x->x_size2d+1, x->x_at);
  }
  else if(argc >= 3)
  {
    x->x_colrow = (int)atom_getint(argv++);
    ambi_encode_do_3d(x, &s_list, 2, argv);
    outlet_anything(x->x_obj.ob_outlet, s, x->x_size3d+1, x->x_at);
  }
  else
  {
    post("ambi_encode-ERROR: col needs <float> col-index + <float> angle ( + <float> angle)");
  }
}

static void ambi_encode_free(t_ambi_encode *x)
{
  freebytes(x->x_ambi_order_weight, (x->x_n_order+1) * sizeof(t_float));
  freebytes(x->x_at, x->x_size * sizeof(t_atom));
}

static void *ambi_encode_new(t_floatarg forder)
{
  t_ambi_encode *x = (t_ambi_encode *)pd_new(ambi_encode_class);
  t_atom *at;
  int i=(int)forder;
  
  if(i < 1)
    i = 1;
  if(i > 12)
    i = 12;
  x->x_n_order = i;
  x->x_size = 6*6 + 1;
  x->x_size2d = 2*i + 1;
  x->x_size3d = (i + 1)*(i + 1);
  
  x->x_sqrt3      = (t_float)(sqrt(3.0));
  x->x_sqrt5_2    = (t_float)(sqrt(5.0) / 2.0);
  x->x_sqrt6_4    = (t_float)(sqrt(6.0) / 4.0);
  x->x_sqrt10_4   = (t_float)(sqrt(10.0) / 4.0);
  x->x_sqrt15     = (t_float)(sqrt(15.0));
  x->x_sqrt35_2   = (t_float)(sqrt(35.0) / 2.0);
  x->x_sqrt70_4   = (t_float)(sqrt(70.0) / 4.0);
  x->x_sqrt126_16 = (t_float)(sqrt(126.0) / 16.0);
  x->x_sqrt315_2  = (t_float)(sqrt(315.0) / 2.0);
  x->x_sqrt105_2  = (t_float)(sqrt(105.0) / 2.0);
  x->x_pi_over_180 = (t_float)(4.0 * atan(1.0)/180.0);
  x->x_colrow = 0;
  x->x_ambi_order_weight = (t_float *)getbytes((x->x_n_order+1) * sizeof(t_float));
  x->x_at = (t_atom *)getbytes(x->x_size * sizeof(t_atom));
  at=x->x_at;
  SETFLOAT(at, -1.0f);/*row index*/
  at++;
  SETFLOAT(at, 1.0f);/*W channel*/
  
  for(i=0; i<=x->x_n_order; i++)
    x->x_ambi_order_weight[i] = 1.0f;
  
  outlet_new(&x->x_obj, &s_list);
  return (x);
}

void ambi_encode_setup(void)
{
  ambi_encode_class = class_new(gensym("ambi_encode"), (t_newmethod)ambi_encode_new, (t_method)ambi_encode_free,
    sizeof(t_ambi_encode), 0, A_DEFFLOAT, 0);
  class_addlist(ambi_encode_class, (t_method)ambi_encode_list);
  class_addfloat(ambi_encode_class, (t_method)ambi_encode_float);
  class_addmethod(ambi_encode_class, (t_method)ambi_encode_row, gensym("row"), A_GIMME, 0);
  class_addmethod(ambi_encode_class, (t_method)ambi_encode_col, gensym("col"), A_GIMME, 0);
  class_addmethod(ambi_encode_class, (t_method)ambi_encode_ambi_weight, gensym("ambi_weight"), A_GIMME, 0);
//  class_sethelpsymbol(ambi_encode_class, gensym("iemhelp2/ambi_encode-help"));
}
