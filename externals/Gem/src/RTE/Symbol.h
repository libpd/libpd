/*-----------------------------------------------------------------
  LOG
  GEM - Graphics Environment for Multimedia

  access symbols of the RTE

  Copyright (c) 2010-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

  -----------------------------------------------------------------*/
#ifndef _INCLUDE__GEM_RTE_SYMBOL_H_
#define _INCLUDE__GEM_RTE_SYMBOL_H_


#include "Gem/ExportDef.h"
#include <string>

struct _symbol;
struct _atom;
namespace gem {
  namespace RTE {
    GEM_EXTERN class Symbol {
    private:
      class PIMPL;
      PIMPL*m_pimpl;
      
    public:
      Symbol(void);
      Symbol(const gem::RTE::Symbol&a);
      Symbol(const std::string&name);
      Symbol(const struct _symbol*name);
      Symbol(const unsigned int, const struct _atom*);
      
      virtual ~Symbol(void);

      virtual Symbol&operator=(const Symbol&);
      virtual Symbol&operator=(const std::string&);
      virtual Symbol&operator=(const struct _symbol*);
      virtual Symbol&setSymbol(const unsigned int, const struct _atom*);

      virtual std::string getString(void) const;
      virtual struct _symbol*getRTESymbol(void) const;

      virtual operator std::string(void) {
	return getString();
      }

      virtual operator struct _symbol*(void) {
	return getRTESymbol();
      }
    };
  };
};
#endif /* _INCLUDE__GEM_RTE_SYMBOL_H_ */
