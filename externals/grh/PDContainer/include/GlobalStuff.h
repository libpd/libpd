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
// GlobalStuff.h 


#ifndef _global_stuff_h___
#define _global_stuff_h___


#include "m_pd.h"
#include <string>
#include <sstream>
#include <fstream>
#include <iterator>
#include <list>
//#include <iostream> //DEBUG

using std::string;
using std::ostringstream;
using std::istringstream;
using std::ofstream;
using std::ifstream;
using std::endl;


// current version
#define PDC_VERSION   "0.2.1"


// TinyXML
//#define TIXML_USE_STL
#include "tinyxml/tinyxml.h"


//---------------------------------------------------
/* This function compares two pd t_atoms
 */
static bool compareAtoms(t_atom &atom1, t_atom &atom2)
{
  if(atom1.a_type == A_FLOAT && atom2.a_type == A_FLOAT)
    return (atom1.a_w.w_float == atom2.a_w.w_float);

  if(atom1.a_type == A_SYMBOL && atom2.a_type == A_SYMBOL)
    return (strcmp(atom1.a_w.w_symbol->s_name,
		   atom2.a_w.w_symbol->s_name) == 0);
  
  if(atom1.a_type == A_POINTER && atom2.a_type == A_POINTER)
    return (atom1.a_w.w_gpointer == atom2.a_w.w_gpointer);

  return false;
}


//---------------------------------------------------
/* one Element holds one data element, which can be 
 * a list, a float or a symbol
 */ 
class Element
{
 private:
  t_atom *atom;
  int length;

 public:
  Element() : atom(NULL), length(0) 
    { }

  Element(int size_, t_atom *atom_) : atom(NULL), length(0)
  {
    if(atom_ && size_)
    {
      length = size_;
      atom = (t_atom*)copybytes(atom_, length*sizeof(t_atom));
    }
  }

  // Copy Constr.    
  Element(const Element &src) : atom(NULL), length(0)
  {
    if(src.atom)
    {
      length = src.length;
      atom = (t_atom*)copybytes(src.atom, length*sizeof(t_atom));
    }
  }

  // Destructor
  ~Element()
  {
    if(atom)
      freebytes(atom, length*sizeof(t_atom));
  }
  
  // set atoms and length
  void setAtoms(int size_, t_atom *atom_)
  {
    if(atom)
    {
      freebytes(atom, length*sizeof(t_atom));
      length=0;
      atom=NULL;
    }

    if(atom_)
    {
      length = size_;
      atom = (t_atom*)copybytes(atom_, length*sizeof(t_atom));
    }
  }
  
  int getLength()
  { return length; }
  
  // shallow copy !!!
  t_atom *getAtom()
  { return atom; }

  //Assignement Operator
  const Element& operator = (const Element &src)
  {
    if(atom)
    {
      freebytes(atom, length*sizeof(t_atom));
      length=0;
      atom=NULL;
    }

    if(src.atom)
    {
      length = src.length;
      atom = (t_atom*)copybytes(src.atom, length*sizeof(t_atom));
    }

    return (*this);
  }

  // operator== to compare the objects
  bool operator== (const Element &key) const
    {
      if (length != key.length)
	return false;

      for (int i=0; i < length; i++)
	{
	  if(!compareAtoms(atom[i],key.atom[i]))
	    return false;
	}

      return true;
    }

  // operator< to compare the objects
  // (needed by map, set, ...)
  bool operator< (const Element &key) const
    {
      if (length == key.length)
	{
	  bool difference = false;
	  int index;

	  for (index = 0; index<length; index++)
	    {
	      if(!compareAtoms(atom[index],key.atom[index]))
		{
		  difference = true;
		  break;
		}
	    }
	  
	  // definition:
	  // A_FLOAT < A_SYMBOL < A_POINTER

	  if( atom[index].a_type == A_FLOAT 
	      && key.atom[index].a_type != A_FLOAT )
	    return true;

	  if( atom[index].a_type == A_SYMBOL )
	  { 
	    if( key.atom[index].a_type == A_FLOAT )
	      return false;
	    if( key.atom[index].a_type == A_POINTER )
	      return true;
	  }
	  
	  
	  // compare, when they are the same type:
	  
	  if( atom[index].a_type == A_POINTER
		     && key.atom[index].a_type != A_POINTER )
	    return false;

	  if( atom[index].a_type == A_FLOAT 
	      && key.atom[index].a_type == A_FLOAT )
	    return (atom[index].a_w.w_float < key.atom[index].a_w.w_float);

	  if( atom[index].a_type == A_SYMBOL 
	      && key.atom[index].a_type == A_SYMBOL )
	    return (strcmp(atom[index].a_w.w_symbol->s_name,
			   key.atom[index].a_w.w_symbol->s_name) < 0);
	  
	  if( atom[index].a_type == A_POINTER 
	      && key.atom[index].a_type == A_POINTER )
	    return (atom[index].a_w.w_gpointer < key.atom[index].a_w.w_gpointer);
	
	  return false;
	} // different length
      else
	return (length < key.length);

    }
};


#endif  //_global_stuff_h___
