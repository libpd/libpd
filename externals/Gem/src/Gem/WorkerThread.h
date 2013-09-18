/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    WorkerThread.h
       - part of GEM
       - baseclass for queueing/dequeueing workloads for threaded 

    Copyright (c) 2011-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_GEM_WORKERTHREAD_H_
#define _INCLUDE__GEM_GEM_WORKERTHREAD_H_

#include "Gem/ExportDef.h"

namespace gem { namespace thread {
    GEM_EXTERN class WorkerThread {
    private:
      class PIMPL;
      PIMPL*m_pimpl;
      friend class PIMPL;
      /* dummy implementations */
      WorkerThread(const WorkerThread&);
      WorkerThread&operator=(const WorkerThread&);
    public:
      WorkerThread(void);
      virtual ~WorkerThread(void);

      ////
      // start/stop thread(s)
      virtual bool start(void);
      virtual bool stop(bool wait=true);

      typedef unsigned int id_t;
      static const id_t INVALID, IMMEDIATE;

      // queue a 'data' chunk onto the TODO queue
      // the returned 'ID' can be used to interact with the queues
      // if queuing failed, FALSE is returned and ID is set to INVALID
      virtual bool queue(id_t&ID, void*data);

      //////
      // cancel a datachunk from the TODO-queue
      // if the chunk was successfully removed, returns TRUE 
      // (FALSE is returned, if e.g. the given datachunk was not found in the queue)
      // note that items already processed cannot be cancelled anymore
      virtual bool cancel(const id_t ID);

      // dequeue the next datachunk from the DONE queue
      // if the queue is empty, FALSE is returned and ID is set to INVALID
      virtual bool dequeue(id_t&ID, void*&data);

    protected:

      //// 
      // the worker!
      // get's called from an alternative thread(s)
      // when the queue is non-empty, 
      // the first element is removed from the TODO queue, 
      // and this function is called with the 1st element as data
      // the result returned is added to the done queue (alongside the ID)
      virtual void* process(id_t ID, void*data) = 0;

      ////
      // this get's called to indicate that new data is in the DONE queue
      // you can use it to set a semaphore in the main thread, to fetch
      // the data
      // it get's called once after process() has been successfull
      // and will nott be called before dequeue has been called at least once
      //
      virtual void signal(void);

    };};};


#endif /* _INCLUDE__GEM_GEM_WORKERTHREAD_H_ */
