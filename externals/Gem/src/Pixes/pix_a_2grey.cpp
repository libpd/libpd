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
//    Copyright (c) 2002 James Tittle & Chris Clepper
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "pix_a_2grey.h"
#include "Gem/PixConvert.h"

CPPEXTERN_NEW_WITH_ONE_ARG(pix_a_2grey, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// pix_a_2grey
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_a_2grey :: pix_a_2grey(t_floatarg alpha)
{
	m_mode = static_cast<int>(alpha * 255.f);
    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("ft1"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_a_2grey :: ~pix_a_2grey()
{ }

/////////////////////////////////////////////////////////
// alphaMess
//
/////////////////////////////////////////////////////////
void pix_a_2grey :: alphaMess(float alphaval)
{
	if (alphaval > 1.f)
		alphaval =  1.f;
	if (alphaval < -1.f)
		alphaval = -1.f;

	m_mode = static_cast<int>(alphaval*255.f);

	setPixModified();
}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_a_2grey :: processRGBAImage(imageStruct &image)
{
  if (!m_mode)return;

  unsigned char *pixels = image.data;
  int count  = image.ysize * image.xsize;

  if (m_mode < 0){
    const int realVal = -m_mode;
    while (count--) {
      if (pixels[chAlpha] < realVal){
	const int grey = (pixels[chRed] * RGB2GRAY_RED + pixels[chGreen] * RGB2GRAY_GREEN + pixels[chBlue] * RGB2GRAY_BLUE)>>8;
	pixels[chRed] = pixels[chGreen] = pixels[chBlue] = (unsigned char)grey;
      }
      pixels += 4;
    }
  }else{
    while (count--){
      if (pixels[chAlpha] > m_mode){
        const int grey = (pixels[chRed] * RGB2GRAY_RED + pixels[chGreen] * RGB2GRAY_GREEN + pixels[chBlue] * RGB2GRAY_BLUE)>>8;
        pixels[chRed] = pixels[chGreen] = pixels[chBlue] = (unsigned char)grey;
      }
      pixels += 4;
    }    
  }
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_a_2grey :: obj_setupCallback(t_class *classPtr)
{
   class_addcreator(reinterpret_cast<t_newmethod>(create_pix_a_2grey), 
		   gensym("pix_a_2gray"), A_NULL);
   class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_a_2grey::alphaMessCallback),
    	    gensym("ft1"), A_FLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_a_2grey::alphaMessCallback),
    	    gensym("alpha"), A_FLOAT, A_NULL);
}

void pix_a_2grey :: alphaMessCallback(void *data, t_floatarg alphaval)
{
    GetMyClass(data)->alphaMess(alphaval);
}
