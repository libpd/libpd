#include "m_pd.h"
#include "math.h"

static t_class *tSeg2D_class;

typedef struct _tSeg2D {
  t_object  x_obj;
  t_float  X1, X2, Y1, Y2, P, P_old;
  t_outlet *force_new, *profondeur, *vitesse;// outlet
} t_tSeg2D;

void tSeg2D_position2D(t_tSeg2D *x, t_float X, t_float Y)
{
	t_float a1, b1, c1, a2, b2, c2, a3, b3, c3, profondeur, tmp;

	b1 = x->X2 - x->X1;
	a1 = -x->Y2 + x->Y1;

	if (!((a1==0) & (b1==0)))
	{
		tmp = sqrt((a1*a1)+(b1*b1));			// = longueur du vecteur pour renormalisation
		a1 = a1/tmp;
		b1 = b1/tmp;
		c1 = a1*x->X1+b1*x->Y1;

		profondeur = ( (a1 * X)  + (b1 *   Y) )  - c1;
	
		tmp = profondeur - x->P_old;

		x->P_old = profondeur;

		a2 = b1;
		b2 = -a1;
		c2 = a2*x->X1+b2*x->Y1;

		a3 = a2;
		b3 = b2;
		c3 = a3*x->X2+b3*x->Y2;

		outlet_float(x->vitesse, tmp);
		outlet_float(x->profondeur, profondeur);

		if ( ( profondeur  < 0) & (profondeur >  - x->P)  & (( (a2 * X)  + (b2 *   Y) )  > c2) & ( ((a3 * X)  + (b3 * Y) )  < c3) )
		{
		outlet_float(x->force_new, 1);
		}
		else
		{
		outlet_float(x->force_new, 0);
		}
	}
	else
	{
	outlet_float(x->force_new, 0);
	}

}

void tSeg2D_Xmin(t_tSeg2D *x, t_float X)
{
   x->X1= X;
}

void tSeg2D_Xmax(t_tSeg2D *x, t_float X)
{
   x->X2= X;
}

void tSeg2D_Ymin(t_tSeg2D *x, t_float X)
{
   x->Y1= X;
}

void tSeg2D_Ymax(t_tSeg2D *x, t_float X)
{
   x->Y2= X;
}

void tSeg2D_P(t_tSeg2D *x, t_float X)
{
   x->P= X;
}

void *tSeg2D_new(t_symbol *s, int argc, t_atom *argv)
{
  t_tSeg2D *x = (t_tSeg2D *)pd_new(tSeg2D_class);
  x->force_new = outlet_new(&x->x_obj, 0);
  x->profondeur =outlet_new(&x->x_obj, 0);
  x->vitesse = outlet_new(&x->x_obj, 0);

  x->P_old = 0;

  if (argc>=5)
    x->P = atom_getfloatarg(4, argc, argv);
  else
    x->P = 1;

    if (argc>=4)
    x->Y2 = atom_getfloatarg(3, argc, argv);
  else
    x->Y2 = 0;

  if (argc>=3)
    x->X2 = atom_getfloatarg(2, argc, argv);
  else
    x->X2 = 1;

  if (argc>=2)
    x->Y1 = atom_getfloatarg(1, argc, argv);
  else
    x->Y1 = 0;

  if (argc>=1)
    x->X1 = atom_getfloatarg(0, argc, argv);
  else
    x->X1 = -1;

  return (x);
}

void tSeg2D_setup(void) 
{

  tSeg2D_class = class_new(gensym("tSeg2D"),
        (t_newmethod)tSeg2D_new,
        0, sizeof(t_tSeg2D),
        CLASS_DEFAULT, A_GIMME, 0);

  class_addcreator((t_newmethod)tSeg2D_new, gensym("pmpd.tSeg2D"), A_GIMME, 0);

  class_addmethod(tSeg2D_class, (t_method)tSeg2D_position2D, gensym("position2D"), A_DEFFLOAT, A_DEFFLOAT, 0);

  class_addmethod(tSeg2D_class, (t_method)tSeg2D_Xmin, gensym("setX1"), A_DEFFLOAT, 0);
  class_addmethod(tSeg2D_class, (t_method)tSeg2D_Ymin, gensym("setY1"), A_DEFFLOAT, 0);
  class_addmethod(tSeg2D_class, (t_method)tSeg2D_Xmax, gensym("setX2"), A_DEFFLOAT, 0);
  class_addmethod(tSeg2D_class, (t_method)tSeg2D_Ymax, gensym("setY2"), A_DEFFLOAT, 0);
 class_addmethod(tSeg2D_class, (t_method)tSeg2D_P, gensym("setPmax"), A_DEFFLOAT, 0);

}
