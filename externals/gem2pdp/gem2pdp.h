/*
 *  gem2pdp : gem to pdp bridge
 *
 *  Capture the contents of the Gem window and transform it to a PDP Packet whenever a bang is received
 *
 *  Copyright (c) 2003 Yves Degoyon
 *
 */

#ifndef INCLUDE_GEM2PDP_H_
#define INCLUDE_GEM2PDP_H_

#include "Base/GemBase.h"
#include "Base/GemPixUtil.h"
#include "pdp.h"

class GEM_EXTERN gem2pdp : public GemBase
{
  CPPEXTERN_HEADER(gem2pdp, GemBase)

  public:
   gem2pdp(void);
    	
  protected:
   imageStruct  *m_image;
   int     	m_x;
   int     	m_y;
   int     	m_width;
   int     	m_height;
   t_outlet     *m_pdpoutlet;
   virtual ~gem2pdp(void);
   virtual void bangMess(void);
   virtual void bufferMess(int);
   virtual void render(GemState *state);
   void         cleanImage();
   int        m_packet0;
   t_pdp        *m_header;
   short int    *m_data;
   void         *m_gemstate;
   GLenum       m_buffer;
    	
 private:
   static void 	bangMessCallback(void *data);
   static void 	bufferMessCallback(void *data, t_floatarg buffer);
};

#endif
