
typedef struct _n2m
{
    t_object x_obj;
    t_float x_err;
} t_n2m;

static void *n2m_new( t_float t  );
static void n2m_set_err( t_n2m *x, t_float f );
static void n2m_float( t_n2m *x, t_float f );
static void n2m_free(t_n2m *x);
static void splitsym( char* buf , char* note, int* octave );
static int midilookup( char* note , int octave );


