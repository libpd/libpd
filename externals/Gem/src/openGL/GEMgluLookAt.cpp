////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// Implementation file
//
// Copyright (c) 2008 zmoelnig@iem.at
//  For information on usage and redistribution, and for a DISCLAIMER
//  OF ALL WARRANTIES, see the file "GEM.LICENSE.TERMS"
//

#include "GEMgluLookAt.h"
#include "Gem/Exception.h"

CPPEXTERN_NEW_WITH_GIMME ( GEMgluLookAt );

/////////////////////////////////////////////////////////
//
// GEMgluLookAt
//
/////////////////////////////////////////////////////////
// Constructor
//
  GEMgluLookAt :: GEMgluLookAt	(int argc, t_atom*argv) :
	  m_eyeX(static_cast<GLdouble>(0)),
	  m_eyeY(static_cast<GLdouble>(0)),
	  m_eyeZ(static_cast<GLdouble>(0)),
	  m_centerX(static_cast<GLdouble>(0)),
	  m_centerY(static_cast<GLdouble>(0)),
	  m_centerZ(static_cast<GLdouble>(0)),
	  m_upX(static_cast<GLdouble>(0)),
	  m_upY(static_cast<GLdouble>(0)),
	  m_upZ(static_cast<GLdouble>(0))
{
  if(argc && argc!=9) {
    throw(GemException("invalid number of arguments"));
  }
  if(argc) {
    int i=0;
    for(i=0; i<9; i++) {
      if(A_FLOAT!=argv[i].a_type)
        throw(GemException("all arguments must be numeric"));
    }
  }

  m_eyeX=static_cast<GLdouble>(atom_getfloat(argv+0));
  m_eyeY=static_cast<GLdouble>(atom_getfloat(argv+1));
  m_eyeZ=static_cast<GLdouble>(atom_getfloat(argv+2));
  m_centerX=static_cast<GLdouble>(atom_getfloat(argv+3));
  m_centerY=static_cast<GLdouble>(atom_getfloat(argv+4));
  m_centerZ=static_cast<GLdouble>(atom_getfloat(argv+5));
  m_upX=static_cast<GLdouble>(atom_getfloat(argv+6));
  m_upY=static_cast<GLdouble>(atom_getfloat(argv+7));
  m_upZ=static_cast<GLdouble>(atom_getfloat(argv+8));
  

	  m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("eyeX"));
	  m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("eyeY"));
	  m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("eyeZ"));
	  m_inlet[3] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("centerX"));
	  m_inlet[4] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("centerY"));
	  m_inlet[5] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("centerZ"));
	  m_inlet[6] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("upX"));
	  m_inlet[7] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("upY"));
	  m_inlet[8] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("upZ"));
}

/////////////////////////////////////////////////////////
// Destructor
//
GEMgluLookAt :: ~GEMgluLookAt () {
	  inlet_free(m_inlet[0]); m_inlet[0]=NULL;
	  inlet_free(m_inlet[1]); m_inlet[1]=NULL;
	  inlet_free(m_inlet[2]); m_inlet[2]=NULL;
	  inlet_free(m_inlet[3]); m_inlet[3]=NULL;
	  inlet_free(m_inlet[4]); m_inlet[4]=NULL;
	  inlet_free(m_inlet[5]); m_inlet[5]=NULL;
	  inlet_free(m_inlet[6]); m_inlet[6]=NULL;
	  inlet_free(m_inlet[7]); m_inlet[7]=NULL;
	  inlet_free(m_inlet[8]); m_inlet[8]=NULL;
}

/////////////////////////////////////////////////////////
// Render
//
void GEMgluLookAt :: render(GemState *state) {
	gluLookAt (m_eyeX, m_eyeY, m_eyeZ, m_centerX, m_centerY, m_centerZ, m_upX, m_upY, m_upZ);
}

void GEMgluLookAt :: eyeXMess(t_float arg1) {
	  m_eyeX=static_cast<GLdouble>(arg1);
	  setModified();
}

void GEMgluLookAt :: eyeYMess(t_float arg1) {
	  m_eyeY=static_cast<GLdouble>(arg1);
	  setModified();
}

void GEMgluLookAt :: eyeZMess(t_float arg1) {
	  m_eyeZ=static_cast<GLdouble>(arg1);
	  setModified();
}

void GEMgluLookAt :: centerXMess(t_float arg1) {
	  m_centerX=static_cast<GLdouble>(arg1);
	  setModified();
}

void GEMgluLookAt :: centerYMess(t_float arg1) {
	  m_centerY=static_cast<GLdouble>(arg1);
	  setModified();
}

void GEMgluLookAt :: centerZMess(t_float arg1) {
	  m_centerZ=static_cast<GLdouble>(arg1);
	  setModified();
}

void GEMgluLookAt :: upXMess(t_float arg1) {
	  m_upX=static_cast<GLdouble>(arg1);
	  setModified();
}

void GEMgluLookAt :: upYMess(t_float arg1) {
	  m_upY=static_cast<GLdouble>(arg1);
	  setModified();
}

void GEMgluLookAt :: upZMess(t_float arg1) {
	  m_upZ=static_cast<GLdouble>(arg1);
	  setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMgluLookAt :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMgluLookAt::eyeXMessCallback),  	gensym("eyeX"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMgluLookAt::eyeYMessCallback),  	gensym("eyeY"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMgluLookAt::eyeZMessCallback),  	gensym("eyeZ"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMgluLookAt::centerXMessCallback),  	gensym("centerX"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMgluLookAt::centerYMessCallback),  	gensym("centerY"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMgluLookAt::centerZMessCallback),  	gensym("centerZ"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMgluLookAt::upXMessCallback),  	gensym("upX"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMgluLookAt::upYMessCallback),  	gensym("upY"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, reinterpret_cast<t_method>(&GEMgluLookAt::upZMessCallback),  	gensym("upZ"), A_DEFFLOAT, A_NULL);
};

void GEMgluLookAt :: eyeXMessCallback (void*data, t_floatarg arg0) {
	GetMyClass(data)->eyeXMess( static_cast<t_float>(arg0));
}
void GEMgluLookAt :: eyeYMessCallback (void*data, t_floatarg arg0) {
	GetMyClass(data)->eyeYMess( static_cast<t_float>(arg0));
}
void GEMgluLookAt :: eyeZMessCallback (void*data, t_floatarg arg0) {
	GetMyClass(data)->eyeZMess( static_cast<t_float>(arg0));
}
void GEMgluLookAt :: centerXMessCallback (void*data, t_floatarg arg0) {
	GetMyClass(data)->centerXMess( static_cast<t_float>(arg0));
}
void GEMgluLookAt :: centerYMessCallback (void*data, t_floatarg arg0) {
	GetMyClass(data)->centerYMess( static_cast<t_float>(arg0));
}
void GEMgluLookAt :: centerZMessCallback (void*data, t_floatarg arg0) {
	GetMyClass(data)->centerZMess( static_cast<t_float>(arg0));
}
void GEMgluLookAt :: upXMessCallback (void*data, t_floatarg arg0) {
	GetMyClass(data)->upXMess( static_cast<t_float>(arg0));
}
void GEMgluLookAt :: upYMessCallback (void*data, t_floatarg arg0) {
	GetMyClass(data)->upYMess( static_cast<t_float>(arg0));
}
void GEMgluLookAt :: upZMessCallback (void*data, t_floatarg arg0) {
	GetMyClass(data)->upZMess( static_cast<t_float>(arg0));
}

