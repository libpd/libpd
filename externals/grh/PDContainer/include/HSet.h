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
// HSet.h


#ifndef _h_set_h__
#define _h_set_h__

#include "include/SimpleBase.h"
#include <set>

using std::set;

//---------------------------------------------------
/* this is the class of the set
 */
class HSet : 
public SimpleBase< set<Element>, set<Element>::iterator >
{

 private:

  /* Copy Construction is not allowed
   */
  HSet(const HSet &src)
    { }

  /* assignement operator is not allowed
   */
  const HSet& operator = (const HSet&)
    { return *this; }

 public:

  /* Constructor
   * no namespace
   */
  HSet()
    { dataname_ = "h_set"; }

  /* Constructor
   * with a namespace
   */
  HSet(string h_namespace)
    {
      dataname_ = "h_set";
      setNamespace(h_namespace); 
    }

  /* Destructor
   */  
  virtual ~HSet() { }

  /* add an element
   */
  virtual void add(Element key)
    {  data_[h_namespace_].insert(key);  }

  /* look if the element is set
   * returns 1 if it is set 
   * 0 if it isn't set
   */
  virtual int get(const Element &key) const
    {  return (data_[h_namespace_].find(key) 
	       != data_[h_namespace_].end());  }

  /* removes an element from the container
   */
  virtual void remove(const Element &key) const
    {  data_[h_namespace_].erase(key);  }
};


#endif   // _h_set_h__
