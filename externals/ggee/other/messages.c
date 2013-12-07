/* (C) Guenter Geiger <geiger@epy.co.at> */


#include <m_pd.h>
#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* ------------------------ messages ----------------------------- */

static t_class *messages_class;


typedef struct _messages
{
     t_object x_obj;
} t_messages;


void messages_bang(t_messages *x)
{
     post("bang");
}

static void *messages_new()
{
    t_messages *x = (t_messages *)pd_new(messages_class);
    outlet_new(&x->x_obj, &s_float);
    return (x);
}

void messages_setup(void)
{
    messages_class = class_new(gensym("messages"), (t_newmethod)messages_new, 0,
				sizeof(t_messages), 0,0);
    class_addbang(messages_class,messages_bang);
}


