/*
 *   Cellular Automata Extension Module for pdp - Main header file
 *   Copyright (c) by Tom Schouten <tom@zwizwa.be>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef PDP_CA_H
#define PDP_CA_H

#include "pdp.h"



/* 2D CA DATA PACKET */
typedef struct
{
    unsigned int encoding;  /* CA data format */
    unsigned int width;     /* CA width (in 1 bit cells) */
    unsigned int height;    /* CA height (in 1 bit cells) */
    unsigned int offset;    /* bit offset of upper left corner */
    unsigned int currow;    /* current row to compute for 1D CA */
    
} t_ca;

/* CA encodings */
#define PDP_CA_STANDARD     1  /* rectangular CA */

/* pdp data packet types */
#define PDP_CA              2  /* 1bit toroidial shifted scanline encoded cellular automaton */


/* all symbols are C-style */
#ifdef __cplusplus
extern "C"
{
#endif

/* constructor */
int pdp_packet_new_ca(int encoding, int width, int height);

/* some utility methods for CA */
int pdp_packet_ca_isvalid(int packet);
int pdp_type_ca2grey(int packet);
int pdp_type_grey2ca(int packet, short int threshold);

/* returns a pointer to the ca subheader given the pdp header */
t_ca *pdp_type_ca_info(t_pdp *x);

/* mmx feeder routine */
unsigned long long scaf_feeder(void *tos, void *reg, void (*ca_rule)(void), void *env);





#ifdef __cplusplus
}
#endif

#endif //PDP_CA_H
