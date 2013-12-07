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

#include "pix_color.h"
#include "Gem/PixConvert.h"

CPPEXTERN_NEW(pix_color);

/////////////////////////////////////////////////////////
//
// pix_color
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_color :: pix_color()
{
    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("vec_gain"));
    m_color[0] = m_color[1] = m_color[2] = 255;
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_color :: ~pix_color()
{ }

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void pix_color :: processRGBAImage(imageStruct &image)
{
    int i = image.xsize * image.ysize;
    
    unsigned char *base = image.data;
	while (i--)
    {
      base[chRed] = m_color[0];
      base[chGreen] = m_color[1];
      base[chBlue] = m_color[2];
      base += 4;
    }
}
void pix_color :: processGrayImage(imageStruct &image)
{
  int i = image.xsize * image.ysize;
  unsigned char grey=(m_color[0]*RGB2GRAY_RED+m_color[1]*RGB2GRAY_GREEN+m_color[2]*RGB2GRAY_BLUE)>>8;
  unsigned char *base = image.data;
  while (i--)*base++=grey;
}
void pix_color :: processYUVImage(imageStruct &image)
{
  int i = image.xsize * image.ysize / 2;
  unsigned char y =(( RGB2YUV_11 * m_color[0] + RGB2YUV_12 * m_color[1] + RGB2YUV_13 * m_color[2])>>8) +  Y_OFFSET;
  unsigned char u =(( RGB2YUV_21 * m_color[0] + RGB2YUV_22 * m_color[1] + RGB2YUV_23 * m_color[2])>>8) + UV_OFFSET;
  unsigned char v =(( RGB2YUV_31 * m_color[0] + RGB2YUV_32 * m_color[1] + RGB2YUV_33 * m_color[2])>>8) + UV_OFFSET;

  unsigned char *base = image.data;
  while (i--){
    base[chU]=u; base[chY0]=y;
    base[chV]=v; base[chY1]=y;
    base+=4;
  }
}
/////////////////////////////////////////////////////////
// vecGainMess
//
/////////////////////////////////////////////////////////
void pix_color :: vecGainMess(float red, float green, float blue, float alpha)
{
    m_color[0] = CLAMP(red   * 255);
    m_color[1] = CLAMP(green * 255);
    m_color[2] = CLAMP(blue  * 255);
    m_color[3] = CLAMP(alpha * 255);
   setPixModified();
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_color :: obj_setupCallback(t_class *classPtr)
{
  class_addcreator(reinterpret_cast<t_newmethod>(create_pix_color), 
		   gensym("pix_colour"), A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_color::vecGainMessCallback),
    	    gensym("vec_gain"), A_GIMME, A_NULL);
}
void pix_color :: vecGainMessCallback(void *data, t_symbol*, int argc, t_atom*argv)
{
  float r=0.0, g=0.0, b=0.0;
  float a=1.0;
  switch(argc){
  case 1:
    r=g=b=atom_getfloat(argv); break;
  case 4:
    a=atom_getfloat(argv+3);
  case 3:
    r=atom_getfloat(argv+0);
    g=atom_getfloat(argv+1);
    b=atom_getfloat(argv+2);
    break;
  default:
    GetMyClass(data)->error("\"color\" must be 1, 3 or 4 values");
    return;
  }
   
  GetMyClass(data)->vecGainMess(r, g, b, a);
}
