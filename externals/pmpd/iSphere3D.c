#include "m_pd.h"
#include "math.h"

static t_class *iSphere3D_class;

typedef struct _iSphere3D {
  t_object  x_obj;
  t_atom  force[17];
  // = Xcentre, Ycentre, .., Rmin, Rmax, Fnormal, K normal, F/R normal, DampNormal, dN, posX_old, posY_old, posZ_old, lia_damp, f/R2
  t_outlet *force_new; // outlet
  t_symbol *x_sym;  // send
  t_float posX_old, posY_old, posZ_old, posX, posY, posZ; // anciennes position, pour calculer la vitesse de cette interacteur.

} t_iSphere3D;



void iSphere3D_bang(t_iSphere3D *x)
{

  SETFLOAT(&(x->force[10]), x->posX_old); // posX_old
  SETFLOAT(&(x->force[11]), x->posY_old); // posY_old
  SETFLOAT(&(x->force[12]), x->posZ_old); // posZ_old

  x->posX_old = x->posX;
  x->posY_old = x->posY;
  x->posZ_old = x->posZ;

  if (x->x_sym->s_thing) typedmess(x->x_sym->s_thing, gensym("interactor_sphere_3D"), 17, x->force);

  outlet_anything(x->force_new, gensym("interactor_sphere_3D"), 17, x->force);
}

void iSphere3D_setXYZ(t_iSphere3D *x, t_float X, t_float Y, t_float Z)
{
  SETFLOAT(&(x->force[0]), X);
	x->posX= X;
  SETFLOAT(&(x->force[1]), Y);
	x->posY = Y;
  SETFLOAT(&(x->force[2]), Z);
	x->posZ = Z;
}

void iSphere3D_setX(t_iSphere3D *x, t_float X)
{
  SETFLOAT(&(x->force[0]), X);
	x->posX= X;
}

void iSphere3D_setY(t_iSphere3D *x, t_float Y)
{
  SETFLOAT(&(x->force[1]), Y);
	x->posY = Y;
}

void iSphere3D_setZ(t_iSphere3D *x, t_float Z)
{
  SETFLOAT(&(x->force[2]), Z);
	x->posZ = Z;
}

void iSphere3D_setRmin(t_iSphere3D *x, t_float X)
{
  SETFLOAT(&(x->force[3]), X);
}

void iSphere3D_setRmax(t_iSphere3D *x, t_float X)
{
  SETFLOAT(&(x->force[4]), X);
}

void iSphere3D_setFN(t_iSphere3D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[5]), f1);
}

void iSphere3D_setKN(t_iSphere3D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[6]), f1);
}

void iSphere3D_setFRN(t_iSphere3D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[7]), f1);
}


void iSphere3D_setDN(t_iSphere3D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[8]), f1);
}

void iSphere3D_setdN(t_iSphere3D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[9]), f1);
}

void iSphere3D_setG(t_iSphere3D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[13]), f1);
}

void iSphere3D_setdKN(t_iSphere3D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[14]), f1);
}

void iSphere3D_setdRN(t_iSphere3D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[15]), f1);
}

void iSphere3D_setdGN(t_iSphere3D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[16]), f1);
}

void *iSphere3D_new(t_symbol *s, int argc, t_atom *argv)
{
  t_iSphere3D *x = (t_iSphere3D *)pd_new(iSphere3D_class);

  x->x_sym = atom_getsymbolarg(0, argc, argv);

  x->force_new=outlet_new(&x->x_obj, 0);

  if (argc>=15)
    SETFLOAT(&(x->force[16]), atom_getfloatarg(14, argc, argv));
  else
    SETFLOAT(&(x->force[16]), 0);
 
  if (argc>=14)
    SETFLOAT(&(x->force[15]), atom_getfloatarg(13, argc, argv));
  else
    SETFLOAT(&(x->force[15]), 0);

  if (argc>=13)
    SETFLOAT(&(x->force[14]), atom_getfloatarg(12, argc, argv));
  else
    SETFLOAT(&(x->force[14]), 0);
 
  if (argc>=12)
    SETFLOAT(&(x->force[13]), atom_getfloatarg(11, argc, argv));
  else
    SETFLOAT(&(x->force[13]), 0);

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
    SETFLOAT(&(x->force[4]), 1);

  if (argc>=5)
    SETFLOAT(&(x->force[3]), atom_getfloatarg(4, argc, argv));
  else
    SETFLOAT(&(x->force[3]), 0);

  if (argc>=4)
  {
    SETFLOAT(&(x->force[12]), atom_getfloatarg(3, argc, argv));
    SETFLOAT(&(x->force[2]), atom_getfloatarg(3, argc, argv));
    x->posZ_old = atom_getfloatarg(3, argc, argv);
  }
  else
  {
	  SETFLOAT(&(x->force[12]), 0);
 	  SETFLOAT(&(x->force[2]), 0);
  	  x->posZ_old = 0;
  }

  if (argc>=3)
  {
    SETFLOAT(&(x->force[11]), atom_getfloatarg(2, argc, argv));
    SETFLOAT(&(x->force[1]), atom_getfloatarg(2, argc, argv));
    x->posY_old = atom_getfloatarg(2, argc, argv);

  }
  else
  {
	  SETFLOAT(&(x->force[1]), 0);
 	  SETFLOAT(&(x->force[11]), 0);
  	  x->posY_old = 0;
  }

  if (argc>=2)
  {
	  SETFLOAT(&(x->force[10]), atom_getfloatarg(1, argc, argv));
	  SETFLOAT(&(x->force[0]), atom_getfloatarg(1, argc, argv));
	  x->posX_old = atom_getfloatarg(1, argc, argv);
  }
  else
  {
	  SETFLOAT(&(x->force[10]), 0);
	  SETFLOAT(&(x->force[0]), 0);
	  x->posX_old = 0;
  }

  return (x);
}

void iSphere3D_setup(void) 
{

  iSphere3D_class = class_new(gensym("iSphere3D"),
        (t_newmethod)iSphere3D_new,
        0, sizeof(t_iSphere3D),
        CLASS_DEFAULT, A_GIMME, 0);

  class_addcreator((t_newmethod)iSphere3D_new, gensym("pmpd.iSphere3D"),  A_GIMME, 0);
 
  class_addbang(iSphere3D_class, iSphere3D_bang);
  class_addmethod(iSphere3D_class, (t_method)iSphere3D_setFN, gensym("setFN"), A_DEFFLOAT, 0);
  class_addmethod(iSphere3D_class, (t_method)iSphere3D_setKN, gensym("setKN"), A_DEFFLOAT, 0);
  class_addmethod(iSphere3D_class, (t_method)iSphere3D_setDN, gensym("setDN"), A_DEFFLOAT, 0);
  class_addmethod(iSphere3D_class, (t_method)iSphere3D_setX, gensym("setX"), A_DEFFLOAT, 0);
  class_addmethod(iSphere3D_class, (t_method)iSphere3D_setY, gensym("setY"), A_DEFFLOAT, 0);
  class_addmethod(iSphere3D_class, (t_method)iSphere3D_setZ, gensym("setZ"), A_DEFFLOAT, 0);
  class_addmethod(iSphere3D_class, (t_method)iSphere3D_setXYZ, gensym("setXYZ"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addmethod(iSphere3D_class, (t_method)iSphere3D_setRmin, gensym("setRmin"), A_DEFFLOAT, 0);
  class_addmethod(iSphere3D_class, (t_method)iSphere3D_setRmax, gensym("setRmax"), A_DEFFLOAT, 0);
  class_addmethod(iSphere3D_class, (t_method)iSphere3D_setFRN, gensym("setFRN"), A_DEFFLOAT, 0);
  class_addmethod(iSphere3D_class, (t_method)iSphere3D_setdN, gensym("setdN"), A_DEFFLOAT, 0);
  class_addmethod(iSphere3D_class, (t_method)iSphere3D_setdRN, gensym("setdRN"), A_DEFFLOAT, 0);
  class_addmethod(iSphere3D_class, (t_method)iSphere3D_setG, gensym("setG"), A_DEFFLOAT, 0);
  class_addmethod(iSphere3D_class, (t_method)iSphere3D_setdKN, gensym("setdKN"), A_DEFFLOAT, 0);
  class_addmethod(iSphere3D_class, (t_method)iSphere3D_setdGN, gensym("setdGN"), A_DEFFLOAT, 0);

}


