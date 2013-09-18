/*
 *      argument.c
 *      
 *      Copyright 2009 Thomas O Fredericks <tom@hp>
 *      
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *      
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *      
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#include "tof.h"

//extern int sys_noloadbang;

t_class *argument_class;

typedef struct argument
{
    t_object x_ob;
    //t_canvas * x_canvas;
    t_outlet* x_outlet;
    t_atom x_a;
	int has_value;
} t_argument;

static void argument_bang(t_argument *x)
{
	
   if (x->has_value) {
		if ( IS_A_SYMBOL(&(x->x_a),0) ) {
			outlet_symbol(x->x_outlet,atom_getsymbol(&(x->x_a)));
		} else {
			outlet_float(x->x_outlet,atom_getfloat(&(x->x_a)));
		}
    }
   

}



static void argument_loadbang(t_argument *x)
{
   // if (!sys_noloadbang)
   //     argument_bang(x);
}

static void argument_free(t_argument *x) {
	
	//freebytes(x->x_a, sizeof(t_atom));
}


//static void *argument_new(t_floatarg level)
static void *argument_new(t_symbol *s, int argc, t_atom *argv)
{
    t_argument *x = (t_argument *)pd_new(argument_class);
    
  
    t_canvas *canvas=tof_get_canvas();
    
    x->x_outlet =  outlet_new(&x->x_ob, &s_list);
    //x->x_a = (t_atom *)getbytes(sizeof(t_atom));
  
   x->has_value = 0;
   int i = 0;
  
   // Check argument i and default value
    if ( argc >= 1 && IS_A_FLOAT(argv,0) ) {
    	i = atom_getfloat(argv);
	}
	
	 // Get the canvas' arguments
   int ac;
   t_atom *av;
   tof_get_canvas_arguments(canvas,&ac, &av);
 

   // Check arguments 
   
   if ( i == 0) { //Is the argument index 0?
	 // Get the dollar zero
	  SETSYMBOL(&(x->x_a),tof_get_dollar(canvas,gensym("$0")));
	   x->has_value = 1;
   } else {
	   //if ( ac  >= i  ) {
			if ( argc > 1 ) { //Is there a default argument?
			   //Are the parent and default arguments the same type?
			   if ( ac  >= i && (av+(i-1))->a_type == (argv+1)->a_type) {
				   x->x_a = av[i-1]; //Use the parent value
			   } else {
				   x->x_a = argv[1]; //Use the default value
				}
				x->has_value = 1;
			} else { //No default argument, so no type check
			   if ( ac  >= i ) {  //Are there enough parent arguments?
					x->x_a = av[i-1]; //Use the parent value
					x->has_value = 1;
				}
			}
	        
	   //}
	}

    return (void *)x;
}

void argument_setup(void)
{
	    
    argument_class = class_new(gensym("argument"), 
    (t_newmethod)argument_new, (t_method)argument_free,
        sizeof(t_argument), CLASS_DEFAULT, A_GIMME,0);
    class_addbang(argument_class, argument_bang);
   
   
   //class_addmethod(argument_class, (t_method)argument_loadbang, gensym("loadbang"), 0);
   
}

