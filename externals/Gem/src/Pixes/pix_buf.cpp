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
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "pix_buf.h"
#include "Gem/State.h"

#include "Gem/Cache.h"

CPPEXTERN_NEW_WITH_ONE_ARG(pix_buf, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// pix_buf
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_buf :: pix_buf(t_floatarg a=0) :
  orgPixBlock(NULL),m_banged(false), m_auto(a!=0.0)
{
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_buf :: ~pix_buf()
{}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void pix_buf :: render(GemState *state)
{
  if(!state)return;
  bool doit=m_banged;
  pixBlock*img=NULL;
  state->get(GemState::_PIX, img);
  orgPixBlock = img;

  if (!img || !img->image.data) return;

  doit|=m_auto;
  doit|=img->newimage;

  if (m_cache&&m_cache->resendImage)
    doit=true;

  cachedPixBlock.newimage = 0;
  if (doit) {
    img->image.copy2Image(&cachedPixBlock.image);
    cachedPixBlock.newimage = 1;
  }
  state->set(GemState::_PIX, &cachedPixBlock);
  m_banged = false;
}

/////////////////////////////////////////////////////////
// postrender
//
/////////////////////////////////////////////////////////
void pix_buf :: postrender(GemState *state)
{
  state->set(GemState::_PIX, orgPixBlock);
}

/////////////////////////////////////////////////////////
// bangMess
//
/////////////////////////////////////////////////////////
void pix_buf :: bangMess()
{
  m_banged = true;
}
/////////////////////////////////////////////////////////
// autoMess
//
/////////////////////////////////////////////////////////
void pix_buf :: autoMess(int a)
{
  m_auto = (a!=0);
}
/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_buf :: obj_setupCallback(t_class *classPtr)
{ 
  class_addcreator(reinterpret_cast<t_newmethod>(create_pix_buf), 
		   gensym("pix_separator"), A_DEFFLOAT, A_NULL);
  class_addbang(classPtr, &pix_buf::bangMessCallback);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_buf::autoMessCallback),
		  gensym("auto"), A_FLOAT, A_NULL);

}

void pix_buf :: bangMessCallback(void *data){
  GetMyClass(data)->bangMess();
}
void pix_buf :: autoMessCallback(void *data, t_floatarg f){
  GetMyClass(data)->autoMess((int)f);
}
