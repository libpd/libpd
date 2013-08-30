
#include "m_pd.h"
#include "s_stuff.h"

static t_class *closebang_class;

typedef struct _closebang
{
    t_object x_obj;
} t_closebang;

static void *closebang_new(void)
{
    t_closebang *x = (t_closebang *)pd_new(closebang_class);
    outlet_new(&x->x_obj, &s_bang);
    return (x);
}
static void closebang_closebang(t_closebang *x)
{
  if (!sys_noloadbang) /* JMZ: hmm, not sure whether we should respect sys_noloadbang here... */
        outlet_bang(x->x_obj.ob_outlet);
}

void closebang_setup(void)
{
  closebang_class = class_new(gensym("closebang"), (t_newmethod)closebang_new, 0,
        sizeof(t_closebang), CLASS_NOINLET, 0);
    class_addmethod(closebang_class, (t_method)closebang_closebang,
        gensym("closebang"), 0);
}
