#include "m_pd.h"
#include "math.h"

static t_class *iAmbient2D_class;

typedef struct _iAmbient2D {
  t_object  x_obj;
  t_atom  force[12];
  /*
  force = 
		forceX, 
		forceY, 
		randomFX, 
		randomFY, 
		damp, 
		Xmin, 
		Xmax,
		Ymin,
		Ymax,
		dX,
		dY;
  */

  t_outlet *force_new;// outlet
  t_symbol *x_sym;  // send
} t_iAmbient2D;

void iAmbient2D_forceX(t_iAmbient2D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[0]), f1);
}

void iAmbient2D_forceY(t_iAmbient2D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[1]), f1);
}

void iAmbient2D_force(t_iAmbient2D *x, t_floatarg f1, t_floatarg f2)
{
  SETFLOAT(&(x->force[0]), f1);
  SETFLOAT(&(x->force[1]), f2 );
}

void iAmbient2D_rndFX(t_iAmbient2D *x, t_float X)
{
  SETFLOAT(&(x->force[2]), X);
}

void iAmbient2D_rndFY(t_iAmbient2D *x, t_float X)
{
  SETFLOAT(&(x->force[3]), X);
}

void iAmbient2D_rndF(t_iAmbient2D *x, t_floatarg f1, t_floatarg f2)
{
  SETFLOAT(&(x->force[2]), f1);
  SETFLOAT(&(x->force[3]), f2);
}

void iAmbient2D_damp(t_iAmbient2D *x, t_float X)
{
  SETFLOAT(&(x->force[4]), X);
}

void iAmbient2D_Xmin(t_iAmbient2D *x, t_float Xmin)
{
  SETFLOAT(&(x->force[6]), Xmin);
}

void iAmbient2D_Xmax(t_iAmbient2D *x, t_float Xmax)
{
  SETFLOAT(&(x->force[7]), Xmax);
}

void iAmbient2D_Ymin(t_iAmbient2D *x, t_float Ymin)
{
  SETFLOAT(&(x->force[8]), Ymin);
}

void iAmbient2D_Ymax(t_iAmbient2D *x, t_float Ymax)
{
  SETFLOAT(&(x->force[9]), Ymax);
}

void iAmbient2D_dXY(t_iAmbient2D *x, t_float dX, t_float dY)
{
  SETFLOAT(&(x->force[10]), dX);
  SETFLOAT(&(x->force[11]), dY);
}

void iAmbient2D_dX(t_iAmbient2D *x, t_float dX)
{
  SETFLOAT(&(x->force[10]), dX);
}

void iAmbient2D_dY(t_iAmbient2D *x, t_float dY)
{
  SETFLOAT(&(x->force[11]), dY);
}

void iAmbient2D_bang(t_iAmbient2D *x)
{
  if (x->x_sym->s_thing) typedmess(x->x_sym->s_thing, gensym("interactor_ambient_2D"), 12, x->force);

  outlet_anything(x->force_new, gensym("interactor_ambient_2D"), 12, x->force);
}

void *iAmbient2D_new(t_symbol *s, int argc, t_atom *argv)
{
  t_iAmbient2D *x = (t_iAmbient2D *)pd_new(iAmbient2D_class);

  x->x_sym = atom_getsymbolarg(0, argc, argv);

  x->force_new=outlet_new(&x->x_obj, 0);

    SETFLOAT(&(x->force[5]), 0);


  if (argc>=12)
    SETFLOAT(&(x->force[11]), atom_getfloatarg(11, argc, argv));
  else
    SETFLOAT(&(x->force[11]), 0);

  if (argc>=11)
    SETFLOAT(&(x->force[10]), atom_getfloatarg(10, argc, argv));
  else
    SETFLOAT(&(x->force[10]), 0);

   if (argc>=10)
    SETFLOAT(&(x->force[9]), atom_getfloatarg(9, argc, argv));
  else
    SETFLOAT(&(x->force[9]), 100000);

  if (argc>=9)
    SETFLOAT(&(x->force[8]), atom_getfloatarg(8, argc, argv));
  else
    SETFLOAT(&(x->force[8]), -100000);

  if (argc>=8)
    SETFLOAT(&(x->force[7]), atom_getfloatarg(7, argc, argv));
  else
    SETFLOAT(&(x->force[7]), 100000);

  if (argc>=7)
    SETFLOAT(&(x->force[6]), atom_getfloatarg(6, argc, argv));
  else
    SETFLOAT(&(x->force[6]), -100000);

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

void iAmbient2D_setup(void) 
{
  iAmbient2D_class = class_new(gensym("iAmbient2D"),
        (t_newmethod)iAmbient2D_new,
        0, sizeof(t_iAmbient2D),
        CLASS_DEFAULT, A_GIMME, 0);

  class_addcreator((t_newmethod)iAmbient2D_new, gensym("pmpd.iAmbient2D"), A_GIMME, 0);

  class_addbang(iAmbient2D_class, iAmbient2D_bang);
  class_addmethod(iAmbient2D_class, (t_method)iAmbient2D_force, gensym("setFXY"), A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addmethod(iAmbient2D_class, (t_method)iAmbient2D_forceX, gensym("setFX"), A_DEFFLOAT, 0);
  class_addmethod(iAmbient2D_class, (t_method)iAmbient2D_forceY, gensym("setFY"), A_DEFFLOAT, 0);
  class_addmethod(iAmbient2D_class, (t_method)iAmbient2D_Xmin, gensym("setXmin"), A_DEFFLOAT, 0);
  class_addmethod(iAmbient2D_class, (t_method)iAmbient2D_Ymin, gensym("setYmin"), A_DEFFLOAT, 0);
  class_addmethod(iAmbient2D_class, (t_method)iAmbient2D_Xmax, gensym("setXmax"), A_DEFFLOAT, 0);
  class_addmethod(iAmbient2D_class, (t_method)iAmbient2D_Ymax, gensym("setYmax"), A_DEFFLOAT, 0);
  class_addmethod(iAmbient2D_class, (t_method)iAmbient2D_rndFY, gensym("setRndFY"), A_DEFFLOAT, 0);
  class_addmethod(iAmbient2D_class, (t_method)iAmbient2D_rndFX, gensym("setRndFX"), A_DEFFLOAT, 0);
  class_addmethod(iAmbient2D_class, (t_method)iAmbient2D_damp, gensym("setD"), A_DEFFLOAT, 0);
  class_addmethod(iAmbient2D_class, (t_method)iAmbient2D_dX, gensym("setdX"), A_DEFFLOAT, 0);
  class_addmethod(iAmbient2D_class, (t_method)iAmbient2D_dY, gensym("setdY"), A_DEFFLOAT, 0);
  class_addmethod(iAmbient2D_class, (t_method)iAmbient2D_dXY, gensym("setdXY"), A_DEFFLOAT, A_DEFFLOAT, 0);
}
