/* Copyright (c) 2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* The tow extern just loads the 'widget' library.
   The tow class itself is defined in widget.c. */

#include "m_pd.h"
#include "common/loud.h"
#include "unstable/loader.h"

void tow_setup(void)
{
    int result = LOADER_OK;
    if (zgetfn(&pd_objectmaker, gensym("widget")))
	loud_warning(0, "tow", "widget is already loaded");
    else
	result = unstable_load_lib("", "widget");
    if (result == LOADER_NOFILE)
	loud_error(0, "widget library is missing");
    else if (!zgetfn(&pd_objectmaker, gensym("widget")))
    {
	loud_error(0, "version mismatch");
	loud_errand(0, "use a more recent Pd release (or recompile toxy).");
    }
}
