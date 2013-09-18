/* This code is too trivial to have a licence or copyright.

   However it was modeled after arraysize from pixlib,
   http://pix.test.at/pd/pixlib
*/

/*
	arraysize -- report the size of an array
	
	usage: |arraysize <array name>|

	methods: bang, set <array name>
*/

#include <m_pd.h>

static t_class *arraysize_class;

typedef struct _arraysize {
  t_object  x_obj;
  t_symbol *array_name;
} t_arraysize;

void arraysize_set(t_arraysize *x, t_symbol *s)
{
  x->array_name = s;
}

void arraysize_bang(t_arraysize *x)
{
  t_garray *garray;

  if(!(garray = (t_garray *)pd_findbyclass(x->array_name,garray_class))) {
    pd_error(x, "%s: no such table", x->array_name->s_name);
  } else {
    outlet_float(x->x_obj.ob_outlet, garray_npoints(garray));
  }
} 

void *arraysize_new(t_symbol *s)
{
  t_arraysize *x = (t_arraysize *)pd_new(arraysize_class);

  symbolinlet_new(&x->x_obj, &x->array_name);
  outlet_new(&x->x_obj, gensym("float"));

  x->array_name = s;

  return (void *)x;
}

void arraysize_setup(void)
{
  arraysize_class = class_new(gensym("arraysize"), (t_newmethod)arraysize_new, 0, sizeof(t_arraysize), CLASS_DEFAULT, A_DEFSYMBOL, 0);
  
  class_addmethod(arraysize_class,(t_method)arraysize_set,gensym("set"), A_DEFSYMBOL, 0);
  class_addbang(arraysize_class,arraysize_bang);
}
