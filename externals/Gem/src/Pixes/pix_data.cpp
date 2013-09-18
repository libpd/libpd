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

#include "pix_data.h"

CPPEXTERN_NEW(pix_data);

/////////////////////////////////////////////////////////
//
// pix_data
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_data :: pix_data() :
  m_quality(0)
{
    // create the new inlet for the X position
    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("xPos"));
    // create the new inlet for the Y position
    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("yPos"));

    m_colorOut = outlet_new(this->x_obj, 0);
    m_grayOut = outlet_new(this->x_obj, 0);

    m_position[0] = m_position[1] = 0.f;
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_data :: ~pix_data()
{
	outlet_free(m_colorOut);
	outlet_free(m_grayOut);
}

/////////////////////////////////////////////////////////
// trigger
//
/////////////////////////////////////////////////////////
void pix_data :: trigger()
{
  // if we don't have a "right hand" image, then just return
  if (!m_pixRight || !m_pixRight->image.data)
    return;

  t_float red, green, blue, alpha, grey;


  t_float maxX= m_pixRight->image.xsize;
  t_float maxY= m_pixRight->image.ysize;

  t_float fxPos = m_position[0] * maxX;
  t_float fyPos = m_position[1] * maxY;

  if(fxPos<0)fxPos=0; if(fxPos>=maxX-1)fxPos=maxX-1;
  if(fyPos<0)fyPos=0; if(fyPos>=maxY-1)fyPos=maxY-1;

  int ixPos0 = 0+static_cast<int>(fxPos);
  int iyPos0 = 0+static_cast<int>(fyPos);

  switch(m_quality) {
  default: {
    int ixPos1 = 1+static_cast<int>(fxPos);
    int iyPos1 = 1+static_cast<int>(fyPos);

    if(ixPos1>=maxX-1)ixPos1=ixPos0;
    if(iyPos1>=maxY-1)iyPos1=iyPos0;

    t_float xFrac = fxPos - ixPos0;
    t_float yFrac = fyPos - iyPos0;
  
    unsigned char r[2][2], g[2][2], b[2][2], a[2][2], G[2][2];

    m_pixRight->image.getRGB(ixPos0, iyPos0, &r[0][0], &g[0][0], &b[0][0], &a[0][0]);
    m_pixRight->image.getRGB(ixPos1, iyPos0, &r[1][0], &g[1][0], &b[1][0], &a[1][0]);
    m_pixRight->image.getRGB(ixPos0, iyPos1, &r[0][1], &g[0][1], &b[0][1], &a[0][1]);
    m_pixRight->image.getRGB(ixPos1, iyPos1, &r[1][1], &g[1][1], &b[1][1], &a[1][1]);

    m_pixRight->image.getGrey(ixPos0, iyPos0, &G[0][0]);
    m_pixRight->image.getGrey(ixPos1, iyPos0, &G[1][0]);
    m_pixRight->image.getGrey(ixPos0, iyPos1, &G[0][1]);
    m_pixRight->image.getGrey(ixPos1, iyPos1, &G[1][1]);

    t_float xy00=(1-xFrac)*(1-yFrac);
    t_float xy01=(1-xFrac)*   yFrac ;
    t_float xy10=   xFrac *(1-yFrac);
    t_float xy11=   xFrac *   yFrac ;

#define BILIN4(x) (xy00*x[0][0] + xy01*x[0][1] + xy10*x[1][0] + xy11*x[1][1])
    red   = BILIN4(r) / 255.;
    green = BILIN4(g) / 255.;
    blue  = BILIN4(b) / 255.;
    alpha = BILIN4(a) / 255.;
    grey  = BILIN4(G) / 255.;
  }
    break;
  case 0: {
    unsigned char r, g, b, a, G;
    m_pixRight->image.getRGB(ixPos0, iyPos0, &r, &g, &b, &a);
    m_pixRight->image.getGrey(ixPos0, iyPos0, &G);

#define NONLIN(x) (x)
    red   = NONLIN(r) / 255.;
    green = NONLIN(g) / 255.;
    blue  = NONLIN(b) / 255.;
    alpha = NONLIN(a) / 255.;
    grey  = NONLIN(G) / 255.;
  }
    break;
  }
  
  t_atom atom[4];
  // send out the color information
  outlet_float(m_grayOut, grey);
  SETFLOAT(&atom[0], red);
  SETFLOAT(&atom[1], green);
  SETFLOAT(&atom[2], blue);
  SETFLOAT(&atom[3], alpha);
  outlet_list(m_colorOut, gensym("list"), 4, atom);	
}

void pix_data :: xPos(t_float f) {
  m_position[0]=FLOAT_CLAMP(f);
}
void pix_data :: yPos(t_float f) {
  m_position[1]=FLOAT_CLAMP(f);
}
void pix_data :: qualityMess(int q) {
  if(q>=0)
    m_quality=q;
  else error("qualiy must be 0|1");
 
}



/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_data :: obj_setupCallback(t_class *classPtr)
{
  CPPEXTERN_MSG0(classPtr, "bang", trigger);
  CPPEXTERN_MSG1(classPtr, "xPos", xPos, t_float);
  CPPEXTERN_MSG1(classPtr, "yPos", yPos, t_float);

  CPPEXTERN_MSG1(classPtr, "quality", qualityMess, int);
}
