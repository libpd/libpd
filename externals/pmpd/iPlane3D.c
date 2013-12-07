#include "m_pd.h"
#include "math.h"

static t_class *iPlane3D_class;

typedef struct _iPlane3D {
  t_object  x_obj;
  t_atom  force[12];
  t_outlet *force_new; // outlet
  t_symbol *x_sym;  // send
 
} t_iPlane3D;



void iPlane3D_bang(t_iPlane3D *x)
{
  if (x->x_sym->s_thing) typedmess(x->x_sym->s_thing, gensym("interactor_plane_3D"), 12, x->force);

  outlet_anything(x->force_new, gensym("interactor_plane_3D"), 12, x->force);
}

void iPlane3D_setXYZ(t_iPlane3D *x, t_float X, t_float Y, t_float Z)
{
  SETFLOAT(&(x->force[3]), X);
  SETFLOAT(&(x->force[4]), Y);
  SETFLOAT(&(x->force[5]), Z);
}
void iPlane3D_setVXYZ(t_iPlane3D *x, t_float X, t_float Y, t_float Z)
{
  SETFLOAT(&(x->force[0]), X);
  SETFLOAT(&(x->force[1]), Y);
  SETFLOAT(&(x->force[2]), Z);
}

void iPlane3D_setVX(t_iPlane3D *x, t_float X)
{
  SETFLOAT(&(x->force[0]), X);
}

void iPlane3D_setVY(t_iPlane3D *x, t_float Y)
{
  SETFLOAT(&(x->force[1]), Y);
}

void iPlane3D_setVZ(t_iPlane3D *x, t_float Z)
{
  SETFLOAT(&(x->force[2]), Z);
}
void iPlane3D_setX(t_iPlane3D *x, t_float X)
{
  SETFLOAT(&(x->force[3]), X);
}

void iPlane3D_setY(t_iPlane3D *x, t_float Y)
{
  SETFLOAT(&(x->force[4]), Y);
}

void iPlane3D_setZ(t_iPlane3D *x, t_float Z)
{
  SETFLOAT(&(x->force[5]), Z);
}

void iPlane3D_setFN(t_iPlane3D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[6]), f1);
}

void iPlane3D_setKN(t_iPlane3D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[7]), f1);
}

void iPlane3D_setD(t_iPlane3D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[8]), f1);
}

void iPlane3D_setP(t_iPlane3D *x, t_float X)
{
  SETFLOAT(&(x->force[9]), X);
}

void iPlane3D_setdN(t_iPlane3D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[10]), f1);
}

void iPlane3D_setdKN(t_iPlane3D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[11]), f1);
}

void *iPlane3D_new(t_symbol *s, int argc, t_atom *argv)
{
  t_iPlane3D *x = (t_iPlane3D *)pd_new(iPlane3D_class);

  x->x_sym = atom_getsymbolarg(0, argc, argv);

  x->force_new=outlet_new(&x->x_obj, 0);


  if (argc>=13)
    SETFLOAT(&(x->force[11]), atom_getfloatarg(12, argc, argv));
  else
    SETFLOAT(&(x->force[11]), 0);

  if (argc>=12)
    SETFLOAT(&(x->force[10]), atom_getfloatarg(11, argc, argv));
  else
    SETFLOAT(&(x->force[10]), 0);
 
  if (argc>=11)
    SETFLOAT(&(x->force[9]), atom_getfloatarg(10, argc, argv));
  else
    SETFLOAT(&(x->force[9]), 10000);
 
  if (argc>=10)
    SETFLOAT(&(x->force[8]), atom_getfloatarg(9, argc, argv));
  else
    SETFLOAT(&(x->force[8]), 0);

  if (argc>=9)
    SETFLOAT(&(x->force[7]), atom_getfloatarg(8, argc, argv));
  else
    SETFLOAT(&(x->force[7]), 0);

  if (argc>=8)
    SETFLOAT(&(x->force[6]), atom_getfloatarg(7, argc, argv));
  else
    SETFLOAT(&(x->force[6]), 0);

    if (argc>=7)
    SETFLOAT(&(x->force[5]), atom_getfloatarg(6, argc, argv));
  else
    SETFLOAT(&(x->force[5]), 0);

    if (argc>=6)
    SETFLOAT(&(x->force[4]), atom_getfloatarg(5, argc, argv));
  else
    SETFLOAT(&(x->force[4]), 0);

  if (argc>=5)
    SETFLOAT(&(x->force[3]), atom_getfloatarg(4, argc, argv));
  else
    SETFLOAT(&(x->force[3]), 0);

  if (argc>=4)
    SETFLOAT(&(x->force[2]), atom_getfloatarg(3, argc, argv));
  else
	  SETFLOAT(&(x->force[2]), 0);

  if (argc>=3)
    SETFLOAT(&(x->force[1]), atom_getfloatarg(2, argc, argv));
  else
	  SETFLOAT(&(x->force[1]), 0);

  if (argc>=2)
	  SETFLOAT(&(x->force[0]), atom_getfloatarg(1, argc, argv));
  else
	  SETFLOAT(&(x->force[0]), 1);

  return (x);
}

void iPlane3D_setup(void) 
{

  iPlane3D_class = class_new(gensym("iPlane3D"),
        (t_newmethod)iPlane3D_new,
        0, sizeof(t_iPlane3D),
        CLASS_DEFAULT, A_GIMME, 0);

  class_addcreator((t_newmethod)iPlane3D_new, gensym("pmpd.iPlane3D"),  A_GIMME, 0);
 
  class_addbang(iPlane3D_class, iPlane3D_bang);
  class_addmethod(iPlane3D_class, (t_method)iPlane3D_setVX, gensym("setVX"), A_DEFFLOAT, 0);
  class_addmethod(iPlane3D_class, (t_method)iPlane3D_setVY, gensym("setVY"), A_DEFFLOAT, 0);
  class_addmethod(iPlane3D_class, (t_method)iPlane3D_setVZ, gensym("setVZ"), A_DEFFLOAT, 0);
  class_addmethod(iPlane3D_class, (t_method)iPlane3D_setX, gensym("setX"), A_DEFFLOAT, 0);
  class_addmethod(iPlane3D_class, (t_method)iPlane3D_setY, gensym("setY"), A_DEFFLOAT, 0);
  class_addmethod(iPlane3D_class, (t_method)iPlane3D_setZ, gensym("setZ"), A_DEFFLOAT, 0);
  class_addmethod(iPlane3D_class, (t_method)iPlane3D_setXYZ, gensym("setXYZ"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addmethod(iPlane3D_class, (t_method)iPlane3D_setVXYZ, gensym("setVXYZ"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addmethod(iPlane3D_class, (t_method)iPlane3D_setP, gensym("setPmax"), A_DEFFLOAT, 0);
  class_addmethod(iPlane3D_class, (t_method)iPlane3D_setFN, gensym("setFN"), A_DEFFLOAT, 0);
  class_addmethod(iPlane3D_class, (t_method)iPlane3D_setKN, gensym("setKN"), A_DEFFLOAT, 0);
  class_addmethod(iPlane3D_class, (t_method)iPlane3D_setdKN, gensym("setdKN"), A_DEFFLOAT, 0);
  class_addmethod(iPlane3D_class, (t_method)iPlane3D_setD, gensym("setD"), A_DEFFLOAT, 0);
  class_addmethod(iPlane3D_class, (t_method)iPlane3D_setdN, gensym("setdN"), A_DEFFLOAT, 0);

}
