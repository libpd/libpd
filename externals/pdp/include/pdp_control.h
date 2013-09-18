#ifndef __PDP_CONTROL_H__
#define __PDP_CONTROL_H__

#include "pdp_pd.h"

struct _pdp_control;
typedef void (t_pdp_control_method_notify)(struct _pdp_control *x);

void pdp_control_notify_broadcast(t_pdp_control_method_notify *notify);
void pdp_control_addmethod(t_method m, t_symbol *s);

#endif
