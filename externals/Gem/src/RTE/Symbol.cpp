////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.at
//
// Implementation file
//
//    Copyright (c) 1997-2000 Mark Danks.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    Copyright (c) 2002 James Tittle & Chris Clepper
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
// a wrapper for accessing the RTE's arrays
// currently only numeric arrays
//
/////////////////////////////////////////////////////////

#include "RTE/Symbol.h"
#include "Gem/RTE.h"

// for snprintf
#include <stdio.h>
#ifdef _WIN32
# define snprintf _snprintf
#endif

class gem::RTE::Symbol::PIMPL {
public:
  const t_symbol*sym;
  
  PIMPL(void) : sym(NULL)
  {
  }
  ~PIMPL(void) {
  }
};

gem::RTE::Symbol :: Symbol(void)
  : m_pimpl(new PIMPL)
{
}

gem::RTE::Symbol :: Symbol(const gem::RTE::Symbol&Sym)
  : m_pimpl(new PIMPL)
{
  m_pimpl->sym=Sym.m_pimpl->sym;
}
gem::RTE::Symbol :: Symbol(const std::string&name) 
  : m_pimpl(new PIMPL)
{
  m_pimpl->sym=gensym(name.c_str());
}
gem::RTE::Symbol :: Symbol(const t_symbol*name) 
  : m_pimpl(new PIMPL)
{
  m_pimpl->sym=name;
}
gem::RTE::Symbol :: Symbol(const unsigned int argc, const t_atom*argv) 
  : m_pimpl(new PIMPL)
{
  setSymbol(argc, argv);
}


gem::RTE::Symbol :: ~Symbol(void)
{
  m_pimpl->sym=NULL;
  delete m_pimpl;
}

gem::RTE::Symbol&gem::RTE::Symbol::operator=(const std::string&name) {
  m_pimpl->sym=gensym(name.c_str());
  return (*this);
}
gem::RTE::Symbol&gem::RTE::Symbol::operator=(const gem::RTE::Symbol&Sym) {
  m_pimpl->sym=Sym.m_pimpl->sym;
  return (*this);
}
gem::RTE::Symbol&gem::RTE::Symbol::operator=(const t_symbol*name) {
  m_pimpl->sym=name;
  return (*this);
}
gem::RTE::Symbol&gem::RTE::Symbol::setSymbol(const unsigned int argc, const t_atom*argv) {
  char buf[MAXPDSTRING];
  std::string name;
  bool firsttime=true;

  unsigned int i;
  for(i=0; i<argc; i++) {
    t_atom*arg=(t_atom*)(argv+i);
    std::string atomname;
    if(A_FLOAT==argv[i].a_type) {
      snprintf(buf, MAXPDSTRING, "%g", atom_getfloat(arg));
      buf[MAXPDSTRING-1]=0;
      atomname=buf;
    } else {
      atomname=std::string(atom_getsymbol(arg)->s_name);
    }
    if(!firsttime)
      name+=" ";
    firsttime=false;
    name+=atomname; 
  }

  m_pimpl->sym=gensym(name.c_str());
  return (*this);
}

t_symbol*gem::RTE::Symbol::getRTESymbol(void) const {
  return (t_symbol*)(m_pimpl->sym);
}
std::string gem::RTE::Symbol::getString(void) const {
  if(m_pimpl->sym)
    return std::string(m_pimpl->sym->s_name);
  return std::string();      
}
