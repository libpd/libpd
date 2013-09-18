////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-1998 Mark Danks.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    Copyright (c) 2002 James Tittle & Chris Clepper
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////
//
//  pix_blob
//
//  0409:forum::für::umläute:2000
//  IOhannes m zmoelnig
//  mailto:zmoelnig@iem.kug.ac.at
//
/////////////////////////////////////////////////////////


#include "pix_blob.h"

CPPEXTERN_NEW_WITH_GIMME(pix_blob);

  /////////////////////////////////////////////////////////
  //
  // pix_blob
  //
  /////////////////////////////////////////////////////////
  // Constructor
  //
  /////////////////////////////////////////////////////////
  pix_blob :: pix_blob(int argc, t_atom *argv)
{
  if (argc) {
    if (argc==1) this->ChannelMess(atom_getint(argv));
    else this->GainMess(argc, argv);
  } else m_method = 0;

  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("channel"));
  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("gain"));

  m_xOut = outlet_new(this->x_obj, &s_float);
  m_yOut = outlet_new(this->x_obj, &s_float);
  m_zOut = outlet_new(this->x_obj, &s_float);
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_blob :: ~pix_blob()
{
  outlet_free(m_xOut);
  outlet_free(m_yOut);
  outlet_free(m_zOut);
}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_blob :: processRGBAImage(imageStruct &image)
{
  unsigned char *pixels = image.data;
  int rows  = image.ysize;

  //t_float x_inv = 1./image.xsize;
  //t_float y_inv = 1./image.ysize;

  int channel = -1;
  float gain_r = 0.3, gain_g = 0.3, gain_b = 0.3, gain_a = 0.1;

  float sum = 0.0, sum_x = 0.0, sum_y = 0.0;

  switch (m_method) {
  case 1:
    channel = chRed;
    break;
  case 2:
    channel = chGreen;
    break;
  case 3:
    channel = chBlue;
    break;
  case 4:
    channel = chAlpha;
    break;
  default:
    error("no method %d: using GREY", m_method);
  case 0:
    gain_r = 0.3086; gain_g = 0.6094; gain_b = 0.082; gain_a = 0.0;
    break;
  case -1:
    gain_r = m_gain[chRed];
    gain_g = m_gain[chGreen];
    gain_b = m_gain[chBlue];
    gain_a = m_gain[chAlpha];
  }

  if (channel==-1){
    while (rows--) {
      int cols = image.xsize;
      while (cols--) {
	float val  = gain_r * pixels[chRed] + gain_g * pixels[chGreen] +
	  gain_b * pixels[chBlue] + gain_a * pixels[chAlpha];
	sum   += val;
	sum_y += rows * val;
	sum_x += cols * val;      
	pixels+=4;
      }
    }
  } else
    while (rows--) {
      int cols = image.xsize;
      while (cols--) {
	int val  = pixels[channel];
	sum   += val;
	sum_y += rows * val;
	sum_x += cols * val;      
	pixels+=4;
      }
    }

  outlet_float(m_zOut, sum/(image.xsize*image.ysize*255*(gain_r+gain_g+gain_b+gain_a)));
  if (sum) {
    outlet_float(m_yOut, 1 - sum_y/(image.ysize*sum));
    outlet_float(m_xOut, 1 - sum_x/(image.xsize*sum));
  }
}
void pix_blob :: processGrayImage(imageStruct &image)
{
  unsigned char *pixels = image.data;
  int rows  = image.ysize;

  //  float sum = 0.0, sum_x = 0.0, sum_y = 0.0;
  int sum = 0, sum_x = 0, sum_y = 0;

  while (rows--) {
    int cols = image.xsize;
    while (cols--) {
      int val  = *pixels++;
      sum   += val;
      sum_y += rows * val;
      sum_x += cols * val;      
    }
  }

  t_float sumf=sum/(image.xsize*image.ysize*255.);
  outlet_float(m_zOut, sumf);
  if (sum) {
    t_float sumxf=(static_cast<t_float>(sum_x)/(sum*image.xsize));
    t_float sumyf=(static_cast<t_float>(sum_y)/(sum*image.ysize));
    outlet_float(m_yOut, 1 - sumyf);
    outlet_float(m_xOut, 1 - sumxf);
  }
}
void pix_blob :: processYUVImage(imageStruct &image)
{
  unsigned char *pixels = image.data;
  int rows  = image.ysize;

  int sum = 0, sum_x = 0,sum_y = 0;
  while (rows--) {
    int cols = image.xsize;
    while (cols--) {
      int val  = pixels[chY0];
      sum   += val;
      sum_y += rows * val;
      sum_x += cols * val;
      pixels+=2;
    }
  }

  t_float sumf=sum/(image.xsize*image.ysize*255.);
  outlet_float(m_zOut, sumf);
  if (sum) {
    t_float sumxf=(static_cast<t_float>(sum_x)/(sum*image.xsize));
    t_float sumyf=(static_cast<t_float>(sum_y)/(sum*image.ysize));
    outlet_float(m_yOut, 1 - sumyf);
    outlet_float(m_xOut, 1 - sumxf);
  }

}

/////////////////////////////////////////////////////////
// trigger
//
/////////////////////////////////////////////////////////
void pix_blob :: ChannelMess(int  channel)
{
  if (channel<0 || channel>4) {
    error("channel out of range");
    return;
  }
  m_method = channel;
}
void pix_blob :: GainMess(int argc, t_atom *argv)
{
  t_float gain = 0.0;
  m_gain[chAlpha]=0.0;
  int i;

  switch (argc) {
  case 1:
    gain = atom_getfloat(argv);
    if (gain<=0) gain=1.0;
    m_gain[chRed]=m_gain[chGreen]=m_gain[chBlue]=gain;
    break;
  case 3:
  case 4:
    for (i=0; i<argc; i++){
      gain = atom_getfloat(argv++);
      m_gain[i]=(gain<0.)?0.:gain;
    }
    break;
  default:
    error("only 1, 3 or 4 gains are allowed");
    return;
  }

  m_method = -1;
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_blob :: obj_setupCallback(t_class *classPtr)
{
  //  class_addbang(classPtr, reinterpret_cast<t_method>(&pix_blob::triggerMessCallback));
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_blob::channelMessCallback),
		  gensym("channel"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_blob::gainMessCallback),
		  gensym("gain"), A_GIMME, A_NULL);
}

void pix_blob :: gainMessCallback(void *data, t_symbol *, int argc, t_atom *argv)
{
  GetMyClass(data)->GainMess(argc, argv);
}

void pix_blob :: channelMessCallback(void *data, t_floatarg channel)
{
  GetMyClass(data)->ChannelMess((int)channel);
}
