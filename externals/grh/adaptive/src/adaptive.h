/******************************************************
 *
 * Adaptive Systems for PD
 *
 * copyleft (c) Gerda Strobl, Georg Holzmann
 * 2005
 *
 * for complaints, suggestions: grh@mur.at
 *
 ******************************************************
 *
 * license: GNU General Public License v.2
 *
 ******************************************************/

#ifndef __ADAPTIVE_H__
#define __ADAPTIVE_H__

#include "m_pd.h"
#include <stdio.h>



/* ---------------------- helpers ----------------------- */

// save all data to file
void adaptation_write(const char *filename, t_int N, t_float mu, t_float *c);

// read data from file
void adaptation_read(const char *filename, t_int *N, t_float *mu, 
                     t_float *c, t_float *buf);


#endif  //__ADAPTIVE_H__
