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
// HQueue.h

#ifndef _h_queue_h__
#define _h_queue_h__


#include "include/QueueStack.h"
#include <queue>

using std::queue;


//---------------------------------------------------
/* this is the class of the queue
 */
class HQueue : 
public QueueStack< queue<Element>, int >
{

 private:

  /* Copy Construction is not allowed
   */
  HQueue(const HQueue &src)
    { }

  /* assignement operator is not allowed
   */
  const HQueue& operator = (const HQueue&)
    { return *this; }

 public:

  /* Standard Constructor
   * no namespace
   */
  HQueue()
    { dataname_ = "h_queue"; }

  /* Constructor
   * with a namespace
   */
  HQueue(string h_namespace)
    {
      dataname_ = "h_queue";
      setNamespace(h_namespace);  
    }

  /* Destructor
   */
  virtual ~HQueue() { }

  /* inserts an element in the container
   */
  virtual void push(Element value)
    {  data_[h_namespace_].push(value);  }

  /* returns the element from the top of
   * the stack
   */
  virtual Element &front() const 
    {  return data_[h_namespace_].front(); }
};



#endif //_h_queue_h__
