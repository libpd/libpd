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

#include "pix_duotone.h"

CPPEXTERN_NEW(pix_duotone);

/////////////////////////////////////////////////////////
//
// pix_duotone
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_duotone :: pix_duotone()
{
  m_color1[0]=m_color1[1]=m_color1[2]=255;
  m_color2[0]=m_color2[1]=m_color2[2]=000;
  m_thresh[0]=m_thresh[1]=m_thresh[2]=127;

  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"),gensym("thresh"));
  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("color1"));
  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("color2"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_duotone :: ~pix_duotone()
{ }

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_duotone :: processRGBAImage(imageStruct &image)
{
  int datasize =  image.xsize * image.ysize;
  unsigned char *pixels = image.data;

  while(datasize--) {
    if ((pixels[chRed] > m_thresh[0]) && (pixels[chGreen] > m_thresh[1]) && (pixels[chBlue] > m_thresh[2])){
      pixels[chRed]   = m_color1[0];
      pixels[chGreen] = m_color1[1];
      pixels[chBlue]  = m_color1[2];
    } else {
      pixels[chRed]   = m_color2[0];
      pixels[chGreen] = m_color2[1];
      pixels[chBlue]  = m_color2[2];
    }
    pixels += 4;
  }
}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_duotone :: processRGBImage(imageStruct &image)
{
  int datasize =  image.xsize * image.ysize;
  unsigned char *pixels = image.data;

  while(datasize--) {
    if ((pixels[chRed] > m_thresh[0]) && (pixels[chGreen] > m_thresh[1]) && (pixels[chBlue] > m_thresh[2])){
    pixels[chRed]   = m_color1[0];
    pixels[chGreen] = m_color1[1];
    pixels[chBlue]  = m_color1[2];
    
  } else {
    pixels[chRed]   = m_color2[0];
    pixels[chGreen] = m_color2[1];
    pixels[chBlue]  = m_color2[2];
  }
  pixels += 3;
  }
}

/////////////////////////////////////////////////////////
// processGrayImage
//
/////////////////////////////////////////////////////////
void pix_duotone :: processGrayImage(imageStruct &image)
{
  int datasize =  image.xsize * image.ysize*image.csize;
  unsigned char *pixels = image.data;
  unsigned char thresh=m_thresh[0], grey1=m_color1[0], grey2=m_color2[0];
  
  while(datasize--) {
    *pixels=(*pixels>thresh)?grey1:grey2;
    pixels++;
  }
}

/////////////////////////////////////////////////////////
// do the YUV processing here
//
/////////////////////////////////////////////////////////
void pix_duotone :: processYUVImage(imageStruct &image)
{
  int h,w;
  long src;

  src = 0;

  for (h=0; h<image.ysize; h++){
    for(w=0; w<image.xsize/2; w++){
      if ((image.data[src] > m_thresh[1]) && (image.data[src+1] > m_thresh[0]) && (image.data[src+2] > m_thresh[2]))
        {
	  image.data[src]   = m_color1[1];
	  image.data[src+1] = m_color1[0];
	  image.data[src+2] = m_color1[2];
	  image.data[src+3] = m_color1[0];
        }else{
	  image.data[src]   = m_color2[1];
	  image.data[src+1] = m_color2[0];
	  image.data[src+2] = m_color2[2];
	  image.data[src+3] = m_color2[0];
        }
      src+=4;
    }
  }
}


/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_duotone :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_duotone::color1MessCallback),
    	    gensym("color1"), A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_duotone::color2MessCallback),
    	    gensym("color2"), A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_duotone::threshMessCallback),
    	    gensym("thresh"), A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);
}
void pix_duotone :: color1MessCallback(void *data, t_floatarg value1, t_floatarg value2, t_floatarg value3)
{
    GetMyClass(data)->m_color1[0]=(unsigned char)(value1*255);
    GetMyClass(data)->m_color1[1]=(unsigned char)(value2*255);
    GetMyClass(data)->m_color1[2]=(unsigned char)(value3*255);
    GetMyClass(data)->setPixModified();
}
void pix_duotone :: color2MessCallback(void *data, t_floatarg value1, t_floatarg value2, t_floatarg value3)
{
    GetMyClass(data)->m_color2[0]=(unsigned char)(value1*255);
    GetMyClass(data)->m_color2[1]=(unsigned char)(value2*255);
    GetMyClass(data)->m_color2[2]=(unsigned char)(value3*255);
    GetMyClass(data)->setPixModified();
}

void pix_duotone :: threshMessCallback(void *data, t_floatarg value1, t_floatarg value2, t_floatarg value3)
{
    GetMyClass(data)->m_thresh[0]=(unsigned char)(value1*255);
    GetMyClass(data)->m_thresh[1]=(unsigned char)(value2*255);
    GetMyClass(data)->m_thresh[2]=(unsigned char)(value3*255);
    GetMyClass(data)->setPixModified();
}

