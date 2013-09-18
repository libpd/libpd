/*=============================================================================*\
 * File: mooPdUtils.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: some generic utilities for pd externals
 *=============================================================================*/

#ifndef _MOO_PD_UTILS_H
#define _MOO_PD_UTILS_H

/*-- MOO_UNUSED : macro for unused attributes; to avoid compiler warnings */
#ifdef __GNUC__
# define MOO_UNUSED __attribute__((unused))
#else
# define MOO_UNUSED
#endif

/*-- PDEXT_UNUSED : alias for MOO_UNUSED --*/
#define PDEXT_UNUSED MOO_UNUSED

#endif /* _MOO_PD_UTILS_H */
