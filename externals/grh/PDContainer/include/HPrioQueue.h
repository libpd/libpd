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
// HPriority_Queue.h

#ifndef _h_priority_queue_h__
#define _h_priority_queue_h__


#include "include/QueueStack.h"
#include <queue>

using std::priority_queue;

class ElementPrio
{
 public:
  Element element;
  float priority;

  bool operator< (const ElementPrio &key) const
    {
      return (priority < key.priority);
    }
  const ElementPrio& operator = (const ElementPrio &src)
    {
      priority = src.priority;
      element = src.element;
      return (*this);
    }
};

//---------------------------------------------------
/* this is the class of the priority_queue
 */
class HPrioQueue : 
public QueueStack< priority_queue<ElementPrio>, int >
{

 private:

  /* Copy Construction is not allowed
   */
  HPrioQueue(const HPrioQueue &src)
    { }

  /* assignement operator is not allowed
   */
  const HPrioQueue& operator = (const HPrioQueue&)
    { return *this; }

 public:

  /* Standard Constructor
   * no namespace
   */
  HPrioQueue()
    { dataname_ = "h_priority_queue"; }

  /* Constructor
   * with a namespace
   */
  HPrioQueue(string h_namespace)
    {
      dataname_ = "h_priority_queue";
      setNamespace(h_namespace);
    }

  /* Destructor
   */
  virtual ~HPrioQueue() { }

  /* inserts an element in the container
   */
  virtual void push(float prio, Element value)
    { 
      ElementPrio data;
      data.priority = prio;
      data.element = value;
 
      data_[h_namespace_].push(data);
    }

  /* returns the element from the top of
   * the stack
   */
  virtual Element top() const 
    {  return data_[h_namespace_].top().element; }
};



#endif //_h_priority_queue_h__
