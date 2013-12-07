////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 2010-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////
  
#include "plugins/videoBase.h"
#include "Gem/RTE.h"

#include <iostream>

#if 0
# define debugPost post
#else
# define debugPost
#endif

#include <pthread.h>

#ifdef _WIN32
# include <winsock2.h>
#endif

using namespace gem::plugins;

/**
 * video capturing states
 *
 *  state                user-pov            system-pov
 * ----------------------------------------------------
 * is device open?       m_haveVideo         m_haveVideo
 * is device streaming?  m_pimpl->shouldrun  m_capturing
 * is thread running     (opaque)            m_pimpl->running
 *
 */

class videoBase :: PIMPL {
public:
  /* interfaces */
  // the list of provided device-classes
  std::vector<std::string>m_providers;

  /* threading */
  bool threading;
  pthread_t thread;
  pthread_mutex_t**locks;
  unsigned int numlocks;

  bool asynchronous;
  pthread_cond_t*condition_cond;
  pthread_mutex_t*condition_mutex;

  unsigned int timeout;

  bool cont;
  bool running;

  bool shouldrun; /* we should be capturing */

  const std::string name;

  PIMPL(const std::string name_, unsigned int locks_, unsigned int timeout_) :
    threading(locks_>0),
    locks(NULL),
    numlocks(0),
    asynchronous(true), condition_cond(NULL), condition_mutex(NULL),
    timeout(timeout_),
    cont(true),
    running(false),
    shouldrun(false),
    name(name_)
  {
    if(locks_>0) {
      numlocks=locks_;
      locks=new pthread_mutex_t*[numlocks];
      unsigned int i=0;
      for(i=0; i<locks_; i++)
        locks[i]=NULL;

      condition_mutex=new pthread_mutex_t;
      condition_cond =new pthread_cond_t;

      pthread_mutex_init(condition_mutex, NULL);
      pthread_cond_init(condition_cond, NULL);
    }
  }
  ~PIMPL(void) {
    cont=false;
    lock_delete();
    delete[]locks; 
    locks=NULL;

    doThaw();

    if(condition_mutex) {
      pthread_mutex_destroy(condition_mutex); 
      delete condition_mutex;
    }
    if(condition_cond) {
      pthread_cond_destroy(condition_cond); 
      delete condition_cond;
    }
  }

  void lock(unsigned int i) {
    //    post("lock %d?\t%d", i, numlocks);

    if(i<numlocks && locks[i]) {
      pthread_mutex_lock(locks[i]);
    }
  }
  void unlock(unsigned int i) {
    //      post("unlock %d? %d", i,numlocks);

    if(i<numlocks && locks[i]) {
      pthread_mutex_unlock(locks[i]);
    }
  }
  bool lock_new(void) {
    if(locks) {
      unsigned int i=0;
      for(i=0; i<numlocks; i++) {
        locks[i]=new pthread_mutex_t;
        if ( pthread_mutex_init(locks[i], NULL) < 0 ) {
          lock_delete();
          return false;
        }
      }
      return true;
    }
    return true;
  }
  void lock_delete(void) {
    if(locks) {
      unsigned int i=0;
      for(i=0; i<numlocks; i++) {
        if(locks[i]) {
          pthread_mutex_destroy(locks[i]); 
          delete locks[i];
          locks[i]=NULL;
        }
      }
    }
  }

  void doFreeze(void) {
    if(condition_mutex && condition_cond) {
      pthread_mutex_lock  ( condition_mutex );
       pthread_cond_wait  ( condition_cond, condition_mutex );
      pthread_mutex_unlock( condition_mutex );
    }
  }

  void freeze(void) {
    if(asynchronous)return;
    doFreeze();
  }

  void doThaw(void) {
    if(condition_mutex && condition_cond) {
      pthread_mutex_lock  (condition_mutex);
      pthread_cond_signal(condition_cond );
      pthread_mutex_unlock(condition_mutex);
    }
  }

  void thaw(void) {
    if(asynchronous)return;
    doThaw();
  }

  static void*threadfun(void*you) {
    videoBase*me=(videoBase*)you;
    pixBlock*pix=NULL;
    post("starting capture thread");
    me->m_pimpl->cont=true;
    me->m_pimpl->running=true;
    
    while(me->m_pimpl->cont) {
      if(!me->grabFrame()) {
        break;
      }
      me->m_pimpl->freeze();
    }
    me->m_pimpl->running=false;

    return NULL;
  }

  bool setAsynchronous(bool cont) {
    bool old=asynchronous;
    asynchronous=cont;
    if(asynchronous){
      doThaw();
    }
    return old;
  }
};


/////////////////////////////////////////////////////////
//
// pix_videoLinux
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
videoBase :: videoBase(const std::string name, unsigned int locks) :
  m_capturing(false), m_haveVideo(false), 
  m_width(64), m_height(64),
  m_reqFormat(GL_RGBA),
  m_devicename(std::string("")), m_devicenum(0),
  m_pimpl(new PIMPL(name.empty()?std::string("<unknown>"):name, locks, 0))
{
  if(!name.empty()) {
    provide(name);
  }
}
videoBase :: videoBase(const std::string name) :
  m_capturing(false), m_haveVideo(false), 
  m_width(64), m_height(64),
  m_reqFormat(GL_RGBA),
  m_devicename(std::string("")), m_devicenum(0),
  m_pimpl(new PIMPL(name.empty()?std::string("<unknown>"):name, 1, 0))
{
  if(!name.empty()) {
    provide(name);
  }
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
videoBase :: ~videoBase()
{
  if(m_pimpl)delete m_pimpl; m_pimpl=NULL;
}
/////////////////////////////////////////////////////////
// open/close
//
/////////////////////////////////////////////////////////
bool videoBase :: open(gem::Properties&props)
{
  debugPost("open: %d -> %d", m_haveVideo, m_capturing);
  if(m_haveVideo)close();
  m_haveVideo=openDevice(props);
  return m_haveVideo;
}
void videoBase :: close()
{
  debugPost("close: %d -> %d", m_capturing, m_haveVideo);
  if(m_capturing)stop();
  if(m_haveVideo)closeDevice();
  m_pimpl->shouldrun=false;
  m_haveVideo=false;
}
/////////////////////////////////////////////////////////
// start/stop
//
/////////////////////////////////////////////////////////
bool videoBase :: start()
{
  debugPost("start: %d -> %d", m_haveVideo, m_capturing);
  if(!m_haveVideo)return false;
  if(m_capturing)stop();
  m_capturing=startTransfer();
  if(m_capturing)
    startThread();

  m_pimpl->shouldrun=true;
  return m_capturing;
}
bool videoBase :: stop()
{
  debugPost("stop(%d): %d -> %d", m_pimpl->shouldrun, m_capturing, m_haveVideo);
  bool running=m_pimpl->shouldrun;
  m_pimpl->shouldrun=false;
  if(!m_haveVideo)return false;
  if(m_capturing) {
    stopThread();
    stopTransfer();
  }

  m_capturing=false;
  return running;
}

/////////////////////////////////////////////////////////
// resetDevice
//
/////////////////////////////////////////////////////////
bool videoBase :: reset()
{
  return(false);
}

/////////////////////////////////////////////////////////
// startTransfer
//
/////////////////////////////////////////////////////////
bool videoBase :: restartTransfer()
{
  debugPost("restartTransfer");
  bool running=stop();
  if(running)return start();

  return false;
}


bool videoBase :: startThread() {
  debugPost("startThread %d", m_pimpl->running);
  if(m_pimpl->running) {
    stopThread();
  }

  if(m_pimpl->threading) {
    if(!m_pimpl->lock_new())return false;

    pthread_create(&m_pimpl->thread, 
                   0,
                   m_pimpl->threadfun, 
                   this);
    while(!m_pimpl->running)
      usleep(10);

    return true;
  }
  return false;
}
bool videoBase :: stopThread(int timeout) {
  int i=0;
  if(!m_pimpl->threading)return true;
  
  debugPost("stopThread: %d", timeout);

  m_pimpl->cont=false;

  m_pimpl->thaw();
  if(timeout<0)timeout=m_pimpl->timeout;

  if(timeout>0) {
    while(m_pimpl->running) {
      usleep(10);
      i+=10;
      if(i>timeout) {
        return false;
      }
    }
  } else {
    while(m_pimpl->running) {
      usleep(10);
      i+=10;
      if(i>1000000) {
        post("waiting for video grabbing thread to terminate...");
        i=0;
      }
      m_pimpl->thaw();
    }
    //pthread_join(m_pimpl->thread, NULL);
  }

  m_pimpl->lock_delete();
  return true;
}
void videoBase :: lock(unsigned int id) {
  m_pimpl->lock(id);
}
void videoBase :: unlock(unsigned int id) {
  m_pimpl->unlock(id);
}
void videoBase :: usleep(unsigned long usec) {
  struct timeval sleep;
  long usec_ = usec%1000000;
  long sec_=0;
  //  long  sec_ = usec\1000000;
  sleep.tv_sec=sec_;
  sleep.tv_usec=usec_; 
  select(0,0,0,0,&sleep);
}

pixBlock* videoBase :: getFrame(void) {
  pixBlock*pix=&m_image;
  if(!(m_haveVideo && m_capturing))return NULL;
  if(m_pimpl->threading) {
     // get from thread
    if(!m_pimpl->running){
      pix=NULL;
    }
  } else {
    // no thread, grab it directly
    if(!grabFrame()) {
      m_capturing=false;
      pix=NULL;
    }
  }
  lock();
  return pix;
}


void videoBase :: releaseFrame(void) {
  m_image.newimage=false;
  unlock();
  m_pimpl->thaw();
}

/////////////////////////////////////////////////////////
// set the color-space
bool videoBase :: setColor(int d){
  post("setting the color-space is not supported by this OS/device");
  return false;
}

/////////////////////////////////////////////////////////
// open a dialog for the settings
bool videoBase :: dialog(std::vector<std::string>dialognames){
  return false;
}
std::vector<std::string>videoBase :: dialogs(void) {
  std::vector<std::string>result;
  return result;
}
std::vector<std::string>videoBase :: enumerate(void) {
  std::vector<std::string>result;
  return result;
}

////////////////////
// set the video device
bool videoBase :: setDevice(int d)
{
  m_devicename.clear();
  if (d==m_devicenum)return true;
  m_devicenum=d;
  return true;
}
bool videoBase :: setDevice(const std::string name)
{
  m_devicenum=-1;
  m_devicename=name;
  return true;
}

const std::string videoBase :: getName() {
  return m_pimpl->name;
}




/////////////////////////////////////////////////////////
// query whether this backend provides a certain type of video decoding, e.g. "dv"
bool videoBase :: provides(const std::string name) {
  if(!m_pimpl)return false;
  unsigned int i;
  for(i=0; i<m_pimpl->m_providers.size(); i++)
    if(name == m_pimpl->m_providers[i])return true;

  return false;
}
std::vector<std::string>videoBase :: provides() {
  std::vector<std::string>result;
  if(m_pimpl) {
    unsigned int i;
    for(i=0; i<m_pimpl->m_providers.size(); i++)
      result.push_back(m_pimpl->m_providers[i]);
  }
  return result;
}


/////////////////////////////////////////////////////////
// remember that this backend provides a certain type of video decoding, e.g. "dv"
void videoBase :: provide(const std::string name) {
  if(!m_pimpl)return;
  if(!provides(name)) {
    m_pimpl->m_providers.push_back(name);
  }
}

bool videoBase :: enumProperties(gem::Properties&readable,
			     gem::Properties&writeable) 
{
  readable.clear();
  writeable.clear();
  return false;
}

void videoBase :: setProperties(gem::Properties&props) {
  // nada

  std::vector<std::string> keys=props.keys();
  unsigned int i=0;
  for(i=0; i<keys.size(); i++) {
    enum gem::Properties::PropertyType typ=props.type(keys[i]);
    //std::cerr  << "key["<<keys[i]<<"]: "<<typ<<" :: ";
    switch(typ) {
    case (gem::Properties::NONE):
      props.erase(keys[i]);
      break;
    case (gem::Properties::DOUBLE):
      //std::cerr << gem::any_cast<double>(props.get(keys[i]));
      break;
    case (gem::Properties::STRING):
      //std::cerr << "'" << gem::any_cast<std::string>(props.get(keys[i])) << "'";
      break;
    default:
      //std::cerr << "<unknown:" << props.get(keys[i]).get_type().name() << ">";
      break;
    }
  }
  //std::cerr << std::endl;
}

void videoBase :: getProperties(gem::Properties&props) {
  // nada
  std::vector<std::string>keys=props.keys();
  unsigned int i=0;
  for(i=0; i<keys.size(); i++) {
    gem::any unset;
    props.set(keys[i], unset);
  }
}


bool videoBase :: grabAsynchronous(bool fast) {
  return m_pimpl->setAsynchronous(fast);
}

bool videoBase :: isThreadable(void) {
  return (m_pimpl->numlocks>0);
}

