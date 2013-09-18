#include "m_pd.h"
#include "math.h"

static t_class *tCircle2D_class;

typedef struct _tCircle2D {
  t_object  x_obj;
  t_float  X, Y, Rmin, Rmax, distance_old;
  t_outlet *force_new, *distance, *vitesse; // outlet
} t_tCircle2D;


void tCircle2D_position2D(t_tCircle2D *x, t_float X, t_float Y)
{

	t_float tmp, vitesse;

	tmp = sqrt((X-x->X)*(X-x->X) + (Y-x->Y)*(Y-x->Y));

	vitesse = tmp - x->distance_old ;

	x->distance_old = tmp;

	outlet_float(x->vitesse, vitesse);

	outlet_float(x->distance, tmp);

 	if (  (tmp < x->Rmax) & (tmp >= x->Rmin))
	{
		outlet_float(x->force_new, 1);
	}
	else
	{
		outlet_float(x->force_new, 0);
	}
}

void tCircle2D_XY(t_tCircle2D *x, t_float X, t_float Y)
{
   x->X= X;
   x->Y= Y;
}

void tCircle2D_X(t_tCircle2D *x, t_float X)
{
   x->X= X;
}

void tCircle2D_Y(t_tCircle2D *x, t_float X)
{
   x->Y= X;
}

void tCircle2D_Rmin(t_tCircle2D *x, t_float X)
{
   x->Rmin= X;
}

void tCircle2D_Rmax(t_tCircle2D *x, t_float X)
{
   x->Rmax= X;
}

void *tCircle2D_new(t_symbol *s, int argc, t_atom *argv)
{
  t_tCircle2D *x = (t_tCircle2D *)pd_new(tCircle2D_class);
  x->force_new=outlet_new(&x->x_obj, 0);
  x->distance=outlet_new(&x->x_obj, 0);
  x->vitesse=outlet_new(&x->x_obj, 0);

  x->distance_old = 0;

  if (argc>=4)
    x->Rmax = atom_getfloatarg(3, argc, argv);
  else
    x->Rmax = 1;

  if (argc>=3)
    x->Rmin = atom_getfloatarg(2, argc, argv);
  else
    x->Rmin = 0;

  if (argc>=2)
    x->Y = atom_getfloatarg(1, argc, argv);
  else
    x->Y = 0;

  if (argc>=1)
    x->X = atom_getfloatarg(0, argc, argv);
  else
    x->X = 0;

  return (x);
}

void tCircle2D_setup(void) 
{

  tCircle2D_class = class_new(gensym("tCircle2D"),
        (t_newmethod)tCircle2D_new,
        0, sizeof(t_tCircle2D),
        CLASS_DEFAULT, A_GIMME, 0);

  class_addcreator((t_newmethod)tCircle2D_new, gensym("pmpd.tCircle2D"), A_GIMME, 0);

  class_addmethod(tCircle2D_class, (t_method)tCircle2D_position2D, gensym("position2D"), A_DEFFLOAT, A_DEFFLOAT, 0);

  class_addmethod(tCircle2D_class, (t_method)tCircle2D_XY, gensym("setXY"), A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addmethod(tCircle2D_class, (t_method)tCircle2D_X, gensym("setX"), A_DEFFLOAT, 0);
  class_addmethod(tCircle2D_class, (t_method)tCircle2D_Y, gensym("setY"), A_DEFFLOAT, 0);
  class_addmethod(tCircle2D_class, (t_method)tCircle2D_Rmin, gensym("setRmin"), A_DEFFLOAT, 0);
  class_addmethod(tCircle2D_class, (t_method)tCircle2D_Rmax, gensym("setRmax"), A_DEFFLOAT, 0);
}
