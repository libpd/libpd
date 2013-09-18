////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-2000 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////
#include "Gem/GemConfig.h"
/* -------------------------- setup function ------------------------------ */

#include "Gem/Manager.h"
#include "Gem/Settings.h"
#include "Gem/Version.h"

#include <stdio.h>

#ifdef _WIN32
# include <io.h>
# include <windows.h>
# define snprintf _snprintf
# define close _close
#else
# include <unistd.h>
#endif

# include <fcntl.h>


static const char GEM_MAINTAINER[] = "IOhannes m zmoelnig";

static const char *GEM_AUTHORS[] = {
  "Chris Clepper",
  "Cyrille Henry",
  "IOhannes m zmoelnig"
};

static const char GEM_OTHERAUTHORS[] =
  "Guenter Geiger, Daniel Heckenberg, James Tittle, Hans-Christoph Steiner, et al.";

extern "C" {
#ifdef HAVE_S_STUFF_H
# include "s_stuff.h"

  /* this is ripped from m_imp.h */
  struct _gemclass
  {
    t_symbol *c_name;                   /* name (mostly for error reporting) */
    t_symbol *c_helpname;               /* name of help file */
    t_symbol *c_externdir;              /* directory extern was loaded from */
    /* ... */ /* the real t_class continues here... */
  };
# define t_gemclass struct _gemclass

  static void Gem_addownpath(const char*filename) {
    char buf[MAXPDSTRING];
    char*bufptr=NULL;
    int fd=-1;

    int flags=O_RDONLY;
# ifdef _WIN32
    flags |= _O_BINARY;
# endif

    /* check whether we can find the abstractions (because they are already in Pd's path) */
    if ((fd=canvas_open(NULL, filename, "", buf, &bufptr, MAXPDSTRING, 1))>=0){
      close(fd);
      return;
    }

    char*mypath=0;
    t_gemclass *c = (t_gemclass*)class_new(gensym("Gem"), 0, 0, 0, 0, A_NULL);
    mypath=c->c_externdir->s_name;

    /* check whether we can find the abstractions in Gem's own path */
    snprintf(buf, MAXPDSTRING-1, "%s/%s", mypath, filename);
    buf[MAXPDSTRING-1]=0;
    if ((fd=open(buf, flags))>=0){
      close(fd);
    } else {
      // can't find this abstraction...giving up
      return;
    }

    logpost(NULL, 5, "eventually adding Gem path '%s' to search-paths", mypath);
# ifndef _MSC_VER
    /* MSVC cannot really handle these non-exported symbols */
    sys_searchpath = namelist_append(sys_searchpath, mypath, 0);
# endif
  }
#else
  static void Gem_addownpath(const char*filename) {  }
#endif

  GEM_EXTERN void Gem_setup()
  {
    // startup GEM
    logpost(NULL, 4, "GEM: Graphics Environment for Multimedia");
    logpost(NULL, 4, "GEM: ver: %s", GemVersion::versionString());
    logpost(NULL, 4, "GEM: compiled: " __DATE__);
    logpost(NULL, 4, "GEM: maintained by %s", GEM_MAINTAINER);
    logpost(NULL, 4, "GEM: Authors :\tMark Danks (original version)");
    for(unsigned int i=0; i<sizeof(GEM_AUTHORS)/sizeof(*GEM_AUTHORS); i++) {
      logpost(NULL, 4, "GEM:\t\t%s", GEM_AUTHORS[i]);
    }  
    logpost(NULL, 4, "GEM: with help by %s", GEM_OTHERAUTHORS);
    logpost(NULL, 4, "GEM: found a bug? miss a feature? please report it:");
    logpost(NULL, 4, "GEM: \thomepage http://gem.iem.at/");
    logpost(NULL, 4, "GEM: \tbug-tracker http://sourceforge.net/projects/pd-gem/");
    logpost(NULL, 4, "GEM: \tmailing-list http://lists.puredata.info/listinfo/gem-dev/");


    GemSettings::init();
    Gem_addownpath("hsv2rgb.pd");
    GemMan::initGem();
  }

  GEM_EXTERN void gem_setup()
  {
    Gem_setup();
  }

  GEM_EXTERN void GEM_setup()
  {
    Gem_setup();
  }

}   // for extern "C"
