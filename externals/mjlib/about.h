
typedef struct _about
{
    t_object x_obj;
    t_float x_err;
} t_about;

static void *about_new( t_float t  );
static void about_set_err( t_about *x, t_float f );
static void about_float( t_about *x, t_float f );
static void about_free(t_about *x);

