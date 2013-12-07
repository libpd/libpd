#include "m_pd.h"
#include "math.h"

static t_class *iCylinder3D_class;

typedef struct _iCylinder3D {
  t_object  x_obj;
  t_atom  force[21];
  t_outlet *force_new; // outlet
  t_symbol *x_sym;  // send
 
} t_iCylinder3D;



void iCylinder3D_bang(t_iCylinder3D *x)
{
  if (x->x_sym->s_thing) typedmess(x->x_sym->s_thing, gensym("interactor_cylinder_3D"), 21, x->force);

  outlet_anything(x->force_new, gensym("interactor_cylinder_3D"), 21, x->force);
}

void iCylinder3D_setXYZ(t_iCylinder3D *x, t_float X, t_float Y, t_float Z)
{
  SETFLOAT(&(x->force[3]), X);
  SETFLOAT(&(x->force[4]), Y);
  SETFLOAT(&(x->force[5]), Z);
}
void iCylinder3D_setVXYZ(t_iCylinder3D *x, t_float X, t_float Y, t_float Z)
{
  SETFLOAT(&(x->force[0]), X);
  SETFLOAT(&(x->force[1]), Y);
  SETFLOAT(&(x->force[2]), Z);
}

void iCylinder3D_setVX(t_iCylinder3D *x, t_float X)
{
  SETFLOAT(&(x->force[0]), X);
}

void iCylinder3D_setVY(t_iCylinder3D *x, t_float Y)
{
  SETFLOAT(&(x->force[1]), Y);
}

void iCylinder3D_setVZ(t_iCylinder3D *x, t_float Z)
{
  SETFLOAT(&(x->force[2]), Z);
}
void iCylinder3D_setX(t_iCylinder3D *x, t_float X)
{
  SETFLOAT(&(x->force[3]), X);
}

void iCylinder3D_setY(t_iCylinder3D *x, t_float Y)
{
  SETFLOAT(&(x->force[4]), Y);
}

void iCylinder3D_setZ(t_iCylinder3D *x, t_float Z)
{
  SETFLOAT(&(x->force[5]), Z);
}

void iCylinder3D_setRmin(t_iCylinder3D *x, t_float X)
{
  SETFLOAT(&(x->force[6]), X);
}

void iCylinder3D_setRmax(t_iCylinder3D *x, t_float X)
{
  SETFLOAT(&(x->force[7]), X);
}

void iCylinder3D_setFN(t_iCylinder3D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[8]), f1);
}

void iCylinder3D_setKN(t_iCylinder3D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[9]), f1);
}

void iCylinder3D_setD(t_iCylinder3D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[10]), f1);
}

void iCylinder3D_setRN(t_iCylinder3D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[11]), f1);
}

void iCylinder3D_setG(t_iCylinder3D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[12]), f1);
}

void iCylinder3D_setPmin(t_iCylinder3D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[13]), f1);
}

void iCylinder3D_setPmax(t_iCylinder3D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[14]), f1);
}

void iCylinder3D_setFT(t_iCylinder3D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[15]), f1);
}

void iCylinder3D_setKT(t_iCylinder3D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[16]), f1);
}

void iCylinder3D_setdN(t_iCylinder3D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[17]), f1);
}

void iCylinder3D_setdT(t_iCylinder3D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[18]), f1);
}

void iCylinder3D_setdKN(t_iCylinder3D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[19]), f1);
}

void iCylinder3D_setdKT(t_iCylinder3D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[20]), f1);
}

void *iCylinder3D_new(t_symbol *s, int argc, t_atom *argv)
{
  t_iCylinder3D *x = (t_iCylinder3D *)pd_new(iCylinder3D_class);

  x->x_sym = atom_getsymbolarg(0, argc, argv);

  x->force_new=outlet_new(&x->x_obj, 0);


  if (argc>=22)
    SETFLOAT(&(x->force[20]), atom_getfloatarg(21, argc, argv));
  else
    SETFLOAT(&(x->force[20]), 0);

   if (argc>=21)
    SETFLOAT(&(x->force[19]), atom_getfloatarg(20, argc, argv));
  else
    SETFLOAT(&(x->force[19]), 0);

   if (argc>=20)
    SETFLOAT(&(x->force[18]), atom_getfloatarg(19, argc, argv));
  else
    SETFLOAT(&(x->force[18]), 0);

   if (argc>=19)
    SETFLOAT(&(x->force[17]), atom_getfloatarg(18, argc, argv));
  else
    SETFLOAT(&(x->force[17]), 0);

   if (argc>=18)
    SETFLOAT(&(x->force[16]), atom_getfloatarg(17, argc, argv));
  else
    SETFLOAT(&(x->force[16]), 0);

   if (argc>=17)
    SETFLOAT(&(x->force[15]), atom_getfloatarg(16, argc, argv));
  else
    SETFLOAT(&(x->force[15]), 0);

   if (argc>=16)
    SETFLOAT(&(x->force[14]), atom_getfloatarg(15, argc, argv));
  else
    SETFLOAT(&(x->force[14]), 1000);
 
  if (argc>=15)
    SETFLOAT(&(x->force[13]), atom_getfloatarg(14, argc, argv));
  else
    SETFLOAT(&(x->force[13]), -1000);
 
  if (argc>=14)
    SETFLOAT(&(x->force[12]), atom_getfloatarg(13, argc, argv));
  else
    SETFLOAT(&(x->force[12]), 0);

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

void iCylinder3D_setup(void) 
{

  iCylinder3D_class = class_new(gensym("iCylinder3D"),
        (t_newmethod)iCylinder3D_new,
        0, sizeof(t_iCylinder3D),
        CLASS_DEFAULT, A_GIMME, 0);

  class_addcreator((t_newmethod)iCylinder3D_new, gensym("pmpd.iCylinder3D"),  A_GIMME, 0);
 
  class_addbang(iCylinder3D_class, iCylinder3D_bang);
  class_addmethod(iCylinder3D_class, (t_method)iCylinder3D_setVX, gensym("setVX"), A_DEFFLOAT, 0);
  class_addmethod(iCylinder3D_class, (t_method)iCylinder3D_setVY, gensym("setVY"), A_DEFFLOAT, 0);
  class_addmethod(iCylinder3D_class, (t_method)iCylinder3D_setVZ, gensym("setVZ"), A_DEFFLOAT, 0);
  class_addmethod(iCylinder3D_class, (t_method)iCylinder3D_setX, gensym("setX"), A_DEFFLOAT, 0);
  class_addmethod(iCylinder3D_class, (t_method)iCylinder3D_setY, gensym("setY"), A_DEFFLOAT, 0);
  class_addmethod(iCylinder3D_class, (t_method)iCylinder3D_setZ, gensym("setZ"), A_DEFFLOAT, 0);
  class_addmethod(iCylinder3D_class, (t_method)iCylinder3D_setRmin, gensym("setRmin"), A_DEFFLOAT, 0);
  class_addmethod(iCylinder3D_class, (t_method)iCylinder3D_setRmax, gensym("setRmax"), A_DEFFLOAT, 0);
  class_addmethod(iCylinder3D_class, (t_method)iCylinder3D_setXYZ, gensym("setXYZ"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addmethod(iCylinder3D_class, (t_method)iCylinder3D_setVXYZ, gensym("setVXYZ"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addmethod(iCylinder3D_class, (t_method)iCylinder3D_setPmin, gensym("setPmin"), A_DEFFLOAT, 0);
  class_addmethod(iCylinder3D_class, (t_method)iCylinder3D_setPmax, gensym("setPmax"), A_DEFFLOAT, 0);
  class_addmethod(iCylinder3D_class, (t_method)iCylinder3D_setFN, gensym("setFN"), A_DEFFLOAT, 0);
  class_addmethod(iCylinder3D_class, (t_method)iCylinder3D_setKN, gensym("setKN"), A_DEFFLOAT, 0);
  class_addmethod(iCylinder3D_class, (t_method)iCylinder3D_setRN, gensym("setRN"), A_DEFFLOAT, 0);
  class_addmethod(iCylinder3D_class, (t_method)iCylinder3D_setG, gensym("setG"), A_DEFFLOAT, 0);
  class_addmethod(iCylinder3D_class, (t_method)iCylinder3D_setD, gensym("setD"), A_DEFFLOAT, 0);
  class_addmethod(iCylinder3D_class, (t_method)iCylinder3D_setFT, gensym("setFT"), A_DEFFLOAT, 0);
  class_addmethod(iCylinder3D_class, (t_method)iCylinder3D_setKT, gensym("setKT"), A_DEFFLOAT, 0);
  class_addmethod(iCylinder3D_class, (t_method)iCylinder3D_setdN, gensym("setdN"), A_DEFFLOAT, 0);
  class_addmethod(iCylinder3D_class, (t_method)iCylinder3D_setdT, gensym("setdT"), A_DEFFLOAT, 0);
  class_addmethod(iCylinder3D_class, (t_method)iCylinder3D_setdKN, gensym("setdKN"), A_DEFFLOAT, 0);
  class_addmethod(iCylinder3D_class, (t_method)iCylinder3D_setdKT, gensym("setdKT"), A_DEFFLOAT, 0);

}
