#include "m_pd.h"
#include "math.h"

static t_class *tSphere3D_class;

typedef struct _tSphere3D {
  t_object  x_obj;
  t_float  X, Y, Z, Rmin, Rmax, position_old;
  t_outlet *force_new, *distance_out, *vitesse;// outlet
} t_tSphere3D;

void tSphere3D_position3D(t_tSphere3D *x, t_float X,  t_float Y, t_float Z)
{
t_float  distance, vitesse;
		distance = sqrt ( (X - x->X)*(X - x->X) + (Y - x->Y)*(Y - x->Y) + (Z - x->Z)*(Z - x->Z));

		vitesse = distance - x->position_old;
		x->position_old = distance;

		outlet_float(x->vitesse, vitesse);

		outlet_float(x->distance_out, distance);

		if ( (distance < x->Rmax) & (distance > x->Rmin) )
		{
			outlet_float(x->force_new, 1);
		}
		else
		{
			outlet_float(x->force_new, 0);
		}

}

void tSphere3D_setXYZ(t_tSphere3D *x, t_float X, t_float Y, t_float Z)
{
  x->X= X;
  x->Y= Y;
  x->Z= Z;
}

void tSphere3D_setX(t_tSphere3D *x, t_float X)
{
  x->X= X;
}

void tSphere3D_setY(t_tSphere3D *x, t_float Y)
{
  x->Y= Y;
}

void tSphere3D_setZ(t_tSphere3D *x, t_float Z)
{
  x->Z= Z;
}

void tSphere3D_setRmax(t_tSphere3D *x, t_float X)
{
  x->Rmax= X;
}

void tSphere3D_setRmin(t_tSphere3D *x, t_float X)
{
  x->Rmin= X;
}



void *tSphere3D_new(t_symbol *s, int argc, t_atom *argv)
{
  t_tSphere3D *x = (t_tSphere3D *)pd_new(tSphere3D_class);

  x->force_new=outlet_new(&x->x_obj, 0);
  x->distance_out=outlet_new(&x->x_obj, 0);
  x->vitesse=outlet_new(&x->x_obj, 0);
    
  x->position_old = 0;


    if (argc>=5)
    x->Rmax= atom_getfloatarg(4, argc, argv);
  else
    x->Rmax= 1;

  if (argc>=4)
    x->Rmin= atom_getfloatarg(3, argc, argv);
  else
    x->Rmin= 0;

  if (argc>=3)
    x->Z= atom_getfloatarg(2, argc, argv);
  else
	x->Z= 0;

  if (argc>=2)
    x->Y= atom_getfloatarg(1, argc, argv);
  else
	x->Y= 0;

  if (argc>=1)
	x->X= atom_getfloatarg(0, argc, argv);
  else
	x->X= 0;

  return (x);
}

void tSphere3D_setup(void) 
{

  tSphere3D_class = class_new(gensym("tSphere3D"),
        (t_newmethod)tSphere3D_new,
        0, sizeof(t_tSphere3D),
        CLASS_DEFAULT, A_GIMME, 0);

  class_addcreator((t_newmethod)tSphere3D_new, gensym("pmpd.tSphere3D"),  A_GIMME, 0);
 
  class_addmethod(tSphere3D_class, (t_method)tSphere3D_position3D, gensym("position3D"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);

  class_addmethod(tSphere3D_class, (t_method)tSphere3D_setX, gensym("setX"), A_DEFFLOAT, 0);
  class_addmethod(tSphere3D_class, (t_method)tSphere3D_setY, gensym("setY"), A_DEFFLOAT, 0);
  class_addmethod(tSphere3D_class, (t_method)tSphere3D_setZ, gensym("setZ"), A_DEFFLOAT, 0);
  class_addmethod(tSphere3D_class, (t_method)tSphere3D_setXYZ, gensym("setXYZ"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addmethod(tSphere3D_class, (t_method)tSphere3D_setRmax, gensym("setRmax"), A_DEFFLOAT, 0);
  class_addmethod(tSphere3D_class, (t_method)tSphere3D_setRmin, gensym("setRmin"), A_DEFFLOAT, 0);


}
