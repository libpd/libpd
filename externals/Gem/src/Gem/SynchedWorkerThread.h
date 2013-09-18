/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    Synchedworkerthread.h
       - part of GEM
       - a worker thread that automatically dequeues in the main thread

    Copyright (c) 2011-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_GEM_SYNCHEDWORKERTHREAD_H_
#define _INCLUDE__GEM_GEM_SYNCHEDWORKERTHREAD_H_

#include "Gem/WorkerThread.h"

namespace gem { namespace thread {
    GEM_EXTERN class SynchedWorkerThread : public WorkerThread {
		  private:
      class PIMPL;
      PIMPL*m_pimpl;
      friend class PIMPL;
      /* dummy implementations */
      SynchedWorkerThread(const SynchedWorkerThread&);
      SynchedWorkerThread&operator=(const SynchedWorkerThread&);

		  public:
      SynchedWorkerThread(bool autostart=true);
      virtual ~SynchedWorkerThread(void);

      /*
       * turn on "polling" mode
       * when in polling mode, the calling thread has to call 'dequeue()' in order to
       * deqeue any DONE data
       * when in pushing mode, the data is pushed automatically within the RTE main thread
       *
       * returns TRUE is now in polling mode, or FALSE if now in pushing mode
       * (might be different from what was requested)
       *
       * this MUST be called from the main thread
       */
      virtual bool setPolling(bool value=true);

      /**
       * deqeues the entire DONE queue
       * returns the number of elements dequeued
       */
      virtual unsigned int dequeue(void);

		  protected:
      // this get's called from the main thread(!) with each
      // finished data chunk
      virtual void done(id_t ID, void*data) = 0;

      //////
      // tell RTE to call back asap
      virtual void signal(void);

    };};};


#endif /* _INCLUDE__GEM_GEM_SYNCHEDWORKERTHREAD_H_ */
