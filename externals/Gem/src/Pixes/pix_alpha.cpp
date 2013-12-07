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

#include "pix_alpha.h"

CPPEXTERN_NEW(pix_alpha);

/////////////////////////////////////////////////////////
//
// pix_alpha
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_alpha :: pix_alpha()
{
    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("ft1"));
    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("ft2"));
    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("high_val"));
    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("low_val"));

    m_highThresh[0] = m_highThresh[1] = m_highThresh[2] = 0;
    m_lowThresh[0] = m_lowThresh[1] = m_lowThresh[2] = 0;
    m_alpha = 0;
    m_otheralpha = 255;

}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_alpha :: ~pix_alpha()
{ }

void pix_alpha :: processRGBAImage(imageStruct &image)
{
    // process the image
    int count = image.xsize * image.ysize;
    
    unsigned char *pixels = image.data;

    while(count--) {
      if ( (pixels[chRed] >= m_lowThresh[0] &&  pixels[chRed] <= m_highThresh[0] ) 
	   &&
	   (pixels[chGreen] >= m_lowThresh[1] &&  pixels[chGreen] <= m_highThresh[1] )
	   &&
	   (pixels[chBlue] >= m_lowThresh[2] &&  pixels[chBlue] <= m_highThresh[2] ) ) {
	pixels[chAlpha] = m_alpha;
      }
      else pixels[chAlpha] = m_otheralpha;
      pixels += 4;
    }    
}


/////////////////////////////////////////////////////////
// lowThreshMess
//
/////////////////////////////////////////////////////////
void pix_alpha :: lowThreshMess(float red, float green, float blue)
{
    m_lowThresh[0] = CLAMP(red * 255.f);
    m_lowThresh[1] = CLAMP(green * 255.f);
    m_lowThresh[2] = CLAMP(blue * 255.f);
    setPixModified();
}

/////////////////////////////////////////////////////////
// highThreshMess
//
/////////////////////////////////////////////////////////
void pix_alpha :: highThreshMess(float red, float green, float blue)
{
    m_highThresh[0] = CLAMP(red * 255.f);
    m_highThresh[1] = CLAMP(green * 255.f);
    m_highThresh[2] = CLAMP(blue * 255.f);
    setPixModified();
}

/////////////////////////////////////////////////////////
// alphaMess
//
/////////////////////////////////////////////////////////
void pix_alpha :: alphaMess(float alpha)
{
    m_alpha = CLAMP(alpha * 255.f);
    setPixModified();
}

/////////////////////////////////////////////////////////
// otheralphaMess
//
/////////////////////////////////////////////////////////
void pix_alpha :: otheralphaMess(float alpha)
{
    m_otheralpha = CLAMP(alpha * 255.f);
    setPixModified();
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_alpha :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_alpha::alphaMessCallback),
    	    gensym("ft1"), A_FLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_alpha::otheralphaMessCallback),
    	    gensym("ft2"), A_FLOAT, A_NULL);

    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_alpha::highThreshMessCallback),
    	    gensym("high_val"), A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_alpha::lowThreshMessCallback),
    	    gensym("low_val"), A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);
}
void pix_alpha :: highThreshMessCallback(void *data, t_floatarg red, t_floatarg green, t_floatarg blue)
{
    GetMyClass(data)->highThreshMess((float)red, (float)green, (float)blue);
}
void pix_alpha :: lowThreshMessCallback(void *data, t_floatarg red, t_floatarg green, t_floatarg blue)
{
    GetMyClass(data)->lowThreshMess((float)red, (float)green, (float)blue);
}
void pix_alpha :: alphaMessCallback(void *data, t_floatarg alpha)
{
    GetMyClass(data)->alphaMess((float)alpha);
}
void pix_alpha :: otheralphaMessCallback(void *data, t_floatarg alpha)
{
    GetMyClass(data)->otheralphaMess((float)alpha);
}
