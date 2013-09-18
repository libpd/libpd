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

#include "SynchedWorkerThread.h"
#include "Gem/RTE.h"
#include "ThreadMutex.h"

namespace gem { namespace thread {

  class SynchedWorkerThread::PIMPL {
  public:
    SynchedWorkerThread*owner;
    t_clock*clock;

    Mutex m_flag;
    bool flag;

    Mutex m_polling;
    bool polling;

    PIMPL(SynchedWorkerThread*x) : owner(x), clock(NULL), 
                                   m_flag(Mutex()), flag(false),
                                   m_polling(Mutex()), polling(false)
    {
      clock=clock_new(this, reinterpret_cast<t_method>(tickCb));
    }
    ~PIMPL(void) {
      if(clock)
        clock_free(clock);
    }

   unsigned int dequeue(void) {
      id_t ID=WorkerThread::IMMEDIATE;
      void*data=0;
      unsigned int counter=0;
      WorkerThread*wt=owner;
      if(!owner)return 0;

      while(wt->dequeue(ID, data)) {
        owner->done(ID, data);
        counter++;
      };
      return counter;
    }

    static void tickCb(void*you) {
      PIMPL*me=reinterpret_cast<PIMPL*>(you);
      me->tick();
    }
    void tick(void) {
      dequeue();

      m_flag.lock();
      flag=false;
      m_flag.unlock();
    }

    void tack(void) {
      m_polling.lock();
      bool poll=polling;
      m_polling.unlock();
      if(poll)return;

      m_flag.lock();
      bool done=flag;
      flag=true;
      m_flag.unlock();
      // already flagged
      if(done)return;

      // this will lock forever is Pd does not idle, ouch!!
      sys_lock();
      clock_delay(clock, 0);
      sys_unlock();
    }

    bool setPolling(bool poll) {
      m_polling.lock();
      polling=poll;
      m_polling.unlock();

      // just in case tack() is still hanging
      // this is really ugly! 
      // it might only work if called from the main thread
      if(sys_trylock()) {
        // system was locked, so unlock and re-lock
        sys_unlock();
        sys_trylock();
      } else {
        // system wasn't locked, but is now, so unlock again
        sys_unlock();
      }

      return polling;
    }
  };


  SynchedWorkerThread::SynchedWorkerThread(bool autostart) :
    m_pimpl(new PIMPL(this)) {
    if(autostart)
	start();
  }

  SynchedWorkerThread::~SynchedWorkerThread(void) {
    delete m_pimpl;
    m_pimpl=0;
  }

  unsigned int SynchedWorkerThread::dequeue(void) {
    return m_pimpl->dequeue();
  }

  bool SynchedWorkerThread::setPolling(bool value) {
    return m_pimpl->setPolling(value);
  }


  void SynchedWorkerThread::signal(void) {
    m_pimpl->tack();
  }

  /* _private_ dummy implementations */
  SynchedWorkerThread&SynchedWorkerThread::operator=(const SynchedWorkerThread&org) {
    return (*this);
  }
  SynchedWorkerThread::SynchedWorkerThread(const SynchedWorkerThread&org) : m_pimpl(new PIMPL(this)) {
  }

};}; // } thread } gem
