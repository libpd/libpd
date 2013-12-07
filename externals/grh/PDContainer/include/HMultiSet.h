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
// HMultiSet.h


#ifndef _h_multi_set_h__
#define _h_multi_set_h__

#include "include/SimpleBase.h"
#include <set>

using std::multiset;

//---------------------------------------------------
/* this is the class of the set
 */
class HMultiSet : 
public SimpleBase< multiset<Element>, multiset<Element>::iterator >
{

 private:

  /* Copy Construction is not allowed
   */
  HMultiSet(const HMultiSet &src)
    { }

  /* assignement operator is not allowed
   */
  const HMultiSet& operator = (const HMultiSet&)
    { return *this; }

 public:

  /* Constructor
   * no namespace
   */
  HMultiSet()
    { dataname_ = "h_multiset"; }

  /* Constructor
   * with a namespace
   */
  HMultiSet(string h_namespace)
    {
      dataname_ = "h_multiset";
      setNamespace(h_namespace);
    }

  /* Destructor
   */  
  virtual ~HMultiSet() { }

  /* add an element
   */
  virtual void add(Element key)
    {  data_[h_namespace_].insert(key);  }

  /* look if the element is set
   * returns how often this element was set 
   * 0 if it isn't set
   */
  virtual int get(const Element &key) const
    {  return data_[h_namespace_].count(key);  } 

  /* removes an element from the container
   */
  virtual void remove(const Element &key) const
      {  data_[h_namespace_].erase(key);  }
};


#endif   // _h_multi_set_h__
