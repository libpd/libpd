#include "m_pd.h"
#include "math.h"

static t_class *iCircle2D_class;

typedef struct _iCircle2D {
  t_object  x_obj;
  t_atom  force[20];
  t_outlet *force_new; // outlet
  t_symbol *x_sym;  // send
  t_float posX_old, posY_old, posX, posY; // anciennes position, pour calculer la vitesse de cette interacteur.

} t_iCircle2D;

void iCircle2D_bang(t_iCircle2D *x)
{
  SETFLOAT(&(x->force[14]), x->posX_old); // posX_old
  SETFLOAT(&(x->force[15]), x->posY_old); // posY_old

  x->posX_old = x->posX;
  x->posY_old = x->posY;

  if (x->x_sym->s_thing) typedmess(x->x_sym->s_thing, gensym("interactor_circle_2D"), 20, x->force);

  outlet_anything(x->force_new, gensym("interactor_circle_2D"), 20, x->force);
}

void iCircle2D_setXY(t_iCircle2D *x, t_float X, t_float Y)
{
  SETFLOAT(&(x->force[0]), X);
	x->posX= X;
  SETFLOAT(&(x->force[1]), Y);
	x->posY = Y;
}

void iCircle2D_setX(t_iCircle2D *x, t_float X)
{
  SETFLOAT(&(x->force[0]), X);
  x->posX= X;
}

void iCircle2D_setY(t_iCircle2D *x, t_float Y)
{
  SETFLOAT(&(x->force[1]), Y);
  x->posY = Y;
}

void iCircle2D_setRmin(t_iCircle2D *x, t_float X)
{
  SETFLOAT(&(x->force[2]), X);
}

void iCircle2D_setRmax(t_iCircle2D *x, t_float X)
{
  SETFLOAT(&(x->force[3]), X);
}

void iCircle2D_setFN(t_iCircle2D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[4]), f1);
}

void iCircle2D_setFT(t_iCircle2D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[5]), f1);
}

void iCircle2D_setKN(t_iCircle2D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[6]), f1);
}

void iCircle2D_setKT(t_iCircle2D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[7]), f1);
}

void iCircle2D_setRN(t_iCircle2D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[8]), f1);
}

void iCircle2D_setRT(t_iCircle2D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[9]), f1);
}

void iCircle2D_setDN(t_iCircle2D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[10]), f1);
}

void iCircle2D_setDT(t_iCircle2D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[11]), f1);
}

void iCircle2D_setdRN(t_iCircle2D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[12]), f1);
}

void iCircle2D_setdRT(t_iCircle2D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[13]), f1);
}

void iCircle2D_setD(t_iCircle2D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[16]), f1);
}

void iCircle2D_setG(t_iCircle2D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[17]), f1);
}

void iCircle2D_setdN(t_iCircle2D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[18]), f1);
}

void iCircle2D_setdT(t_iCircle2D *x, t_floatarg f1)
{
  SETFLOAT(&(x->force[19]), f1);
}

void *iCircle2D_new(t_symbol *s, int argc, t_atom *argv)
{
  t_iCircle2D *x = (t_iCircle2D *)pd_new(iCircle2D_class);

  x->x_sym = atom_getsymbolarg(0, argc, argv);

  x->force_new=outlet_new(&x->x_obj, 0);
  
  if (argc>=19)
    SETFLOAT(&(x->force[19]), atom_getfloatarg(18, argc, argv));
  else
    SETFLOAT(&(x->force[19]), 0);

  if (argc>=18)
    SETFLOAT(&(x->force[18]), atom_getfloatarg(17, argc, argv));
  else
    SETFLOAT(&(x->force[18]), 0);

 if (argc>=17)
    SETFLOAT(&(x->force[17]), atom_getfloatarg(16, argc, argv));
  else
    SETFLOAT(&(x->force[17]), 0);

  if (argc>=16)
    SETFLOAT(&(x->force[16]), atom_getfloatarg(15, argc, argv));
  else
    SETFLOAT(&(x->force[16]), 0);

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
    SETFLOAT(&(x->force[3]), 1);

  if (argc>=4)
    SETFLOAT(&(x->force[2]), atom_getfloatarg(3, argc, argv));
  else
    SETFLOAT(&(x->force[2]), 0);

  if (argc>=3)
  {
    SETFLOAT(&(x->force[1]), atom_getfloatarg(2, argc, argv));
    SETFLOAT(&(x->force[15]), atom_getfloatarg(2, argc, argv));
    x->posY_old = atom_getfloatarg(2, argc, argv);
  }
  else
  {
	  SETFLOAT(&(x->force[1]), 0);
 	  SETFLOAT(&(x->force[15]), 0);
  	  x->posY_old = 0;
  }

  if (argc>=2)
  {
	  SETFLOAT(&(x->force[0]), atom_getfloatarg(1, argc, argv));
	  SETFLOAT(&(x->force[14]), atom_getfloatarg(1, argc, argv));
	  x->posX_old = atom_getfloatarg(1, argc, argv);
  }
  else
  {
	  SETFLOAT(&(x->force[0]), 0);
	  SETFLOAT(&(x->force[14]), 0);
	  x->posX_old = 0;
  }

  return (x);
}

void iCircle2D_setup(void) 
{

  iCircle2D_class = class_new(gensym("iCircle2D"),
        (t_newmethod)iCircle2D_new,
        0, sizeof(t_iCircle2D),
        CLASS_DEFAULT, A_GIMME, 0);

  class_addcreator((t_newmethod)iCircle2D_new, gensym("pmpd.iCircle2D"),  A_GIMME, 0);
 
  class_addbang(iCircle2D_class, iCircle2D_bang);
  class_addmethod(iCircle2D_class, (t_method)iCircle2D_setFN, gensym("setFN"), A_DEFFLOAT, 0);
  class_addmethod(iCircle2D_class, (t_method)iCircle2D_setFT, gensym("setFT"), A_DEFFLOAT, 0);
  class_addmethod(iCircle2D_class, (t_method)iCircle2D_setKN, gensym("setKN"), A_DEFFLOAT, 0);
  class_addmethod(iCircle2D_class, (t_method)iCircle2D_setKT, gensym("setKT"), A_DEFFLOAT, 0);
  class_addmethod(iCircle2D_class, (t_method)iCircle2D_setDN, gensym("setDN"), A_DEFFLOAT, 0);
  class_addmethod(iCircle2D_class, (t_method)iCircle2D_setDT, gensym("setDT"), A_DEFFLOAT, 0);
  class_addmethod(iCircle2D_class, (t_method)iCircle2D_setD, gensym("setD"), A_DEFFLOAT, 0);
  class_addmethod(iCircle2D_class, (t_method)iCircle2D_setX, gensym("setX"), A_DEFFLOAT, 0);
  class_addmethod(iCircle2D_class, (t_method)iCircle2D_setY, gensym("setY"), A_DEFFLOAT, 0);
  class_addmethod(iCircle2D_class, (t_method)iCircle2D_setXY, gensym("setXY"), A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addmethod(iCircle2D_class, (t_method)iCircle2D_setRmin, gensym("setRmin"), A_DEFFLOAT, 0);
  class_addmethod(iCircle2D_class, (t_method)iCircle2D_setRmax, gensym("setRmax"), A_DEFFLOAT, 0);
  class_addmethod(iCircle2D_class, (t_method)iCircle2D_setRN, gensym("setRN"), A_DEFFLOAT, 0);
  class_addmethod(iCircle2D_class, (t_method)iCircle2D_setRT, gensym("setRT"), A_DEFFLOAT, 0);
  class_addmethod(iCircle2D_class, (t_method)iCircle2D_setdN, gensym("setdN"), A_DEFFLOAT, 0);
  class_addmethod(iCircle2D_class, (t_method)iCircle2D_setdT, gensym("setdT"), A_DEFFLOAT, 0);
  class_addmethod(iCircle2D_class, (t_method)iCircle2D_setdRN, gensym("setdRN"), A_DEFFLOAT, 0);
  class_addmethod(iCircle2D_class, (t_method)iCircle2D_setdRT, gensym("setdRT"), A_DEFFLOAT, 0);
  class_addmethod(iCircle2D_class, (t_method)iCircle2D_setG, gensym("setG"), A_DEFFLOAT, 0);

}
