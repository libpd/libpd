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
// SequBase.h

#ifndef _sequ_base_h__
#define _sequ_base_h__


#include "include/SimpleBase.h"


//---------------------------------------------------
/* this is the base class of vector and deque
 */
template <class ContainerType, class ContTypeIterator>
class SequBase : public SimpleBase<ContainerType,ContTypeIterator>
{

 private:

  /* Copy Construction is not allowed
   */
  SequBase(const SequBase<ContainerType,ContTypeIterator> &src)
    { }

  /* assignement operator is not allowed
   */
  const SequBase<ContainerType,ContTypeIterator>& operator = 
    (const SequBase<ContainerType,ContTypeIterator>&)
    { }

 public:

  /* Standard Constructor
   * no namespace
   */
  SequBase()
    { }

  /* Destructor
   */
  virtual ~SequBase() { };

  /* change the element at the index
   */
  virtual void set(int index, Element value) 
    {  this->data_[this->h_namespace_][index] = value;  }

  /* get the element from the index
   */
  virtual Element &get(int index) const 
    {  return this->data_[this->h_namespace_][index]; }

  /* resize the sequence
   */
  virtual void resize(int size)
    {  this->data_[this->h_namespace_].resize(size);  }

  /* inserts an element at the end of 
   * the container (so then the size
   * increases by one !!!)
   */
  virtual void pushBack(Element value)
    {  this->data_[this->h_namespace_].push_back(value);  }

  /* removes the element from the end of 
   * the container (so then the size
   * decreases by one !!!)
   */
  virtual void popBack()
    {  this->data_[this->h_namespace_].pop_back();  }

  /* returns the last element
   */
  virtual Element &back() const 
    {  return this->data_[this->h_namespace_].back(); }

  /* returns the first element
   */
  virtual Element &front() const 
    {  return this->data_[this->h_namespace_].front(); }

  /* inserts an element before the element
   * with the given index
   */
  virtual void insert(int index, Element value)
    {  this->data_[this->h_namespace_].insert(this->data_[this->h_namespace_].begin()+index, value);  }

  /* removes the element with that index from the
   * container
   */
  virtual void remove(int index)
    {  this->data_[this->h_namespace_].erase(this->data_[this->h_namespace_].begin()+index);  }

  /* reads from an input file and adds the data to
   * the current namespace
   * Fileformat: see saveToFile
   * index: inserts the data starting with this index
   * returns true on success
   */
  virtual bool readFromFile2(string filename, int index);
  
  /* reads from a XML input file and adds the data to
  * the current namespace
  * index: inserts the data starting with this index
  * returns true on success
  */
  virtual bool readFromFile2XML(string filename, int index);  
};


//----------------------------------------------------
/* reads the data from the file into the current
 * namespace
 * Fileformat: see saveToFile
 * index: inserts the data starting with this index
 * returns true on success
 */
template<class ContainerType, class ContTypeIterator>
bool SequBase<ContainerType,ContTypeIterator>::readFromFile2(string filename, int index)
{
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
  int max_index = this->data_[this->h_namespace_].size();

  while (getline(infile, line))
    {
      // first parse the instream, to get the number of atoms
      // (= size of the list)

      if(index < 0 || index >= max_index)
	{
	  post("%s, read: wrong index !!",this->dataname_.c_str());
	  return false;
	}

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
	post("Fatal Error Out Of Memory (%s-readFromFile)",this->dataname_.c_str());

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
      this->data_[this->h_namespace_][index] = key;
      index++;
      
      freebytes(key_atom, key_count*sizeof(t_atom));
    }

  infile.close();

  return true;
}

//----------------------------------------------------
/* reads the data from the XML file into the current
 * namespace
 * index: inserts the data starting with this index
 * returns true on success
 */
template<class ContainerType, class ContTypeIterator>
    bool SequBase<ContainerType,ContTypeIterator>::readFromFile2XML(string filename, int index)
{
  int max_index = this->data_[this->h_namespace_].size();
  if(index < 0 || index >= max_index)
  {
    post("%s, read: wrong index !!",this->dataname_.c_str());
    return false;
  }
  
  
  TiXmlDocument doc( filename.c_str() );
  
  if( !doc.LoadFile() ) return false;

  TiXmlNode *parent = 0;
  TiXmlElement *child1 = 0;
  TiXmlElement *child2 = 0;
  
  t_atom *el_atom = 0;
  Element el;
  t_float f;
  bool parsed=false;

      
	// Get the <PDContainer> tag and check type
  parent = doc.FirstChild( "PDContainer" );
  if(!parent) return false;
  
  if(!parent->ToElement()) return false;
  if(!parent->ToElement()->Attribute("type"))
  {
    post("readXML: you must specify an attribute type in <PDContainer> !");
    return false;
  }
  
  string type(parent->ToElement()->Attribute("type"));
  
  if( type != "h_vector" && type != "h_list" && type != "h_deque" &&
      type != "h_set" && type != "h_multiset")
  {
    post("readXML: wrong container type (attribute type in <PDContainer>) !");
    return false;
  }
  
  if( type != this->dataname_ )
    post("readXML: importing data from %s!", type.c_str() );
  
  // iterate through all the <element> tags
  for( child1 = parent->FirstChildElement("element"); child1; 
       child1 = child1->NextSiblingElement("element") )
  {
    // get nr of atoms and allocate mem for them
    // (if its a pd list)
    int atoms = 0;
    for( child2 = child1->FirstChildElement(); child2; 
         child2 = child2->NextSiblingElement() )
      atoms++;
    
    el_atom = (t_atom*)getbytes(atoms*sizeof(t_atom));

    if(el_atom == NULL)
    {
      post("Fatal Error Out Of Memory (%s-readFromFile)",this->dataname_.c_str());
      return false;
    }   
    
    // iterate through all the atoms of one <element>
    atoms = 0;
    for( child2 = child1->FirstChildElement(); child2; 
         child2 = child2->NextSiblingElement() )
    {
      string tag(child2->Value());
      
      if(!child2->FirstChild()) continue;     
      istringstream in(child2->FirstChild()->Value());
      
      if(tag == "f" || tag == "float")
      {
        in >> f;
        SETFLOAT(&el_atom[atoms], f);
      }
      if(tag == "s" || tag == "symbol")
      {
        SETSYMBOL(&el_atom[atoms],
                   gensym(const_cast<char*>(in.str().c_str())));
      }
      
      atoms++;
    }
    
    if(!atoms) continue;
    
    
    // add the element to the container
    el.setAtoms(atoms,el_atom);
    
    // insert the data
    this->data_[this->h_namespace_][index] = el;
    index++;

    freebytes(el_atom, atoms*sizeof(t_atom));
    
    parsed = true;
  }
  return parsed;

}



#endif //_sequ_base_h__
