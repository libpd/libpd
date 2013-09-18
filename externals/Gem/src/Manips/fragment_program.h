/*-----------------------------------------------------------------

GEM - Graphics Environment for Multimedia

Load an ARB fragment program/shader
 
 *  Created by tigital on 10/16/04.
 *  Copyright 2004 tigital.
 
Copyright (c) 1997-1999 Mark Danks. mark@danks.org
Copyright (c) Günther Geiger. geiger@epy.co.at
Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_MANIPS_FRAGMENT_PROGRAM_H_
#define _INCLUDE__GEM_MANIPS_FRAGMENT_PROGRAM_H_

#include "Manips/vertex_program.h"

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  fragment_program
    
  Loads in an ARB fragment program/shader
    
  KEYWORDS
  
    
  DESCRIPTION

  -----------------------------------------------------------------*/
class GEM_EXTERN fragment_program : public vertex_program
{
  CPPEXTERN_HEADER(fragment_program, vertex_program);
    
    public:
  
  //////////
  // Constructor
  fragment_program(t_symbol *filename);

 protected:
    
  //////////
  // Destructor
  virtual ~fragment_program(void);

  //////////
  // which Program do we have (ARB, NV,...)
  virtual GLint queryProgramtype(char*program);

  //////////
  // Print Info about Hardware limits
  virtual void printInfo(void);
 
  //////////
  // extension check
  virtual bool isRunnable(void);
};

#endif	// for header file
