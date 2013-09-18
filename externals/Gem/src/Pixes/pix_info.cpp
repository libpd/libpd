////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-1998 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    Copyright (c) 2002 James Tittle & Chris Clepper
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "pix_info.h"
#include "Gem/State.h"

CPPEXTERN_NEW(pix_info);

/////////////////////////////////////////////////////////
//
// pix_info
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_info :: pix_info()
{
  m_x        = outlet_new(this->x_obj, 0);
  m_y        = outlet_new(this->x_obj, 0);
  m_c        = outlet_new(this->x_obj, 0);
  m_format   = outlet_new(this->x_obj, 0);
  m_misc     = outlet_new(this->x_obj, 0);
  m_pixblock = outlet_new(this->x_obj, 0);
  m_data     = outlet_new(this->x_obj, 0);
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_info :: ~pix_info()
{
  outlet_free(m_x);
  outlet_free(m_y);
  outlet_free(m_c);
  outlet_free(m_format);
  outlet_free(m_misc);
  outlet_free(m_pixblock);
  outlet_free(m_data);
}

/////////////////////////////////////////////////////////
// trigger
//
/////////////////////////////////////////////////////////
void pix_info :: render(GemState *state)
{
  
  // 0 0 0  6408  5121 1 1 0  0 9.59521e+08
  t_atom abuf[3];
  pixBlock*img=NULL;
  if(state)state->get(GemState::_PIX, img);
  if (!state || !img){ //no pixblock (or even no image!)!
    outlet_float(m_pixblock, (t_float)-1);
    outlet_float(m_misc,     (t_float)-1);
    outlet_float(m_format,   (t_float)-1);
    outlet_float(m_c,        (t_float)-1);
    outlet_float(m_y,        (t_float)-1);
    outlet_float(m_x,        (t_float)-1);
    return;
  }
  SETFLOAT(  &abuf[0], (t_float)img->newimage);
  SETFLOAT(  &abuf[1], (t_float)img->newfilm);
  if (!&img->image){ // we have a pixblock, but no image!
    outlet_list(m_pixblock, gensym("list"), 2, abuf);

    outlet_float(m_misc,   (t_float)-1);
    outlet_float(m_format, (t_float)-1);
    outlet_float(m_c,      (t_float)-1);
    outlet_float(m_y,      (t_float)-1);
    outlet_float(m_x,      (t_float)-1);
    return;
  }
  if(img->image.data){
    t_gpointer*gp=(t_gpointer*)img->image.data;
    SETPOINTER(&abuf[2], gp);
    outlet_anything(m_data, gensym("data"), 1, abuf+2);
  }
  outlet_list(m_pixblock, gensym("list"), 2, abuf);
  SETFLOAT  (&abuf[0], (t_float)img->image.type);
  SETFLOAT  (&abuf[1], (t_float)img->image.upsidedown);
  SETFLOAT  (&abuf[2], (t_float)img->image.notowned);
  outlet_list(m_misc, gensym("list"), 3, abuf);
  // send out the colorspace (as integer!)
  outlet_float(m_format, (t_float)img->image.format);
  // send out the width/height/csize information
  outlet_float(m_c, (t_float)img->image.csize);
  outlet_float(m_y, (t_float)img->image.ysize);
  outlet_float(m_x, (t_float)img->image.xsize);
}
void pix_info :: obj_setupCallback(t_class *classPtr){
}
