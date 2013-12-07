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
// HStack.h

#ifndef _h_stack_h__
#define _h_stack_h__


#include "include/QueueStack.h"
#include <stack>

using std::stack;


//---------------------------------------------------
/* this is the class of the stack
 */
class HStack : 
public QueueStack< stack<Element>, int >
{

 private:

  /* Copy Construction is not allowed
   */
  HStack(const HStack &src)
    { }

  /* assignement operator is not allowed
   */
  const HStack& operator = (const HStack&)
    { return *this; }

 public:

  /* Standard Constructor
   * no namespace
   */
  HStack()
    { dataname_ = "h_stack"; }

  /* Constructor
   * with a namespace
   */
  HStack(string h_namespace)
    {
      dataname_ = "h_stack";
      setNamespace(h_namespace);  
    }

  /* Destructor
   */
  virtual ~HStack() { }

  /* inserts an element in the container
   */
  virtual void push(Element value)
    {  data_[h_namespace_].push(value);  }

  /* returns the element from the top of
   * the stack
   */
  virtual Element &top() const 
    {  return data_[h_namespace_].top(); }
};



#endif //_h_stack_h__
