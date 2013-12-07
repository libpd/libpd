/*-----------------------------------------------------------------
  LOG
  GEM - Graphics Environment for Multimedia

  access outlets of the RTE

  Copyright (c) 2011-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

  -----------------------------------------------------------------*/
#ifndef _INCLUDE__GEM_RTE_OUTLET_H_
#define _INCLUDE__GEM_RTE_OUTLET_H_

#include "Gem/ExportDef.h"
#include <vector>
#include <string>
#include "Utils/any.h"

class CPPExtern;
namespace gem { namespace RTE {
    GEM_EXTERN class Outlet {
    private:
      class PIMPL;
      PIMPL*m_pimpl;

    public:
      Outlet(CPPExtern*parent);
      Outlet(const Outlet&);

      virtual ~Outlet(void);

      void send(void);   // bang
      void send(double); // float
      void send(std::string, std::vector<gem::any>data);

      Outlet& operator=(const Outlet&);
    };
  };
};


#endif /* header file */
