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
 
#include "pix_offset.h"

CPPEXTERN_NEW(pix_offset);
 
/////////////////////////////////////////////////////////
//
// pix_offset
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_offset :: pix_offset()
  : Y(0), U(0), V(0),
	m_saturate(true)
{
  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("ft1"));
  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("vec_offset"));
  m_offset[chRed] = m_offset[chGreen] = m_offset[chBlue] = m_offset[chAlpha] = 0;
}

////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_offset :: ~pix_offset()
{ }

////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_offset :: processRGBAImage(imageStruct &image)
{
  int datasize =  image.xsize * image.ysize;
  unsigned char *pixels = image.data;
  unsigned char o_red=m_offset[chRed], o_green=m_offset[chGreen], o_blue=m_offset[chBlue], o_alpha=m_offset[chAlpha];

  if(m_saturate) {

    while(datasize--) {
      short red, blue, green, alpha;

      red=  pixels[chRed]  +o_red;
      green=pixels[chGreen]+o_green;
      blue= pixels[chBlue] +o_blue;
      alpha=pixels[chAlpha]+o_alpha;

      pixels[chRed]   = CLAMP(red);
      pixels[chGreen] = CLAMP(green);
      pixels[chBlue]  = CLAMP(blue);
      pixels[chAlpha] = CLAMP(alpha);
      pixels += 4;
    }

  } else { // no saturated logic

    while(datasize--) {
      pixels[chRed]   += o_red;
      pixels[chGreen] += o_green;
      pixels[chBlue]  += o_blue;
      pixels[chAlpha] += o_alpha;
      pixels += 4;
    }
  }
}

////////////////////////////////////////////////////////
// processGrayImage
//
/////////////////////////////////////////////////////////
void pix_offset :: processGrayImage(imageStruct &image)
{
  int datasize =  image.xsize * image.ysize*image.csize;
  unsigned char *pixels = image.data;
  unsigned char m_grey=m_offset[chRed];

  if(m_saturate) {
    short grey;
    while(datasize--){
      grey = *pixels + m_grey;
      *pixels++ = CLAMP(grey);
    }
  } else 
    while(datasize--)*pixels++ += m_grey;
}

////////////////////////////////////////////////////////
// do the YUV processing here
//
////////////////////////////////////////////////////////
void pix_offset :: processYUVImage(imageStruct &image)
{
  int h,w;
  long src = 0;
  
  //format is U Y V Y  
  for (h=0; h<image.ysize; h++){
    for(w=0; w<image.xsize/2; w++){
      image.data[src+chU ] = CLAMP( image.data[src+chU] + U );
      image.data[src+chY0] = CLAMP( image.data[src+chY0]+ Y );
      image.data[src+chV ] = CLAMP( image.data[src+chV] + V );
      image.data[src+chY1] = CLAMP( image.data[src+chY1]+ Y );

      src+=4;
    }
  }
}
#ifdef __MMX__
void pix_offset :: processRGBAMMX(imageStruct &image)
{
  char  R = m_offset[chRed];
  char  G = m_offset[chGreen];
  char  B = m_offset[chBlue];
  char  A = m_offset[chAlpha];

  register int pixsize = (image.ysize * image.xsize)>>1;

  register __m64 offset_64 = _mm_setr_pi8(R, G, B, A, R, G, B, A);
  register __m64*data_p= (__m64*)image.data;
  _mm_empty();

  if(m_saturate) {
    while(pixsize--) {
      data_p[0]=_mm_adds_pu8(data_p[0], offset_64);
      data_p++;
    }
  } else {
    while(pixsize--) {
      data_p[0]=_mm_add_pi8(data_p[0], offset_64);
      data_p++;      
    }
  }
  _mm_empty();
}

void pix_offset :: processYUVMMX(imageStruct &image)
{
  register int pixsize = (image.ysize * image.xsize)>>2;

  register __m64 offset_64 = _mm_setr_pi8(U, Y, V, Y, U, Y, V, Y);
  register __m64*data_p= (__m64*)image.data;
  _mm_empty();

  while(pixsize--) {
    data_p[0]=_mm_add_pi8(data_p[0], offset_64);
    data_p++;      
  }
  _mm_empty();
}
void pix_offset :: processGrayMMX(imageStruct &image)
{
  unsigned char m_grey=m_offset[chRed];

  register int pixsize = (image.ysize * image.xsize)>>3;

  register __m64 offset_64 = _mm_set1_pi8(m_grey);
  register __m64*data_p= reinterpret_cast<__m64*>(image.data);
  _mm_empty();

  if(m_saturate) {
    while(pixsize--) {
      data_p[0]=_mm_adds_pu8(data_p[0], offset_64);
      data_p++;
    }
  } else {
    while(pixsize--) {
      data_p[0]=_mm_add_pi8(data_p[0], offset_64);
      data_p++;      
    }
  }
  _mm_empty();
}
#endif /* MMX */

#ifdef __VEC__
/* more optimized version - unrolled and load-hoisted */
void pix_offset :: processYUVAltivec(imageStruct &image)
{
  register int h,w,width,height;
  width = image.xsize/16; //for altivec
  height = image.ysize;
  //format is U Y V Y
  // start of working altivec function 
  union
  {
    short	elements[8];
    vector	signed short v;
  }transferBuffer;
    
  register vector signed short c, hi, lo;
  register vector signed short hi1, lo1;
  register vector signed short loadhi, loadhi1, loadlo, loadlo1;
  register vector unsigned char zero = vec_splat_u8(0);
  register vector unsigned char *inData = (vector unsigned char*) image.data;

  //Write the pixel (pair) to the transfer buffer
  //transferBuffer.i = (U << 24) | (Y << 16) | (V << 8 ) | Y;
  transferBuffer.elements[0] = U;
  transferBuffer.elements[1] = Y;
  transferBuffer.elements[2] = V;
  transferBuffer.elements[3] = Y;
  transferBuffer.elements[4] = U;
  transferBuffer.elements[5] = Y;
  transferBuffer.elements[6] = V;
  transferBuffer.elements[7] = Y;

  //Load it into the vector unit
  c = transferBuffer.v;

    
#ifndef PPC970
  UInt32			prefetchSize = GetPrefetchConstant( 16, 1, 256 );
  vec_dst( inData, prefetchSize, 0 );
  vec_dst( inData+16, prefetchSize, 1 );
  vec_dst( inData+32, prefetchSize, 2 );
  vec_dst( inData+64, prefetchSize, 3 );
#endif  
     
  //expand the UInt8's to short's
  loadhi = (vector signed short) vec_mergeh( zero, inData[0] );
  loadlo = (vector signed short) vec_mergel( zero, inData[0] );
           
  loadhi1 = (vector signed short) vec_mergeh( zero, inData[1] );
  loadlo1 = (vector signed short) vec_mergel( zero, inData[1] );  \
           
        
  for ( h=0; h<height; h++){
    for (w=0; w<width; w++){
        
#ifndef PPC970
      vec_dst( inData, prefetchSize, 0 );
      vec_dst( inData+16, prefetchSize, 1 );
      vec_dst( inData+32, prefetchSize, 2 );
      vec_dst( inData+64, prefetchSize, 3 );
#endif
                    
      //add the constant to it
      hi = vec_add( loadhi, c );
      lo = vec_add( loadlo, c );
            
      hi1 = vec_add( loadhi1, c );
      lo1 = vec_add( loadlo1, c );
            
            
      //expand the UInt8's to short's
      loadhi = (vector signed short) vec_mergeh( zero, inData[2] );
      loadlo = (vector signed short) vec_mergel( zero, inData[2] );
           
            
      loadhi1 = (vector signed short) vec_mergeh( zero, inData[3] );
      loadlo1 = (vector signed short) vec_mergel( zero, inData[3] );
            
      //pack the result back down, with saturation
      inData[0] = vec_packsu( hi, lo );
      inData++;
            
            
      inData[0] = vec_packsu( hi1, lo1 );
      inData++;
    }
  }
    
  //
  // finish the last iteration after the loop
  //
  hi = vec_add( loadhi, c );
  lo = vec_add( loadlo, c );
            
  hi1 = vec_add( loadhi1, c );
  lo1 = vec_add( loadlo1, c );
            
  //pack the result back down, with saturation
  inData[0] = vec_packsu( hi, lo );
            
  inData++;
            
  inData[0] = vec_packsu( hi1, lo1 );
            
  inData++;
    
#ifndef PPC970
  vec_dss( 0 );
  vec_dss( 1 );
  vec_dss( 2 );
  vec_dss( 3 );  //end of working altivec function 
#endif
}
#endif


/////////////////////////////////////////////////////////
// vecOffsetMess
//
/////////////////////////////////////////////////////////
void pix_offset :: vecOffsetMess(int argc, t_atom *argv)
{
  if (argc >= 4) m_offset[chAlpha] = static_cast<int>(255.*atom_getfloat(&argv[3]));
  else if (argc == 3) m_offset[chAlpha] = 0;
  else
    {
      error("not enough offset values");
      return;
    }
  m_offset[chRed]   = static_cast<int>(255*atom_getfloat(&argv[0]));
  m_offset[chGreen] = static_cast<int>(255*atom_getfloat(&argv[1]));
  m_offset[chBlue]  = static_cast<int>(255*atom_getfloat(&argv[2]));
  Y = static_cast<short>(255*atom_getfloat(&argv[0]));
  U = static_cast<short>(255*atom_getfloat(&argv[1]));
  V = static_cast<short>(255*atom_getfloat(&argv[2]));
  setPixModified();
}

/////////////////////////////////////////////////////////
// floatOffsetMess
//
/////////////////////////////////////////////////////////
void pix_offset :: floatOffsetMess(float foffset)
{
  // assumption that the alpha should be one
  m_offset[chAlpha] = 0;
  m_offset[chRed] = m_offset[chGreen] = m_offset[chBlue] = static_cast<int>(255*foffset);
  Y = U = V = static_cast<short>(255*foffset);
  setPixModified();
}
/////////////////////////////////////////////////////////
// saturated math
//
/////////////////////////////////////////////////////////
void pix_offset :: saturateMess(int sat)
{
  m_saturate=(sat!=0);
  setPixModified();
}


/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_offset :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_offset::vecOffsetMessCallback),
                  gensym("vec_offset"), A_GIMME, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_offset::floatOffsetMessCallback),
                  gensym("ft1"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_offset::saturateMessCallback),
                  gensym("saturate"), A_FLOAT, A_NULL);
}
void pix_offset :: vecOffsetMessCallback(void *data, t_symbol *, int argc, t_atom *argv)
{
  GetMyClass(data)->vecOffsetMess(argc, argv);
}
void pix_offset :: floatOffsetMessCallback(void *data, t_floatarg offset)
{
  GetMyClass(data)->floatOffsetMess(offset);
}
void pix_offset :: saturateMessCallback(void *data, t_floatarg sat)
{
  GetMyClass(data)->saturateMess(static_cast<int>(sat));
}
