////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 2002-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    Copyright (c) 2005 Georg Holzmann
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "pix_buffer.h"
#include "pix_buffer_read.h"
#include "Gem/State.h"

/*
 * we export the "pix_buffer_class"
 * so other objects can bind to it with "pd_findbyclass()"
 * NOTE: we need NO_STATIC_CLASS to be defined in pix_buffer.cpp for this to work
 * NOTE: we define it only in pix_buffer.cpp (before pix_buffer.h&CPPExtern.h are included)
 *       in order to not interfere with the class-status (non-static/static) of objects that 
 *       include this header-file
 */

extern t_class *pix_buffer_class;

/////////////////////////////////////////////////////////
//
// pix_buffer_read
//
/////////////////////////////////////////////////////////

CPPEXTERN_NEW_WITH_ONE_ARG(pix_buffer_read, t_symbol*,A_DEFSYM);


/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_buffer_read :: pix_buffer_read(t_symbol *s) : 
  m_frame(0.f), m_auto(0.f), m_loop(0),
  m_haveImage(false),
  m_bindname(NULL),
  m_needsupdate(false)
{
  if ((s)&&(&s_!=s)){
    setMess(s);
  }
  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("frame"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_buffer_read :: ~pix_buffer_read(){
  
}

/////////////////////////////////////////////////////////
// setMess
//
/////////////////////////////////////////////////////////
void pix_buffer_read :: setMess(t_symbol*s){
  if (s!=&s_){
    m_bindname = s;
  }
  m_needsupdate=true;
}
/////////////////////////////////////////////////////////
// frameMess
//
/////////////////////////////////////////////////////////
void pix_buffer_read :: frameMess(t_float f){
  if (f<0.){
    error("frame# must not be less than zero (%f)", f);
  }
  m_frame=f;
  m_needsupdate=true;
}
/////////////////////////////////////////////////////////
// autoMess
//   specify an incrementor to proceed to the next image
//
/////////////////////////////////////////////////////////
void pix_buffer_read :: autoMess(t_float f){
  m_auto=f;
}
/////////////////////////////////////////////////////////
// loopMess
//   should we loop when reaching the end of the buffer ?
//
/////////////////////////////////////////////////////////
void pix_buffer_read :: loopMess(int i){
  m_loop=(i!=0);
}

/////////////////////////////////////////////////////////
// update current image
/////////////////////////////////////////////////////////
void pix_buffer_read :: update_image()
{
  imageStruct *img=NULL;
  Obj_header*ohead=NULL;
  pix_buffer *buffer=NULL;

  m_haveImage=false;
  
  if(m_bindname==NULL || m_bindname->s_name==NULL)
    {
      error("you must set a buffer name!");
      return;
    }
  
  ohead=(Obj_header*)pd_findbyclass(m_bindname, pix_buffer_class);
  if(ohead==NULL)
    {
      error("couldn't find pix_buffer '%s'", m_bindname->s_name);
      return;
    }
  buffer=(pix_buffer *)(ohead)->data;

  if (!buffer) return;

  if(m_loop)
    {
      int numFrames=buffer->numFrames();
      m_frame=fmod(m_frame, numFrames);
      if(m_frame<0.f)m_frame+=numFrames;
    }

  img=buffer->getMess((int)m_frame);

  if (img && img->data)
    {
      img->copy2ImageStruct(&m_pixBlock.image);
      m_pixBlock.newimage = 1;
      m_haveImage=true;
    }
  m_needsupdate=false;
}

/////////////////////////////////////////////////////////
// get the image at <m_frame> from the buffer
// and insert it into the gemchain
/////////////////////////////////////////////////////////
void pix_buffer_read :: render(GemState*state)
{
  if(m_needsupdate)
    update_image();


  // if we don't have an image, just return
  if (!m_haveImage) return;

  /* push the incoming state->image into a temporary memory */
  state->get(GemState::_PIX, orgPixBlock);

  /*
    pd_findbyclass costs at least 2 if's
    if m_bindname is also used for other classes too, we get an 
    additional penalty for traversing the list of classes;
    all in all, msp has done a good job
  */ 
  if (NULL==pd_findbyclass(m_bindname, pix_buffer_class)) return;

  state->set(GemState::_PIX, &m_pixBlock);

}

/////////////////////////////////////////////////////////
// postrender
/////////////////////////////////////////////////////////
void pix_buffer_read :: postrender(GemState *state)
{
  m_pixBlock.newimage = 0;

  // auto-mode logic:
  if(m_auto > 0.00001 || m_auto < -0.00001)
    {
      m_frame+=m_auto;
      update_image();
    }

  /* restore the original incoming image */
  state->set(GemState::_PIX, orgPixBlock);
}

/////////////////////////////////////////////////////////
// startRendering
/////////////////////////////////////////////////////////
void pix_buffer_read :: startRendering()
{
  update_image();
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_buffer_read :: obj_setupCallback(t_class *classPtr)
{
  class_addcreator(reinterpret_cast<t_newmethod>(create_pix_buffer_read),
                   gensym("pix_get"),
                   A_DEFSYM, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_buffer_read::setMessCallback),
  		  gensym("set"), A_SYMBOL, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_buffer_read::frameMessCallback),
  		  gensym("frame"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_buffer_read::autoMessCallback),
  		  gensym("auto"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_buffer_read::loopMessCallback),
  		  gensym("loop"), A_FLOAT, A_NULL);
}
void pix_buffer_read :: setMessCallback(void *data, t_symbol*s)
{
  GetMyClass(data)->setMess(s);
}
void pix_buffer_read :: frameMessCallback(void *data, t_floatarg f)
{
  GetMyClass(data)->frameMess((int)f);
}
void pix_buffer_read :: autoMessCallback(void *data, t_floatarg f)
{
  GetMyClass(data)->autoMess(f);
}
void pix_buffer_read :: loopMessCallback(void *data, t_floatarg f)
{
  GetMyClass(data)->loopMess(f>0.f);
}
