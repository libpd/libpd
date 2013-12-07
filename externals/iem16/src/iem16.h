/* ********************************************** */
/* the IEM16 external                              */
/* ********************************************** */
/*                            forum::für::umläute */
/* ********************************************** */

/* the IEM16 external is a runtime-library for miller s. puckette's realtime-computermusic-software "pure data"
 * therefore you NEED "pure data" to make any use of the IEM16 external
 * (except if you want to use the code for other things)
 * download "pure data" at

 http://pd.iem.at
 ftp://iem.at/pd

 *
 * if you are looking for the latest release of the IEM16-external you should have another look at

 ftp://iem.at/pd/Externals/IEM16

 * 
 * IEM16 is published under the GNU GeneralPublicLicense, that must be shipped with IEM16.
 * if you are using Debian GNU/linux, the GNU-GPL can be found under /usr/share/common-licenses/GPL
 * if you still haven't found a copy of the GNU-GPL, have a look at http://www.gnu.org
 *
 * "pure data" has it's own license, that comes shipped with "pure data".
 *
 * there are ABSOLUTELY NO WARRANTIES for anything
 */

#ifndef INCLUDE_IEM16_H__
#define INCLUDE_IEM16_H__

#include "m_pd.h"

typedef short t_iem16_16bit;

#define IEM16_SCALE_UP (32767)
#define IEM16_SCALE_DOWN (1./32767)

#define VERSION "0.2"

#define UNITBIT32 1572864.  /* 3*2^19; bit 32 has place value 1 */

    /* machine-dependent definitions.  These ifdefs really
    should have been by CPU type and not by operating system! */
#ifdef __irix__
    /* big-endian.  Most significant byte is at low address in memory */
# define HIOFFSET 0    /* word offset to find MSB */
# define LOWOFFSET 1    /* word offset to find LSB */
# define int32 long  /* a data type that has 32 bits */
#elif defined __WIN32__
    /* little-endian; most significant byte is at highest address */
# define HIOFFSET 1
# define LOWOFFSET 0
# define int32 long
#elif defined __FreeBSD__
# include <machine/endian.h>
# if BYTE_ORDER == LITTLE_ENDIAN
#  define HIOFFSET 1
#  define LOWOFFSET 0
# else
#  define HIOFFSET 0    /* word offset to find MSB */
#  define LOWOFFSET 1    /* word offset to find LSB */
# endif /* BYTE_ORDER */
# include <sys/types.h>
# define int32 int32_t
#elif defined __linux__
# include <endian.h>
# if !defined(__BYTE_ORDER) || !defined(__LITTLE_ENDIAN)                         
#  error No byte order defined                                                    
# endif
# if __BYTE_ORDER == __LITTLE_ENDIAN                                             
#  define HIOFFSET 1                                                              
#  define LOWOFFSET 0                                                             
# else                                                                           
#  define HIOFFSET 0    /* word offset to find MSB */                             
#  define LOWOFFSET 1    /* word offset to find LSB */                            
# endif /* __BYTE_ORDER */                                                       
# include <sys/types.h>
# define int32 int32_t
#elif defined __APPLE__
# ifdef __BIG_ENDIAN__
#  define HIOFFSET 0    /* word offset to find MSB */
#  define LOWOFFSET 1    /* word offset to find LSB */
# else
#  define HIOFFSET 1
#  define LOWOFFSET 0
# endif
# define int32 int  /* a data type that has 32 bits */
#endif /* system */


#endif
