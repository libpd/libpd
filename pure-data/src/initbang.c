
#include "m_pd.h"
#include "s_stuff.h"

static t_class *initbang_class;

typedef struct _initbang
{
    t_object x_obj;
} t_initbang;

static void *initbang_new(void)
{
    t_initbang *x = (t_initbang *)pd_new(initbang_class);
    outlet_new(&x->x_obj, &s_bang);
    return (x);
}

static void initbang_initbang(t_initbang *x)
{
  if (!sys_noloadbang) /* JMZ: hmm, not sure whether we should respect sys_noloadbang here... */
        outlet_bang(x->x_obj.ob_outlet);
}

void initbang_setup(void)
{
  initbang_class = class_new(gensym("initbang"), (t_newmethod)initbang_new, 0,
        sizeof(t_initbang), CLASS_NOINLET, 0);
    class_addmethod(initbang_class, (t_method)initbang_initbang,
        gensym("initbang"), 0);
}
