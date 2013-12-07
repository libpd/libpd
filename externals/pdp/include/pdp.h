/*
 *   Pure Data Packet header file.
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



#ifndef PDP_H
#define PDP_H


/* header and subheader size in bytes */

#include <string.h>
#include <stdlib.h>

#include "pdp_pd.h"

/* config stuff */
#include "pdp_config.h"

/* debug stuff */
#include "pdp_debug.h"

/* some typedefs */
#include "pdp_types.h"

/* pdp's symbol handling */
#include "pdp_symbol.h"

/* the list object */
#include "pdp_list.h"

/* memory management */
#include "pdp_mem.h"

/* console messages */
#include "pdp_post.h"


/* PDP_IMAGE COMPONENTS */

/* header and methods for the built in image packet type */
#include "pdp_image.h"

/* low level image processing and high level dispatching routines */
#include "pdp_imageproc.h"

/* low level image conversion routines */
#include "pdp_llconv.h"

/* low level image resampling routines */
#include "pdp_resample.h"



/* PDP_BITMAP COMPONENTS */

/* header and methods for the built in bitmap packet type */
#include "pdp_bitmap.h"



/* PDP_MATRIX COMPONENTS */
#include "pdp_matrix.h"




/* PDP SYSTEM COMPONENTS */

/* packet pool stuff */
#include "pdp_packet.h"

/* processing queue object */
#include "pdp_queue.h"

/* several communication helper methods (pd specific) */
#include "pdp_comm.h"

/* type handling subsystem */
#include "pdp_type.h"

/* dpd command stuff */
#include "pdp_dpd_command.h"


/* BACKWARDS COMPAT STUFF */
#include "pdp_compat.h"




#endif 

/*

   PDP CORE API OVERVIEW

   pdp_packet_* : packet methods, first argument is packet id

      new:                construct a raw packet (depreciated)
      new_*:              construct packet of specific type/subtype/...
      mark_unused:        release
      mark_passing:       conditional release (release on first copy ro/rw)
      copy_ro:            readonly (shared) copy
      copy_rw:            private copy
      clone_rw:           private copy (copies only meta data, not the content)
      header:             get the raw header (t_pdp *)
      data:               get the raw data (void *)
      pass_if_valid:      send a packet to pd outlet, if it is valid
      replace_if_valid    delete packet and replace with new one, if new is valid
      copy_ro_or_drop:    copy readonly, or don't copy if dest slot is full + send drop notify
      copy_rw_or_drop:    same, but private copy
      get_description:    retrieve type info
      convert_ro:         same as copy_ro, but with an automatic conversion matching a type template
      convert_rw:         same as convert_ro, but producing a private copy

   pdp_pool_* : packet pool methods

      collect_garbage:    manually free all unused resources in packet pool

   pdp_queue_* : processing queue methods

      add:                add a process method + callback
      finish:             wait until a specific task is done
      wait:               wait until processing queue is done

   pdp_control_* : central pdp control hub methods

      notify_drop:        notify that a packet has been dropped

   pdp_type_* : packet type mediator methods

      description_match:   check if two type templates match
      register_conversion: register a type conversion program



   NOTE: it is advised to derive your module from the pdp base class defined in pdp_base.h
         instead of communicating directly with the pdp core

*/
