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
// HList.cpp


#include "include/HList.h"


//----------------------------------------------------
/* reads from an input file and adds the data to
 * the current namespace
 * Fileformat: see saveToFile
 * returns true on success
 */
bool HList::readFromFile(string filename)
{
  makeIterator();

  ifstream infile;
  infile.open(filename.c_str());

  if(!infile)
      return false;

  Element key;

  string line;
  bool go_on = false;
  char type;
  string symbol;
  t_float number;
  int key_count;

  while (getline(infile, line))
    {
      // first parse the instream, to get the number of atoms
      // (= size of the list)

      istringstream instream(line);
      ostringstream key_str("");
      
      go_on = false; key_count = 0;
      while(!go_on)
	{
	  instream >> type;
	  if (instream.eof())
	    {
	      go_on = true;
	      break;
	    }
	  if (type == 's')
	    {
	      key_count++;
	      instream >> symbol;
	      key_str << "s " << symbol;
	    }
	  if (type == 'f')
	    {
	      key_count++;
	      instream >> number;
	      key_str << "f " << number;
	    }
	  if (instream.eof())
	    go_on = true;
	  key_str << " ";
	}

      // now objects, parse again the data
      // into the objects and add them to the container

      t_atom *key_atom = (t_atom*)getbytes(key_count*sizeof(t_atom));
      if(key_atom == NULL)
	post("Fatal Error Out Of Memory (%s-readFromFile)",dataname_.c_str());

      istringstream key_istr(key_str.str());
 
      for(int i = 0; i < key_count; i++)
	{
	  key_istr >> type;
	  if (type == 's')
	    {
	      key_istr >> symbol;
	      SETSYMBOL(&key_atom[i],gensym(const_cast<char*>(symbol.c_str())));
	    }
	  if (type == 'f')
	    {
	      key_istr >> number;
	      SETFLOAT(&key_atom[i],number);
	    }
	}

	key.setAtoms(key_count,key_atom);

      // insert the data
      data_[h_namespace_].insert(iter_,key);
      
      freebytes(key_atom, key_count*sizeof(t_atom));
    }

  infile.close();

  return true;
}
