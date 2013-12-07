#include "m_pd.h"
#include "math.h"

static t_class *tSquare2D_class;

typedef struct _tSquare2D {
  t_object  x_obj;
  t_float  Xmin, Xmax, Ymin, Ymax;
  //extrem =  Xmin, Ymin, Xmax, Ymax;
  t_outlet *force_new;// outlet
} t_tSquare2D;

void tSquare2D_position2D(t_tSquare2D *x, t_float X, t_float Y)
{

 	if ((X > x->Xmin) & (X < x->Xmax) & (Y > x->Ymin) & (Y < x->Ymax))
	{
		outlet_float(x->force_new, 1);
	}
	else
	{
		outlet_float(x->force_new, 0);
	}
}

void tSquare2D_Xmin(t_tSquare2D *x, t_float X)
{
   x->Xmin= X;
}

void tSquare2D_Xmax(t_tSquare2D *x, t_float X)
{
   x->Xmax= X;
}

void tSquare2D_Ymin(t_tSquare2D *x, t_float X)
{
   x->Ymin= X;
}

void tSquare2D_Ymax(t_tSquare2D *x, t_float X)
{
   x->Ymax= X;
}

void *tSquare2D_new(t_symbol *s, int argc, t_atom *argv)
{
  t_tSquare2D *x = (t_tSquare2D *)pd_new(tSquare2D_class);
  x->force_new=outlet_new(&x->x_obj, 0);

  if (argc>=4)
    x->Ymax = atom_getfloatarg(3, argc, argv);
  else
    x->Ymax = 1;

  if (argc>=3)
    x->Ymin = atom_getfloatarg(2, argc, argv);
  else
    x->Ymin = -1;

  if (argc>=2)
    x->Xmax = atom_getfloatarg(1, argc, argv);
  else
    x->Xmax = 1;

  if (argc>=1)
    x->Xmin = atom_getfloatarg(0, argc, argv);
  else
    x->Xmin = -1;

  return (x);
}

void tSquare2D_setup(void) 
{

  tSquare2D_class = class_new(gensym("tSquare2D"),
        (t_newmethod)tSquare2D_new,
        0, sizeof(t_tSquare2D),
        CLASS_DEFAULT, A_GIMME, 0);

  class_addcreator((t_newmethod)tSquare2D_new, gensym("pmpd.tSquare2D"), A_GIMME, 0);

  class_addmethod(tSquare2D_class, (t_method)tSquare2D_position2D, gensym("position2D"), A_DEFFLOAT, A_DEFFLOAT, 0);

  class_addmethod(tSquare2D_class, (t_method)tSquare2D_Xmin, gensym("setXmin"), A_DEFFLOAT, 0);
  class_addmethod(tSquare2D_class, (t_method)tSquare2D_Ymin, gensym("setYmin"), A_DEFFLOAT, 0);
  class_addmethod(tSquare2D_class, (t_method)tSquare2D_Xmax, gensym("setXmax"), A_DEFFLOAT, 0);
  class_addmethod(tSquare2D_class, (t_method)tSquare2D_Ymax, gensym("setYmax"), A_DEFFLOAT, 0);


}
