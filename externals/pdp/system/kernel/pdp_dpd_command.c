
/*
 *   Pure Data Packet header file. DPD command class
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

/* this object implements a dpd command queue and command object */

#include "pdp_dpd_command.h"
#include "pdp_mem.h"

void pdp_dpd_commandfactory_init(t_pdp_dpd_commandfactory *x, u32 size)
{
    x->nb_commands = 0;
    x->command_size = size;
    x->command = 0;
}

void _pdp_dpd_commandfactory_free(t_pdp_dpd_command *x)
{
    if (x) _pdp_dpd_commandfactory_free(x->next);
    pdp_dealloc(x);
}

void pdp_dpd_commandfactory_free(t_pdp_dpd_commandfactory *x)
{
    _pdp_dpd_commandfactory_free(x->command);
    x->command = 0;
    x->nb_commands = 0;
}


/* factory method */
t_pdp_dpd_command *pdp_dpd_commandfactory_get_new_command(t_pdp_dpd_commandfactory *x)
{

    t_pdp_dpd_command *c = x->command;
    t_pdp_dpd_command *oldhead = c;

    /* check if we can reuse */
    while (c){
	if (!c->used){
	    c->used = 1;
	    //post("reusing command %x", c, c->used);
	    return c;
	}
	//post("command %x is used %d", c, c->used);
	c = c->next;
    }

    /* create a new command */
    x->command = (t_pdp_dpd_command *)pdp_alloc(x->command_size);
    x->command->next = oldhead;
    x->command->used = 1;
    x->nb_commands++;
    //post("created command %x, nbcommands: %d", x->command, x->nb_commands);
    return x->command;

}


/* (self)destructor */
void pdp_dpd_command_suicide(void *x)
{
    t_pdp_dpd_command *c = (t_pdp_dpd_command *)x;
    c->used = 0;
    //post("command %x committed suicide %d", c, c->used);
}




