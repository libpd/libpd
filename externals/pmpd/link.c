#include "m_pd.h"
#include "math.h"

static t_class *linkKD_class;

typedef struct _linkKD {
  t_object  x_obj;
  t_float raideur, viscosite, D2, longueur, distance_old, position1,  position2, position_old1, position_old2;
  t_outlet *force1;
  t_outlet *force2;
  t_float Lmin, Lmax;
  t_symbol *x_sym;  // receive
} t_linkKD;

void linkKD_float(t_linkKD *x, t_floatarg f1)
{
  x->position1 = f1;
}

void linkKD_bang(t_linkKD *x)
{
  t_float force1, force2, distance;

  distance = (x->position2 - x->position1);
//distance = abs(x->position2 - x->position1);
  if (distance<0) distance = -distance;

  force1 =  x->raideur*(distance-(x->longueur)) + x->viscosite*(distance - x->distance_old) ;

  x->distance_old = distance;  

  if (distance > x->Lmax) force1=0;
  if (distance < x->Lmin) force1=0;

   if (distance != 0)
  {
    force1 = force1 * (x->position2 - x->position1) / distance;
  }

  force2 = -force1 + (x->position_old2 - x->position2)*x->D2;
  force1 += (x->position_old1 - x->position1)*x->D2;
  // masse damping

  outlet_float(x->force1, force1);
  outlet_float(x->force2, force2);
  
 
  x->position_old1 = x->position1;
  x->position_old2 = x->position2;

}

void linkKD_reset(t_linkKD *x)
{
  x->position1 = 0;
  x->position2 = 0;

  x->position_old1 = 0;
  x->position_old2 = 0;

  x->distance_old = x->longueur;
}

void linkKD_resetF(t_linkKD *x)
{
  x->position_old1 = x->position1;
  x->position_old2 = x->position2;

  x->distance_old = x->longueur;
}

void linkKD_resetl(t_linkKD *x)
{
  x->longueur = (x->position1 - x->position2);
}

void linkKD_setL(t_linkKD *x, t_float L)
{
  x->longueur = L;
}

void linkKD_setK(t_linkKD *x, t_float K)
{
  x->raideur = K;
}

void linkKD_setD(t_linkKD *x, t_float D)
{
  x->viscosite = D;
}

void linkKD_setD2(t_linkKD *x, t_float D2)
{
  x->D2 = D2;
}

void linkKD_Lmin(t_linkKD *x, t_float Lmin)
{
  x->Lmin = Lmin;
}

void linkKD_Lmax(t_linkKD *x, t_float Lmax)
{
  x->Lmax = Lmax;
}

static void linkKD_free(t_linkKD *x)
{
    pd_unbind(&x->x_obj.ob_pd, x->x_sym);
}

void *linkKD_new(t_symbol *s, t_floatarg L, t_floatarg K, t_floatarg D, t_floatarg D2 )
{
  
  t_linkKD *x = (t_linkKD *)pd_new(linkKD_class);

  x->x_sym = s;
  pd_bind(&x->x_obj.ob_pd, s);

  floatinlet_new(&x->x_obj, &x->position2);

  x->force1=outlet_new(&x->x_obj, 0);
  x->force2=outlet_new(&x->x_obj, 0);

  x->position1 = 0;
  x->position2 = 0;
 
  x->raideur=K;
  x->viscosite=D;
  x->D2=D2;

  x->Lmin= 0;
  x->Lmax= 10000;

  x->longueur=L;

  return (void *)x;
}

void link_setup(void) 
{
  linkKD_class = class_new(gensym("link"),
        (t_newmethod)linkKD_new,
        (t_method)linkKD_free,
		sizeof(t_linkKD),
        CLASS_DEFAULT, A_DEFSYM, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);

  class_addcreator((t_newmethod)linkKD_new, gensym("lia"), A_DEFSYM, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);

  class_addfloat(linkKD_class, linkKD_float);
  class_addbang(linkKD_class, linkKD_bang);
  class_addmethod(linkKD_class, (t_method)linkKD_reset, gensym("reset"), 0);
  class_addmethod(linkKD_class, (t_method)linkKD_resetl, gensym("resetL"), 0);
  class_addmethod(linkKD_class, (t_method)linkKD_resetF, gensym("resetF"), 0);
  class_addmethod(linkKD_class, (t_method)linkKD_setD, gensym("setD"), A_DEFFLOAT, 0);
  class_addmethod(linkKD_class, (t_method)linkKD_setD2, gensym("setD2"), A_DEFFLOAT, 0);
  class_addmethod(linkKD_class, (t_method)linkKD_setK, gensym("setK"), A_DEFFLOAT, 0);
  class_addmethod(linkKD_class, (t_method)linkKD_setL, gensym("setL"), A_DEFFLOAT, 0);
  class_addmethod(linkKD_class, (t_method)linkKD_Lmin, gensym("setLmin"), A_DEFFLOAT, 0);
  class_addmethod(linkKD_class, (t_method)linkKD_Lmax, gensym("setLmax"), A_DEFFLOAT, 0);
}
