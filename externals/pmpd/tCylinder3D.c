#include "m_pd.h"
#include "math.h"

static t_class *tCylinder3D_class;

typedef struct _tCylinder3D {
  t_object  x_obj;
  t_float   X, Y, Z, VX, VY, VZ, Pmin, Pmax, Rmin, Rmax, position_old;
  t_outlet *force_new, *profondeur, *vitesse;// outlet
} t_tCylinder3D;

void tCylinder3D_position3D(t_tCylinder3D *x, t_float X,  t_float Y, t_float Z)
{
	t_float d, tmp, profondeur, Xb, Yb, Zb, rayon, vitesse;	

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

		Xb = X - x->X - profondeur * x->VX;
		Yb = Y - x->Y - profondeur * x->VY;
		Zb = Z - x->Z - profondeur * x->VZ;

		rayon = sqrt ( pow(Xb, 2) + pow(Yb, 2)  + pow(Zb, 2) );

		if ( rayon != 0 )
		{
			Xb /= rayon;  // normalisation
			Yb /= rayon;
			Zb /= rayon;
		}

		vitesse = rayon - x->position_old;
		x->position_old = rayon;

		outlet_float(x->vitesse, vitesse);

		outlet_float(x->profondeur, rayon);

		if ( (profondeur < x->Pmin) & (profondeur > x->Pmax) & (rayon < x->Rmax) & (rayon > x->Rmin) )
		{
			outlet_float(x->force_new, 1);
		}
		else
		{
			outlet_float(x->force_new, 0);
		}
//C optimiser ca : pas faire le calcul de l'orientation du cylindre a chaques fois...

}

void tCylinder3D_setXYZ(t_tCylinder3D *x, t_float X, t_float Y, t_float Z)
{
  x->X= X;
  x->Y= Y;
  x->Z= Z;
}
void tCylinder3D_setVXYZ(t_tCylinder3D *x, t_float X, t_float Y, t_float Z)
{
  x->VX= X;
  x->VY= Y;
  x->VZ= Z;
}

void tCylinder3D_setVX(t_tCylinder3D *x, t_float X)
{
  x->VX= X;
}

void tCylinder3D_setVY(t_tCylinder3D *x, t_float Y)
{
  x->VY= Y;
}

void tCylinder3D_setVZ(t_tCylinder3D *x, t_float Z)
{
  x->VZ= Z;
}
void tCylinder3D_setX(t_tCylinder3D *x, t_float X)
{
  x->X= X;
}

void tCylinder3D_setY(t_tCylinder3D *x, t_float Y)
{
  x->Y= Y;
}

void tCylinder3D_setZ(t_tCylinder3D *x, t_float Z)
{
  x->Z= Z;
}

void tCylinder3D_setPmin(t_tCylinder3D *x, t_float X)
{
  x->Pmin= X;
}

void tCylinder3D_setPmax(t_tCylinder3D *x, t_float X)
{
  x->Pmax= X;
}

void tCylinder3D_setRmin(t_tCylinder3D *x, t_float R)
{
  x->Rmin = R;
}

void tCylinder3D_setRmax(t_tCylinder3D *x, t_float R)
{
  x->Rmax = R;
}


void *tCylinder3D_new(t_symbol *s, int argc, t_atom *argv)
{
  t_tCylinder3D *x = (t_tCylinder3D *)pd_new(tCylinder3D_class);

  x->force_new=outlet_new(&x->x_obj, 0);
  x->profondeur=outlet_new(&x->x_obj, 0);
  x->vitesse=outlet_new(&x->x_obj, 0);

  x->position_old = 0;

  if (argc>=10)
    x->Pmax= atom_getfloatarg(9, argc, argv);
  else
    x->Pmax= 1000;

  if (argc>=9)
    x->Pmin= atom_getfloatarg(8, argc, argv);
  else
    x->Pmin= -1000;

    if (argc>=8)
    x->Z= atom_getfloatarg(7, argc, argv);
  else
    x->Rmax= 1;
  
  if (argc>=7)
    x->Rmin= atom_getfloatarg(6, argc, argv);
  else
    x->Rmin= 0;

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

void tCylinder3D_setup(void) 
{

  tCylinder3D_class = class_new(gensym("tCylinder3D"),
        (t_newmethod)tCylinder3D_new,
        0, sizeof(t_tCylinder3D),
        CLASS_DEFAULT, A_GIMME, 0);

  class_addcreator((t_newmethod)tCylinder3D_new, gensym("pmpd.tCylinder3D"),  A_GIMME, 0);
 
  class_addmethod(tCylinder3D_class, (t_method)tCylinder3D_position3D, gensym("position3D"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);

  class_addmethod(tCylinder3D_class, (t_method)tCylinder3D_setVX, gensym("setVX"), A_DEFFLOAT, 0);
  class_addmethod(tCylinder3D_class, (t_method)tCylinder3D_setVY, gensym("setVY"), A_DEFFLOAT, 0);
  class_addmethod(tCylinder3D_class, (t_method)tCylinder3D_setVZ, gensym("setVZ"), A_DEFFLOAT, 0);
  class_addmethod(tCylinder3D_class, (t_method)tCylinder3D_setX, gensym("setX"), A_DEFFLOAT, 0);
  class_addmethod(tCylinder3D_class, (t_method)tCylinder3D_setY, gensym("setY"), A_DEFFLOAT, 0);
  class_addmethod(tCylinder3D_class, (t_method)tCylinder3D_setZ, gensym("setZ"), A_DEFFLOAT, 0);
  class_addmethod(tCylinder3D_class, (t_method)tCylinder3D_setRmin, gensym("setRmin"), A_DEFFLOAT, 0);
  class_addmethod(tCylinder3D_class, (t_method)tCylinder3D_setRmax, gensym("setRmax"), A_DEFFLOAT, 0);
  class_addmethod(tCylinder3D_class, (t_method)tCylinder3D_setXYZ, gensym("setXYZ"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addmethod(tCylinder3D_class, (t_method)tCylinder3D_setVXYZ, gensym("setVXYZ"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addmethod(tCylinder3D_class, (t_method)tCylinder3D_setPmin, gensym("setPmin"), A_DEFFLOAT, 0);
  class_addmethod(tCylinder3D_class, (t_method)tCylinder3D_setPmax, gensym("setPmax"), A_DEFFLOAT, 0);


}
