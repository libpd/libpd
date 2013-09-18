#include "pdp_opengl.h"
#include "pdp_dpd_base.h"


typedef struct _pdp_3dp_base
{
    t_pdp_dpd_base b_base;
    
} t_pdp_3dp_base;

/* destructor */
void pdp_3dp_base_free(void *x);

/* init method */
void pdp_3dp_base_init(void *x);

/* class setup method */
void pdp_3dp_base_setup(t_class *class);


/* base class methods */
#define pdp_3dp_base_get_context_packet          pdp_dpd_base_get_context_packet
#define pdp_3dp_base_set_context_packet          pdp_dpd_base_set_context_packet
#define pdp_3dp_base_add_outlet                  pdp_dpd_base_add_outlet
#define pdp_3dp_base_add_cleanup                 pdp_dpd_base_add_cleanup
#define pdp_3dp_base_add_inspect                 pdp_dpd_base_add_inspect
#define pdp_3dp_base_disable_active_inlet        pdp_dpd_base_disable_active_inlet
#define pdp_3dp_base_move_context_packet         pdp_dpd_base_move_context_packet
#define pdp_3dp_base_bang                        pdp_dpd_base_bang
#define pdp_3dp_base_get_queue                   pdp_dpd_base_get_queue
#define pdp_3dp_base_enable_outlet               pdp_dpd_base_enable_outlet
#define pdp_3dp_base_register_complete_notify    pdp_dpd_base_register_complete_notify
#define pdp_3dp_base_register_get_command_object pdp_dpd_base_register_get_command_object
#define pdp_3dp_base_queue_wait                  pdp_dpd_base_queue_wait
#define pdp_3dp_base_queue_command               pdp_dpd_base_queue_command

