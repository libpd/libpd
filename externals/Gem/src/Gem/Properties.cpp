#include <map>

#include "Properties.h"

#include <iostream>

namespace gem {

  class Properties::PIMPL {
  public:
    std::map<std::string, any> valuemap;
    std::map<std::string, enum Properties::PropertyType> typemap;

    PIMPL(void) {;}
  };


  Properties::Properties() :
    pimpl(new PIMPL())
  { }

  Properties::Properties(const gem::Properties&org) :
    pimpl(new PIMPL())
  { 
    assign(org);
  }


  Properties::~Properties() {
    delete pimpl;
  }
#if 0
  any&Properties::operator[](const std::string&key) {
    return pimpl->valuemap[key]; 
  }
#endif

  any Properties::get(const std::string&key) const {
    return pimpl->valuemap[key]; 
  }

  void Properties::set(const std::string&key, gem::any value) {    
    const std::type_info*typ=&value.get_type();
    PropertyType pt = UNKNOWN;

    double d=0;
    std::string s;

#define ISTYPE(type) (*typ == typeid(type))

    if (value.empty()) {
      pt = NONE;
    } else if(ISTYPE(char)) {
      pt = DOUBLE;
      d=any_cast<char>(value);
    } else if(ISTYPE(unsigned char)) {
      pt = DOUBLE;
      d=any_cast<unsigned char>(value);

    } else if(ISTYPE(short)) {
      pt = DOUBLE;
      d=any_cast<short>(value);
    } else if(ISTYPE(unsigned short)) {
      pt = DOUBLE;
      d=any_cast<unsigned short>(value);

    } else if(ISTYPE(int)) {
      pt = DOUBLE;
      d=any_cast<int>(value);
    } else if(ISTYPE(unsigned int)) {
      pt = DOUBLE;
      d=any_cast<unsigned int>(value);
#if 0
      /* "long"s cannot be stored in "double"s without loosing precision... */
    } else if(ISTYPE(long)) {
      pt = DOUBLE;
      d=any_cast<long>(value);
    } else if(ISTYPE(unsigned long)) {
      pt = DOUBLE;
      d=any_cast<unsigned long>(value);
#endif
    } else if(ISTYPE(float)) {
      pt = DOUBLE;
      d=any_cast<float>(value);
    } else if(ISTYPE(double)) {
      pt = DOUBLE;
      d=any_cast<double>(value);

    } else if(ISTYPE(char*)) {
      pt = STRING;
      s=std::string(any_cast<char*>(value));
#ifdef SETSYMBOL
      /* only use this, if compiled with Pd... */
    } else if(ISTYPE(t_symbol*)) {
      pt = STRING;
      s=std::string((any_cast<t_symbol*>(value))->s_name);
#endif
    } else if(ISTYPE(std::string)) {
      pt = STRING;
      s=any_cast<std::string>(value);
    }

    switch(pt) {
    case NONE:
      pimpl->valuemap[key]=value;
      pimpl->typemap[key]=NONE;
      break;
    case DOUBLE:
      pimpl->valuemap[key]=d;
      pimpl->typemap[key]=DOUBLE;
      break;
    case STRING:
      pimpl->valuemap[key]=s;
      pimpl->typemap[key]=STRING;
      break;
    default:
      pimpl->valuemap[key]=value;
      pimpl->typemap[key]=UNKNOWN;
      break;
    }
  } 

  std::vector<std::string>Properties::keys() const {
    std::vector<std::string>result;
    std::map<std::string,gem::any>::iterator it;
    for(it = pimpl->valuemap.begin(); it != pimpl->valuemap.end(); ++it)
      result.push_back(it->first);
    return result;
  }

  enum Properties::PropertyType Properties::type(std::string key) const {
    return pimpl->typemap[key];
  }

  void Properties::erase(std::string key) {
    pimpl->typemap.erase(key);
    pimpl->valuemap.erase(key);
  }
  void Properties::clear() {
    pimpl->typemap.clear();
    pimpl->valuemap.clear();
  }

  Properties& Properties::assign(const Properties&org) {
    pimpl->valuemap=org.pimpl->valuemap;
    pimpl->typemap =org.pimpl->typemap;

    return(*this);
  }
};
