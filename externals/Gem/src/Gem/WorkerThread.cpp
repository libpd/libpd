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
#define WORKERTHREAD_DEQUEUE


#include "WorkerThread.h"
#ifdef WORKERTHREAD_DEQUEUE
# include <deque>
# define QUEUE std::deque
# define POP pop_front
# define PUSH push_back
#else
# include <queue>
# define QUEUE std::queue
# define POP pop
# define PUSH push
#endif

#include "ThreadMutex.h"
#include "ThreadSemaphore.h"

#include <pthread.h>
#if defined __linux__ || defined __APPLE__
# include <unistd.h>
# include <sys/time.h>
#endif
#ifdef _WIN32
# include <winsock2.h>
#endif

#include <iostream>

namespace gem { namespace thread {

  const WorkerThread::id_t WorkerThread::IMMEDIATE =  0;
  const WorkerThread::id_t WorkerThread::INVALID   = ~0;

  class WorkerThread::PIMPL {
  public:
    WorkerThread*owner;
    WorkerThread::id_t ID; /* for generating the next ID */

    bool keeprunning;
    bool isrunning;

    QUEUE< std::pair<WorkerThread::id_t, void*> > q_todo;
    QUEUE< std::pair<WorkerThread::id_t, void*> > q_done;
    Mutex m_todo;
    Mutex m_done;
    Semaphore s_newdata;

    WorkerThread::id_t processingID; /* the ID currently processed or INVALID: must only be written in the thread! */

    pthread_t p_thread;

    PIMPL(WorkerThread*x) : owner(x), ID(0),
                            keeprunning(true), isrunning(false),
                            m_todo(Mutex()), m_done(Mutex()),
                            s_newdata(Semaphore()),
                            processingID(WorkerThread::INVALID)
    {

    }
    ~PIMPL(void) {
      stop(true);
    }

    inline WorkerThread::id_t nextID(void) {
      ID++;
      while(ID == WorkerThread::IMMEDIATE || ID == WorkerThread::INVALID)
        ID++;
      return ID;
    }

    static inline void*process(void*you) {
      PIMPL*me=reinterpret_cast<PIMPL*>(you);
      WorkerThread*wt=me->owner;
      me->isrunning=true;
      std::pair <id_t, void*> in, out;

      while(me->keeprunning) {
        // wait till we are signalled new data

        me->m_todo.lock();
        if(me->q_todo.empty()) {
        empty:
          me->m_todo.unlock();
          //std::cerr << "THREAD: waiting for new data...freeze"<<std::endl;
          me->s_newdata.freeze();
          //std::cerr << "THREAD: waiting for new data...thawed "<<me->keeprunning<<std::endl;

          // either new data has arrived or we are told to stop
          if(!me->keeprunning)
            break;

          me->m_todo.lock();
        }
        if(me->q_todo.empty())
          goto empty;
        in=me->q_todo.front();
         me->processingID=in.first;
         me->q_todo.POP();
        me->m_todo.unlock();

        //std::cerr << "THREAD: processing data " << in.second  << " as "<<in.first<<std::endl;

        out.first = in.first;
        out.second=wt->process(in.first, in.second);

        //std::cerr << "THREAD: done data " << out.second  << " as "<<out.first<<std::endl;

        me->m_done.lock();
        bool newdata=true;//me->q_done.empty();
        //std::cerr<<"THREAD: processed "<< out.first <<" -> "<< newdata<<std::endl;
        me->q_done.PUSH(out);
        me->processingID=WorkerThread::INVALID;
        me->m_done.unlock();
        //std::cerr << "THREAD: signaling newdata "<<newdata<<" for "<< out.first << std::endl;
        if(newdata)wt->signal();
        //std::cerr << "THREAD: signalled" << std::endl;
      }
      //std::cerr << "THREAD: FINISHED" << std::endl;
      me->isrunning=false;
      return 0;
    }

    bool start(void) {
      if(isrunning)return true;
      
      keeprunning=true;
      pthread_create(&p_thread, 0, process, this);

      struct timeval sleep;
      while(!isrunning) {
        sleep.tv_sec=0;
        sleep.tv_usec=10;
        select(0,0,0,0,&sleep);
      }

      return true;

    }
    bool stop(bool wait=true) {
      if(!isrunning)return true;

      keeprunning=false;
      s_newdata.thaw();

      if(!wait)
        return (!isrunning);

      struct timeval sleep;
      while(isrunning) {
        sleep.tv_sec=0;
        sleep.tv_usec=10;
        select(0,0,0,0,&sleep);
        s_newdata.thaw();
      }
      return true;
    }

  };


  WorkerThread::WorkerThread(void) :
    m_pimpl(new PIMPL(this)) {
  }
  WorkerThread::~WorkerThread(void) {
    stop(true);

    delete m_pimpl;
    m_pimpl=0;
  }

  /* _private_ dummy implementations */
  WorkerThread&WorkerThread::operator=(const WorkerThread&org) {
    return (*this);
  }
  WorkerThread::WorkerThread(const WorkerThread&org) : m_pimpl(new PIMPL(this)) {
  }


  bool WorkerThread::start(void) {
    return m_pimpl->start();
  }
  bool WorkerThread::stop(bool wait) {
    return m_pimpl->stop(wait);
  }



  bool WorkerThread::queue(WorkerThread::id_t&ID, void*data) {
    std::pair <id_t, void*> DATA;
    DATA.second = data;

    m_pimpl->m_todo.lock();
    ID=m_pimpl->nextID();

    //std::cerr << "queuing data " << data  << " as "<<ID<<std::endl;
    if(ID==INVALID) {
      m_pimpl->m_todo.unlock();
      return false;
    }

    DATA.first = ID;
    m_pimpl->q_todo.PUSH(DATA);
    m_pimpl->m_todo.unlock();

    m_pimpl->s_newdata.thaw();
    //std::cerr << "new data thawed" << std::endl;
    return true;
  }
  bool WorkerThread::cancel(WorkerThread::id_t ID) {
    bool success=false;
#ifdef WORKERTHREAD_DEQUEUE
    if(!success) {
      /* cancel from TODO list */
      QUEUE< std::pair<WorkerThread::id_t, void*> > :: iterator it;
      //std::cerr << "cancelling "<< (int)ID <<" from TODO" << std::endl;
      m_pimpl->m_todo.lock();
      
      for(it=m_pimpl->q_todo.begin(); it!=m_pimpl->q_todo.end(); it++) {
        if(it->first == ID) {
          m_pimpl->q_todo.erase(it);
          success=true;
          break;
        }
      }
      m_pimpl->m_todo.unlock();

      /* TODO: if ID is currently in the process, cancel that as well ... */
      if(WorkerThread::INVALID != ID) {
        /* ... or at least block until it is done... */
        struct timeval sleep;
        while(ID==m_pimpl->processingID) {
          sleep.tv_sec=0;
          sleep.tv_usec=10;
          select(0,0,0,0,&sleep);
        }
      }
    }
    m_pimpl->m_todo.unlock();
#endif
    //    std::cerr << "cancelling "<< (int)ID <<" success " << success << std::endl;
    return success;
  }
  bool WorkerThread::dequeue(WorkerThread::id_t&ID, void*&data) {
    std::pair <id_t, void*> DATA;
    DATA.first=WorkerThread::INVALID;
    DATA.second=0;
    //std::cerr << "dequeuing "<< (int)ID << std::endl;
    m_pimpl->m_done.lock();
    if(!m_pimpl->q_done.empty()) {
      DATA=m_pimpl->q_done.front();
      m_pimpl->q_done.POP();
    }
    m_pimpl->m_done.unlock();

    ID=DATA.first;
    data=DATA.second;
    //std::cerr<<"dequeuing "<<data<<" as "<< ID<<std::endl;

    return (WorkerThread::INVALID != ID);
  }

  void WorkerThread::signal(void) {
    // nada
  }

};}; // } thread } gem
