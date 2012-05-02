#ifndef __Z_HOOKSET_H__
#define __Z_HOOKSET_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "z_libpd.h"

EXTERN void libpd_set_printhook(const t_libpd_printhook hook);

#ifdef __cplusplus
}
#endif

#endif