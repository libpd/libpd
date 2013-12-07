
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

/* this object implements a dpd command 'factory and recycling center' */

/* a small note on commands & dpd

   dpd uses a different synchronization model than pdp.

   in pdp, objects only pass once the process method in the process thread
   has finished.

   in dpd a context packet propagates a context trough an tree of objects, 
   depth first, following the pd messages. it is possible to send a list of
   contexts trough a tree before the actual processing starts. therefore,
   each time a context passes trough an object, a new command object (or memento)
   needs to be created that saves the state of the rendering context. the command
   factory class can be used to create these commands.

   the dpd base class queues a command object and calls the registered method
   on the object instead of the dpd object. so a command object is in fact
   a delegate of the dpd object in question.

*/


#ifndef PDP_DPD_COMMAND
#define PDP_DPD_COMMAND

#include "pdp_types.h"


/* COMMAND BASE CLASS */
typedef struct _pdp_dpd_command
{
    struct _pdp_dpd_command *next;
    u32 used;

} t_pdp_dpd_command;


/* COMMAND LIST (COMMAND FACTORY) CLASS */
typedef struct _pdp_dpd_commandfactory
{
    u32 nb_commands;
    u32 command_size;
    t_pdp_dpd_command *command;
} t_pdp_dpd_commandfactory;


/* COMMAND LIST METHODS */
void pdp_dpd_commandfactory_init(t_pdp_dpd_commandfactory *x, u32 size);
void pdp_dpd_commandfactory_free(t_pdp_dpd_commandfactory *x);
t_pdp_dpd_command *pdp_dpd_commandfactory_get_new_command(t_pdp_dpd_commandfactory *x);


/* COMMAND METHODS */
void pdp_dpd_command_suicide(void *);


#endif
