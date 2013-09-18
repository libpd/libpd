////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 2002-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "pix_buffer.h"
#include "pix_buffer_write.h"
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
// pix_buffer_write
//
/////////////////////////////////////////////////////////

CPPEXTERN_NEW_WITH_ONE_ARG(pix_buffer_write, t_symbol*,A_DEFSYM);

/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_buffer_write :: pix_buffer_write(t_symbol *s) : m_frame(-2), m_lastframe(-1), m_bindname(NULL) {
  if ((s)&&(&s_!=s)){
    setMess(s);
  }
  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("frame"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_buffer_write :: ~pix_buffer_write(){

}

/////////////////////////////////////////////////////////
// setMess
//
/////////////////////////////////////////////////////////
void pix_buffer_write :: setMess(t_symbol*s){
  if (s!=&s_){
    m_bindname = s;
  }
}
/////////////////////////////////////////////////////////
// frameMess
//
/////////////////////////////////////////////////////////
void pix_buffer_write :: frameMess(int f){
  if (f<0){
    error("frame# must not be less than zero (%d)", f);
  }
  m_frame=f;
}
/////////////////////////////////////////////////////////
// put the current image into the buffer,
// and reset the position
// (so we don't do a put in the next cycle)
/////////////////////////////////////////////////////////
void pix_buffer_write :: render(GemState*state){
  if (m_frame<0)return;
  if(!state)return;
  pixBlock*img=NULL;
  state->get(GemState::_PIX, img);
  if (state && img && &img->image){
    if (img->newimage || m_frame!=m_lastframe){
      if(m_bindname==NULL || m_bindname->s_name==NULL){
	error("cowardly refusing to write to no pix_buffer");
	m_frame=-1; return;
      }
      Obj_header*ohead=(Obj_header*)pd_findbyclass(m_bindname, pix_buffer_class);
      if(ohead==NULL){
	error("couldn't find pix_buffer '%s'", m_bindname->s_name);
	m_frame=-1; return;
      }
      pix_buffer *buffer=(pix_buffer *)(ohead)->data;
      if (buffer){
	buffer->putMess(&img->image,m_lastframe=m_frame);
	m_frame=-1;
      }
    }
  }
}
/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_buffer_write :: obj_setupCallback(t_class *classPtr)
{
  class_addcreator(reinterpret_cast<t_newmethod>(create_pix_buffer_write),
                   gensym("pix_put"),
                   A_DEFSYM, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_buffer_write::setMessCallback),
  		  gensym("set"), A_SYMBOL, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_buffer_write::frameMessCallback),
  		  gensym("frame"), A_FLOAT, A_NULL);
}
void pix_buffer_write :: setMessCallback(void *data, t_symbol*s)
{
  GetMyClass(data)->setMess(s);
}
void pix_buffer_write :: frameMessCallback(void *data, t_floatarg f)
{
  GetMyClass(data)->frameMess((int)f);
}
