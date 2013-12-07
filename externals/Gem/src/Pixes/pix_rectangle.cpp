////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-2000 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////
//
//  pix_rectangle
//
//  2002:forum::für::umläute:2000
//  iohannes m zmoelnig
//  mailto:zmoelnig@iem.mhsg.ac.at
//
/////////////////////////////////////////////////////////


#include "pix_rectangle.h"
#include "Gem/PixConvert.h"

CPPEXTERN_NEW(pix_rectangle);

/////////////////////////////////////////////////////////
//
// pix_rectangle
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_rectangle :: pix_rectangle()
{
    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("coord"));
    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("color"));

    m_color[chRed] = m_color[chGreen] = m_color[chBlue] = m_color[chAlpha] = 255;
    m_lower_left[0] = m_lower_left[1] = m_upper_right[0] = m_upper_right[1] = 0;
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_rectangle :: ~pix_rectangle()
{}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_rectangle :: processRGBAImage(imageStruct &image)
{
  int pixelsize = image.csize;
  int rowsize  = image.xsize * pixelsize;
  unsigned char *pixels = image.data;
  int col, row;

  unsigned char r=m_color[chRed];
  unsigned char g=m_color[chGreen];
  unsigned char b=m_color[chBlue];
  unsigned char a=m_color[chAlpha];

  if (m_upper_right[0] > image.xsize) m_upper_right[0] = image.xsize;
  if (m_lower_left[0] > image.xsize)  m_lower_left[0] = image.xsize;
 
  if (m_upper_right[1] > image.ysize) m_upper_right[1] = image.ysize;
  if (m_lower_left[1] > image.ysize)  m_lower_left[1] = image.ysize;

  row = (m_upper_right[1] - m_lower_left[1]);
  while (row--)	{
    pixels = image.data + rowsize * (m_lower_left[1] + row) + m_lower_left[0] * pixelsize;
    col = (m_upper_right[0] - m_lower_left[0]);
    while (col--)		{
      pixels[chRed]   = r;
      pixels[chGreen] = g;
      pixels[chBlue]  = b;
      pixels[chAlpha] = a;
      pixels += 4;
    }
  }
}
void pix_rectangle :: processYUVImage(imageStruct &image)
{
  int pixelsize = image.csize;
  int rowsize  = image.xsize * pixelsize;
  unsigned char *pixels = image.data;
  int col, row;

  unsigned char y =((RGB2YUV_11*m_color[chRed]+RGB2YUV_12*m_color[chGreen]+RGB2YUV_13*m_color[chBlue])>>8)+ Y_OFFSET;
  unsigned char u =((RGB2YUV_21*m_color[chRed]+RGB2YUV_22*m_color[chGreen]+RGB2YUV_23*m_color[chBlue])>>8)+UV_OFFSET;
  unsigned char v =((RGB2YUV_31*m_color[chRed]+RGB2YUV_32*m_color[chGreen]+RGB2YUV_33*m_color[chBlue])>>8)+UV_OFFSET;

  if (m_upper_right[0] > image.xsize)  m_upper_right[0] = image.xsize;
  if (m_lower_left[0]  > image.xsize)   m_lower_left[0] = image.xsize;
  if (m_upper_right[1] > image.ysize)  m_upper_right[1] = image.ysize;
  if (m_lower_left[1] > image.ysize)    m_lower_left[1] = image.ysize;

  row = (m_upper_right[1] - m_lower_left[1]);
  
  while (row--)	{
    int offset=rowsize*(m_lower_left[1]+row) + m_lower_left[0] * pixelsize;
    offset-=(offset%4);
    pixels = image.data+offset;

    col = (m_upper_right[0] - m_lower_left[0])/2;
    
    while (col--)	{
      pixels[chY0]= y;
      pixels[chU] = u;
      pixels[chY1]= y;
      pixels[chV] = v;
      pixels += 4;
    }
  }
}
void pix_rectangle :: processGrayImage(imageStruct &image)
{
  int pixelsize = image.csize;
  int rowsize  = image.xsize * pixelsize;
  unsigned char *pixels = image.data;
  int col, row;
	
  if (m_upper_right[0] > image.xsize)m_upper_right[0] = image.xsize;
  if (m_lower_left[0]  > image.xsize)m_lower_left [0] = image.xsize;

  if (m_upper_right[1] > image.ysize)m_upper_right[1] = image.ysize;
  if (m_lower_left[1]  > image.ysize)m_lower_left [1] = image.ysize;

  unsigned char g=(m_color[chRed]*RGB2GRAY_RED+m_color[chGreen]*RGB2GRAY_GREEN+m_color[chBlue]*RGB2GRAY_BLUE)>>8;

  row = (m_upper_right[1] - m_lower_left[1]);

  while (row--)    {
    pixels = image.data + rowsize * (m_lower_left[1] + row) + m_lower_left[0] * pixelsize;
    col = (m_upper_right[0] - m_lower_left[0]);
    while (col--)        {
      pixels[chGray] = g;
      pixels++;
    }
  }
}

/////////////////////////////////////////////////////////
// vecColorMess
//
/////////////////////////////////////////////////////////
void pix_rectangle :: vecColorMess(int argc, t_atom *argv)
{
  float alpha, red, green, blue;

  if (argc >= 4)alpha = atom_getfloat(&argv[3]);
  else if (argc == 3)	alpha = 1.f;
  else    {
    error("not enough color values");
    return;
  }
  red   = atom_getfloat(&argv[0]);
  green = atom_getfloat(&argv[1]);
  blue  = atom_getfloat(&argv[2]);

  m_color[chRed]   = static_cast<unsigned char>(255.*red);
  m_color[chGreen] = static_cast<unsigned char>(255.*green);
  m_color[chBlue]  = static_cast<unsigned char>(255.*blue);
  m_color[chAlpha] = static_cast<unsigned char>(255.*alpha);

  setPixModified();
}

/////////////////////////////////////////////////////////
// vecCoordMess
//
/////////////////////////////////////////////////////////
void pix_rectangle :: vecCoordMess(int argc, t_atom *argv)
{
  int X1, Y1, X2, Y2;

  if (argc < 4)    {
    error("not enough coordinates");
    return;
  }
  X1 = atom_getint(&argv[0]);
  Y1 = atom_getint(&argv[1]);
  X2 = atom_getint(&argv[2]);
  Y2 = atom_getint(&argv[3]);
  
  // check if within range
  if (X1 < 0)X1 = 0;
  if (X2 < 0)X2 = 0;
  if (Y1 < 0)Y1 = 0;
  if (Y2 < 0)Y2 = 0;

  // set
  m_lower_left [0] = (X1<X2)?X1:X2;
  m_lower_left [1] = (Y1<Y2)?Y1:Y2;
  m_upper_right[0] = (X1>X2)?X1:X2;
  m_upper_right[1] = (Y1>Y2)?Y1:Y2;

  setPixModified();
}


/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_rectangle :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_rectangle::vecCoordMessCallback),
    	    gensym("coord"), A_GIMME, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_rectangle::vecColorMessCallback),
    	    gensym("color"), A_GIMME, A_NULL);

}
void pix_rectangle :: vecCoordMessCallback(void *data, t_symbol *, int argc, t_atom *argv)
{
    GetMyClass(data)->vecCoordMess(argc, argv);
}
void pix_rectangle :: vecColorMessCallback(void *data, t_symbol *, int argc, t_atom *argv)
{
    GetMyClass(data)->vecColorMess(argc, argv);
}
