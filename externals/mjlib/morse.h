


typedef struct _msglet
{
	char* msg;
	int idx;
	int length;
    struct _msglet *next;
} morse_msglet;

typedef struct _morse
{
    t_object x_obj;
    	
	 morse_msglet *x_msg;
	 morse_msglet *x_curmsg;
	int x_spaceticks;
	t_outlet *x_dot;
	t_outlet *x_dash;
	t_outlet *x_end;
} t_morse;


static void morse_add_msg_part(  t_morse *x , char *msgpart );
static char *morse_lookup( char c );
static void morse_freemsg( morse_msglet* msg);
static void morse_free(t_morse *x);
static void *morse_new(t_symbol *s, int argc, t_atom *argv);
static void morse_message( t_morse *x, t_symbol *s, int ac, t_atom *av );
static void morse_do_beat( t_morse* x );



