////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// Implementation file
//
// Copyright (c) 2002-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//	zmoelnig@iem.kug.ac.at
//  For information on usage and redistribution, and for a DISCLAIMER
//  *  OF ALL WARRANTIES, see the file, "GEM.LICENSE.TERMS"
//
//  this file has been generated...
////////////////////////////////////////////////////////

#include "GEMglBitmap.h"
#include "Gem/Image.h"
#include "Gem/State.h"

CPPEXTERN_NEW_WITH_FOUR_ARGS ( GEMglBitmap , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglBitmap :: GEMglBitmap	(t_float arg0=0, t_float arg1=0,
				 t_float arg2=1, t_float arg3=1) :
		xorig(static_cast<GLfloat>(arg0)), 
		yorig(static_cast<GLfloat>(arg1)), 
		xmove(static_cast<GLfloat>(arg2)), 
		ymove(static_cast<GLfloat>(arg3))
{
  // img info: width, height, bitmap
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("xorig"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("yorig"));
	m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("xmove"));
	m_inlet[3] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("ymove"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglBitmap :: ~GEMglBitmap () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
inlet_free(m_inlet[2]);
inlet_free(m_inlet[3]);

}

/////////////////////////////////////////////////////////
// Render
//
void GEMglBitmap :: render(GemState *state) {
  if (!state)return;
  pixBlock*img=NULL;
  state->get(GemState::_PIX, img);
  if(!img || !&img->image)return;
  
  glBitmap (img->image.xsize, img->image.ysize,
	    xorig, yorig, xmove, ymove, 
	    img->image.data);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglBitmap :: xorigMess (t_float arg1) {	// FUN
	xorig = static_cast<GLfloat>(arg1);
	setModified();
}

void GEMglBitmap :: yorigMess (t_float arg1) {	// FUN
	yorig = static_cast<GLfloat>(arg1);
	setModified();
}

void GEMglBitmap :: xmoveMess (t_float arg1) {	// FUN
	xmove = static_cast<GLfloat>(arg1);
	setModified();
}

void GEMglBitmap :: ymoveMess (t_float arg1) {	// FUN
	ymove = static_cast<GLfloat>(arg1);
	setModified();
}

/////////////////////////////////////////////////////////
// static member functions
//

void GEMglBitmap :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglBitmap::xorigMessCallback),  	gensym("xorig"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglBitmap::yorigMessCallback),  	gensym("yorig"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglBitmap::xmoveMessCallback),  	gensym("xmove"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMglBitmap::ymoveMessCallback),  	gensym("ymove"), A_DEFFLOAT, A_NULL);
};

void GEMglBitmap :: xorigMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->xorigMess ( static_cast<t_float>(arg0));
}
void GEMglBitmap :: yorigMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->yorigMess ( static_cast<t_float>(arg0));
}
void GEMglBitmap :: xmoveMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->xmoveMess ( static_cast<t_float>(arg0));
}
void GEMglBitmap :: ymoveMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->ymoveMess ( static_cast<t_float>(arg0));
}
