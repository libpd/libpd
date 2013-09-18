#include "m_pd.h"
#include "math.h"

#define max(a,b) ( ((a) > (b)) ? (a) : (b) ) 
#define min(a,b) ( ((a) < (b)) ? (a) : (b) ) 


static t_class *mass_class;

typedef struct _mass {
  t_object  x_obj;
  t_float pos_old_1, pos_old_2, Xinit;
  t_float force, mass, dX;
  t_float minX, maxX;
  t_outlet *position_new, *vitesse_out, *force_out;
  t_symbol *x_sym; // receive
  unsigned int x_state; // random
  t_float x_f; // random

} t_mass;

static int makeseed(void)
{
    static unsigned int random_nextseed = 1489853723;
    random_nextseed = random_nextseed * 435898247 + 938284287;
    return (random_nextseed & 0x7fffffff);
}

static float random_bang(t_mass *x)
{
    int nval;
    int range = 2000000;
	float rnd;
	unsigned int randval = x->x_state;
	x->x_state = randval = randval * 472940017 + 832416023;
    nval = ((double)range) * ((double)randval)
    	* (1./4294967296.);
    if (nval >= range) nval = range-1;

	rnd=nval;

	rnd-=1000000;
	rnd=rnd/1000000.;	//pour mettre entre -1 et 1;
    return (rnd);
}

void mass_minX(t_mass *x, t_floatarg f1)
{
  x->minX = f1;
}

void mass_maxX(t_mass *x, t_floatarg f1)
{
  x->maxX = f1;
}

void mass_float(t_mass *x, t_floatarg f1)
{
  x->force += f1;
}

void mass_bang(t_mass *x)
{
  t_float pos_new;

	if (x->mass > 0)
  pos_new = x->force/x->mass + 2*x->pos_old_1 - x->pos_old_2;
	else pos_new = x->pos_old_1;

  pos_new = max(min(x->maxX, pos_new), x->minX);

  pos_new += x->dX;

  x->pos_old_1 += x->dX; // pour ne pas avoir d'inertie suplementaire du a ce deplacement
 
  outlet_float(x->vitesse_out, x->pos_old_1 - x->pos_old_2);
  outlet_float(x->force_out, x->force);
  outlet_float(x->position_new, pos_new);

  x->pos_old_2 = x->pos_old_1;
  x->pos_old_1 = pos_new;

//  x->force = 0;

  x->force = random_bang(x)*1e-25; // avoiding denormal problem by adding low amplitude noise

  x->dX = 0;

}

void mass_reset(t_mass *x)
{
  x->pos_old_2 = x->Xinit;
  x->pos_old_1 = x->Xinit;

  x->force=0;

  outlet_float(x->position_new, x->Xinit);
}

void mass_resetF(t_mass *x)
{
  x->force=0;

}

void mass_dX(t_mass *x, t_float posX)
{
  x->dX += posX;
}

void mass_setX(t_mass *x, t_float posX)
{
  x->pos_old_2 = posX;			// clear history for stability (instability) problem
  x->pos_old_1 = posX;

  x->force=0;

  outlet_float(x->position_new, posX);
}

void mass_loadbang(t_mass *x)
{
  outlet_float(x->position_new, x->Xinit);
}

void mass_set_mass(t_mass *x, t_float mass)
{
  x->mass=mass;
}

static void mass_free(t_mass *x)
{
    pd_unbind(&x->x_obj.ob_pd, x->x_sym);
}

void *mass_new(t_symbol *s, t_floatarg M, t_floatarg X)
{
  
  t_mass *x = (t_mass *)pd_new(mass_class);

  x->x_sym = s;
  pd_bind(&x->x_obj.ob_pd, s);

  x->position_new=outlet_new(&x->x_obj, 0);
  x->force_out=outlet_new(&x->x_obj, 0);
  x->vitesse_out=outlet_new(&x->x_obj, 0); 

  x->Xinit=X;

  x->pos_old_1 = X;
  x->pos_old_2 = X;
  x->force=0;
  x->mass=M;

  x->minX = -100000;
  x->maxX = 100000;

  if (x->mass<=0) x->mass=1;

  makeseed();

  return (void *)x;
}

void mass_setup(void) 
{

  mass_class = class_new(gensym("mass"),
        (t_newmethod)mass_new,
        (t_method)mass_free,
		sizeof(t_mass),
        CLASS_DEFAULT, A_DEFSYM, A_DEFFLOAT, A_DEFFLOAT,0);
  class_addcreator((t_newmethod)mass_new, gensym("masse"), A_DEFSYM, A_DEFFLOAT, A_DEFFLOAT,0);
  class_addfloat(mass_class, mass_float);
  class_addbang(mass_class, mass_bang);
  class_addmethod(mass_class, (t_method)mass_set_mass, gensym("setM"), A_DEFFLOAT, 0);
  class_addmethod(mass_class, (t_method)mass_setX, gensym("setX"), A_DEFFLOAT, 0);
  class_addmethod(mass_class, (t_method)mass_dX, gensym("dX"), A_DEFFLOAT, 0);
  class_addmethod(mass_class, (t_method)mass_reset, gensym("reset"), 0);
  class_addmethod(mass_class, (t_method)mass_resetF, gensym("resetF"), 0);
  class_addmethod(mass_class, (t_method)mass_minX, gensym("setXmin"), A_DEFFLOAT, 0);
  class_addmethod(mass_class, (t_method)mass_maxX, gensym("setXmax"), A_DEFFLOAT, 0);
  class_addmethod(mass_class, (t_method)mass_loadbang, gensym("loadbang"), 0);
}

