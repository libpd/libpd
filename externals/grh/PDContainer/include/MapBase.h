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
// MapBase.h

#ifndef _map_base_h__
#define _map_base_h__

#include "include/ContainerBase.h"


//---------------------------------------------------
/* this is the base class of map and multimap
 */
template <class ContainerType, class ContTypeIterator>
class MapBase : public ContainerBase<ContainerType,ContTypeIterator>
{

 private:

  /* Copy Construction is not allowed
   */
  MapBase(const MapBase<ContainerType,ContTypeIterator> &src)
    { }

  /* assignement operator is not allowed
   */
  const MapBase<ContainerType,ContTypeIterator>& operator = 
    (const MapBase<ContainerType,ContTypeIterator>&)
    { }

 public:

  /* Standard Constructor
   * no namespace
   */
  MapBase()
    { }

  /* Destructor
   */
  virtual ~MapBase() { };

  /* Add a key-value pair
   */
  virtual void add(Element key, Element value)
  {
    // first remove old entry, then insert
    this->data_[this->h_namespace_].erase(key);
    this->data_[this->h_namespace_].insert(std::pair<Element,Element>(key,value));
  }

  /* Get a value from the specific Key
   */
  virtual Element &get(const Element &key) const
    {
      ContTypeIterator iter = this->data_[this->h_namespace_].find(key);

      // key was not found if iterator is pointing to the end
      if(iter == this->data_[this->h_namespace_].end())
        throw "PDContainer, get: Element not found !";

      return (*iter).second;
    }

  /* removes a pair with this key
   */
  virtual void remove(const Element &key)
    {
      if(!this->data_[this->h_namespace_].erase(key))
        throw "PDContainer, remove: Element not found !";
    }

  /* prints all the data of the current namespace to the console
   */
  virtual void printAll();

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
  
  /* reads from an XML input file and adds the data to
   * the current namespace
   * returns true on success
   */
  virtual bool readFromFileXML(string filename);
};

//----------------------------------------------------
/* prints all the data of the current namespace to the console
 */
template<class ContainerType, class ContTypeIterator>
void MapBase<ContainerType,ContTypeIterator>::printAll()
{
  ContTypeIterator iter  = this->data_[this->h_namespace_].begin();

  post("\n%s: printing namespace %s",this->dataname_.c_str(),this->h_namespace_.c_str());
  post("--------------------------------------------------");

  bool data_here = false;
  while(iter != this->data_[this->h_namespace_].end())
    {
      ostringstream output("");

      // Key:
      Element key = (*iter).first;
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
      
      // Value:
      output << "  --  ";
      Element el = (*iter).second;
      if (el.getLength() > 1)  // list
	{
	  output << "list ";
	  for (int i=0; i < el.getLength(); i++)
	    {
	      if (el.getAtom()[i].a_type == A_FLOAT)
		output << el.getAtom()[i].a_w.w_float << " ";
	      if (el.getAtom()[i].a_type == A_SYMBOL)
		output << el.getAtom()[i].a_w.w_symbol->s_name << " ";
	      if (el.getAtom()[i].a_type == A_POINTER)
		output << "(gpointer)" << el.getAtom()[i].a_w.w_gpointer << " ";
	    }
	}
      else // no list
	{
	  if (el.getAtom()[0].a_type == A_FLOAT)  // hier segfault nach get !!!
	    output << "float " << el.getAtom()[0].a_w.w_float << " ";
	  if (el.getAtom()[0].a_type == A_SYMBOL)
	    output << "symbol " 
		   << el.getAtom()[0].a_w.w_symbol->s_name << " ";
	  if (el.getAtom()[0].a_type == A_POINTER)
	    output << "pointer " << el.getAtom()[0].a_w.w_gpointer << " ";
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
bool MapBase<ContainerType,ContTypeIterator>::saveToFile(string filename)
{
  ofstream outfile;
  ContTypeIterator iter  = this->data_[this->h_namespace_].begin();
  
  outfile.open(filename.c_str());
  
  if(!outfile)
    return false;

  while(iter != this->data_[this->h_namespace_].end())
    {
      Element key = (*iter).first;
      Element el = (*iter).second;
      bool have_pointer = false;
      
      // check if there is a pointer and then don't store it
      for (int i=0; i < key.getLength(); i++)
	if (key.getAtom()[i].a_type == A_POINTER)
	 have_pointer=true;
      for (int i=0; i < el.getLength(); i++)
	if (el.getAtom()[i].a_type == A_POINTER) 
	  have_pointer=true;
      
      if(have_pointer)
      {
	post("PDContainer: will not store pointers !!!");
	iter++;
      }
      else
      {
      // add key:
      for (int i=0; i < key.getLength(); i++)
	{
	  if (key.getAtom()[i].a_type == A_FLOAT)
	    outfile << "f " << key.getAtom()[i].a_w.w_float << " ";
	  if (key.getAtom()[i].a_type == A_SYMBOL)
	    outfile << "s " << key.getAtom()[i].a_w.w_symbol->s_name << " ";
	}

      outfile << "- ";

      // add Value:
      for (int i=0; i < el.getLength(); i++)
	{
	  if (el.getAtom()[i].a_type == A_FLOAT)
	    outfile << "f " << el.getAtom()[i].a_w.w_float << " ";
	  if (el.getAtom()[i].a_type == A_SYMBOL)
	    outfile << "s " << el.getAtom()[i].a_w.w_symbol->s_name << " ";
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
    bool MapBase<ContainerType,ContTypeIterator>::saveToFileXML(string filename)
{
  ostringstream output("");
  ContTypeIterator iter  = this->data_[this->h_namespace_].begin();

  // add XML Header:
  output << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>\n"
      << "<!DOCTYPE PDContainer SYSTEM "
      << "\"http://grh.mur.at/software/pdcontainer_multi.dtd\">\n"
      << "<PDContainer type=\"" << this->dataname_ << "\">\n";
    
  while(iter != this->data_[this->h_namespace_].end())
  {
    Element key((*iter).first);
    Element el((*iter).second);
    bool have_pointer = false;
    
    // check if there is a pointer and then don't store it
    for (int i=0; i < key.getLength(); i++)
      if (key.getAtom()[i].a_type == A_POINTER)
	have_pointer=true;
    for (int i=0; i < el.getLength(); i++)
      if (el.getAtom()[i].a_type == A_POINTER) 
	have_pointer=true;
    
    if(have_pointer)
    {
      post("PDContainer: will not store pointers !!!");
      iter++;
    }
    else
    {
    
    output << "<element>\n";  

    // add Key:  
    output << "<key>\n";
    
    for (int i=0; i < key.getLength(); i++)
    {
      if (key.getAtom()[i].a_type == A_FLOAT)
        output << "<f> " << key.getAtom()[i].a_w.w_float << " </f>\n";
      if (key.getAtom()[i].a_type == A_SYMBOL)
        output << "<s>" << key.getAtom()[i].a_w.w_symbol->s_name << " </s>\n";
    }
    
    output << "</key>\n";
      

    // add Value:
    output << "<value>\n";
    
    for (int i=0; i < el.getLength(); i++)
    {
      if (el.getAtom()[i].a_type == A_FLOAT)
        output << "<f> " << el.getAtom()[i].a_w.w_float << " </f>\n";
      if (el.getAtom()[i].a_type == A_SYMBOL)
        output << "<s>" << el.getAtom()[i].a_w.w_symbol->s_name << " </s>\n";
    }
    
    output << "</value>\n";
    
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
bool MapBase<ContainerType,ContTypeIterator>::readFromFile(string filename)
{
  ifstream infile;
  infile.open(filename.c_str());

  if(!infile)
      return false;

  Element key;
  Element el;

  string line;
  bool go_on = false;
  char type;
  string symbol;
  t_float number;
  int key_count, val_count;

  while (getline(infile, line))
    {
      // first parse the instream, to get the number of atoms
      // (= length of the elements)

      istringstream instream(line);
      ostringstream key_str("");
      ostringstream value_str("");

      // Key:
      go_on = false; key_count = 0;
      while(!go_on)
	{
	  instream >> type;
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
	  if (type == '-')
	    go_on = true;
	  key_str << " ";
	}

      // Value:
      go_on = false; val_count = 0;
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
	      val_count++;
	      instream >> symbol;
	      value_str << "s " << symbol;
	    }
	  if (type == 'f')
	    {
	      val_count++;
	      instream >> number;
	      value_str << "f " << number;
	    }
	  if (instream.eof())
	    go_on = true;
	  value_str << " ";
	}


      // now make the key and value objects, parse again the data
      // into the objects and add them to the container

      // Key:

      t_atom *key_atom = (t_atom*)getbytes(key_count*sizeof(t_atom));
      t_atom *val_atom = (t_atom*)getbytes(val_count*sizeof(t_atom));
      if(key_atom == NULL || val_atom == NULL)
	post("Fatal Error Out Of Memory (%s-readFromFile)",this->dataname_.c_str());

      istringstream key_istr(key_str.str());
      istringstream value_istr(value_str.str());

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

      for(int i = 0; i < val_count; i++)
	{
	  value_istr >> type;
	  if (type == 's')
	    {
	      value_istr >> symbol;
	      SETSYMBOL(&val_atom[i],gensym(const_cast<char*>(symbol.c_str())));
	    }
	  if (type == 'f')
	    {
	      value_istr >> number;
	      SETFLOAT(&val_atom[i],number);
	    }
	}

      key.setAtoms(key_count,key_atom);
      el.setAtoms(val_count, val_atom);
      // insert the data
      this->data_[this->h_namespace_].insert(std::pair<Element,Element>(key,el));

      freebytes(key_atom, key_count*sizeof(t_atom));
      freebytes(val_atom, val_count*sizeof(t_atom));
    }

  infile.close();

  return true;
}

//----------------------------------------------------
/* reads the data from the XML file into the current
 * namespace
 * returns true on success
 */
template<class ContainerType, class ContTypeIterator>
    bool MapBase<ContainerType,ContTypeIterator>::readFromFileXML(string filename)
{
  TiXmlDocument doc( filename.c_str() );
  
  if( !doc.LoadFile() ) return false;

  TiXmlNode *parent = 0;
  TiXmlElement *child1 = 0;
  TiXmlElement *child2 = 0;
  TiXmlElement *child3 = 0;
  
  t_atom *key_atom = 0;
  t_atom *val_atom = 0;
  Element key;
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
  if( type != "h_map" && type != "h_multimap" )
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
    // get the <key> tag
    child2 = child1->FirstChildElement( "key" );
    if(!child2) return false;
    
    // get nr of keys and allocate mem for them
    // (if its a pd list)
    int key_count = 0;
    for( child3 = child2->FirstChildElement(); child3; 
         child3 = child3->NextSiblingElement() )
      key_count++;
    
    key_atom = (t_atom*)getbytes(key_count*sizeof(t_atom));
    if(key_atom == NULL)
    {
      post("Fatal Error Out Of Memory (%s-readFromFile)",this->dataname_.c_str());
      return false;
    }   
    
    // iterate through all the atoms of <key>
    key_count = 0;
    for( child3 = child2->FirstChildElement(); child3; 
         child3 = child3->NextSiblingElement() )
    {
      string tag(child3->Value());
      
      if(!child3->FirstChild()) continue;     
      istringstream in(child3->FirstChild()->Value());
      
      if(tag == "f" || tag == "float")
      {
        in >> f;
        SETFLOAT(&key_atom[key_count], f);
      }
      if(tag == "s" || tag == "symbol")
      {
        SETSYMBOL(&key_atom[key_count],
                   gensym(const_cast<char*>(in.str().c_str())));
      }
      
      key_count++;
    }
    
    if(!key_count) continue;
    
    //----------------------
    
    // get the <value> tag
    child2 = child1->FirstChildElement( "value" );
    if(!child2) return false;
    
    // get nr of values and allocate mem for them
    // (if its a pd list)
    int val_count = 0;
    for( child3 = child2->FirstChildElement(); child3; 
         child3 = child3->NextSiblingElement() )
      val_count++;
    
    val_atom = (t_atom*)getbytes(val_count*sizeof(t_atom));
    if(val_atom == NULL)
    {
      post("Fatal Error Out Of Memory (%s-readFromFile)",this->dataname_.c_str());
      return false;
    }
    
    // iterate through all the atoms of <value>
    val_count = 0;
    for( child3 = child2->FirstChildElement(); child3; 
         child3 = child3->NextSiblingElement() )
    {
      string tag(child3->Value());

      if(!child3->FirstChild()) continue;
      istringstream in(child3->FirstChild()->Value());
      
      if(tag == "f" || tag == "float")
      {
        in >> f;
        SETFLOAT(&val_atom[val_count],f);
      }
      if(tag == "s" || tag == "symbol")
      {
        SETSYMBOL(&val_atom[val_count],
                  gensym(const_cast<char*>(in.str().c_str())));
      }      
      
      val_count++;
    }
    
    if(!val_count) continue;
    
    // add the element to the container
    key.setAtoms(key_count,key_atom);
    el.setAtoms(val_count,val_atom);
    this->data_[this->h_namespace_].insert(std::pair<Element,Element>(key,el));

    freebytes(key_atom, key_count*sizeof(t_atom));
    freebytes(val_atom, val_count*sizeof(t_atom));

    parsed = true;
  }
  return parsed;
}


#endif  // _map_base_h__
