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
// HMap.h


#ifndef _h_map_h__
#define _h_map_h__

#include "include/MapBase.h"

using std::map;

//---------------------------------------------------
/* this is the class of the map
 */
class HMap : 
public MapBase< map<Element,Element>, map<Element,Element>::iterator >
{
  // gcc4.0 food:
  //this->data_;
  //this->h_namespace_;
  //this->dataname_;

 private:

  /* Copy Construction is not allowed
   */
  HMap(const HMap &src)
    { }

  /* assignement operator is not allowed
   */
  const HMap& operator = (const HMap&)
    { return *this; }

 public:

  /* Constructor
   * no namespace
   */
  HMap()
    { dataname_ = "h_map"; }

  /* Constructor
   * with a namespace
   */
  HMap(string h_namespace)
    {
      dataname_ = "h_map";
      setNamespace(h_namespace);
    }

  /* Destructor
   */  
  virtual ~HMap() { }
};


#endif   // _h_map_h__
