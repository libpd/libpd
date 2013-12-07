
typedef struct _prob
{
    t_object x_obj;
    t_clock *x_clock;
	t_float x_time;
	t_float x_probability;
	t_float x_running;
} t_prob;

static void prob_tick(t_prob *x);
static void prob_start(t_prob *x);
static void prob_stop(t_prob *x);
static void prob_free(t_prob *x);
static void *prob_new( t_float , t_float);
static void prob_set_time( t_prob *x, t_float f );
static void prob_set_probability( t_prob *x, t_float f );
static int prob_event(t_prob *x);
