////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.at
//
// Implementation file
//
//    Copyright (c) 1997-2000 Mark Danks.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    Copyright (c) 2002 James Tittle & Chris Clepper
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
// a wrapper for calling Pd's sys_register_loader()
//
/////////////////////////////////////////////////////////
#ifdef _MSC_VER
# pragma warning( disable: 4091)
#endif /* _MSC_VER */


#include "Loaders.h"

#if defined __linux__ || defined __APPLE__
# define DL_OPEN
#endif

#ifdef DL_OPEN
# include <dlfcn.h>
#endif

#if defined _WIN32
# include <io.h>
# include <windows.h>
#endif

extern "C" {
  typedef void (*loader_registrar_t)(gem_loader_t loader);
}
static loader_registrar_t pd_register_loader = NULL;

static int find_pd_loader(void) {
  if(pd_register_loader)return 1;

#ifdef DL_OPEN
  pd_register_loader=(loader_registrar_t)dlsym(RTLD_DEFAULT, "sys_register_loader");
#elif defined _WIN32
  /* no idea whether this actually works... */
  pd_register_loader = (loader_registrar_t)GetProcAddress( GetModuleHandle("pd.dll"), "sys_register_loader");  
#else
  // no loader for older Pd's....
#endif

  return(NULL!=pd_register_loader);
}

void gem_register_loader(gem_loader_t loader) {
  if(find_pd_loader()) {
    pd_register_loader(loader);
  }
}
