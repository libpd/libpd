

typedef struct _metroplus
{
    t_object x_obj;
    t_clock *x_clock;
	t_float *x_times ;
	int x_idx;
	int x_size;
    int x_hit;
	void* x_shadow;
} t_metroplus;

static void metroplus_tick(t_metroplus *x);
static void metroplus_float(t_metroplus *x, t_float f);
static void metroplus_bang(t_metroplus *x);
static void metroplus_stop(t_metroplus *x);
static void metroplus_ft1(t_metroplus *x, t_floatarg g);
static void metroplus_free(t_metroplus *x);
static void *metroplus_new(t_symbol *s, int argc, t_atom *argv);
static float metroplus_getNextDelay( t_metroplus *x );
static void metroplus_time_seq( t_metroplus *x, t_symbol *s, int ac, t_atom *av );
static void metoplus_time_float( t_metroplus *x, t_float f );

