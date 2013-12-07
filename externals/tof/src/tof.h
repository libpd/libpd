
#ifndef TOF
#define TOF
#include "m_pd.h"
#include "g_canvas.h" 
#include <string.h>


#define IS_A_SYMBOL(atom,index) ((atom+index)->a_type == A_SYMBOL)
#define IS_A_FLOAT(atom,index) ((atom+index)->a_type == A_FLOAT)



//static char tof_buf_temp_a[MAXPDSTRING];
//static char tof_buf_temp_b[MAXPDSTRING];


/* 
 * ALLOCATE WITH THE FOLLOWING LINE
 * t_atom *at = getbytes((SIZE)*sizeof(*at));	
 * FREE WITH THIS ONE
 * freebytes(at, (SIZE)*sizeof(*at));
*/
static void tof_copy_atoms(t_atom *src, t_atom *dst, int n)
{
  while(n--)
    *dst++ = *src++;
}




static t_symbol* tof_get_dollar(t_canvas* canvas, t_symbol* dollar) {
	return canvas_realizedollar(canvas, dollar);
}

static t_symbol* tof_get_dollarzero(t_canvas* canvas) {
	return canvas_realizedollar(canvas, gensym("$0"));
}

//canvas_isabstraction(t_canvas *x)
static int tof_canvas_is_not_subpatch(t_canvas* canvas) {
	
	 return (canvas->gl_env != 0);
}


static t_canvas* tof_get_canvas(void) 
{
    t_glist *glist=(t_glist *)canvas_getcurrent();
    
    return (t_canvas*)glist_getcanvas(glist);
    //return glist_getcanvas((t_glist *)canvas_getcurrent());
}


static t_symbol* tof_get_dir(t_canvas* canvas) {
	
	return canvas_getdir(canvas);
}

static t_symbol* tof_get_canvas_name(t_canvas* canvas) {
	
    return	canvas->gl_name;
    
}

// Use &s_list selector if you do not want to process the selector

static int tof_anything_to_string( t_symbol* s, int ac, t_atom* av,char* buffer ) {

    
	
	if ( s == &s_bang) {
		buffer[0] = '\0';
		return 0;
	} else if ( s == &s_symbol) {
		if ( ac ) {
			 strcpy(buffer,atom_getsymbol(av)->s_name);
		 } else {
			 buffer[0] = '\0';
		 }
		 return strlen(buffer);
		 
	} else {
		
		char* buf;
		t_binbuf* binbuf = binbuf_new();
		
		if ( !(s == &s_list || s == &s_float )  ) {
			t_atom selector;
			SETSYMBOL(&selector,s);
			binbuf_add(binbuf, 1, &selector);
		
		}
		
		binbuf_add(binbuf, ac, av);
	
		int length;
		
		binbuf_gettext(binbuf, &buf, &length);
		
		
		int i;
		for (i = 0; i < length; i++ ) {
			buffer[i] = buf[i];
		}
		
		
		buffer[length] = '\0';
		
		freebytes(buf, length);
		binbuf_free(binbuf);
		
		return length;
    }
	buffer[0] = '\0';
	return 0;

}

// returns the last canvas before the root canvas
static t_canvas* tof_get_canvas_before_root(t_canvas* canvas) 
{
    // Find the proper parent canvas
    while ( canvas->gl_owner && canvas->gl_owner->gl_owner) {
        canvas = canvas->gl_owner;
    }
    return canvas;
}

static t_canvas* tof_get_root_canvas(t_canvas* canvas) 
{
    // Find the root canvas
    while ( canvas->gl_owner) {
       
        canvas = canvas->gl_owner;
    }
    return canvas;
}

static void tof_get_canvas_arguments(t_canvas *canvas, int *ac_p, t_atom **av_p) {
    pd_pushsym(&canvas->gl_pd); //canvas_setcurrent(canvas);
    canvas_getargs(ac_p , av_p);
    pd_popsym(&canvas->gl_pd); //canvas_unsetcurrent(canvas);
	
}





/*
static int tof_get_tagged_argument(char tag, int ac, t_atom *av, int *start, int *count) {
	int i;
	
    if ( ac == 0 || *start >= ac) {
		*count = 0;
		return 0;
	}

    for ( i= *start + 1; i < ac; i++ ) {
        if ( (av+i)->a_type == A_SYMBOL 
          && (atom_getsymbol(av+i))->s_name[0] == tag) break;
     }
     *count = i - *start;

     return (i-*start);     
}
*/

// Returned value permits a while operation on the function
static int tof_next_tagged_argument(char tag, int ac, t_atom *av, int* ac_a, t_atom ** av_a, int* iter) {
	int i;
	
    if ( ac == 0 || *iter >= ac) {
		*ac_a = 0;
		*av_a = NULL;
		return 0;
	}

    for ( i= *iter + 1; i < ac; i++ ) {
        if ( (av+i)->a_type == A_SYMBOL 
          && (atom_getsymbol(av+i))->s_name[0] == tag) break;
     }
	 *ac_a = i - *iter;
	 *av_a = av + *iter;
	 *iter = i;
     
     return (*ac_a);     
}



static int tof_find_symbol(t_symbol* name, int ac, t_atom* av) {
	int i;
	for (i=0; i<ac;i++) {
		if ( IS_A_SYMBOL(av,i) && name == atom_getsymbol(av+i) ) {
			return 1;
		}
	}
	return 0;
}




// Returns 1 if the tag was found
// Fills the given int* and t_atom** with the arguments of the tag
static int tof_find_tagged_argument(char tag,t_symbol *name, int ac, t_atom *av, int *ac_r,t_atom** av_r) {
	
	int i;
	int match = 0;
	int j = 0;
	for (i=0;i<ac;i++) {
		if ( IS_A_SYMBOL(av,i) && name == atom_getsymbol(av+i) ) {
			// FOUND MATCH
			match = 1;
			if ( (i+1)<ac ) { // IS THERE SPACE FOR AT LEAST ONE ARGUMENT?
				i=i+1;
				for (j=i;j<ac;j++) { //FIND ITS END
					if (  IS_A_SYMBOL(av,j) && (atom_getsymbol(av+j))->s_name[0] == tag ) {
						//j = j-1;
						break;
					}
				}
				break;
			}
		}
	}
	j = j-i;
	//post("i:%d j:%d",i,j);
	
	
	if ( j > 0) {
		*ac_r = j;
		*av_r = av+i;
	} else {
		*ac_r = 0;
		*av_r = NULL;
	}
	
	return match;
	
 }



static void tof_outlet_list_prepend(t_outlet* outlet, t_symbol* s, int argc, t_atom* argv, t_symbol* pp) {
	
	if (s == &s_list || s == &s_float || s == &s_symbol ) {
		int ac = argc + 1;
		t_atom *av = (t_atom *)getbytes(ac*sizeof(t_atom));	
		tof_copy_atoms(argv,av+1,argc);
		SETSYMBOL(av, pp);
		outlet_list(outlet,&s_list,ac,av);
		freebytes(av, ac*sizeof(t_atom));
		
		//outlet_list(outlet,&s_list,argc,argv);
	} else if (s == &s_bang ) {
		t_atom a;
		SETSYMBOL(&a,pp);
		outlet_list(outlet,&s_list,1,&a);
	} else {
		int ac = argc + 2;
		t_atom *av = (t_atom *)getbytes(ac*sizeof(t_atom));	
		tof_copy_atoms(argv,av+2,argc);
		SETSYMBOL(av+1, s);
		SETSYMBOL(av, pp);
		outlet_list(outlet,&s_list,ac,av);
		freebytes(av, ac*sizeof(t_atom));
		//outlet_symbol(outlet,pp);
	}
}

static void tof_outlet_anything_prepend(t_outlet* outlet, t_symbol* s, int argc, t_atom* argv, t_symbol* pp) {
	
	if (s == &s_list || s == &s_float  || s == &s_symbol) {
			outlet_anything(outlet,pp,argc,argv);
	} else if (s != &s_bang) {
			int ac = argc + 1;
			t_atom *av = (t_atom *)getbytes(ac*sizeof(t_atom));	
			tof_copy_atoms(argv,av+1,argc);
			SETSYMBOL(av, s);
			outlet_anything(outlet,pp,ac,av);
			freebytes(av, ac*sizeof(t_atom));
	} else {
		//t_atom a;
		outlet_anything(outlet,pp,0,NULL);
		//outlet_symbol(outlet,pp);
	}
}
	
	
static void tof_send_anything_prepend(t_symbol* target,t_symbol* selector, int ac, t_atom* av,t_symbol* prepend) {

	if (target->s_thing) {
		if ( selector == &s_list || selector == &s_float || selector == &s_symbol ) {
			typedmess(target->s_thing, prepend, ac, av);
		} else if ( selector == &s_bang) {
			
			typedmess(target->s_thing, prepend, 0, NULL);
		} else {
			int new_ac = ac + 1;
			t_atom *new_av = getbytes(new_ac*sizeof(*new_av));	
			tof_copy_atoms(av,new_av+1,ac);
			SETSYMBOL(new_av, selector);
			typedmess(target->s_thing, prepend, new_ac, new_av);
			freebytes(new_av, new_ac*sizeof(*new_av));
		}
	}
	
}


static t_symbol* tof_remove_extension(t_symbol* s) {
	t_symbol* newsymbol = s;
	int length = strlen(s->s_name) + 1;
	char* newstring = getbytes(length * sizeof(*newstring));
	strcpy(newstring,s->s_name);
	char* last = strrchr( newstring, '.');
	if (last != NULL) {
		*last = '\0';
		newsymbol = gensym(newstring);
	}
	freebytes(newstring,length * sizeof(*newstring));
	return newsymbol;
}
#endif
