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
// HDeque.h

#ifndef _h_deque_h__
#define _h_deque_h__


#include "include/SequBase.h"
#include <deque>

using std::deque;


//---------------------------------------------------
/* this is the class of the deque
 */
class HDeque : 
public SequBase< deque<Element>, deque<Element>::iterator >
{

 private:

  /* Copy Construction is not allowed
   */
  HDeque(const HDeque &src)
    { }

  /* assignement operator is not allowed
   */
  const HDeque& operator = (const HDeque&)
    { return *this; }

 public:

  /* Standard Constructor
   * no namespace
   */
  HDeque()
    { dataname_ = "h_deque"; }

  /* Constructor
   * with a namespace
   */
  HDeque(string h_namespace)
    {
      dataname_ = "h_deque";
      setNamespace(h_namespace);  
    }

  /* Destructor
   */
  virtual ~HDeque() { }

  /* inserts an element at the front of 
   * the container (so then the size
   * increases by one !!!)
   */
  virtual void pushFront(Element value)
    {  data_[h_namespace_].push_front(value);  }

  /* removes the element from the front of 
   * the container (so then the size
   * decreases by one !!!)
   */
  virtual void popFront()
    {  data_[h_namespace_].pop_front();  }
};



#endif //_h_deque_h__
