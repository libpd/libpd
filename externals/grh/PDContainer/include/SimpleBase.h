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
// SimpleBase.h

#ifndef _simple_base_h__
#define _simple_base_h__


#include "include/ContainerBase.h"


//---------------------------------------------------
/* this is the base class of all simple containers
 */
template <class ContainerType, class ContTypeIterator>
class SimpleBase : public ContainerBase<ContainerType,ContTypeIterator>
{

 private:

  /* Copy Construction is not allowed
   */
  SimpleBase(const SimpleBase<ContainerType,ContTypeIterator> &src)
    { }

  /* assignement operator is not allowed
   */
  const SimpleBase<ContainerType,ContTypeIterator>& operator = 
    (const SimpleBase<ContainerType,ContTypeIterator>&)
    { }

 public:

  /* Standard Constructor
   * no namespace
   */
  SimpleBase()
    { }

  /* Destructor
   */
  virtual ~SimpleBase() { };

  /* prints all the data of the current namespace to the console
   */
  virtual void printAll();

  /* prints all the data of the current namespace to the console
   * (with the index as prefix)
   */
  virtual void printAllIndex();

  /* saves all the data of the current namespace to a file
   * Fileformat: 
   * <data_atom_type1> <data_atom1> <data_atom_type2> <data_atom2> ...
   * e.g.:
   * f 1 f 2 f 4
   * s foo f 12.34234
   * types: s=symbol, f=float
   * ATTENTION: if the file exists, all the old data of
   * the file is lost
   * returns true on success
   */
  virtual bool saveToFile(string filename);
  
  /* saves all the data of the current namespace to a XML file
   * ATTENTION: if the file exists, all the old data of
   * the file is lost
   * returns true on success
   */
  virtual bool saveToFileXML(string filename);

  /* reads from an input file and adds the data to
   * the current namespace
   * Fileformat: see saveToFile
   * returns true on success
   */
  virtual bool readFromFile(string filename);
  
  /* reads from an input XML file and adds the data to
   * the current namespace
   * returns true on success
   */
  virtual bool readFromFileXML(string filename);
};


//----------------------------------------------------
/* prints all the data of the current namespace to the console
 */
template<class ContainerType, class ContTypeIterator>
void SimpleBase<ContainerType,ContTypeIterator>::printAll()
{
  ContTypeIterator iter  = this->data_[this->h_namespace_].begin();

  post("\n%s: printing namespace %s",this->dataname_.c_str(),this->h_namespace_.c_str());
  post("--------------------------------------------------");

  bool data_here = false;
  while(iter != this->data_[this->h_namespace_].end())
    {
      ostringstream output("");

      Element key((*iter));
      if(key.getLength() == 0)
	{
	  iter++;
	  continue;
	}

      if (key.getLength() > 1)  // list
  {
	  output << "list ";
	  for (int i=0; i < key.getLength(); i++)
	    {
	      if (key.getAtom()[i].a_type == A_FLOAT)
		output << key.getAtom()[i].a_w.w_float << " ";
	      if (key.getAtom()[i].a_type == A_SYMBOL)
		output << key.getAtom()[i].a_w.w_symbol->s_name << " ";
	      if (key.getAtom()[i].a_type == A_POINTER)
		output << "(gpointer)" << key.getAtom()[i].a_w.w_gpointer << " ";
	    }
	}
      else // no list
	{
	  if (key.getAtom()[0].a_type == A_FLOAT)
	    output << "float " << key.getAtom()[0].a_w.w_float << " ";
	  if (key.getAtom()[0].a_type == A_SYMBOL)
	    output << "symbol " 
		   << key.getAtom()[0].a_w.w_symbol->s_name << " ";
	  if (key.getAtom()[0].a_type == A_POINTER)
	    output << "pointer " << key.getAtom()[0].a_w.w_gpointer << " ";
	}
      
      post("%s",output.str().c_str());
      data_here = true;
	
      iter++;
    }
  if(!data_here)
    post("no data in current namespace!");
  post("--------------------------------------------------");
}

//----------------------------------------------------
/* prints all the data of the current namespace to the console
 * (with the index as prefix)
 */
template<class ContainerType, class ContTypeIterator>
void SimpleBase<ContainerType,ContTypeIterator>::printAllIndex()
{
  ContTypeIterator iter  = this->data_[this->h_namespace_].begin();

  post("\n%s: printing namespace %s",this->dataname_.c_str(),this->h_namespace_.c_str());
  post("--------------------------------------------------");

  bool data_here = false; int i=0;
  while(iter != this->data_[this->h_namespace_].end())
    {
      ostringstream output("");

      Element key((*iter));
      if(key.getLength() == 0)
	{
	  iter++; i++;
	  continue;
	}

      if (key.getLength() > 1)  // list
	{
	  output << "list ";
	  for (int i=0; i < key.getLength(); i++)
	    {
	      if (key.getAtom()[i].a_type == A_FLOAT)
		output << key.getAtom()[i].a_w.w_float << " ";
	      if (key.getAtom()[i].a_type == A_SYMBOL)
		output << key.getAtom()[i].a_w.w_symbol->s_name << " ";
	      if (key.getAtom()[i].a_type == A_POINTER)
		output << "(gpointer)" << key.getAtom()[i].a_w.w_gpointer << " ";
	    }
	}
      else // no list
	{
	  if (key.getAtom()[0].a_type == A_FLOAT)
	    output << "float " << key.getAtom()[0].a_w.w_float << " ";
	  if (key.getAtom()[0].a_type == A_SYMBOL)
	    output << "symbol " 
		   << key.getAtom()[0].a_w.w_symbol->s_name << " ";
	  if (key.getAtom()[0].a_type == A_POINTER)
	    output << "pointer " << key.getAtom()[0].a_w.w_gpointer << " ";
	}
      
      post("%d: %s",i,output.str().c_str());
      data_here = true;
	
      iter++; i++;
    }
  if(!data_here)
    post("no data in current namespace!");
  post("--------------------------------------------------");
}

//----------------------------------------------------
/* saves all the data of the current namespace to a file
 * Fileformat: 
 * <key_atom_type1> <key_atom1> ... -  <data_atom_type1> <data_atom1> ...
 * e.g.:
 * f 1 f 2 - f 4
 * s foo f 12.34234 - f 3 f 5 s gege
 * types: s=symbol, f=float
 * ATTENTION: if the file exists, all the old data of
 * the file is lost
 * returns true on success
 */
template<class ContainerType, class ContTypeIterator>
bool SimpleBase<ContainerType,ContTypeIterator>::saveToFile(string filename)
{
  ofstream outfile;
  ContTypeIterator iter  = this->data_[this->h_namespace_].begin();

  outfile.open(filename.c_str());

  if(!outfile)
    return false;

  while(iter != this->data_[this->h_namespace_].end())
    {	  
      Element key((*iter));
      bool have_pointer = false;
      
      // check for pointers first
      for (int i=0; i < key.getLength(); i++)
        if (key.getAtom()[i].a_type == A_POINTER)
	  have_pointer = true;
      
      if(have_pointer)
      {
	post("PDContainer warning: pointers can't be saved and are ignored !!!");
	iter++;
      }
      else
      {
      
      for (int i=0; i < key.getLength(); i++)
	{
	  if (key.getAtom()[i].a_type == A_FLOAT)
	    outfile << "f " << key.getAtom()[i].a_w.w_float << " ";
	  if (key.getAtom()[i].a_type == A_SYMBOL)
	    outfile << "s " << key.getAtom()[i].a_w.w_symbol->s_name << " ";
	}
      
      outfile << endl;
      iter++;
      
      }
    }

  outfile.close();

  return true;
}

//----------------------------------------------------
/* saves all the data of the current namespace to a XML file
 * ATTENTION: if the file exists, all the old data of
 * the file is lost
 * returns true on success
 */
template<class ContainerType, class ContTypeIterator>
    bool SimpleBase<ContainerType,ContTypeIterator>::saveToFileXML(string filename)
{
  ostringstream output("");
  ContTypeIterator iter  = this->data_[this->h_namespace_].begin();

  // add XML Header:
  output << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>\n"
      << "<!DOCTYPE PDContainer SYSTEM "
      << "\"http://grh.mur.at/software/pdcontainer_simple.dtd\">\n"
      << "<PDContainer type=\"" << this->dataname_ << "\">\n";
  
  
  while(iter != this->data_[this->h_namespace_].end())
  {
    // add Element:
    Element el((*iter));
    bool have_pointer = false;
      
    // check for pointers first
    for (int i=0; i < el.getLength(); i++)
      if (el.getAtom()[i].a_type == A_POINTER)
	have_pointer = true;
    
    if(have_pointer)
    {
      post("PDContainer warning: pointers can't be saved and are ignored !!!");
      iter++;
    }
    else
    {
      
    output << "<element>\n";
    
    for (int i=0; i < el.getLength(); i++)
    {
      if (el.getAtom()[i].a_type == A_FLOAT)
        output << "<f> " << el.getAtom()[i].a_w.w_float << " </f>\n";
      if (el.getAtom()[i].a_type == A_SYMBOL)
        output << "<s>" << el.getAtom()[i].a_w.w_symbol->s_name << " </s>\n";
    }
    
    output << "</element>\n";
     
    iter++;
    
    }
  }

  output << "</PDContainer>\n";
  
  // now write to file:
  TiXmlDocument outfile( filename.c_str() );
  outfile.Parse( output.str().c_str() );

  if ( outfile.Error() ) return false;
  
  outfile.SaveFile();

  return true;
}

//----------------------------------------------------
/* reads the data from the file into the current
 * namespace
 * Fileformat: see saveToFile
 * returns true on success
 */
template<class ContainerType, class ContTypeIterator>
bool SimpleBase<ContainerType,ContTypeIterator>::readFromFile(string filename)
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
      this->data_[this->h_namespace_].insert(this->data_[this->h_namespace_].end(),key);
      
      freebytes(key_atom, key_count*sizeof(t_atom));
    }

  infile.close();

  return true;
}

//----------------------------------------------------
/* reads the data from th XML file into the current
 * namespace
 * returns true on success
 */
template<class ContainerType, class ContTypeIterator>
    bool SimpleBase<ContainerType,ContTypeIterator>::readFromFileXML(string filename)
{
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
    
    // insert it in the container
    this->data_[this->h_namespace_].insert(this->data_[this->h_namespace_].end(),el);

    freebytes(el_atom, atoms*sizeof(t_atom));
    
    parsed = true;
  }
  return parsed;
}



#endif  // _simple_base_h__
