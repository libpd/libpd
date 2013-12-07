/*
 *  pix_share_read.h
 *  GEM_darwin
 *
 *  Created by lincoln on 9/29/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */


#ifndef _INCLUDE__GEM_PIXES_PIX_SHARE_READ_H_
#define _INCLUDE__GEM_PIXES_PIX_SHARE_READ_H_

#include "pix_share_write.h"

class GEM_EXTERN pix_share_read : public pix_share_write
{
  CPPEXTERN_HEADER(pix_share_read, pix_share_write);

    public:
  pix_share_read(int,t_atom*);
		
 protected:
  ~pix_share_read();
		
  virtual void render(GemState *state);
		
  pixBlock	pix;
};

#endif

