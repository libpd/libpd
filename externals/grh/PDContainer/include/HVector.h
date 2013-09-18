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
// HVector.h

#ifndef _h_vector_h__
#define _h_vector_h__


#include "include/SequBase.h"
#include <vector>

using std::vector;


//---------------------------------------------------
/* this is the class of the vector
 */
class HVector : 
public SequBase< vector<Element>, vector<Element>::iterator >
{

 private:

  /* Copy Construction is not allowed
   */
  HVector(const HVector &src)
    { }

  /* assignement operator is not allowed
   */
  const HVector& operator = (const HVector&)
    { return *this; }

 public:

  /* Standard Constructor
   * no namespace
   */
  HVector()
    { dataname_ = "h_vector"; }

  /* Constructor
   * with a namespace
   */
  HVector(string h_namespace)
    {
      dataname_ = "h_vector";
      setNamespace(h_namespace);  
    }

  /* Destructor
   */
  virtual ~HVector() { }
};



#endif //_h_vector_h__
