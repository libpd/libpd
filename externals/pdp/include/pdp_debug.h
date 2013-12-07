#ifndef __PDP_DEBUG_H_
#define __PDP_DEBUG_H_

#include "pdp_config.h" // needed for PDP_DEBUG define

void pdp_assert_hook (char *condition, char *file, int line);



#if PDP_DEBUG
#define PDP_ASSERT(x) if (!(x)) {pdp_assert_hook(#x, __FILE__, __LINE__);}
#else
#define PDP_ASSERT(x)
#endif

#endif //__PDP_DEBUG_H_
