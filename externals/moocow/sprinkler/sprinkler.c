/* File: sprinkler.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Contributors:
 *    Krzysztof Czaja pointed out the MAX-incompatibility of the name 'forward'
 *    Miller Puckette suggested the name 'sprinkler'
 *    Erasmus Zipfel diagnosed a bug in sprinkler_list()
 *
 * Description: dynamic message-forwarding object
 *
 *   + code adapted from 'send_class' in $PD_ROOT/src/x_connective.c
 *   + formerly 'forward.c'
 *
 *
 * Copyright (c) 2002-2009 Bryan Jurish.
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See file LICENSE for further informations on licensing terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *=============================================================================*/

#include <m_pd.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "mooPdUtils.h"

/*--------------------------------------------------------------------
 * DEBUG
 *--------------------------------------------------------------------*/
//#define SPRINKLER_DEBUG 1


/*=====================================================================
 * Constants
 *=====================================================================*/
#ifdef SPRINKLER_DEBUG
// error-message buffer
#define EBUFSIZE 256
static char sprinkler_errbuf[EBUFSIZE];
#endif

/*=====================================================================
 * Structures and Types
 *=====================================================================*/

static char *sprinkler_banner =
"\n"
"sprinkler: dynamic message dissemination v" PACKAGE_VERSION " by Bryan Jurish\n"
"sprinkler: compiled by " PACKAGE_BUILD_USER " on " PACKAGE_BUILD_DATE;

static t_class *sprinkler_class;

typedef struct _sprinkler
{
  t_object x_obj;
  t_outlet *x_thru; //-- pass-through outlet
} t_sprinkler;


/*--------------------------------------------------------------------
 * the guts:
 *  + send (the tail of) a list or message to the control-bus
 *    named by its initial element
 *  + [DEPRECATED] : HACK for single-element arglists *ONLY*
 *    - sprinkler float- and pointer-initial arglists with 'pd_sprinklermess',
 *      everything else with 'pd_forwardmess'
 *--------------------------------------------------------------------*/
static void sprinkler_anything(t_sprinkler *x, t_symbol *dst, int argc, t_atom *argv)
{

#ifdef SPRINKLER_DEBUG
  atom_string(argv, sprinkler_errbuf, EBUFSIZE);
  post("sprinkler_debug : sprinkler_anything : dst=%s, argc=%d, arg1=%s",
       dst->s_name, argc, argc ? sprinkler_errbuf : "NULL");
#endif

  if (dst->s_thing) {

#if !defined(ALL_FORWARDMESS)

    /*-----------------------------------------------------------------------
     * HACK (obsolete):
     * + single-element arglists *ONLY*
     * + sprinkler float- and pointer-initial arglists with 'pd_sprinklermess',
     *   everything else with 'pd_forwardmess'
     *------------------------------------------------------------------------
     */
    if (argc == 1) {
      switch (argv->a_type) {
      case A_FLOAT:
	pd_typedmess(dst->s_thing,&s_float,argc,argv);
	return;
      case A_SYMBOL:
	//-- special handling for 'bang'
	if (argv->a_w.w_symbol == &s_bang) {
	  pd_typedmess(dst->s_thing,&s_bang,0,0);
	} else {
	  pd_typedmess(dst->s_thing,&s_symbol,argc,argv);
	}
	return;
      case A_POINTER:
	pd_typedmess(dst->s_thing,&s_pointer,argc,argv);
	return;

      // everything else (stop 'gcc -Wall' from complaining)
      case A_NULL:
      case A_SEMI:
      case A_COMMA:
      case A_DEFFLOAT:
      case A_DOLLAR:
      case A_DOLLSYM:
      case A_GIMME:
      case A_CANT:
      default:
	break;
	// just fall though
      }
    }

#endif /* !defined(ALL_FORWARDMESS) */

    // default -- sprinkler anything else with 'pd_forwardmess'
    pd_forwardmess(dst->s_thing,argc,argv);
    return;
  }

  //post("sprinkler: no destination for `%s'", dst ? dst->s_name : "(null)");
  //-- pass through
  outlet_anything(x->x_thru, dst, argc, argv);
}

static void sprinkler_list(t_sprinkler *x, MOO_UNUSED t_symbol *s, int argc, t_atom *argv)
{
#ifdef SPRINKLER_DEBUG
  post("sprinkler_debug : sprinkler_list : argc=%d", argc);
#endif
  sprinkler_anything(x,atom_getsymbol(argv), argc-1, argv+1);
}


/*--------------------------------------------------------------------
 * newmethod, freemethod
 */
void *sprinkler_new(MOO_UNUSED t_symbol *s)
{
    t_sprinkler *x = (t_sprinkler *)pd_new(sprinkler_class);
    x->x_thru = outlet_new(&x->x_obj, &s_anything);
    return (x);
}

void sprinkler_free(t_sprinkler *x) {
  outlet_free(x->x_thru);
}

/*--------------------------------------------------------------------
 * setup
 *--------------------------------------------------------------------*/
void sprinkler_setup(void)
{
  post(sprinkler_banner);

  sprinkler_class = class_new(gensym("sprinkler"),
			      (t_newmethod)sprinkler_new,
			      (t_method)sprinkler_free,
			      sizeof(t_sprinkler),
			      0, 0);

#ifdef NON_MAX_FORWARD
  //-- add aliases [forward] and [fw]
  post("sprinkler: non-MAX [forward] alias enabled");
  class_addcreator((t_newmethod)sprinkler_new, gensym("forward"), A_DEFSYM, 0);
  class_addcreator((t_newmethod)sprinkler_new, gensym("fw"), A_DEFSYM, 0);
#endif

#ifdef ALL_FORWARDMESS
  //-- report new semantics
  post("sprinkler: will use pd_forwardmess() for all messages");
#endif

#ifdef SPRINKLER_DEBUG
  post("sprinkler: debugging enabled");
#endif
  
  //-- methods
  class_addlist(sprinkler_class, sprinkler_list);    
  class_addanything(sprinkler_class, sprinkler_anything);
  
  // help symbol
  class_sethelpsymbol(sprinkler_class, gensym("sprinkler-help.pd"));
}
