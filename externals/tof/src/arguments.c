

 #include "m_pd.h"

#define IS_A_SYMBOL(atom,index) ((atom+index)->a_type == A_SYMBOL)
#define IS_A_FLOAT(atom,index) ((atom+index)->a_type == A_FLOAT)

#define TOKEN 0
#define COMMA 1
#define ALL 2

static t_class *arguments_class;


typedef struct _arguments
{
  t_object                  	x_obj;
  t_outlet*			       	 	outlet;
  t_canvas*						canvas;
  t_symbol*						s_comma;
  int 							mode;
  char							token;
  int                           size;
  int                           ac;
  t_atom*                       av;
} t_arguments;


// Prepends the proper selector
static void arguments_output(t_outlet* o, int argc, t_atom* argv) {

		if (!argc) outlet_bang(o);
		else if (IS_A_SYMBOL(argv,0)) outlet_anything(o,atom_getsymbol(argv),argc-1,argv+1);
		else if (IS_A_FLOAT(argv,0) && argc==1) outlet_float(o,atom_getfloat(argv));
		else outlet_anything(o,&s_list,argc,argv);
}


static int next_token(char tag, int ac, t_atom *av, int* ac_a, t_atom ** av_a, int* iter) {
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

// Dump arguments
static void arguments_bang(t_arguments *x) {
	
	
	
	
	if ( x->mode == TOKEN ) {
		int ac_a;
		t_atom* av_a;
		t_symbol* selector_a;
		int iter = 0 ;
		while ( next_token(x->token, x->ac, x->av, &ac_a, &av_a,&iter)) {
			arguments_output(x->outlet,ac_a,av_a);
		}
	} else if (x->mode == COMMA) {
		// we are trusting the system that the commas are split from the
		// neighboring floats and symbols
		int ac = x->ac;
		t_atom* av = x->av;
		t_symbol* selector;
		
		int i =0;
		int j;
		for (j=0; j<ac; j++) {
			if ( (IS_A_SYMBOL(av,j) && atom_getsymbol(av+j)==x->s_comma ) ) {
				if ( (j != i) ) arguments_output(x->outlet, j - i, av+i);
				
				i = j+1;
			}
		}
		// Output any leftovers
		if ( j != i) arguments_output(x->outlet, ac-i, av+i);
	} else { //x->mode = ALL
		arguments_output(x->outlet, x->ac, x->av);
	}
}


static void arguments_free(t_arguments *x)
{
    freebytes(x->av,x->ac*sizeof(*(x->av)));
}

static void copy_atoms(t_atom *src, t_atom *dst, int n)
{
  while(n--)
    *dst++ = *src++;
}



static void *arguments_new(t_symbol *selector, int argc, t_atom* argv) {
  t_arguments *x = (t_arguments *)pd_new(arguments_class);
  
  x->s_comma = gensym(",");
  
  if ( argc && IS_A_SYMBOL(argv,0) ) {
	  t_symbol* s = atom_getsymbol(argv);
	  if ( s == gensym("comma") || s == x->s_comma ) {
		  x->mode = COMMA;
		 // post("COMMA");
	  } else {
		  x->mode = TOKEN;
		  x->token = s->s_name[0];
		  //post("TOKEN: %c",x->token);
	  }
  } else {
	   //post("ALL");
	  x->mode = ALL;
  }
  
 
  int ac;
  t_atom* av;
  canvas_getargs(&ac, &av);
   
   x->ac = ac;
   x->av = getbytes(x->ac * sizeof(*(x->av)));
   copy_atoms(av,x->av,x->ac);
   
   x->outlet = outlet_new(&x->x_obj, &s_anything);
    
  return (x);
}

void arguments_setup(void) {
  arguments_class = class_new(gensym("arguments"),
    (t_newmethod)arguments_new, (t_method)arguments_free,
    sizeof(t_arguments), 0, A_GIMME, 0);

 class_addbang(arguments_class, arguments_bang);
 

}
