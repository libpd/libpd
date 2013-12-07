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

#include "pix_sig2pix.h"
#include "Gem/State.h"

CPPEXTERN_NEW_WITH_TWO_ARGS(pix_sig2pix, t_float,A_DEFFLOAT,t_float, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// pix_sig2pix
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_sig2pix :: pix_sig2pix(t_floatarg width=0, t_floatarg height=0) : m_reqFormat(GL_RGBA)
{

  m_pixBlock.image = m_imageStruct;
  m_pixBlock.image.data=NULL;

  dimenMess((int)width, (int)height);	//tigital

  int i;
  for (i=0; i<3; i++)
    inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_signal, &s_signal); /* channels inlet */
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_sig2pix :: ~pix_sig2pix()
{}

void pix_sig2pix :: dimenMess(int width, int height) {
  if (width>32000)width=8;
  if (height>32000)height=8;
  if (width  < 0) width  = 0;
  if (height < 0) height = 0;

  m_width =width;
  m_height=height;

  if (width  == 0) width = 8;
  if (height == 0) height = 8;

  m_pixBlock.image.xsize =(GLint) width;
  m_pixBlock.image.ysize = (GLint) height;
  m_pixBlock.image.setCsizeByFormat(m_reqFormat);

  m_pixsize = m_pixBlock.image.xsize*m_pixBlock.image.ysize;
  m_pixBlock.image.reallocate();
  m_pixBlock.image.setBlack();
}

void pix_sig2pix :: csMess(GLint cs) {
  m_reqFormat=cs;
  m_pixBlock.image.setCsizeByFormat(m_reqFormat);
  m_pixsize = m_pixBlock.image.xsize*m_pixBlock.image.ysize;
  m_pixBlock.image.reallocate();
  m_pixBlock.image.setBlack();
}


/////////////////////////////////////////////////////////
void pix_sig2pix :: render(GemState *state)
{
  state->set(GemState::_PIX,&m_pixBlock);
}

/////////////////////////////////////////////////////////
// postrender
//
/////////////////////////////////////////////////////////
void pix_sig2pix :: postrender(GemState *state)
{
  m_pixBlock.newimage = 0;
  state->set(GemState::_PIX, static_cast<pixBlock*>(NULL));
}

/////////////////////////////////////////////////////////
// startRendering
//
/////////////////////////////////////////////////////////
void pix_sig2pix :: startRendering()
{
  m_pixBlock.newimage = 1;
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////

t_int* pix_sig2pix :: perform(t_int* w)
{
  pix_sig2pix *x = GetMyClass((void*)w[1]);
  t_float* in_red =   (t_float*)(w[2]);
  t_float* in_green = (t_float*)(w[3]);
  t_float* in_blue =  (t_float*)(w[4]);
  t_float* in_alpha = (t_float*)(w[5]);
  t_int n = (t_int)(w[6]);

  unsigned char* data = x->m_pixBlock.image.data;
  if (n > x->m_pixsize) n = x->m_pixsize;

  switch(x->m_pixBlock.image.format){
  case GL_RGBA:  default:
    while(n--){
      data[chRed]   = (unsigned char) (*in_red++  *255.0);
      data[chGreen] = (unsigned char) (*in_green++*255.0);
      data[chBlue]  = (unsigned char) (*in_blue++ *255.0);
      data[chAlpha] = (unsigned char) (*in_alpha++*255.0);
      data+=4;
    }
    break;
  case GL_YUV422_GEM:
    n/=2;
    while(n--){
      data[chY0] = (unsigned char) (*in_red++  *255.0);
      data[chU ] = (unsigned char) (*in_green++  *255.0);
      in_green++;
      data[chY1] = (unsigned char) (*in_red++  *255.0);
      data[chV ] = (unsigned char) (*in_blue++  *255.0);
      in_blue++;

      data+=4;
    }
    break;
  case GL_LUMINANCE:
    while(n--){
      *data++ = (unsigned char) (*in_red++  *255.0);
    }
    break;
  }
  x->m_pixBlock.newimage = 1;
  return (w+7);
}

void pix_sig2pix :: dspMess(void *data, t_signal** sp)
{
  if (m_width==0 && m_height==0){
    int w = powerOfTwo((int)sqrt((double)sp[0]->s_n));
    int h = (sp[0]->s_n / w);
    dimenMess(w, h);
    m_width = 0;
    m_height= 0;
  }
  m_pixBlock.image.setBlack();
  dsp_add(perform, 6, data, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[0]->s_n);
}

/////////////////////////////////////////////////////////
// Callback functions
//
/////////////////////////////////////////////////////////

void pix_sig2pix :: obj_setupCallback(t_class *classPtr)
{
  class_addcreator(reinterpret_cast<t_newmethod>(create_pix_sig2pix), gensym("pix_sig2pix~"), A_DEFFLOAT, A_DEFFLOAT, A_NULL);

  class_addmethod(classPtr, nullfn, gensym("signal"), A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(pix_sig2pix::dspMessCallback), 
		  gensym("dsp"), A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(pix_sig2pix::dimenMessCallback), 
		  gensym("dimen"), A_DEFFLOAT,A_DEFFLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(pix_sig2pix::csMessCallback), 
		  gensym("colorspace"), A_DEFSYMBOL, A_NULL);
}


void pix_sig2pix :: dspMessCallback(void *data,t_signal** sp)
{
  GetMyClass(data)->dspMess(data, sp);
}

void pix_sig2pix ::dimenMessCallback(void *data, t_float w, t_float h)
{
  GetMyClass(data)->dimenMess((int)w, (int)h);
}
void pix_sig2pix ::csMessCallback(void *data, t_symbol*s)
{
  int cs = getPixFormat(s->s_name);
  if(cs>0)GetMyClass(data)->csMess(cs);
  else GetMyClass(data)->error("colorspace must be Grey, YUV or RGBA");
}
