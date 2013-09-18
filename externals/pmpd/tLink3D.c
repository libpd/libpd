#include "m_pd.h"
#include "math.h"

static t_class *tLink3D_class;

typedef struct _tLink3D {
  t_object  x_obj;
  t_float distance_old, position2Dx1, position2Dy1, position2Dz1, position2Dx2, position2Dy2,  position2Dz2;
  t_outlet *force1;
  t_outlet *force2;
  t_outlet *force3;
  t_outlet *force4;
  t_symbol *x_sym;  // receive
} t_tLink3D;

void tLink3D_reset(t_tLink3D *x)
{
}

void tLink3D_resetF(t_tLink3D *x)
{
}

void tLink3D_resetL(t_tLink3D *x)
{
}


void tLink3D_setK(t_tLink3D *x, t_float K)
{
}

void tLink3D_setL(t_tLink3D *x, t_float L)
{
}

void tLink3D_setD(t_tLink3D *x, t_float D)
{
}

void tLink3D_setD2(t_tLink3D *x, t_float D)
{
}

void tLink3D_Lmin(t_tLink3D *x, t_float Lmin)
{
}

void tLink3D_Lmax(t_tLink3D *x, t_float Lmax)
{
}


void tLink3D_position3D(t_tLink3D *x, t_floatarg f1, t_floatarg f2, t_floatarg f3)
{
  x->position2Dx1 = f1;
  x->position2Dy1 = f2;
  x->position2Dz1 = f3;
}

void tLink3D_position3D2(t_tLink3D *x, t_floatarg f1, t_floatarg f2, t_floatarg f3)
{
  x->position2Dx2 = f1;
  x->position2Dy2 = f2;
  x->position2Dz2 = f3;
}

void tLink3D_bang(t_tLink3D *x)
{
  t_float vitesse, distance, orientation;
  t_atom force1[3];

  distance = sqrt ( pow((x->position2Dx2-x->position2Dx1), 2) + pow((x->position2Dy2-x->position2Dy1), 2) + pow((x->position2Dz2-x->position2Dz1), 2) );

  vitesse = x->distance_old - distance;


  SETFLOAT(&(force1[0]), (x->position2Dx2 + x->position2Dx1)/ 2);
  SETFLOAT(&(force1[1]), (x->position2Dy2 + x->position2Dy1)/ 2);
  SETFLOAT(&(force1[2]), (x->position2Dz2 + x->position2Dz1)/ 2);
 
  outlet_anything(x->force4, gensym("position3D"), 3, force1);

  
  SETFLOAT(&(force1[0]), (x->position2Dx2 - x->position2Dx1)/ distance);
  SETFLOAT(&(force1[1]), (x->position2Dy2 - x->position2Dy1)/ distance);
  SETFLOAT(&(force1[2]), (x->position2Dz2 - x->position2Dz1)/ distance);
 
  outlet_anything(x->force3, gensym("vector3D"), 3, force1);

  outlet_float(x->force2, vitesse);

  outlet_float(x->force1, distance);

  x->distance_old = distance;

}


static void tLink3D_free(t_tLink3D *x)
{
    pd_unbind(&x->x_obj.ob_pd, x->x_sym);
}

void *tLink3D_new(t_symbol *s)
{
  
  t_tLink3D *x = (t_tLink3D *)pd_new(tLink3D_class);

  x->x_sym = s;
  pd_bind(&x->x_obj.ob_pd, s);

  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("position3D"), gensym("position3D2"));
 
  x->force1=outlet_new(&x->x_obj, 0);
  x->force2=outlet_new(&x->x_obj, 0);
  x->force3=outlet_new(&x->x_obj, 0);
  x->force4=outlet_new(&x->x_obj, 0);

  x->distance_old = 0;

  return (x);
}

void tLink3D_setup(void) 
{

  tLink3D_class = class_new(gensym("tLink3D"),
        (t_newmethod)tLink3D_new,
        (t_method)tLink3D_free, 
		sizeof(t_tLink3D),
        CLASS_DEFAULT, A_DEFSYM, 0);

  class_addcreator((t_newmethod)tLink3D_new, gensym("tLia3D"), A_DEFSYM, 0);

  class_addbang(tLink3D_class, tLink3D_bang);

  class_addmethod(tLink3D_class, (t_method)tLink3D_position3D, gensym("position3D"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addmethod(tLink3D_class, (t_method)tLink3D_position3D2, gensym("position3D2"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);

  // only for the object not to output erreor when having the same name as the link
  class_addmethod(tLink3D_class, (t_method)tLink3D_reset, gensym("reset"), 0);
  class_addmethod(tLink3D_class, (t_method)tLink3D_resetL, gensym("resetL"), 0);
  class_addmethod(tLink3D_class, (t_method)tLink3D_resetF, gensym("resetF"), 0);
  class_addmethod(tLink3D_class, (t_method)tLink3D_setD, gensym("setD"), A_DEFFLOAT, 0);
  class_addmethod(tLink3D_class, (t_method)tLink3D_setD2, gensym("setD2"), A_DEFFLOAT, 0);
  class_addmethod(tLink3D_class, (t_method)tLink3D_setK, gensym("setK"), A_DEFFLOAT, 0);
  class_addmethod(tLink3D_class, (t_method)tLink3D_setL, gensym("setL"), A_DEFFLOAT, 0);
  class_addmethod(tLink3D_class, (t_method)tLink3D_Lmin, gensym("setLmin"), A_DEFFLOAT, 0);
  class_addmethod(tLink3D_class, (t_method)tLink3D_Lmax, gensym("setLmax"), A_DEFFLOAT, 0);

}
