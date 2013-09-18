// *********************(c)*2004*********************>
// -holzilib--holzilib--holzilib--holzilib--holzilib->
// ++++PD-External++by+Georg+Holzmann++grh@gmx.at++++>
//
// PDContainer: 
// this is a port of the containers from the C++ STL
// (Standard Template Library)
// for usage see the documentation and PD help files
// for license see readme.txt
//
// HQueueStack.h

#ifndef _h_queue_stack_h__
#define _h_queue_stack_h__


#include "include/ContainerBase.h"


//---------------------------------------------------
/* this is the class of the queue, stack and priority queue
 */
template <class ContainerType, class ContTypeIterator>
class QueueStack : public ContainerBase<ContainerType,ContTypeIterator>
{

 private:

  /* Copy Construction is not allowed
   */
  QueueStack(const QueueStack<ContainerType,ContTypeIterator> &src)
    { }

  /* assignement operator is not allowed
   */
  const QueueStack<ContainerType,ContTypeIterator>& operator = 
    (const QueueStack<ContainerType,ContTypeIterator>&)
    { }

 public:

  /* Standard Constructor
   * no namespace
   */
  QueueStack()
    { }

  /* Destructor
   */
  virtual ~QueueStack() { }

  /* removes the value from the top of
   * the container
   */
  virtual void pop()
    {  this->data_[this->h_namespace_].pop();  }
};



#endif //_h_queue_stack_h__
