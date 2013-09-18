////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 2011-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "ThreadSemaphore.h"


#include <pthread.h>

class gem::thread::Semaphore::PIMPL {
public:
  pthread_mutex_t*mutex;
  pthread_cond_t *cond;
  unsigned int *refcount;
  PIMPL(void) : mutex(new pthread_mutex_t), cond(new pthread_cond_t), refcount(new unsigned int) {
    pthread_mutex_init(mutex, NULL); 
    pthread_cond_init (cond , NULL); 
    *refcount=1;
  }
  PIMPL(const PIMPL&org) : mutex(org.mutex), cond(org.cond), refcount(org.refcount) {
    *refcount++;
  }

  ~PIMPL(void) {
    *refcount--;
    if(0==*refcount) {
      if(mutex) {
        pthread_mutex_destroy(mutex); 
        delete mutex;
        mutex=NULL;
      }
      if(cond) {
        pthread_cond_destroy(cond); 
        delete cond;
        cond=NULL;
      }
    }
  }
  inline void freeze(void) {
    pthread_mutex_lock  ( mutex );
    pthread_cond_wait   ( cond, mutex );
    pthread_mutex_unlock( mutex );
  }
  inline void thaw(void) {
    pthread_mutex_lock  (mutex);
    pthread_cond_signal (cond );
    pthread_mutex_unlock(mutex);
  }
};


gem::thread::Semaphore::Semaphore(void) : m_pimpl(new PIMPL()) {
}
gem::thread::Semaphore::Semaphore(const gem::thread::Semaphore&org) : m_pimpl(new PIMPL(*org.m_pimpl)) {
}
gem::thread::Semaphore::~Semaphore(void) {
  delete(m_pimpl);
  m_pimpl=NULL;
}


void gem::thread::Semaphore::freeze(void) {
  m_pimpl->freeze();
}

void gem::thread::Semaphore::thaw(void) {
  m_pimpl->thaw();
}


gem::thread::Semaphore&gem::thread::Semaphore::operator=(const gem::thread::Semaphore&org) {
  if(this!=&org && m_pimpl->refcount != org.m_pimpl->refcount) {
    PIMPL*pimpl=new PIMPL(*org.m_pimpl);
    delete m_pimpl;
    m_pimpl=pimpl;
  }
  return *this;
}
