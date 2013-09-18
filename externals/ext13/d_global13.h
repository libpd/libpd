t_class *sigsend13_class;
t_class *sigthrow13_class;
t_class *sigreceive13_class;
t_class *sigcatch13_class;

typedef struct _sigsend13
{
    t_object x_obj;
    t_symbol *x_sym;
    int x_n;
    float *x_vec;
} t_sigsend13;


typedef struct _sigreceive13
{
    t_object x_obj;
    t_symbol *x_sym;
    t_float *x_wherefrom;
    int x_n;
} t_sigreceive13;


typedef struct _sigcatch13
{
    t_object x_obj;
    t_symbol *x_sym;
    int x_n;
    float *x_vec;
} t_sigcatch13;


typedef struct _sigthrow13
{
    t_object x_obj;
    t_symbol *x_sym;
    t_float *x_whereto;
    int x_n;
} t_sigthrow13;

