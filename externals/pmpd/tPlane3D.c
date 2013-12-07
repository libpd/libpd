#include "m_pd.h"
#include "math.h"

static t_class *tPlane3D_class;

typedef struct _tPlane3D {
  t_object  x_obj;
  t_float   X, Y, Z, VX, VY, VZ, P, distance_old;
  t_outlet *force_new, *profondeur, *vitesse;// outlet
} t_tPlane3D;


void tPlane3D_position3D(t_tPlane3D *x, t_float X,  t_float Y, t_float Z)
{
t_float d, tmp, profondeur, vitesse;

		tmp = sqrt (x->VX*x->VX + x->VY*x->VY + x->VZ*x->VZ);
	if (tmp != 0)
	{
		x->VX /= tmp;
		x->VY /= tmp;
		x->VZ /= tmp;

	}
	else
	{
		x->VX=1;
		x->VY=0;
		x->VZ=0;
	}

		d = x->VX * x->X  + x->VY * x->Y + x->VZ * x->Z;

		profondeur = x->VX * X + x->VY * Y + x->VZ * Z - d;

		vitesse = profondeur - x->distance_old ;

		x->distance_old = profondeur;

		outlet_float(x->vitesse, vitesse);

		outlet_float(x->profondeur, profondeur);


		if ( (profondeur < 0) & (profondeur > - x->P) )
		{
			outlet_float(x->force_new, 1);
		}
		else
		{
			outlet_float(x->force_new, 0);
		}

}

void tPlane3D_setXYZ(t_tPlane3D *x, t_float X, t_float Y, t_float Z)
{
  x->X= X;
  x->Y= Y;
  x->Z= Z;
}
void tPlane3D_setVXYZ(t_tPlane3D *x, t_float X, t_float Y, t_float Z)
{
  x->VX= X;
  x->VY= Y;
  x->VZ= Z;
}

void tPlane3D_setVX(t_tPlane3D *x, t_float X)
{
  x->VX= X;
}

void tPlane3D_setVY(t_tPlane3D *x, t_float Y)
{
  x->VY= Y;
}

void tPlane3D_setVZ(t_tPlane3D *x, t_float Z)
{
  x->VZ= Z;
}
void tPlane3D_setX(t_tPlane3D *x, t_float X)
{
  x->X= X;
}

void tPlane3D_setY(t_tPlane3D *x, t_float Y)
{
  x->Y= Y;
}

void tPlane3D_setZ(t_tPlane3D *x, t_float Z)
{
  x->Z= Z;
}

void tPlane3D_setP(t_tPlane3D *x, t_float X)
{
  x->P= X;
}


void *tPlane3D_new(t_symbol *s, int argc, t_atom *argv)
{
  t_tPlane3D *x = (t_tPlane3D *)pd_new(tPlane3D_class);

  x->force_new=outlet_new(&x->x_obj, 0);
  x->profondeur=outlet_new(&x->x_obj, 0);
  x->vitesse=outlet_new(&x->x_obj, 0);

  x->distance_old = 0;

  if (argc>=7)
    x->P= atom_getfloatarg(6, argc, argv);
  else
    x->P= 10000;

    if (argc>=6)
    x->Z= atom_getfloatarg(5, argc, argv);
  else
    x->Z= 0;

    if (argc>=5)
    x->Y= atom_getfloatarg(4, argc, argv);
  else
    x->Y= 0;

  if (argc>=4)
    x->X= atom_getfloatarg(3, argc, argv);
  else
    x->X= 0;

  if (argc>=3)
    x->VZ= atom_getfloatarg(2, argc, argv);
  else
	x->VZ= 0;

  if (argc>=2)
    x->VY= atom_getfloatarg(1, argc, argv);
  else
	x->VY= 0;

  if (argc>=1)
	x->VX= atom_getfloatarg(0, argc, argv);
  else
	x->VX= 1;

  return (x);
}

void tPlane3D_setup(void) 
{

  tPlane3D_class = class_new(gensym("tPlane3D"),
        (t_newmethod)tPlane3D_new,
        0, sizeof(t_tPlane3D),
        CLASS_DEFAULT, A_GIMME, 0);

  class_addcreator((t_newmethod)tPlane3D_new, gensym("pmpd.tPlane3D"),  A_GIMME, 0);
 
  class_addmethod(tPlane3D_class, (t_method)tPlane3D_position3D, gensym("position3D"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);

  class_addmethod(tPlane3D_class, (t_method)tPlane3D_setVX, gensym("setVX"), A_DEFFLOAT, 0);
  class_addmethod(tPlane3D_class, (t_method)tPlane3D_setVY, gensym("setVY"), A_DEFFLOAT, 0);
  class_addmethod(tPlane3D_class, (t_method)tPlane3D_setVZ, gensym("setVZ"), A_DEFFLOAT, 0);
  class_addmethod(tPlane3D_class, (t_method)tPlane3D_setX, gensym("setX"), A_DEFFLOAT, 0);
  class_addmethod(tPlane3D_class, (t_method)tPlane3D_setY, gensym("setY"), A_DEFFLOAT, 0);
  class_addmethod(tPlane3D_class, (t_method)tPlane3D_setZ, gensym("setZ"), A_DEFFLOAT, 0);
  class_addmethod(tPlane3D_class, (t_method)tPlane3D_setXYZ, gensym("setXYZ"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addmethod(tPlane3D_class, (t_method)tPlane3D_setVXYZ, gensym("setVXYZ"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addmethod(tPlane3D_class, (t_method)tPlane3D_setP, gensym("setPmax"), A_DEFFLOAT, 0);


}
