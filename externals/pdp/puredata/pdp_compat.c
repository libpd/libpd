/*
 *   Pure Data Packet system implementation. Compatibility routines.
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

/* this file contains misc communication methods */

#include <stdio.h>

#include "pdp_pd.h"
#include "pdp_comm.h"
#include "pdp_internals.h"

/* all symbols are C style */
#ifdef __cplusplus
extern "C"
{
#endif



void
pdp_pass_if_valid(t_outlet *outlet, int *packet)
{
    pdp_packet_pass_if_valid(outlet, packet);
}

void
pdp_replace_if_valid(int *dpacket, int *spacket)
{
    pdp_packet_replace_if_valid(dpacket, spacket);
    
}






#ifdef __cplusplus
}
#endif
