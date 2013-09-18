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
// ContainerBase.h 


#ifndef _container_base_h___
#define _container_base_h___


#include "include/GlobalStuff.h"
#include <map>

using std::map;


//---------------------------------------------------
/* this is the base class of all the containers
 */
template <class ContainerType, class ContTypeIterator>
class ContainerBase
{
 protected:

  /* this is the static container with all the datas
   * the string is the namespace of the Container
   */
  static map<string,ContainerType> data_;

  /* holds the number of the current namespace
   * (the string of the namespace is stored in the map above)
   */
  string h_namespace_;

  /* this string is the name of the datatype
   * (e.g. h_set, h_map, ...)
   */
  string dataname_;

 private:

  /* Copy Construction is not allowed
   */
  ContainerBase(const ContainerBase<ContainerType,ContTypeIterator> &src)
    { }

  /* assignement operator is not allowed
   */
  const ContainerBase<ContainerType,ContTypeIterator>& operator = 
    (const ContainerBase<ContainerType,ContTypeIterator>&)
    { }

 public:

  /* Standard Constructor
   * no namespace
   */
  ContainerBase()
    { }

  /* Destructor
   */
  virtual ~ContainerBase() 
    { } 

  /* sets the namespace variable
   */
  void setNamespace(string h_namespace) 
    {   
      h_namespace_ = h_namespace;
      //ContainerType container;
      //data_.insert(std::pair<string,ContainerType>(h_namespace_,container));
    }

  /* get the namespace string
   */ 
  string getNamespace() const
    {   return h_namespace_;   }

  /* prints out the help text
   */
  virtual void help();

  /* clears all the data of the current namespaces
   * in all objects from the same container
   */
  virtual void clearNamespace()
    { data_.erase(h_namespace_); }

  /* returns a reference to the whole Container
  * of the current namespace
   */
  virtual ContainerType &getAll()
  { return data_[h_namespace_]; }
  
  /* clears all the data of the current container
   * ( in all namespaces !!!!! )
   * so be carefull !!!
   */
  virtual void clearAll()
    { data_.clear(); }

  /* get the size of the container
   */
  virtual int getSize() const
    {  return data_[h_namespace_].size();  }
};


//---------------------------------------------------
/* defines the static members
 */
template<class ContainerType, class ContTypeIterator>
map<string,ContainerType> ContainerBase<ContainerType,ContTypeIterator>::data_;

//---------------------------------------------------
/* prints out the help text
 */
template<class ContainerType, class ContTypeIterator>
void ContainerBase<ContainerType,ContTypeIterator>::help()
{
  post("\nPD-Container, Version: "PDC_VERSION"");
  post("object: %s",dataname_.c_str());
  post("------------------------------------------");
  post("this is an implementation of the container");
  post("objects from the Standard Template");
  post("Library (STL) of C++");
  post("for documentation see the help patches");
  post("(by Georg Holzmann <grh@mur.at>, 2004-2005)");
  post("------------------------------------------\n");
}



#endif //_container_base_h___
