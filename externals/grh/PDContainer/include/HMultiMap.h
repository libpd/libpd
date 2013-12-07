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
// HMultiMap.h


#ifndef _h_multi_map_h__
#define _h_multi_map_h__

#include "include/MapBase.h"

using std::multimap;

//---------------------------------------------------
/* this is the class of the map
 */
class HMultiMap : 
public MapBase< multimap<Element,Element>, multimap<Element,Element>::iterator >
{

 private:

  /* Copy Construction is not allowed
   */
  HMultiMap(const HMultiMap &src)
    { }

  /* assignement operator is not allowed
   */
  const HMultiMap& operator = (const HMultiMap&)
    { return *this; }

 public:

  /* Constructor
   * no namespace
   */
  HMultiMap()
    { dataname_ = "h_multimap"; }

  /* Constructor
   * with a namespace
   */
  HMultiMap(string h_namespace)
    {
      dataname_ = "h_multimap";
      setNamespace(h_namespace); 
    }

  /* Destructor
   */  
  virtual ~HMultiMap() { }
  
  /* Add a key-value pair
  */
  virtual void add(Element key, Element value)
  {
    this->data_[this->h_namespace_].insert(std::pair<Element,Element>(key,value));
  }

  /* Get the Nr. of values from the specific Key
   */
  virtual int getNr(Element &key) const
    { return data_[h_namespace_].count(key); }

  /* Get a value from the specific Key with the index number
   * index starts with 0
   */
  virtual Element &get(Element &key, int index) const;
};


#endif   // _h_multi_map_h__
