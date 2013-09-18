#include "m_pd.h"

//static char* from_ascii_code_text;
// Common binbuf to all objects
static t_binbuf* from_ascii_code_binbuf;

typedef struct _from_ascii_code
{
	
	t_object 			x_ob;
	int					eof_is_set;
	int				    eof;
	t_outlet*			outlet_left;
	//t_outlet*			outlet_right;
	char*				text;
	int					text_size;
	int 				size;
} t_from_ascii_code;

static t_class *from_ascii_code_class;

// Force output even if eof has not been received
static void from_ascii_code_bang(t_from_ascii_code *x) {
	
	if ( x->size ) {
		binbuf_clear(from_ascii_code_binbuf);
		binbuf_text(from_ascii_code_binbuf, x->text, x->size);
		t_atom *argv = binbuf_getvec(from_ascii_code_binbuf);
		int     argc = binbuf_getnatom(from_ascii_code_binbuf);
		if ( argc) {
			if ( (argv->a_type == A_SYMBOL)	) {
				outlet_anything(x->outlet_left,atom_getsymbol(argv),argc-1,argv+1);
			} else {
				outlet_anything(x->outlet_left,&s_list,argc,argv);
			}
		}
		x->size = 0;
	}
}


static void from_ascii_code_float(t_from_ascii_code *x, t_float f) {
	
	if ( x->eof_is_set && f == x->eof ) {
		from_ascii_code_bang(x);
	} else if ( f > 31 && f < 127 ) {
		x->text[x->size] = (char) f;
		x->size = x->size + 1;
		if ( x->size >= x->text_size ) {
			
			x->text = resizebytes(x->text, x->text_size * sizeof(*(x->text)), 
					(x->text_size + 100) * sizeof(*(x->text)));
			x->text_size = x->text_size + 100;
		}
	}
}

static void  from_ascii_code_list(t_from_ascii_code *x, t_symbol *s, int argc, t_atom *argv) {
	int i;
	for ( i=0; i < argc; i++ ) {
		if ( ((argv+i)->a_type == A_FLOAT) ) {
			from_ascii_code_float(x,atom_getfloat(argv+i));
		}
	}
}

static void from_ascii_code_free(t_from_ascii_code *x) {
	
	freebytes(x->text,x->text_size*sizeof(*(x->text)));
}

static void *from_ascii_code_new(t_symbol *s, int ac, t_atom *av)
{
    t_from_ascii_code *x = (t_from_ascii_code *)pd_new(from_ascii_code_class);
    
    // set eof if eof is set
    if ( ac && ((av)->a_type == A_FLOAT)) {
		x->eof = (int) atom_getfloat(av);
		x->eof_is_set = 1;
    } else {
		x->eof_is_set = 0;
	}
    
    // create string
    x->text_size = 100;
	x->text = getbytes(x->text_size*sizeof(*x->text));	
    
    x->size = 0;
    
    x->outlet_left = outlet_new(&x->x_ob, &s_list);
    //x->outlet_right = outlet_new(&x->x_ob, &s_float);
    
    return (x);
}

void from_ascii_code_setup(void)
{
	// create binbuf (no need to ever free)
	from_ascii_code_binbuf = binbuf_new();
	
	
	
    from_ascii_code_class = class_new(gensym("from_ascii_code"),
			    (t_newmethod)from_ascii_code_new, (t_method)from_ascii_code_free,
			    sizeof(t_from_ascii_code), 0, A_GIMME, 0);
    
    class_addbang(from_ascii_code_class, from_ascii_code_bang);
    class_addfloat(from_ascii_code_class, from_ascii_code_float);
    class_addlist(from_ascii_code_class, from_ascii_code_list);
    
    }
