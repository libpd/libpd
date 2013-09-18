/* configuration-file */
#ifndef HAVE_BASE_GEMCONFIG_H_
#define HAVE_BASE_GEMCONFIG_H_

#ifdef _MSC_VER
# ifndef _WIN32
#  define _WIN32
# endif
#endif

#ifdef _WIN32
# ifndef NT
#  define NT
# endif
# ifndef MSW
#  define MSW
# endif
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
# endif
#endif

#define GEM_FILMBACKEND_undefined 0
#define GEM_FILMBACKEND_Darwin 1

#define GEM_VIDEOBACKEND_undefined 0
#define GEM_VIDEOBACKEND_Darwin 1
#define GEM_VIDEOBACKEND_DS 2
#define GEM_VIDEOBACKEND_NT 3
#define GEM_VIDEOBACKEND_SGI 4

#ifdef HAVE_CONFIG_H

# include "config.h"

/* stupid hack to avoid duplicate inclusion of config.h e.g. in avifile.h */
# undef HAVE_CONFIG_H

#else /* includes system-specific files */

# ifdef __linux__
#  include "Gem/configLinux.h"
# elif defined __APPLE__
#  include "Gem/configDarwin.h"
# elif defined _WIN32
#  include "Gem/configNT.h"
# endif
#endif

#if defined GEM_VIDEOBACKEND && GEM_VIDEOBACKEND == GEM_VIDEOBACKEND_undefined
# warning ignoring unknown video backend
# undef GEM_VIDEOBACKEND
#endif

#if defined GEM_FILMBACKEND && GEM_FILMBACKEND == GEM_FILMBACKEND_undefined
# warning ignoring unknown film backend
# undef GEM_FILMBACKEND
#endif

#ifdef HAVE_LIBFTGL
#  define FTGL
#endif

#endif /* HAVE_BASE_GEMCONFIG_H_ */
