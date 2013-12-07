
/*
 *   Pure Data Packet internal header file.
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


/* this file contains prototypes for "private" pdp methods.
   DON'T CALL THESE FROM OUTSIDE OF PDP! unless you really
   know what you are doing.
 */



#ifndef PDP_INTERNALS_H
#define PDP_INTERNALS_H

#ifdef __cplusplus
extern "C" 
{
#endif

/* INTERNAL SYSTEM METHODS */

/* set/unset main pdp thread usage */
void pdp_queue_use_thread(int t);


/* INTERNAL PACKET METHODS */

/* create a new packet, reuse if possible.
   ONLY USE THIS IN A TYPE SPECIFIC CONSTRUCTOR! */
int pdp_packet_new(unsigned int datatype, unsigned int datasize); 





#ifdef __cplusplus
}
#endif


#endif 
