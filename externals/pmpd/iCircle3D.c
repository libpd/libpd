#include "m_pd.h"
#include "math.h"

static t_class *iCircle3D_class;

typedef struct _iCircle3D {
  t_object  x_obj;
  t_atom  force[14];
  t_outlet *force_new; // outlet
  t_symbol *x_sym;  // send
 
} t_iCircle3D;

void iCircle3D_bang(t_iCircle3D *x)
{
  if (x->x_sym->s_thing) typedmess(x->x_sym->s_thing, gensym("interactor_circle_3D"), 14, x->force);

  outlet_anything(x->force_new, gensym("interactor_circle_3D"), 14, x->force);
}

void iCircle3D_setXYZ(t_iCircle3D *x, t_float X, t_float Y, t_float Z)
{
  SETFLOAT(&(x->force[3]), X);
  SETFLOAT(&(x->force[4]), Y);
  SETFLOAT(&(x->force[5]), Z);
}
void iCircle3D_setVXYZ(t_iCircle3D *x, t_float X, t_float Y, t_float Z)
{
  SETFLOAT(&(x->force[0]), X);
  SETFLOAT(&(x->force[1]), Y);
  SETFLOAT(&(x->force[2]), Z);
}

void iCircle3D_setVX(t_iCircle3D *x, t_float X)
{
  SETFLOAT(&(x->force[0]), X);
}

void iCircle3D_setVY(t_iCircle3D *x, t_float Y)
{
  SETFLOAT(&(x->force[1]), Y);
}

void iCircle3D_setVZ(t_iCircle3D *x, t_float Z)
{
  SETFLOAT(&(x->force[2]), Z);
}
void iCircle3D_setX(t_iCircle3D *x, t_float X)
{
  SETFLOAT(&(x->force[3]), X);
}

void iCircle3D_setY(t_iCircle3D *x, t_float Y)
{
  SETFLOAT(&(x->force[4]), Y);
}

void iCircle3D_setZ(t_iCircle3D *x, t_float Z)
{
  SETFLOAT(&(x->force[5]), Z);
}

void iCircle3D_setRmin(t_iCircle3D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[6]), f1);
}

void iCircle3D_setRmax(t_iCircle3D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[7]), f1);
}

void iCircle3D_setFN(t_iCircle3D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[8]), f1);
}

void iCircle3D_setKN(t_iCircle3D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[9]), f1);
}

void iCircle3D_setD(t_iCircle3D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[10]), f1);
}

void iCircle3D_setPmax(t_iCircle3D *x, t_float X)
{
  SETFLOAT(&(x->force[11]), X);
}

void iCircle3D_setdN(t_iCircle3D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[12]), f1);
}

void iCircle3D_setdKN(t_iCircle3D *x, t_float X)
{
  SETFLOAT(&(x->force[13]), X);
}

void *iCircle3D_new(t_symbol *s, int argc, t_atom *argv)
{
  t_iCircle3D *x = (t_iCircle3D *)pd_new(iCircle3D_class);

  x->x_sym = atom_getsymbolarg(0, argc, argv);

  x->force_new=outlet_new(&x->x_obj, 0);


  if (argc>=15)
    SETFLOAT(&(x->force[13]), atom_getfloatarg(14, argc, argv));
  else
    SETFLOAT(&(x->force[13]), 0);
 
  if (argc>=14)
    SETFLOAT(&(x->force[12]), atom_getfloatarg(13, argc, argv));
  else
    SETFLOAT(&(x->force[12]), 0);
  
  if (argc>=13)
    SETFLOAT(&(x->force[11]), atom_getfloatarg(12, argc, argv));
  else
    SETFLOAT(&(x->force[11]), 10000);
 
  if (argc>=12)
    SETFLOAT(&(x->force[10]), atom_getfloatarg(11, argc, argv));
  else
    SETFLOAT(&(x->force[10]), 0);
  
  if (argc>=11)
    SETFLOAT(&(x->force[9]), atom_getfloatarg(10, argc, argv));
  else
    SETFLOAT(&(x->force[9]), 0);
 
  if (argc>=10)
    SETFLOAT(&(x->force[8]), atom_getfloatarg(9, argc, argv));
  else
    SETFLOAT(&(x->force[8]), 0);

  if (argc>=9)
    SETFLOAT(&(x->force[7]), atom_getfloatarg(8, argc, argv));
  else
    SETFLOAT(&(x->force[7]), 1);

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

void iCircle3D_setup(void) 
{
  iCircle3D_class = class_new(gensym("iCircle3D"),
        (t_newmethod)iCircle3D_new,
        0, sizeof(t_iCircle3D),
        CLASS_DEFAULT, A_GIMME, 0);

  class_addcreator((t_newmethod)iCircle3D_new, gensym("pmpd.iCircle3D"),  A_GIMME, 0);

  class_addbang(iCircle3D_class, iCircle3D_bang);
  class_addmethod(iCircle3D_class, (t_method)iCircle3D_setVX, gensym("setVX"), A_DEFFLOAT, 0);
  class_addmethod(iCircle3D_class, (t_method)iCircle3D_setVY, gensym("setVY"), A_DEFFLOAT, 0);
  class_addmethod(iCircle3D_class, (t_method)iCircle3D_setVZ, gensym("setVZ"), A_DEFFLOAT, 0);
  class_addmethod(iCircle3D_class, (t_method)iCircle3D_setX, gensym("setX"), A_DEFFLOAT, 0);
  class_addmethod(iCircle3D_class, (t_method)iCircle3D_setY, gensym("setY"), A_DEFFLOAT, 0);
  class_addmethod(iCircle3D_class, (t_method)iCircle3D_setZ, gensym("setZ"), A_DEFFLOAT, 0);
  class_addmethod(iCircle3D_class, (t_method)iCircle3D_setRmin, gensym("setRmin"), A_DEFFLOAT, 0);
  class_addmethod(iCircle3D_class, (t_method)iCircle3D_setRmax, gensym("setRmax"), A_DEFFLOAT, 0);
  class_addmethod(iCircle3D_class, (t_method)iCircle3D_setXYZ, gensym("setXYZ"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addmethod(iCircle3D_class, (t_method)iCircle3D_setVXYZ, gensym("setVXYZ"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addmethod(iCircle3D_class, (t_method)iCircle3D_setPmax, gensym("setPmax"), A_DEFFLOAT, 0);
  class_addmethod(iCircle3D_class, (t_method)iCircle3D_setFN, gensym("setFN"), A_DEFFLOAT, 0);
  class_addmethod(iCircle3D_class, (t_method)iCircle3D_setKN, gensym("setKN"), A_DEFFLOAT, 0);
  class_addmethod(iCircle3D_class, (t_method)iCircle3D_setdN, gensym("setdN"), A_DEFFLOAT, 0);
  class_addmethod(iCircle3D_class, (t_method)iCircle3D_setdKN, gensym("setdKN"), A_DEFFLOAT, 0);
  class_addmethod(iCircle3D_class, (t_method)iCircle3D_setD, gensym("setD"), A_DEFFLOAT, 0);
}
