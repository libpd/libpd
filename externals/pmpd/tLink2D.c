#include "m_pd.h"
#include "math.h"

static t_class *tLink2D_class;

typedef struct _tLink2D {
  t_object  x_obj;
  t_float distance_old, position2Dx1, position2Dy1, position2Dx2, position2Dy2;
  t_outlet *force1;
  t_outlet *force2;
  t_outlet *force3;
  t_outlet *force4;
  t_symbol *x_sym;  // receive
} t_tLink2D;

void tLink2D_reset(t_tLink2D *x)
{
}

void tLink2D_resetF(t_tLink2D *x)
{
}

void tLink2D_resetL(t_tLink2D *x)
{
}

void tLink2D_setK(t_tLink2D *x, t_float K)
{
}

void tLink2D_setL(t_tLink2D *x, t_float L)
{
}

void tLink2D_setD(t_tLink2D *x, t_float D)
{
}

void tLink2D_setD2(t_tLink2D *x, t_float D)
{
}

void tLink2D_Lmin(t_tLink2D *x, t_float Lmin)
{
}

void tLink2D_Lmax(t_tLink2D *x, t_float Lmax)
{
}

void tLink2D_position2D(t_tLink2D *x, t_floatarg f1, t_floatarg f2)
{
  x->position2Dx1 = f1;
  x->position2Dy1 = f2;
}

void tLink2D_position2D2(t_tLink2D *x, t_floatarg f1, t_floatarg f2)
{
  x->position2Dx2 = f1;
  x->position2Dy2 = f2;
}

void tLink2D_bang(t_tLink2D *x)
{
  t_float vitesse, distance, orientation;
  t_atom force1[2];

  distance = sqrt ( pow((x->position2Dx2-x->position2Dx1), 2) + pow((x->position2Dy2-x->position2Dy1), 2) );

  vitesse = x->distance_old - distance;

  SETFLOAT(&(force1[0]), (x->position2Dx2 + x->position2Dx1)/ 2);
  SETFLOAT(&(force1[1]), (x->position2Dy2 + x->position2Dy1)/ 2);
 
  outlet_anything(x->force4, gensym("position2D"), 2, force1);

  if ((x->position2Dx2-x->position2Dx1) != 0)
  {
	  orientation = 180/3.14159 * atan((float)(x->position2Dy2 - x->position2Dy1)/(x->position2Dx2 - x->position2Dx1));
		  if ((x->position2Dx2 - x->position2Dx1)<0)
			  orientation +=180;
		  if (orientation<0)
			  orientation +=360;

	  outlet_float(x->force3, orientation);
  }
  else
  {
	  if ((x->position2Dy2 - x->position2Dy1)<0)
		  outlet_float(x->force3,270);
	  else 	
		  outlet_float(x->force3,90);
  }

  outlet_float(x->force2, vitesse);

  outlet_float(x->force1, distance);

  x->distance_old = distance;

}


static void tLink2D_free(t_tLink2D *x)
{
    pd_unbind(&x->x_obj.ob_pd, x->x_sym);
}

void *tLink2D_new(t_symbol *s)
{ 
  t_tLink2D *x = (t_tLink2D *)pd_new(tLink2D_class);

  x->x_sym = s;
  pd_bind(&x->x_obj.ob_pd, s);

  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("position2D"), gensym("position2D2"));
 
  x->force1=outlet_new(&x->x_obj, 0);
  x->force2=outlet_new(&x->x_obj, 0);
  x->force3=outlet_new(&x->x_obj, 0);
  x->force4=outlet_new(&x->x_obj, 0);

  x->distance_old = 0;

  return (x);
}

void tLink2D_setup(void) 
{

  tLink2D_class = class_new(gensym("tLink2D"),
        (t_newmethod)tLink2D_new,
        (t_method)tLink2D_free, 
		sizeof(t_tLink2D),
        CLASS_DEFAULT, A_DEFSYM, 0);

  class_addcreator((t_newmethod)tLink2D_new, gensym("tLia2D"), A_DEFSYM, 0);


  class_addbang(tLink2D_class, tLink2D_bang);

  class_addmethod(tLink2D_class, (t_method)tLink2D_position2D, gensym("position2D"), A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addmethod(tLink2D_class, (t_method)tLink2D_position2D2, gensym("position2D2"), A_DEFFLOAT, A_DEFFLOAT, 0);

  // only for the object not to output erreor when having the same name as the link
  class_addmethod(tLink2D_class, (t_method)tLink2D_reset, gensym("reset"), 0);
  class_addmethod(tLink2D_class, (t_method)tLink2D_resetL, gensym("resetL"), 0);
  class_addmethod(tLink2D_class, (t_method)tLink2D_resetF, gensym("resetF"), 0);
  class_addmethod(tLink2D_class, (t_method)tLink2D_setD, gensym("setD"), A_DEFFLOAT, 0);
  class_addmethod(tLink2D_class, (t_method)tLink2D_setD2, gensym("setD2"), A_DEFFLOAT, 0);
  class_addmethod(tLink2D_class, (t_method)tLink2D_setK, gensym("setK"), A_DEFFLOAT, 0);
  class_addmethod(tLink2D_class, (t_method)tLink2D_setL, gensym("setL"), A_DEFFLOAT, 0);
  class_addmethod(tLink2D_class, (t_method)tLink2D_Lmin, gensym("setLmin"), A_DEFFLOAT, 0);
  class_addmethod(tLink2D_class, (t_method)tLink2D_Lmax, gensym("setLmax"), A_DEFFLOAT, 0);

}
