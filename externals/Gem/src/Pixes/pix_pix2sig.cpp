////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at 
//
// Implementation file
//
//    Copyright (c) 2000 Guenter Geiger geiger@epy.co.at
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    Copyright (c) 2002 James Tittle & Chris Clepper
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#define HELPSYMBOL "pix_sig2pix~"

#include "pix_pix2sig.h"

CPPEXTERN_NEW(pix_pix2sig);

/////////////////////////////////////////////////////////
//
// pix_pix2sig
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_pix2sig :: pix_pix2sig() : m_data(NULL), m_size(0)
{
  int i=4;
  while(i--)
    o_col[i]=outlet_new(this->x_obj, &s_signal);
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_pix2sig :: ~pix_pix2sig()
{
  int i=4;
  while(i--)outlet_free(o_col[i]);
  m_data = NULL;
  m_size = 0;
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////
// processImage
/////////////////////////////////////////////////////////
void pix_pix2sig :: processImage(imageStruct &image)
{
  m_data = image.data;
  m_size = image.xsize * image.ysize;
  m_csize = image.csize;
  m_format = image.format;
}

/////////////////////////////////////////////////////////
// signal Performance
t_int* pix_pix2sig :: perform(t_int* w)
{
  pix_pix2sig *x = GetMyClass((void*)w[1]);
  // hey this is RGBA only !!!
  t_float* out_red =   (t_float*)(w[2]);
  t_float* out_green = (t_float*)(w[3]);
  t_float* out_blue =  (t_float*)(w[4]);
  t_float* out_alpha = (t_float*)(w[5]);
  int N = (t_int)(w[6]);

  unsigned char* data = x->m_data;
  long int pix_size   = x->m_size;
  int n = (N<pix_size)?N:pix_size;

  if (x->m_data){
    t_float scale0, scale1, scale2, scale3;
    scale0 = scale1 = scale2 = scale3 = 1./255.;
    int csize=x->m_csize;
    switch (csize){
    default:
      scale3=1./255.;
    case 3:
      scale2=1./255.;
    case 2:
      scale1=1./255.;
    case 1:
      scale0=1./255.;
    case 0:
      break;
    }
    switch(x->m_format){
    case GL_RGBA: default:
      while(n--){
	*(out_red  ++) = data[chRed]  *scale0;
	*(out_green++) = data[chGreen]*scale1;
	*(out_blue ++) = data[chBlue] *scale2;
	*(out_alpha++) = data[chAlpha]*scale3;
	data+=csize;
      }
      break;
    case GL_YUV422_GEM:
      n/=2;
      while(n--){
	*(out_red  ++) = data[chY0]   *scale0;
	*(out_red  ++) = data[chY1]   *scale0;
	*(out_green++) = data[chU] *scale1;
	*(out_green++) = data[chU] *scale1;
	*(out_blue ++) = data[chV] *scale2;
	*(out_blue ++) = data[chV] *scale2;
	*(out_alpha++) = 0; *(out_alpha++) = 0;
	data+=4;
      }
      break;
    case GL_LUMINANCE:
      while(n--){
	*(out_red  ++) = data[chGray]*scale0;
	*(out_green++) = 0;
	*(out_blue ++) = 0;
	*(out_alpha++) = 0;
	data++;
      }
      break;
    }
  } else {
    n=N;
    while (n--) *out_red++=*out_green++=*out_blue++=*out_alpha++=0;
  }


  return (w+7);
}

void pix_pix2sig :: dspMess(void *data, t_signal** sp)
{
  dsp_add(perform, 6, data, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[0]->s_n);
}

/////////////////////////////////////////////////////////
// Callback functions
//
/////////////////////////////////////////////////////////

void pix_pix2sig :: obj_setupCallback(t_class *classPtr)
{
  class_addcreator(reinterpret_cast<t_newmethod>(create_pix_pix2sig), gensym("pix_pix2sig~"), A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(pix_pix2sig::dspMessCallback), 
		  gensym("dsp"), A_NULL);
}

void pix_pix2sig :: dspMessCallback(void *data,t_signal** sp)
{
  GetMyClass(data)->dspMess(data, sp);
}
