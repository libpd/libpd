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
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "pix_bitmask.h"

CPPEXTERN_NEW(pix_bitmask);

/////////////////////////////////////////////////////////
//
// pix_bitmask
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_bitmask :: pix_bitmask()
{
    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("ft1"));
    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("vec_mask"));

    m_mask[chRed] = m_mask[chGreen] = m_mask[chBlue] = m_mask[chAlpha] = 255;
    m_mode=0;
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_bitmask :: ~pix_bitmask()
{ }

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_bitmask :: processRGBAImage(imageStruct &image)
{
  int datasize = image.xsize * image.ysize;
  unsigned char *pixels = image.data;
    
  while(datasize--)	{
    pixels[chRed] &= m_mask[chRed];
    pixels[chGreen] &= m_mask[chGreen];
    pixels[chBlue] &= m_mask[chBlue];
    pixels[chAlpha] &= m_mask[chAlpha];
    pixels += 4;
  }
}
void pix_bitmask :: processYUVImage(imageStruct &image)
{
  int datasize = image.xsize * image.ysize / 2;
  unsigned char *pixels = image.data;
  
  if (m_mode)
    while(datasize--)	{
      pixels[chU] &= m_mask[chGreen];
      pixels[chY0] &= m_mask[chRed];
      pixels[chV] &= m_mask[chBlue];
      pixels[chY1] &= m_mask[chRed];
      pixels += 4;
    }
  else
    while(datasize--)	{
      pixels[chY0] &= m_mask[chRed];
      pixels[chY1] &= m_mask[chRed];
      pixels += 4;
    }
}
void pix_bitmask :: processGrayImage(imageStruct &image)
{
  int datasize = image.xsize * image.ysize;
  unsigned char *pixels = image.data;
    
  while(datasize--)	{
    pixels[chGray] &= m_mask[chRed];
    pixels++;
  }
}

#ifdef __MMX__
void pix_bitmask :: processRGBAMMX(imageStruct &image){
  int i = image.xsize * image.ysize/2;
  const int*mask_ip=reinterpret_cast<const int*>(m_mask);
  __m64 mask = _mm_set_pi32(*mask_ip, *mask_ip);
  __m64 *input = reinterpret_cast<__m64*>(image.data);

  while(i--){
    input[0]= _mm_and_si64(input[0], mask);
    input++;
  }
  _mm_empty();

}
void pix_bitmask :: processYUVMMX(imageStruct &image){
  int i = image.xsize * image.ysize/4;

  const __m64 mask = _mm_set_pi8(m_mask[chRed],
				 m_mask[chBlue],
				 m_mask[chRed],
				 m_mask[chGreen],
				 m_mask[chRed],
				 m_mask[chBlue],
				 m_mask[chRed],
				 m_mask[chGreen]);
  __m64 *input = (__m64*)image.data;

  while(i--){
    input[0]= _mm_and_si64(input[0], mask);
    input++;
  }
  _mm_empty();
}
void pix_bitmask :: processGrayMMX(imageStruct &image){
  int i = image.xsize * image.ysize/8;
  const char grey=m_mask[chRed];

  const __m64 mask = _mm_set_pi8(grey, grey, grey, grey, grey, grey, grey, grey);
  __m64 *input = (__m64*)image.data;

  while(i--){
    input[0]= _mm_and_si64(input[0], mask);
    input++;
  }
  _mm_empty();
}
#endif

/////////////////////////////////////////////////////////
// vecMaskMess
//
/////////////////////////////////////////////////////////
void pix_bitmask :: vecMaskMess(int argc, t_atom *argv)
{
    if (argc >= 4) m_mask[chAlpha] = atom_getint(&argv[3]);
    else if (argc == 3) m_mask[chAlpha] = 255;
    else
    {
    	error("not enough mask values");
    	return;
    }
    m_mask[chRed] = atom_getint(&argv[0]);
    m_mask[chGreen] = atom_getint(&argv[1]);
    m_mask[chBlue] = atom_getint(&argv[2]);
    setPixModified();
    m_mode=1;
}

/////////////////////////////////////////////////////////
// intMaskMess
//
/////////////////////////////////////////////////////////
void pix_bitmask :: intMaskMess(int mask)
{
    // assumption that the alpha should be 255
    m_mask[chAlpha] = 255;
    m_mask[chRed] = m_mask[chGreen] = m_mask[chBlue] = mask;
    setPixModified();
    m_mode=0;
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_bitmask :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_bitmask::vecMaskMessCallback),
    	    gensym("vec_mask"), A_GIMME, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_bitmask::floatMaskMessCallback),
    	    gensym("ft1"), A_FLOAT, A_NULL);
}
void pix_bitmask :: vecMaskMessCallback(void *data, t_symbol *, int argc, t_atom *argv)
{
    GetMyClass(data)->vecMaskMess(argc, argv);
}
void pix_bitmask :: floatMaskMessCallback(void *data, t_floatarg mask)
{
    GetMyClass(data)->intMaskMess((int)mask);
}
