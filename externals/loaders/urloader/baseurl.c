/* Copyright (c) 2008 Steve Sinclair <radarsat1@gmail.com>
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," that comes with Pd.  
 */

/*
 * This code provides a canvas with a base URL to inform the baseurl
 * where to look.
 */


#ifdef __WIN32__
# define MSW
#endif

#include "m_pd.h"

#if (PD_MINOR_VERSION >= 40)

#include "s_stuff.h"
#include "g_canvas.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "baseurl.h"

typedef void (*t_baseurl_setup)(void);

/* definitions taken from s_loader.c  */
typedef int (*loader_t)(t_canvas *canvas, char *classname);
void sys_register_loader(loader_t loader);
void class_set_extern_dir(t_symbol *s);

/* ==================================================== */

static t_class *baseurl_class;

static char *version = "0.0.1";

#endif /* PD_MINOR_VERSION>=40 */

static void*baseurl_new(t_symbol *s, int argc, t_atom *argv)
{
  t_baseurl *x = (t_baseurl*)pd_new(baseurl_class);
  SETSYMBOL(&x->url, argv[0].a_w.w_symbol);
  return (x);
}

void baseurl_setup(void)
{
  post("baseurl %s",version);  
  post("\twritten by Steve Sinclair");
  post("\tcompiled on "__DATE__" at "__TIME__ " ");
  post("\tcompiled against Pd version %d.%d.%d.%s", PD_MAJOR_VERSION, PD_MINOR_VERSION, PD_BUGFIX_VERSION, PD_TEST_VERSION);

  baseurl_class = class_new(gensym("baseurl"), (t_newmethod)baseurl_new, 0, sizeof(t_baseurl), CLASS_NOINLET, A_GIMME, 0);
}
