#include "m_pd.h"
#include "math.h"

static t_class *iAmbient3D_class;

typedef struct _iAmbient3D {
  t_object  x_obj;
  t_atom  force[17];
/*	1 : name of the mass (send interactors informations to this masses)
	2 : FX (constant X force apply to the masses) (0)
	3 : FY (constant Y force apply to the masses) (0)
	4 : FZ (constant Z force apply to the masses) (0)
	5 : Rnd FX (random X force apply to the masses) (0)
	6 : Rnd FY (random Y force apply to the masses) (0)
	7 : Rnd FZ (random Z force apply to the masses) (0)
	8 : Damping (velocity damping of the masses) (0)
	9 : Rien
    10 : Xmin (minimum and maximum limit of the interactors) (-10000)
	9 : Xmax (a mass interact with this object only if it is inside the interactor bounds) (10000)
	10 : Ymin (-10000)
	11 : Ymax (10000)
	12 : Zmin (-10000)
	13 : Zmax (10000)
	14 : dX (displace the mass by a constant value) (0)
	15 : dY (0)
	16 : dZ (0)
*/
  t_outlet *force_new;// outlet
  t_symbol *x_sym;  // send
} t_iAmbient3D;

void iAmbient3D_forceX(t_iAmbient3D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[0]), f1);
}

void iAmbient3D_forceY(t_iAmbient3D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[1]), f1);
}

void iAmbient3D_forceZ(t_iAmbient3D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[2]), f1);
}

void iAmbient3D_force(t_iAmbient3D *x, t_floatarg f1, t_floatarg f2, t_floatarg f3)
{
  SETFLOAT(&(x->force[0]), f1 );
  SETFLOAT(&(x->force[1]), f2 );
  SETFLOAT(&(x->force[2]), f3 );
}

void iAmbient3D_rndFX(t_iAmbient3D *x, t_float X)
{
  SETFLOAT(&(x->force[3]), X);
}

void iAmbient3D_rndFY(t_iAmbient3D *x, t_float X)
{
  SETFLOAT(&(x->force[4]), X);
}

void iAmbient3D_rndFZ(t_iAmbient3D *x, t_float X)
{
  SETFLOAT(&(x->force[5]), X);
}

void iAmbient3D_rndF(t_iAmbient3D *x, t_floatarg f1, t_floatarg f2, t_floatarg f3)
{
  SETFLOAT(&(x->force[3]), f1);
  SETFLOAT(&(x->force[4]), f2);
  SETFLOAT(&(x->force[5]), f3);
}

void iAmbient3D_damp(t_iAmbient3D *x, t_float X)
{
  SETFLOAT(&(x->force[6]), X);
}


void iAmbient3D_Xmin(t_iAmbient3D *x, t_float Xmin)
{
  SETFLOAT(&(x->force[8]), Xmin);
}

void iAmbient3D_Xmax(t_iAmbient3D *x, t_float Xmax)
{
  SETFLOAT(&(x->force[9]), Xmax);
}

void iAmbient3D_Ymin(t_iAmbient3D *x, t_float Ymin)
{
  SETFLOAT(&(x->force[10]), Ymin);
}

void iAmbient3D_Ymax(t_iAmbient3D *x, t_float Ymax)
{
  SETFLOAT(&(x->force[11]), Ymax);
}

void iAmbient3D_Zmin(t_iAmbient3D *x, t_float Zmin)
{
  SETFLOAT(&(x->force[12]), Zmin);
}

void iAmbient3D_Zmax(t_iAmbient3D *x, t_float Zmax)
{
  SETFLOAT(&(x->force[13]), Zmax);
}

void iAmbient3D_dXYZ(t_iAmbient3D *x, t_float dX, t_float dY, t_float dZ)
{
  SETFLOAT(&(x->force[14]), dX);
  SETFLOAT(&(x->force[15]), dY);
  SETFLOAT(&(x->force[16]), dZ);
}

void iAmbient3D_dX(t_iAmbient3D *x, t_float dX)
{
  SETFLOAT(&(x->force[14]), dX);
}

void iAmbient3D_dY(t_iAmbient3D *x, t_float dY)
{
  SETFLOAT(&(x->force[15]), dY);
}

void iAmbient3D_dZ(t_iAmbient3D *x, t_float dZ)
{
  SETFLOAT(&(x->force[16]), dZ);
}

void iAmbient3D_bang(t_iAmbient3D *x)
{
  if (x->x_sym->s_thing) typedmess(x->x_sym->s_thing, gensym("interactor_ambient_3D"), 17, x->force);

  outlet_anything(x->force_new, gensym("interactor_ambient_3D"), 17, x->force);
}

void *iAmbient3D_new(t_symbol *s, int argc, t_atom *argv)
{
  t_iAmbient3D *x = (t_iAmbient3D *)pd_new(iAmbient3D_class);

  x->x_sym = atom_getsymbolarg(0, argc, argv);

  x->force_new=outlet_new(&x->x_obj, 0);

  if (argc>=16)
    SETFLOAT(&(x->force[15]), atom_getfloatarg(15, argc, argv));
  else
    SETFLOAT(&(x->force[15]), 0);
  
  if (argc>=15)
    SETFLOAT(&(x->force[14]), atom_getfloatarg(14, argc, argv));
  else
    SETFLOAT(&(x->force[14]), 0);
  
  if (argc>=14)
    SETFLOAT(&(x->force[13]), atom_getfloatarg(13, argc, argv));
  else
    SETFLOAT(&(x->force[13]), 10000);
 
  if (argc>=13)
    SETFLOAT(&(x->force[12]), atom_getfloatarg(12, argc, argv));
  else
    SETFLOAT(&(x->force[12]), -10000);

  if (argc>=12)
    SETFLOAT(&(x->force[11]), atom_getfloatarg(11, argc, argv));
  else
    SETFLOAT(&(x->force[11]), 10000);

  if (argc>=11)
    SETFLOAT(&(x->force[10]), atom_getfloatarg(10, argc, argv));
  else
    SETFLOAT(&(x->force[10]), -10000);

   if (argc>=10)
    SETFLOAT(&(x->force[9]), atom_getfloatarg(9, argc, argv));
  else
    SETFLOAT(&(x->force[9]), 10000);

  if (argc>=9)
    SETFLOAT(&(x->force[8]), atom_getfloatarg(8, argc, argv));
  else
    SETFLOAT(&(x->force[8]), -10000);

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
    SETFLOAT(&(x->force[0]), 0);

  return (x);
}

void iAmbient3D_setup(void) 
{

  iAmbient3D_class = class_new(gensym("iAmbient3D"),
        (t_newmethod)iAmbient3D_new,
        0, sizeof(t_iAmbient3D),
        CLASS_DEFAULT, A_GIMME, 0);

  class_addcreator((t_newmethod)iAmbient3D_new, gensym("pmpd.iAmbient3D"), A_GIMME, 0);

  class_addbang(iAmbient3D_class, iAmbient3D_bang);
  class_addmethod(iAmbient3D_class, (t_method)iAmbient3D_force, gensym("setFXYZ"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addmethod(iAmbient3D_class, (t_method)iAmbient3D_forceX, gensym("setFX"), A_DEFFLOAT, 0);
  class_addmethod(iAmbient3D_class, (t_method)iAmbient3D_forceY, gensym("setFY"), A_DEFFLOAT, 0);
  class_addmethod(iAmbient3D_class, (t_method)iAmbient3D_forceZ, gensym("setFZ"), A_DEFFLOAT, 0);
  class_addmethod(iAmbient3D_class, (t_method)iAmbient3D_Xmin, gensym("setXmin"), A_DEFFLOAT, 0);
  class_addmethod(iAmbient3D_class, (t_method)iAmbient3D_Ymin, gensym("setYmin"), A_DEFFLOAT, 0);
  class_addmethod(iAmbient3D_class, (t_method)iAmbient3D_Zmin, gensym("setZmin"), A_DEFFLOAT, 0);
  class_addmethod(iAmbient3D_class, (t_method)iAmbient3D_Xmax, gensym("setXmax"), A_DEFFLOAT, 0);
  class_addmethod(iAmbient3D_class, (t_method)iAmbient3D_Ymax, gensym("setYmax"), A_DEFFLOAT, 0);
  class_addmethod(iAmbient3D_class, (t_method)iAmbient3D_Zmax, gensym("setZmax"), A_DEFFLOAT, 0);
  class_addmethod(iAmbient3D_class, (t_method)iAmbient3D_rndFY, gensym("setRndFY"), A_DEFFLOAT, 0);
  class_addmethod(iAmbient3D_class, (t_method)iAmbient3D_rndFX, gensym("setRndFX"), A_DEFFLOAT, 0);
  class_addmethod(iAmbient3D_class, (t_method)iAmbient3D_rndFZ, gensym("setRndFZ"), A_DEFFLOAT, 0);
  class_addmethod(iAmbient3D_class, (t_method)iAmbient3D_damp, gensym("setD"), A_DEFFLOAT, 0);
  class_addmethod(iAmbient3D_class, (t_method)iAmbient3D_dXYZ, gensym("dXYZ"), A_DEFFLOAT, 0);
  class_addmethod(iAmbient3D_class, (t_method)iAmbient3D_dX, gensym("setdX"), A_DEFFLOAT, 0);
  class_addmethod(iAmbient3D_class, (t_method)iAmbient3D_dY, gensym("setdY"), A_DEFFLOAT, 0);
  class_addmethod(iAmbient3D_class, (t_method)iAmbient3D_dZ, gensym("setdZ"), A_DEFFLOAT, 0);

}
