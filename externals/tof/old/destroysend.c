// Made by tof@danslchamp.prg

#include "m_pd.h"
#include <string.h>
#include <stdio.h>

static t_class *destroysend_class;

typedef struct _destroysend {
  t_object  x_obj;
  t_symbol *x_sym; //from pd_send
} t_destroysend;

void destroysend_bang(t_destroysend *x)
{
  //post("Hello world !!");
//From pd_send  
  if (x->x_sym->s_thing) pd_bang(x->x_sym->s_thing);
//END 
}


void *destroysend_new(t_symbol *s) //Added args from pd send
{
  t_destroysend *x = (t_destroysend *)pd_new(destroysend_class);

//From pd_send  

    if (!*s->s_name) symbolinlet_new(&x->x_obj, &x->x_sym);
    x->x_sym = s;

//END 

  return (void *)x; //return (x);
}


void *destroysend_free(t_destroysend *x)
{
   if (x->x_sym->s_thing) pd_bang(x->x_sym->s_thing);
	//post("Killing !!");
	return 0;
}

void destroysend_setup(void) {

  destroysend_class = class_new(gensym("destroysend"),(t_newmethod)destroysend_new,(t_method)destroysend_free, 
sizeof(t_destroysend),0, A_DEFSYM, 0);
  class_addbang(destroysend_class, destroysend_bang);
}
