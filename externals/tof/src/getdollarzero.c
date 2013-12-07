// Original object by (C) 2005 Guenter Geiger
// New version by Thomas Ouellet Fredericks


#include "m_pd.h"
#include "g_canvas.h"

#define IS_A_SYMBOL(atom,index) ((atom+index)->a_type == A_SYMBOL)
#define IS_A_FLOAT(atom,index) ((atom+index)->a_type == A_FLOAT)

typedef struct getdollarzero
{
    t_object x_ob;
    t_canvas * x_canvas;
    t_outlet* x_outlet;
    //int x_level;
} t_getdollarzero;




static void getdollarzero_bang(t_getdollarzero *x)
{
    
// x->s_parent_unique = canvas_realizedollar((t_canvas *)this_canvas->gl_owner, gensym("$0"));
    //outlet_symbol(x->x_outlet,canvas_getdir(last));
    outlet_symbol(x->x_outlet,canvas_realizedollar(x->x_canvas, gensym("$0")));
}

t_class *getdollarzero_class;

static void *getdollarzero_new(t_symbol *s, int argc, t_atom *argv)
{
    t_getdollarzero *x = (t_getdollarzero *)pd_new(getdollarzero_class);
	
	
		t_canvas* last = canvas_getcurrent();
		
		if ( argc > 0 && IS_A_FLOAT(argv,0) ) {
			int i = atom_getfloat(argv);
			
			while (i>0) {
				i--;
				if (last->gl_owner) {
					last = last->gl_owner;
				} else {
					break;
				}
			}
			
		} else if ( argc > 0 && IS_A_SYMBOL(argv,0) && atom_getsymbol(argv) == gensym("root") ) {
			
			 while ( last->gl_owner) {
       
				last = last->gl_owner;
			}
		}
		
	x->x_canvas = last;
	
	
    
    x->x_outlet =  outlet_new(&x->x_ob, &s_);
   
	
    return (void *)x;
}

void getdollarzero_setup(void)
{
    getdollarzero_class = class_new(gensym("getdollarzero"), (t_newmethod)getdollarzero_new, 0,
    	sizeof(t_getdollarzero), 0, A_GIMME,0);
    class_addbang(getdollarzero_class, getdollarzero_bang);
}

