#include "m_pd.h"

/* -------------------------- ixprint ------------------------------ 
 * print without "print: " ...
 */
static t_class *ixprint_class;

typedef struct _ixprint
{
    t_object x_obj;
    t_symbol *x_sym;
} t_ixprint;

static void *ixprint_new(t_symbol *s)
{
    t_ixprint *x = (t_ixprint *)pd_new(ixprint_class);
    if (*s->s_name) x->x_sym = s;
    // change, plain print ..
    else x->x_sym = gensym("");
    return (x);
}

static void ixprint_bang(t_ixprint *x)
{
    post("bang", x->x_sym->s_name);
}

static void ixprint_pointer(t_ixprint *x, t_gpointer *gp)
{
    post("(gpointer)", x->x_sym->s_name);
}

static void ixprint_float(t_ixprint *x, t_float f)
{
    post("%g",f);
}

static void ixprint_list(t_ixprint *x, t_symbol *s, int argc, t_atom *argv)
{
    int i;
    char buf[80];
    if (argc && argv->a_type != A_SYMBOL) startpost("%s:", x->x_sym->s_name);
    else startpost("%s",
    	(argc > 1 ? s_list.s_name : s_symbol.s_name));
    postatom(argc, argv);
    endpost();
}

static void ixprint_anything(t_ixprint *x, t_symbol *s, int argc, t_atom *argv)
{
    int i;
    char buf[80];
    startpost("%s", s->s_name);
    postatom(argc, argv);
    endpost();
}

/* -------------------------- logger ------------------------------
 * log text input into file , output linecount ..
 */

/* static t_class *logger_class;

typedef struct _logger
{
  t_object x_obj;
  t_symbol x_fname;
  t_float x_lc;
} t_logger;

static void *logger_new(t_symbol *s)
{
    t_logger *x = (t_logger *)pd_new(logger_class);
    x->x_fname = gensym("pd.log");
    return (x);
}
*/

void ixprint_setup(void)
{
    ixprint_class = class_new(gensym("ixprint"), (t_newmethod)ixprint_new, 0,
    	sizeof(t_ixprint), 0, A_DEFSYM, 0);
    class_addbang(ixprint_class, ixprint_bang);
    class_addfloat(ixprint_class, ixprint_float);
    class_addpointer(ixprint_class, ixprint_pointer);
    class_addlist(ixprint_class, ixprint_list);
    class_addanything(ixprint_class, ixprint_anything);
}
