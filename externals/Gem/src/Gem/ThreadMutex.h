/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

	- locks a thread (wrapper around pthread_mutex)

    Copyright (c) 2011-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_GEM_THREADMUTEX_H_
#define _INCLUDE__GEM_GEM_THREADMUTEX_H_


#include "Gem/ExportDef.h"

namespace gem { 
  namespace thread {
    GEM_EXTERN class Mutex {
    private:
      class PIMPL;
      PIMPL*m_pimpl;
    public:
      Mutex(void);
      virtual ~Mutex(void);

      Mutex(const Mutex&);
      Mutex&operator=(const Mutex&);
      
      void lock   (void);
      void unlock (void);
      bool trylock(void);
    };
  };
};
#endif /* _INCLUDE__GEM_GEM_THREADMUTEX_H_ */
