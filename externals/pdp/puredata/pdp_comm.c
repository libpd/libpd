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

/* this file contains misc communication (packet) methods for pd */


#include <stdio.h>
#include "pdp_pd.h"
#include "pdp_internals.h"
#include "pdp_packet.h"
#include "pdp_comm.h"
#include "pdp_type.h"
#include "pdp_control.h"
#include "pdp_mem.h"
#include "pdp_queue.h" // for notify drop: fix this (should be from pdp_control.h)
#include "pdp_debug.h"

/* all symbols are C style */
#ifdef __cplusplus
extern "C"
{
#endif


/* interface to pd system:
   pdp/dpd communication protocol in pd
   pd <-> pdp atom and list conversion */
   
    /* NOTE: when using the outlet_pdp methods, the packet
       is no longer read/write, but can be readonly */


    /* NOTE: since 0.13 the passing packet is no more.
       in order to limit copying. processors should always register ro,
       and replace with writable when packet needs to be written in the process method */


/* send a packet to an outlet */
void outlet_pdp_register(t_outlet *out, int packetid)
{
    t_atom atom[2];

    SETFLOAT(atom+1, (float)packetid);

    /* during the following phase, 
       objects can register a ro copy */

    SETSYMBOL(atom+0, S_REGISTER_RO);         
    outlet_anything(out, S_PDP, 2, atom);

    /* DEPRECIATED: objects can register a rw copy
       but this will always copy the packet. it is better
       to perform a pdp_packet_replace_with_writable operation during the process step */

    SETSYMBOL(atom+0, S_REGISTER_RW);         
    outlet_anything(out, S_PDP, 2, atom);    

}
/* send a packet to an outlet */
void outlet_pdp_process(t_outlet *out)
{
    t_atom atom[2];

    /* set a dummy invalid packet.
       this is for uniform pdp messages in pd, for ease of routing. */
    SETFLOAT(atom+1, (float)-1);

    /* during the process phase, objects can perform pdp_packet_replace_with_writable
       and process the packet data */
    SETSYMBOL(atom+0, S_PROCESS);
    outlet_anything(out, S_PDP, 2, atom);

}

/* for compat */
void outlet_pdp(t_outlet *out, int packetid)
{
    outlet_pdp_register(out, packetid);
    outlet_pdp_process(out);
}

/* send an accumulation packet to an outlet */
void outlet_dpd(t_outlet *out, int packetid)
{
    t_atom atom[2];

    SETFLOAT(atom+1, (float)packetid);

    SETSYMBOL(atom+0, S_INSPECT);
    outlet_anything(out, S_DPD, 2, atom);

    SETSYMBOL(atom+0, S_ACCUMULATE);
    outlet_anything(out, S_DPD, 2, atom);

}

/* unregister a packet and send it to an outlet */
void

pdp_packet_pass_if_valid(t_outlet *outlet, int *packet_ptr)
{


    t_pdp *header = pdp_packet_header(*packet_ptr);
    if (header){


	/* send register phase */
	outlet_pdp_register(outlet, *packet_ptr);


	/* unregister */
	pdp_packet_mark_unused(*packet_ptr);
	*packet_ptr = -1;

	/* send process phase */
	outlet_pdp_process(outlet);


    }

}

void
pdp_packet_replace_if_valid(int *dpacket, int *spacket)
{
    if (-1 != *spacket){
	pdp_packet_mark_unused(*dpacket);
	*dpacket = *spacket;
	*spacket = -1;
    }
    
}


int
pdp_packet_copy_ro_or_drop(int *dpacket, int spacket)
{
    int drop = 0;
    if (*dpacket == -1) *dpacket = pdp_packet_copy_ro(spacket);
    else {
	/* send a notification there is a dropped packet */
	pdp_control_notify_drop(spacket);
	drop = 1;
    }
    return drop;
}


int
pdp_packet_copy_rw_or_drop(int *dpacket, int spacket)
{
    int drop = 0;
    if (*dpacket == -1) *dpacket = pdp_packet_copy_rw(spacket);
    else {
	/* send a notification there is a dropped packet */
	pdp_control_notify_drop(spacket);
	drop = 1;
    }
    return drop;
}

int
pdp_packet_convert_ro_or_drop(int *dpacket, int spacket, t_pdp_symbol *template)
{
    int drop = 0;

    if (!template) return pdp_packet_copy_ro_or_drop(dpacket, spacket);

    if (*dpacket == -1) *dpacket = pdp_packet_convert_ro(spacket, template);
    else {
	/* send a notification there is a dropped packet */
	pdp_control_notify_drop(spacket);
	drop = 1;
    }
    return drop;
}


int
pdp_packet_convert_rw_or_drop(int *dpacket, int spacket, t_pdp_symbol *template)
{
    int drop = 0;

    if (!template) return pdp_packet_copy_rw_or_drop(dpacket, spacket);

    if (*dpacket == -1) *dpacket = pdp_packet_convert_rw(spacket, template);
    else {
	/* send a notification there is a dropped packet */
	pdp_control_notify_drop(spacket);
	drop = 1;
    }
    return drop;
}


/* send a pdp list to a pd outlet. packets are not copied but passed! */
void outlet_pdp_atom(t_outlet *out, t_pdp_atom *a)
{
    int packet = -1;
    if (!a) return;
    switch(a->t){
    case a_float:
	outlet_float(out, a->w.w_float);
	return;
    case a_int:
	outlet_float(out, (float)a->w.w_int);
	return;
    case a_symbol:
	outlet_symbol(out, gensym(a->w.w_symbol->s_name));
	return;
    case a_list:
	outlet_pdp_list(out, a->w.w_list);
	return;
    case a_packet:
	pdp_packet_pass_if_valid(out, &a->w.w_packet);
	return;
    default:
	return;
    }
}

void outlet_pdp_list(t_outlet *out, struct _pdp_list *l)
{
    int elements;
    t_atom *atomlist;
    t_pdp_atom *pdp_a;
    t_atom *pd_a;
    t_symbol *pd_selector;
    
    if (!l) return;
    switch(l->elements){
    case 0: /* bang */
	outlet_bang(out);
	return;
    case 1: /* atom */
	outlet_pdp_atom(out, l->first);
	return;
    default: /* proper list*/
	elements = l->elements;

	/* allocate list */
	atomlist = pdp_alloc(sizeof (t_atom) * l->elements);
	pd_a = atomlist;
	pdp_a = l->first;

	/* setup selector */
	if (pdp_a->t != a_symbol){
	    pd_selector = gensym("list");
	}
	else {
	    pd_selector = gensym(pdp_a->w.w_symbol->s_name);
	    elements--;
	    pdp_a = pdp_a->next;
	}

	/* setup atoms */
	while (pdp_a){
	    switch(pdp_a->t){
	    case a_float:
		SETFLOAT(pd_a, pdp_a->w.w_float);
		break;
	    case a_int:
		SETFLOAT(pd_a, (float)pdp_a->w.w_int);
		break;
	    case a_symbol:
		SETSYMBOL(pd_a, gensym(pdp_a->w.w_symbol->s_name));
		break;
	    default:
		SETSYMBOL(pd_a, gensym("invalid"));
		break;
	    }
			  
	    pdp_a = pdp_a->next;
	    pd_a++;
	}

	/* send out */
	outlet_anything(out, pd_selector, elements, atomlist);
	    


	/* clean up */
	pdp_dealloc(atomlist);
	
    }	
	
		
}


void pd_atom_to_pdp_atom(t_atom *pdatom, t_pdp_atom *pdpatom)
{
    switch (pdatom->a_type){
    case A_FLOAT:
	pdpatom->t = a_float;
	pdpatom->w.w_float = pdatom->a_w.w_float;
	break;
    case A_SYMBOL:
	pdpatom->t = a_symbol;
	pdpatom->w.w_symbol = pdp_gensym(pdatom->a_w.w_symbol->s_name);
	break;
    default:
	pdpatom->t = a_undef;
	break;
    }
}

#if PDP_SYMBOL_HACK

/* some "accelerated" pd symbols */
t_symbol s_pdp         = {"pdp", 0, 0};
t_symbol s_register_ro = {"register_ro", 0, 0};
t_symbol s_register_rw = {"register_rw", 0, 0};
t_symbol s_process     = {"process", 0, 0};
t_symbol s_dpd         = {"dpd", 0, 0};
t_symbol s_inspect     = {"inspect", 0, 0};
t_symbol s_accumulate  = {"accumulate", 0, 0};
t_symbol s_chanmask    = {"chanmask", 0, 0};



// internal pd method
t_symbol *dogensym(char *s, t_symbol *oldsym);
static void _addsym(t_symbol *s)
{

    /* don't kill me for this one..
       if the symbol is already defined and used, .. well, that's a problem
       but right now it seems a reasonable hack */

    t_symbol *sret = dogensym(s->s_name, s);
    if (s != sret){
	post("PDP INIT ERROR: pd symbol clash adding symbol %s: new=%08x old=%08x", s->s_name, s, sret);
	post("try loading pdp before other libraries");
    }
}


void
pdp_pdsym_setup(void)
{

    _addsym(&s_pdp);
    _addsym(&s_register_ro);
    _addsym(&s_register_rw);
    _addsym(&s_process);
    _addsym(&s_dpd);
    _addsym(&s_inspect);
    _addsym(&s_accumulate);
    _addsym(&s_chanmask);

}

#else

void pdp_pdsym_setup(void){
}

#endif


#ifdef __cplusplus
}
#endif
