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
// HMultiMap.cpp


#include "include/HMultiMap.h"


//---------------------------------------------------
/* Get a value from the specific Key with the index number
 * index starts with 0
 * returns an element wich points to 0 if nothing was found !!!
 */
Element &HMultiMap::get(Element &key, int index) const
{
  int count = 0;

  for (multimap<Element,Element>::iterator it = data_[h_namespace_].begin();
       it != data_[h_namespace_].end(); ++it)
    {
      if ((*it).first == key)
	{
	  if(index == count++)
	    return ((*it).second);
	}
    }

  // throw an exception if nothing was found
  throw "h_multimap, get: Element not found !";
}
