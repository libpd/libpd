////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-1999 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "pix_draw.h"

#include "Gem/Image.h"
#include "Gem/State.h"

CPPEXTERN_NEW(pix_draw);

/////////////////////////////////////////////////////////
//
// pix_draw
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_draw :: pix_draw()
{
  static int firsttime=1;

  if(firsttime) {
    post("requesting [pix_draw] - consider using [pix_texture]");
  } else {
    logpost(NULL, 5, "requesting [pix_draw]: consider using [pix_texture]");
  }
  firsttime=0;
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_draw :: ~pix_draw()
{ }

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void pix_draw :: render(GemState *state)
{
  int orientation=1;
  pixBlock*img=NULL;
  state->get(GemState::_PIX, img);
  if ( !img || !&img->image ) return;
  glRasterPos2i(0, 0);
  // hack to center image at 0,0
  if(img->image.upsidedown)
    orientation=-1;

  glPixelZoom(1,orientation);

  glBitmap(0, 0, 0.f, 0.f, -(img->image.xsize)/2.f, 
           -orientation*(img->image.ysize)/2.f, 0);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glDrawPixels(img->image.xsize,
               img->image.ysize,
               img->image.format,
               img->image.type,
               img->image.data);
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_draw :: obj_setupCallback(t_class *)
{ }
