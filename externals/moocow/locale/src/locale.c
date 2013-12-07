/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: locale.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: C99 locale support
 *
 * Copyright (c) 2009 Bryan Jurish.
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

#include <string.h>
#include <stdlib.h>
#include <stddef.h>

#include <m_pd.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_WCHAR_H
# include <wchar.h>
#endif
#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif

/*--------------------------------------------------------------------
 * DEBUG
 *--------------------------------------------------------------------*/
/*#define LOCALE_DEBUG 1*/

/*--------------------------------------------------------------------
 * "unused" pragma
 */
#ifdef __GNUC__
# define MOO_UNUSED __attribute__((unused))
#else
# define MOO_UNUSED
#endif

/*=====================================================================
 * Constants
 *=====================================================================*/

/*=====================================================================
 * Structures and Types
 *=====================================================================*/

static char *locale_banner =
  "\nlocale: C99 locale support version " PACKAGE_VERSION " by Bryan Jurish\n"
    "locale: compiled by " PACKAGE_BUILD_USER " on " PACKAGE_BUILD_DATE ;

static t_class *locale_class;

typedef struct _locale
{
  t_object       x_obj;
  t_outlet      *x_outlet;   //-- data outlet
} t_locale;

/*=====================================================================
 * Constants
 *=====================================================================*/

/*-- LC_* constants defined in locale.h (debian libc6-dev 2.7-5)--

LC_ALL
LC_CTYPE
LC_NUMERIC
LC_TIME
LC_COLLATE
LC_MONETARY
LC_MESSAGES
LC_PAPER
LC_NAME
LC_ADDRESS
LC_TELEPHONE
LC_MEASUREMENT
LC_IDENTIFICATION

*/

//-- Constants: categories
#if HAVE_DECL_LC_ALL
 static t_symbol *sp_LC_ALL;
#endif
#if HAVE_DECL_LC_CTYPE
 static t_symbol *sp_LC_CTYPE;
#endif
#if HAVE_DECL_LC_NUMERIC
 static t_symbol *sp_LC_NUMERIC;
#endif
#if HAVE_DECL_LC_TIME
 static t_symbol *sp_LC_TIME;
#endif
#if HAVE_DECL_LC_COLLATE
 static t_symbol *sp_LC_COLLATE;
#endif
#if HAVE_DECL_LC_MONETARY
 static t_symbol *sp_LC_MONETARY;
#endif
#if HAVE_DECL_LC_MESSAGES
 static t_symbol *sp_LC_MESSAGES;
#endif
#if HAVE_DECL_LC_PAPER
 static t_symbol *sp_LC_PAPER;
#endif
#if HAVE_DECL_LC_NAME
 static t_symbol *sp_LC_NAME;
#endif
#if HAVE_DECL_LC_ADDRESS
 static t_symbol *sp_LC_ADDRESS;
#endif
#if HAVE_DECL_LC_TELEPHONE
 static t_symbol *sp_LC_TELEPHONE;
#endif
#if HAVE_DECL_LC_MEASUREMENT
 static t_symbol *sp_LC_MEASUREMENT;
#endif
#if HAVE_DECL_LC_IDENTIFICATION
 static t_symbol *sp_LC_IDENTIFICATION;
#endif

/*=====================================================================
 * Utilities
 *=====================================================================*/

/*--------------------------------------------------------------------
 * sym2cat(symbol)
 */
static int sym2cat(t_object *obj, t_symbol *sym)
{
  if (sym==sp_LC_ALL || sym==&s_) return LC_ALL;
#if HAVE_DECL_LC_CTYPE
  else if (sym==sp_LC_CTYPE) return LC_CTYPE;
#endif
#if HAVE_DECL_LC_NUMERIC
  else if (sym==sp_LC_NUMERIC) return LC_NUMERIC;
#endif
#if HAVE_DECL_LC_TIME
  else if (sym==sp_LC_TIME) return LC_TIME;
#endif
#if HAVE_DECL_LC_COLLATE
  else if (sym==sp_LC_COLLATE) return LC_COLLATE;
#endif
#if HAVE_DECL_LC_MONETARY
  else if (sym==sp_LC_MONETARY) return LC_MONETARY;
#endif
#if HAVE_DECL_LC_MESSAGES
  else if (sym==sp_LC_MESSAGES) return LC_MESSAGES;
#endif
#if HAVE_DECL_LC_PAPER
  else if (sym==sp_LC_PAPER) return LC_PAPER;
#endif
#if HAVE_DECL_LC_NAME
  else if (sym==sp_LC_NAME) return LC_NAME;
#endif
#if HAVE_DECL_LC_ADDRESS
  else if (sym==sp_LC_ADDRESS) return LC_ADDRESS;
#endif
#if HAVE_DECL_LC_TELEPHONE
  else if (sym==sp_LC_TELEPHONE) return LC_TELEPHONE;
#endif
#if HAVE_DECL_LC_MEASUREMENT
  else if (sym==sp_LC_MEASUREMENT) return LC_MEASUREMENT;
#endif
#if HAVE_DECL_LC_IDENTIFICATION
  else if (sym==sp_LC_IDENTIFICATION) return LC_IDENTIFICATION;
#endif
#if HAVE_DECL_LC_FOOBAR
  else if (sym==sp_LC_FOOBAR) return LC_FOOBAR;
#endif
  pd_error(obj, ": sym2cat() could not find locale category for symbol '%s', using LC_ALL", sym->s_name);
  return LC_ALL;
}

/*--------------------------------------------------------------------
 * cat2sym(cat)
 */
MOO_UNUSED
static t_symbol *cat2sym(t_object *obj, int cat)
{
  switch (cat) {
#if HAVE_DECL_LC_ALL
  case LC_ALL: return sp_LC_ALL; break;
#endif
#if HAVE_DECL_LC_CTYPE
  case LC_CTYPE: return sp_LC_CTYPE; break;
#endif
#if HAVE_DECL_LC_NUMERIC
  case LC_NUMERIC: return sp_LC_NUMERIC; break;
#endif
#if HAVE_DECL_LC_TIME
  case LC_TIME: return sp_LC_TIME; break;
#endif
#if HAVE_DECL_LC_COLLATE
  case LC_COLLATE: return sp_LC_COLLATE; break;
#endif
#if HAVE_DECL_LC_MONETARY
  case LC_MONETARY: return sp_LC_MONETARY; break;
#endif
#if HAVE_DECL_LC_MESSAGES
  case LC_MESSAGES: return sp_LC_MESSAGES; break;
#endif
#if HAVE_DECL_LC_PAPER
  case LC_PAPER: return sp_LC_PAPER; break;
#endif
#if HAVE_DECL_LC_NAME
  case LC_NAME: return sp_LC_NAME; break;
#endif
#if HAVE_DECL_LC_ADDRESS
  case LC_ADDRESS: return sp_LC_ADDRESS; break;
#endif
#if HAVE_DECL_LC_TELEPHONE
  case LC_TELEPHONE: return sp_LC_TELEPHONE; break;
#endif
#if HAVE_DECL_LC_MEASUREMENT
  case LC_MEASUREMENT: return sp_LC_MEASUREMENT; break;
#endif
#if HAVE_DECL_LC_IDENTIFICATION
  case LC_IDENTIFICATION: return sp_LC_IDENTIFICATION; break;
#endif
  default: break;
  }
  pd_error(obj, ": cat2sym() unknown locale category '%d'", cat);
  return &s_;
}

/*=====================================================================
 * Methods
 *=====================================================================*/

/*--------------------------------------------------------------------
 * bang
 */
static void locale_bang(t_locale *x)
{
  setlocale(LC_ALL,"");
  setlocale(LC_NUMERIC,"C");
}

/*--------------------------------------------------------------------
 * get , get CATEGORY
 */
static void locale_get(t_locale *x, t_symbol *catsym)
{
  int   cat = sym2cat((t_object*)x, catsym);
  char *val;
  t_atom valatom;
  val = setlocale(cat, NULL);
  if (val) {
    SETSYMBOL(&valatom,gensym(val));
  } else {
    SETSYMBOL(&valatom,&s_);
  }
  outlet_anything(x->x_outlet, catsym, 1, &valatom);
}


/*--------------------------------------------------------------------
 * set , set CATEGORY VALUE
 */
static void locale_set(t_locale *x, t_symbol *catsym, t_symbol *valsym)
{
  int cat;
  if (catsym==&s_ && valsym==&s_) { setlocale(LC_ALL,""); return; }
  cat = sym2cat((t_object*)x,catsym);
  setlocale(cat,valsym->s_name);
}

/*--------------------------------------------------------------------
 * reset -> set LC_ALL C
 */
static void locale_reset(t_locale *x)
{
  setlocale(LC_ALL,"C");
}

/*--------------------------------------------------------------------
 * which
 */
static void locale_which(t_locale *x)
{
#if HAVE_DECL_LC_ALL
  outlet_symbol(x->x_outlet,sp_LC_ALL);
#endif
#if HAVE_DECL_LC_CTYPE
  outlet_symbol(x->x_outlet,sp_LC_CTYPE);
#endif
#if HAVE_DECL_LC_NUMERIC
  outlet_symbol(x->x_outlet,sp_LC_NUMERIC);
#endif
#if HAVE_DECL_LC_TIME
  outlet_symbol(x->x_outlet,sp_LC_TIME);
#endif
#if HAVE_DECL_LC_COLLATE
  outlet_symbol(x->x_outlet,sp_LC_COLLATE);
#endif
#if HAVE_DECL_LC_MONETARY
  outlet_symbol(x->x_outlet,sp_LC_MONETARY);
#endif
#if HAVE_DECL_LC_MESSAGES
  outlet_symbol(x->x_outlet,sp_LC_MESSAGES);
#endif
#if HAVE_DECL_LC_PAPER
  outlet_symbol(x->x_outlet,sp_LC_PAPER);
#endif
#if HAVE_DECL_LC_NAME
  outlet_symbol(x->x_outlet,sp_LC_NAME);
#endif
#if HAVE_DECL_LC_ADDRESS
  outlet_symbol(x->x_outlet,sp_LC_ADDRESS);
#endif
#if HAVE_DECL_LC_TELEPHONE
  outlet_symbol(x->x_outlet,sp_LC_TELEPHONE);
#endif
#if HAVE_DECL_LC_MEASUREMENT
  outlet_symbol(x->x_outlet,sp_LC_MEASUREMENT);
#endif
#if HAVE_DECL_LC_IDENTIFICATION
  outlet_symbol(x->x_outlet,sp_LC_IDENTIFICATION);
#endif
}


/*--------------------------------------------------------------------
 * new
 */
static void *locale_new(void)
{
    t_locale *x = (t_locale *)pd_new(locale_class);
    x->x_outlet = outlet_new(&x->x_obj, &s_anything);
    return (void *)x;
}

/*--------------------------------------------------------------------
 * free
 */
static void locale_free(t_locale *x)
{
  outlet_free(x->x_outlet);
  return;
}

/*=====================================================================
 * Setup
 *=====================================================================*/

/*--------------------------------------------------------------------
 * setup: symbols
 */
static void locale_setup_constants(void)
{
#if HAVE_DECL_LC_ALL
  sp_LC_ALL = gensym("LC_ALL");
#endif
#if HAVE_DECL_LC_CTYPE
  sp_LC_CTYPE = gensym("LC_CTYPE");
#endif
#if HAVE_DECL_LC_NUMERIC
  sp_LC_NUMERIC = gensym("LC_NUMERIC");
#endif
#if HAVE_DECL_LC_TIME
  sp_LC_TIME = gensym("LC_TIME");
#endif
#if HAVE_DECL_LC_COLLATE
  sp_LC_COLLATE = gensym("LC_COLLATE");
#endif
#if HAVE_DECL_LC_MONETARY
  sp_LC_MONETARY = gensym("LC_MONETARY");
#endif
#if HAVE_DECL_LC_MESSAGES
  sp_LC_MESSAGES = gensym("LC_MESSAGES");
#endif
#if HAVE_DECL_LC_PAPER
  sp_LC_PAPER = gensym("LC_PAPER");
#endif
#if HAVE_DECL_LC_NAME
  sp_LC_NAME = gensym("LC_NAME");
#endif
#if HAVE_DECL_LC_ADDRESS
  sp_LC_ADDRESS = gensym("LC_ADDRESS");
#endif
#if HAVE_DECL_LC_TELEPHONE
  sp_LC_TELEPHONE = gensym("LC_TELEPHONE");
#endif
#if HAVE_DECL_LC_MEASUREMENT
  sp_LC_MEASUREMENT = gensym("LC_MEASUREMENT");
#endif
#if HAVE_DECL_LC_IDENTIFICATION
  sp_LC_IDENTIFICATION = gensym("LC_IDENTIFICATION");
#endif
}

/*--------------------------------------------------------------------
 * setup
 */
void locale_setup(void)
{
  post(locale_banner);
#ifdef LOCALE_DEBUG
  post("locale : debugging enabled");
#endif

  //-- constants
  locale_setup_constants();

  //-- class
  locale_class = class_new(gensym("locale"),
			   (t_newmethod)locale_new,
			   (t_method)locale_free,
			   sizeof(t_locale),
			   CLASS_DEFAULT,
			   0);
  
  //-- methods: get
  class_addmethod(locale_class, (t_method)locale_get,     gensym("get"), A_DEFSYMBOL, 0);
  class_addmethod(locale_class, (t_method)locale_which,   gensym("which"), 0);
  //
  //-- methods: set
  class_addmethod(locale_class, (t_method)locale_set,     gensym("set"), A_DEFSYMBOL, A_DEFSYMBOL, 0);
  class_addmethod(locale_class, (t_method)locale_bang,    &s_bang, 0);
  class_addmethod(locale_class, (t_method)locale_reset,   gensym("reset"), 0);


  //-- help symbol
  class_sethelpsymbol(locale_class, gensym("locale-help.pd"));
}
