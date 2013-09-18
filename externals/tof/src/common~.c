//#include "commonget~.h"
#include "m_pd.h"


typedef struct _common {
	t_symbol*			name;
	t_sample 			a[64]; 
	t_sample 			b[64];
	t_sample*		w;
	t_sample* 		r; 
	//int				writers;
	//int				written;
	int 			users;
	struct _common* 	next; 
    struct _common* 	previous; 
	t_clock*		clock;
	int 			armed;
    
} t_common;


static t_common* common;


// This should be triggered by the clock
static void common_swap(t_common* pg) {
	//post("clock");
	
	t_sample* temp = pg->r;
	pg->r = pg->w;
	pg->w = temp;
	
	int i;
	t_sample* samples = pg->w;
	for (i=0;i<64;i++) {
		*samples++ = 0;
		
	}
	
	pg->armed = 0;
	
	//if (pg == NULL) post("ouc");
	//pg->written = 0;
	
}

static t_common* common_register(t_symbol* name) {
  
  t_common* new_common;
  
  //is_writer = is_writer ? 1 : 0;

  t_common* pg = common;
  
  if ( pg != NULL) {
		
	// Search for previous common
	while( pg ) {
			if ( pg->name == name) {
				//#ifdef PARAMDEBUG
				 // post("Found put/get with same name");
				//#endif
				//pg->writers = pg->writers + is_writer;
				pg->users = pg->users + 1;
				return pg;
			}
			if ( pg->next == NULL ) break;
			pg = pg->next; 
		}
	}
	
	
	//post("Appending new put/get");
	// Append new common
    new_common = getbytes(sizeof(*new_common));
	new_common->name = name;
	//new_common->writers = is_writer;
	new_common->users = 1; 
    new_common->armed = 0;
	
	new_common->clock = clock_new(new_common, (t_method)common_swap);
	
	new_common->r = new_common->a;
	new_common->w = new_common->b;
	
	new_common->previous = pg;
	if ( pg) {
		pg->next = new_common;
	} else {
		common = new_common;
	}
	
	return new_common;
	
}


static void common_unregister(t_common* pg) {
	
	//post("Trying to remove %s",pg->name->s_name);
	
	//if ( is_writer) pg->writers = pg->writers - 1;
	pg->users = pg->users - 1;
	if ( pg->users <= 0) {
		//post("Removing last put/get of this name");
		if (pg->previous) {
				pg->previous->next = pg->next;
				if (pg->next) pg->next->previous = pg->previous;
			} else {
				common = pg->next;
				if ( pg->next != NULL) pg->next->previous = NULL;
			}
		clock_free(pg->clock);
		freebytes(pg, sizeof *(pg) );
	}
}


static void common_arm(t_common* pg) {
	
	if (!pg->armed) {
		pg->armed = 1;
		clock_delay(pg->clock, 0);
	}
	
}

///////////////
// The class //
///////////////

static t_class *common_tilde_class;

typedef struct _common_tilde {
  t_object  		x_obj;
  //t_sample 			f_common;
  t_common*	pg; 
  t_sample 			f;
} t_common_tilde;

static t_int* common_tilde_perform(t_int *w)
{
	
  t_common_tilde *x = (t_common_tilde *)(w[1]);
  
   t_sample  *in =    (t_sample *)(w[2]);
   t_sample  *out=    (t_sample *)(w[3]);
   int          n =           (int)(w[4]);
   int          m;
   t_sample *samples;
  
  
  if ( x->pg) {
      
      // Do adding
      //if (x->pg->users > x->pg->writers) {
          samples = x->pg->w;
          m = n;
          while (m--) {
              *samples = *samples + *in;
              samples++; in++;
            }
                
        //}
       
	 
      // Do reading
	   samples = x->pg->r;
		m = n;
		while (m--) {
		  *out++ = *samples++;
		}
	
         // Arm for swaping
	   common_arm( x->pg);
		
	} else {
        
        while (n--) {
		  *out++ = 0;
		}
        
    }

  return (w+5);
}


static void common_tilde_set(t_common_tilde *x, t_symbol* s) {
	
	if (gensym("") != s ) {
		if ( x->pg ) {
			if ( x->pg->name != s) {
				 common_unregister(x->pg);
				 x->pg = common_register(s);
			}
		} else {
			x->pg = common_register(s);
		}
	}
	
}


static  void common_tilde_dsp(t_common_tilde *x, t_signal **sp)
{
	
	if ( (int) sp[0]->s_n == 64 ) {
		dsp_add(common_tilde_perform, 4, x,sp[0]->s_vec,sp[1]->s_vec, sp[0]->s_n);
		
	  } else {
		  error("common~ only works with a block size of 64");
	  }
	  
}

static void common_tilde_free( t_common_tilde *x) {

	 if (x->pg) common_unregister(x->pg);
}


static void *common_tilde_new(t_symbol* s)
{
  t_common_tilde *x = (t_common_tilde *)pd_new(common_tilde_class);

  if (gensym("") != s ) x->pg = common_register(s);
 
   outlet_new(&x->x_obj, &s_signal);
 
  return (void *)x;
}

void common_tilde_setup(void) {
  common_tilde_class = class_new(gensym("common~"),
        (t_newmethod)common_tilde_new,
        (t_method)common_tilde_free, sizeof(t_common_tilde),
        0, A_DEFSYM, 0);
  
  class_addmethod(common_tilde_class,
        (t_method)common_tilde_dsp, gensym("dsp"), 0);
		
  class_addmethod(common_tilde_class,
        (t_method)common_tilde_set, gensym("set"), A_SYMBOL, 0);
		
  CLASS_MAINSIGNALIN(common_tilde_class, t_common_tilde, f);
}
