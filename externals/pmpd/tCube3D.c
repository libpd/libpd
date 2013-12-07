#include "m_pd.h"
#include "math.h"

static t_class *tCube3D_class;

typedef struct _tCube3D {
  t_object  x_obj;
  t_float  Xmin, Xmax, Ymin, Ymax, Zmin, Zmax;
  //extrem =  Xmin, Ymin, Xmax, Ymax;
  t_outlet *force_new;// outlet
} t_tCube3D;

void tCube3D_position3D(t_tCube3D *x, t_float X, t_float Y, t_float Z)
{

 	if ((X > x->Xmin) & (X < x->Xmax) & (Y > x->Ymin) & (Y < x->Ymax) & (Z > x->Zmin) & (Z < x->Zmax) )
	{
		outlet_float(x->force_new, 1);
	}
	else
	{
		outlet_float(x->force_new, 0);
	}
}

void tCube3D_Xmin(t_tCube3D *x, t_float X)
{
   x->Xmin= X;
}

void tCube3D_Xmax(t_tCube3D *x, t_float X)
{
   x->Xmax= X;
}

void tCube3D_Ymin(t_tCube3D *x, t_float X)
{
   x->Ymin= X;
}

void tCube3D_Ymax(t_tCube3D *x, t_float X)
{
   x->Ymax= X;
}

void tCube3D_Zmin(t_tCube3D *x, t_float X)
{
   x->Zmin= X;
}

void tCube3D_Zmax(t_tCube3D *x, t_float X)
{
   x->Zmax= X;
}

void *tCube3D_new(t_symbol *s, int argc, t_atom *argv)
{
  t_tCube3D *x = (t_tCube3D *)pd_new(tCube3D_class);
  x->force_new=outlet_new(&x->x_obj, 0);

  if (argc>=6)
    x->Zmax = atom_getfloatarg(5, argc, argv);
  else
    x->Zmax = 1;

  if (argc>=5)
    x->Zmin = atom_getfloatarg(4, argc, argv);
  else
    x->Zmin = -1;

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

void tCube3D_setup(void) 
{

  tCube3D_class = class_new(gensym("tCube3D"),
        (t_newmethod)tCube3D_new,
        0, sizeof(t_tCube3D),
        CLASS_DEFAULT, A_GIMME, 0);

  class_addcreator((t_newmethod)tCube3D_new, gensym("pmpd.tCube3D"), A_GIMME, 0);

  class_addmethod(tCube3D_class, (t_method)tCube3D_position3D, gensym("position3D"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);

  class_addmethod(tCube3D_class, (t_method)tCube3D_Xmin, gensym("setXmin"), A_DEFFLOAT, 0);
  class_addmethod(tCube3D_class, (t_method)tCube3D_Ymin, gensym("setYmin"), A_DEFFLOAT, 0);
  class_addmethod(tCube3D_class, (t_method)tCube3D_Ymin, gensym("setZmin"), A_DEFFLOAT, 0);
  class_addmethod(tCube3D_class, (t_method)tCube3D_Xmax, gensym("setXmax"), A_DEFFLOAT, 0);
  class_addmethod(tCube3D_class, (t_method)tCube3D_Ymax, gensym("setYmax"), A_DEFFLOAT, 0);
  class_addmethod(tCube3D_class, (t_method)tCube3D_Ymax, gensym("setZmax"), A_DEFFLOAT, 0);


}
