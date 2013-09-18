////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "pix_crop.h"

CPPEXTERN_NEW_WITH_FOUR_ARGS(pix_crop, t_float,A_DEFFLOAT,t_float, A_DEFFLOAT, t_float,A_DEFFLOAT,t_float, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// pix_crop
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_crop :: pix_crop(t_floatarg x=0, t_floatarg y=0, t_floatarg w=64, t_floatarg h=64)
{
  m_data = NULL;
  m_size = 0;

  if(w<=1. && h<=1.) {
    w=h=64.;
  }

  offsetMess((int)x,(int)y);
  dimenMess((int)w,(int)h);

  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("dimenX"));
  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("dimenY"));
  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("offsetX"));
  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("offsetY"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_crop :: ~pix_crop()
{
  if (m_data) delete [] m_data;
}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_crop :: processImage(imageStruct &image)
{
  int csize=image.csize;
  int x=(wantSizeX<image.xsize&&wantSizeX>0)?wantSizeX:image.xsize;
  int y=(wantSizeY<image.ysize&&wantSizeY>0)?wantSizeY:image.ysize;

  if (x*y*csize>m_size){
    if (m_data)delete[]m_data;
    m_size = x*y*csize;
    m_data = new unsigned char [m_size];
  }

  int offX=offsetX;
  int offY=offsetY;
  if (offX>(image.xsize-x)) offX=image.xsize-x;
  if (offX<0)offX=0;
  if (offY>(image.ysize-y)) offY=image.ysize-y;
  if (offY<0)offY=0;

  int i=0;
  while(i<y){
    int oldrow=image.upsidedown?(image.ysize-((offY+i)%image.ysize)-1):(offY+i)%image.ysize;
    unsigned char *newdata = m_data+(x*i)*csize;
    unsigned char *olddata = image.data+(offX+image.xsize*oldrow)*csize;
    int j=x*csize;
    while(j--)*newdata++=*olddata++;
    i++;
  }
  image.upsidedown=0;
  image.data   = m_data;
  image.xsize  = x;
  image.ysize  = y;
}


void pix_crop :: dimenMess(int x, int y){
  if(x<0)x=0;
  if(y<0)y=0;
  
  wantSizeX=x;
  wantSizeY=y;
  
  setPixModified();
}
void pix_crop :: dimXMess(int x){
  if(x<0)x=0;
  wantSizeX=x;
  setPixModified();
}
void pix_crop :: dimYMess(int y){
  if(y<0)y=0;
  wantSizeY=y;
  setPixModified();
}
void pix_crop :: offXMess(int x){
  offsetX=x;
  setPixModified();
}
void pix_crop :: offYMess(int y){
  offsetY=y;
  setPixModified();
}
void pix_crop :: offsetMess(int x, int y){
  offsetX=x;
  offsetY=y;
  
  setPixModified();
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_crop :: obj_setupCallback(t_class *classPtr){
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_crop::dimenMessCallback), 
		  gensym("dimen"), A_FLOAT, A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_crop::offsetMessCallback), 
		  gensym("offset"), A_FLOAT, A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_crop::dimXMessCallback), 
		  gensym("dimenX"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_crop::dimYMessCallback), 
		  gensym("dimenY"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_crop::offXMessCallback), 
		  gensym("offsetX"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_crop::offYMessCallback), 
		  gensym("offsetY"), A_FLOAT, A_NULL);}

void pix_crop :: dimenMessCallback(void *data, t_float x, t_float y){
  GetMyClass(data)->dimenMess((int)x, (int)y);
}
void pix_crop :: offsetMessCallback(void *data, t_float x, t_float y){
  GetMyClass(data)->offsetMess((int)x, (int)y);
}
void pix_crop :: dimXMessCallback(void *data, t_float x){
  GetMyClass(data)->dimXMess((int)x);
}
void pix_crop :: dimYMessCallback(void *data, t_float x){
  GetMyClass(data)->dimYMess((int)x);
}
void pix_crop :: offXMessCallback(void *data, t_float x){
  GetMyClass(data)->offXMess((int)x);
}
void pix_crop :: offYMessCallback(void *data, t_float x){
  GetMyClass(data)->offYMess((int)x);
}
