/*-----------------------------------------------------------------
  LOG
  GEM - Graphics Environment for Multimedia

  access arrays of the RTE

  Copyright (c) 2010-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

  -----------------------------------------------------------------*/
#ifndef _INCLUDE__GEM_RTE_ARRAY_H_
#define _INCLUDE__GEM_RTE_ARRAY_H_


#include "Gem/ExportDef.h"
#include <string>

namespace gem {
  namespace RTE {
    GEM_EXTERN class Array {
    private:
      class PIMPL;
      PIMPL*m_pimpl;

    public:
      Array(void);
      Array(const gem::RTE::Array&a);
      Array(const std::string&name);

      virtual ~Array(void);

      /* check whether we hold a valid reference to an array */
      virtual bool isValid();

      /* reference another array */
      virtual bool name(const std::string&s);
      virtual const std::string name(void);

      virtual bool resize(const size_t newsize);
      virtual size_t size(void);

      virtual t_float&operator[](const unsigned int&index);

      virtual void set(const t_float f);

      virtual Array&operator=(const Array&);
    };
  };
};
#endif /* _INCLUDE__GEM_RTE_ARRAY_H_ */
