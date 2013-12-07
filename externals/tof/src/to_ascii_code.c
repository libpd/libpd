#include "m_pd.h"

// Common string to all objects
static char* to_ascii_code_text;
// Common binbuf to all objects
static t_binbuf* to_ascii_code_binbuf;

typedef struct _to_ascii_code
{
	
	t_object 			x_ob;
	int					eof_is_set;
	int				    eof;
	t_outlet*			outlet_left;
	//t_outlet*			outlet_right;
} t_to_ascii_code;

static t_class *to_ascii_code_class;

static void to_ascii_code_bang(t_to_ascii_code *x) {
	// Do nothing
}


static void to_ascii_code_anything(t_to_ascii_code *x, t_symbol *s, int ac, t_atom *av) {
	
	binbuf_clear(to_ascii_code_binbuf);
	// Add selector if it is not standard
	if ( s != &s_list && s !=  &s_float && s != &s_symbol && s != &s_) {
		t_atom a;
		SETSYMBOL(&a, s);
		binbuf_add(to_ascii_code_binbuf,1,&a);
	}
	// Add all the atoms to the binbuf
	binbuf_add(to_ascii_code_binbuf, ac, av);
	
	
	
	// Get the contents as a text
	int size=0;
	binbuf_gettext(to_ascii_code_binbuf, &to_ascii_code_text, &size); //void binbuf_gettext(t_binbuf *x, char **bufp, int *lengthp);
	
	// Convert to a list of floats
	
	t_atom *list_floats = getbytes((size+1)*sizeof(*list_floats));	// Add some space for the eof character 
	int i;
	for ( i=0; i < size; i++ ) {
		SETFLOAT(list_floats+i,(t_float)to_ascii_code_text[i]);
	}
	if ( x->eof_is_set ) { // Append eof if set
		SETFLOAT(list_floats+size, x->eof);
		//outlet_float(x->outlet_right,size+1);
		outlet_list(x->outlet_left,&s_list,size+1,list_floats);
	} else {
		//outlet_float(x->outlet_right,size);
		outlet_list(x->outlet_left,&s_list,size,list_floats);
	}
	freebytes(list_floats, (size+1)*sizeof(*list_floats));
	
}


static void *to_ascii_code_new(t_symbol *s, int ac, t_atom *av)
{
    t_to_ascii_code *x = (t_to_ascii_code *)pd_new(to_ascii_code_class);
    
    // set eof if eof is set
    if ( ac && ((av)->a_type == A_FLOAT)) {
		x->eof = (int) atom_getfloat(av);
		x->eof_is_set = 1;
    } else {
		x->eof_is_set = 0;
	}
    
    x->outlet_left = outlet_new(&x->x_ob, &s_list);
    //x->outlet_right = outlet_new(&x->x_ob, &s_float);
    
    return (x);
}

void to_ascii_code_setup(void)
{
	// create binbuf (no need to ever free)
	to_ascii_code_binbuf = binbuf_new();
	
	// create text (no need to ever free)
	to_ascii_code_text = getbytes(MAXPDSTRING*sizeof(*to_ascii_code_text));	
	
    to_ascii_code_class = class_new(gensym("to_ascii_code"),
			    (t_newmethod)to_ascii_code_new, 0,
			    sizeof(t_to_ascii_code), 0, A_GIMME, 0);
    
    class_addbang(to_ascii_code_class, to_ascii_code_bang);
    class_addanything(to_ascii_code_class, to_ascii_code_anything);
    
    
    }
