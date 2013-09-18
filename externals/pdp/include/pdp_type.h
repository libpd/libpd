/*
 *   Pure Data Packet system implementation. Type handling interface
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


/* COMMENTS

Since version 0.11 all packets have an (optional) high level type description.
This can be thought of as their mime type. It has several uses:

* automatic type conversion
* better reuse strategy for non pure packets
* debugging

The description is text-encoded, in the following form:

type/subtype/subsubtype/..

This is implemented using t_pdp_symbol.

Type descriptors can have wildcards. This is to give some freedom to a desired
type conversion. The following are all compatible:

*/

// image/grey/320x240
// image/*/320x240
// image/*/*

/*

From a user pov, the type conversion is centralized. A single object (pdp_convert)
can do most of the conversions.

Type conversion implementation has to be done decentralized. It is subdivided into
two steps: inter-type and intra-type conversions.

Intra-type is the full responsability of each type implementation and can be handled
in a decentralized way (at linkage the type central intra-converter is registered
at the pdp framework.

Inter-type conversion is harder to do decentralized, therefore each new type should
provide some conversions to the basic built in types. (internal image, bitmap or matrix
types.

The point of this whole business is to

* enable automatic conversion from anything to a desired type for operators that combine objects.
  i.e. pdp_add but receive incompatible objects.
* enable manual anything to anything conversion using a pdp_convert object, i.e. have a consistent
  packet conversion api for users.


The solution is type conversion programs. A program's behaviour is specified as follows:

* the program is registered with a source and destination (result) template
* it is passed a packet and a destination template
* it can assume the source packet complies to the program's registerd source template
* it should convert the packet to a packet that will comply to it's registered destination template
* if for some reason a conversion fails, an invalid packet (handle == -1) should be returned

about type templates:

* they are hierarchical, with subtypes separated by a '/' character
* they can contain a wildcard '*', meaning that a certain level in the type hierarchy is:
  - a don't care value, when the wildcard is used
    -> as a destination template in a requested conversion
    -> as a source template in a conversion program's specification
  - uspecified, when the wildcard is used
    -> as a destination template in a conversion program's specification



NOTE: 

  a wildcard can't be used in a source template for a conversion request
  this assymetry requires there be 2 kinds of template matching mechanisms:

  - source type description (without wildcards) to conversion program source template matching
  - destination type description (with wildcards) to conversion program destination template matching

  since a packet's type description cannot have wildcards, a symmetric matching (both sides have
  wildcards) can be used for matching.

*/



/*

implementation:

there are 2 lists with conversion progams:
* the global list, containing all registered programs.
* the cached list, containing all recently used registered programs, or combinations thereof

if there is no cached, perfectly matching rule, a new one will be created, and added to
the head of the conversion list.

all conversion methods should keep their hands off the source packet. it is treated as readonly.
this is to ensure a more flexible operation (i.e. be able to put the conversion at the register_ro
level)


TODO: add a breadth first search algorithm to do multiple stage conversion.

*/

#ifndef PDP_TYPE_H
#define PDP_TYPE_H

#include "pdp_symbol.h"
#include "pdp_list.h"

/* the conversion method accepts a packet (which is freed) and a destination wildcard 
   and produces a new packet, or the invalid packet if the conversion failed */
typedef int (*t_pdp_conversion_method)(int, t_pdp_symbol *);

/* a conversion program is alist of conversion methods */
typedef t_pdp_list t_pdp_conversion_program;

/* a conversion has source and dest wildcards, and a conversion program */
typedef struct _pdp_conversion
{
    t_pdp_symbol *src_pattern;                   // source type pattern
    t_pdp_symbol *dst_pattern;                   // destination type pattern 
    t_pdp_conversion_program *program;       // the conversion program for this conversion
} t_pdp_conversion;

/* all symbols are C style */
#ifdef __cplusplus
extern "C"
{
#endif

/* pdp_packet methods */
t_pdp_symbol *pdp_packet_get_description(int packet);
int pdp_packet_convert_ro(int packet, t_pdp_symbol *dest_pattern);
int pdp_packet_convert_rw(int packet, t_pdp_symbol *dest_pattern);


/* pdp_conversion_program methods */
void pdp_conversion_program_free(t_pdp_conversion_program *program);
t_pdp_conversion_program *pdp_conversion_program_new(t_pdp_conversion_method method, ...);
t_pdp_conversion_program *pdp_conversion_program_copy(t_pdp_conversion_program *program);
void pdp_conversion_program_add(t_pdp_conversion_program *program, t_pdp_conversion_program *tail);

/* pdp_type (central type object) methods */
int pdp_type_description_match(t_pdp_symbol *description, t_pdp_symbol *pattern);
void pdp_type_register_conversion (t_pdp_symbol *src_pattern, t_pdp_symbol *dst_pattern, t_pdp_conversion_program *program);
void pdp_type_register_cached_conversion (t_pdp_symbol *src_pattern, t_pdp_symbol *dst_pattern, t_pdp_conversion_program *program);

    //t_pdp_symbol *pdp_type_gendesc(char *desc); //generate a type description (with description list attached)
t_pdp_list *pdp_type_to_list(t_pdp_symbol *type);

/* pdp's (threadsafe) symbol */
t_pdp_symbol *pdp_gensym(char *s);


#ifdef __cplusplus
}
#endif

#endif
