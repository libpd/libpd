#include "pdp_opengl.h"
#include "pdp_3dp_base.h"

#define THIS(b) t_pdp_3pd_base *b = (t_pdp_3pd_base *)x

/* destructor */
void pdp_3dp_base_free(void *x)
{
    // free super
    pdp_dpd_base_free(x);
}

/* init method */
void pdp_3dp_base_init(void *x)
{
    // init super
    pdp_dpd_base_init(x);

    // set processing queue to pdp_opengl system queue
    pdp_dpd_base_set_queue(x, pdp_opengl_get_queue());

}

/* class setup method */
void pdp_3dp_base_setup(t_class *class)
{
    // setup super
    pdp_dpd_base_setup(class);
}

