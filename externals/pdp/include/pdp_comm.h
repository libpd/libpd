/*
 *   Pure Data Packet system implementation.
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

#ifndef PDP_COMM_H
#define PDP_COMM_H

#include "pdp_symbol.h"
#include "pdp_list.h"

/* all symbols are C style */
#ifdef __cplusplus
extern "C"
{
#endif



/* pdp's pd symbols for communication 
   don't use these directly!!
   use the macros instead, in case
   this is proven to be too much of a hack..

   it's too much of a hack. getting rid of them.

 */


#define PDP_SYMBOL_HACK 0


#if PDP_SYMBOL_HACK
extern t_symbol s_pdp;
extern t_symbol s_register_ro;
extern t_symbol s_register_rw;
extern t_symbol s_process;
extern t_symbol s_dpd;
extern t_symbol s_inspect;
extern t_symbol s_accumulate;
extern t_symbol s_chanmask;


#define S_PDP         &s_pdp
#define S_REGISTER_RO &s_register_ro
#define S_REGISTER_RW &s_register_rw
#define S_PROCESS     &s_process
#define S_DPD         &s_dpd
#define S_INSPECT     &s_inspect
#define S_ACCUMULATE  &s_accumulate
#define S_CHANMASK    &s_chanmask

#else

#define S_PDP         gensym("pdp")
#define S_REGISTER_RO gensym("register_ro")
#define S_REGISTER_RW gensym("register_rw")
#define S_PROCESS     gensym("process")
#define S_DPD         gensym("dpd")
#define S_INSPECT     gensym("inspect")
#define S_ACCUMULATE  gensym("accumulate")
#define S_CHANMASK    gensym("chanmask")


#endif


/* utility methods */



/* if packet is valid, mark it unused and send it to an outlet */
void pdp_packet_pass_if_valid(t_outlet *outlet, int *packet);

/* if source packet is valid, release dest packet and move src->dest */
void pdp_packet_replace_if_valid(int *dpacket, int *spacket);

/* copy_ro if dest packet if invalid, else drop source 
   (don't copy) + send drop notif to pdp system 
   returns 1 if dropped, 0 if copied */
int pdp_packet_copy_ro_or_drop(int *dpacket, int spacket);
int pdp_packet_convert_ro_or_drop(int *dpacket, int spacket, t_pdp_symbol *type_template);

/* copy_rw if dest packit is invalid, else drop source 
   (don't copy) + send drop notif to pdp system 
   returns 1 if dropped, zero if copied */
int pdp_packet_copy_rw_or_drop(int *dpacket, int spacket);
int pdp_packet_convert_rw_or_drop(int *dpacket, int spacket, t_pdp_symbol *type_template);


/* pd and pdp conversion stuff */
void pd_atom_to_pdp_atom(t_atom *pdatom, t_pdp_atom *pdpatom);


/* send pdp lists and atoms */
void outlet_pdp_atom(t_outlet *out, struct _pdp_atom *a);
void outlet_pdp_list(t_outlet *out, struct _pdp_list *l);



/* send a packet to an outlet: it is only legal to call this on a "passing packet"
   or a "read only packet".
   this means it is illegal to change a packet after you have passed it to others,
   since this would mess up all read only references to the packet.
*/

/* this seems like a nice place to hide a comment on the notion of read/write in pdp
   which packets are writable? all packets with exactly 1 user. this includes all packets 
   aquired with pdp_packet_*_rw or a constructor, and all packets that are not registered
   after being sent out by outlet_pdp.
   which packets are readable? all packets */

void outlet_pdp(t_outlet *out, int packetid);

/* send an accumulation (context) packet to an outlet. this is for usage in the dpd
   base class. */
void outlet_dpd(t_outlet *out, int packetid);


#ifdef __cplusplus
}
#endif



#endif
