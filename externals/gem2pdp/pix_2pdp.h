/*
 *  pix_2pdp : pix to pdp bridge
 *
 *  Capture the contents of the Gem pix and transform it to a PDP Packet whenever a bang is received
 *
 *  Based on code of gem2pdp by Yves Degoyon
 *  Many thanks to IOhannes M Zmölnig
 *
 *  Copyright (c) 2005-06 Georg Holzmann <grh@mur.at>
 *
 */

#ifndef INCLUDE_PIX2PDP_H_
#define INCLUDE_PIX2PDP_H_

#include "Base/GemPixObj.h"

#include "pdp.h"

class GEM_EXTERN pix_2pdp : public GemPixObj
{
  CPPEXTERN_HEADER(pix_2pdp, GemPixObj)

  public:
    
    // Constructor
    pix_2pdp(void);
    	
  protected:
    
    // Destructor
    virtual ~pix_2pdp(void);
    
    // Image processing
    virtual void 	processImage(imageStruct &image);
    
    // pdp processing
    virtual void bangMess(void);

    // the pixBlock with the current image
    unsigned char *gem_image;
    int gem_xsize;
    int gem_ysize;
    int gem_csize;
    int gem_format;
    int gem_upsidedown;

    // pdp data
    t_outlet *m_pdpoutlet;
    int m_packet0;
    t_pdp *m_header;
    short int *m_data;
    	
 private:
   static void bangMessCallback(void *data);
};

#endif
