
typedef struct _monorhythm
{
    t_object x_obj;
    t_clock *x_clock;
	t_float *x_pattern;
	int x_idx;
	int x_size;
	t_float x_time;
	t_float x_beattime;
	int t_running;
	int t_exclusive;
	t_outlet *x_bang;
	t_outlet *x_sync;
	t_outlet *x_accent;
} t_monorhythm;

static void monorhythm_tick(t_monorhythm *x);
static void monorhythm_start(t_monorhythm *x);
static void monorhythm_stop(t_monorhythm *x);
static void monorhythm_free(t_monorhythm *x);
static void *monorhythm_new(t_symbol *s, int argc, t_atom *argv);
static void monorhythm_pattern_seq( t_monorhythm *x, t_symbol *s, int ac, t_atom *av );
static void monorhythm_time_float( t_monorhythm *x1, t_float f );
static void monorhythm_calculate_beat_interval( t_monorhythm *x );
static void monorhythm_set_time( t_monorhythm *x, t_float f );
static void monorhythm_restart(t_monorhythm *x);
static void monorhythm_do_beat( t_monorhythm* x );
static void monorhythm_set_exclusive(t_monorhythm *x);
static void monorhythm_set_nonexclusive(t_monorhythm *x);



