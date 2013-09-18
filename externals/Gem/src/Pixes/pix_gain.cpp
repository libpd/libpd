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

#include "pix_gain.h"
#include "Gem/Exception.h"

CPPEXTERN_NEW_WITH_GIMME(pix_gain);
  
/////////////////////////////////////////////////////////
//
// pix_gain
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_gain :: pix_gain(int argc, t_atom *argv)
  : m_saturate(true)
{
  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("ft1"));
  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("vec_gain"));
  m_gain[chRed] = m_gain[chGreen] = m_gain[chBlue] = m_gain[chAlpha] = 1.0f;

  switch(argc){
  case 3:
  case 4:
    vecGainMess(argc,argv);
    break;
  case 1:
    floatGainMess(atom_getfloat(argv));
  case 0:
    break;
  default:
    throw(GemException("needs 0, 1, 3, or 4 arguments"));
    break;
  }
}

////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_gain :: ~pix_gain()
{ }

////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_gain :: processRGBAImage(imageStruct &image)
{
  int datasize =  image.xsize * image.ysize;
  unsigned char *pixels = image.data;
  short R,G,B,A;
  int red,green,blue,alpha;
  R = static_cast<int>(256 * m_gain[chRed]);
  G = static_cast<int>(256 * m_gain[chGreen]);
  B = static_cast<int>(256 * m_gain[chBlue]);
  A = static_cast<int>(256 * m_gain[chAlpha]);

  if(m_saturate) {
    while(datasize--)
      {
	red =   (pixels[chRed  ] * R)>>8;
	pixels[chRed  ] = CLAMP(red);
	green = (pixels[chGreen] * G)>>8;
	pixels[chGreen] = CLAMP(green);
	blue =  (pixels[chBlue ] * B)>>8;
	pixels[chBlue ] = CLAMP(blue);
	alpha = (pixels[chAlpha] * A)>>8;
	pixels[chAlpha] = CLAMP(alpha);
	pixels += 4;
      }
  } else {
    while(datasize--)
      {
	pixels[chRed  ] = (pixels[chRed  ] * R)>>8;
	pixels[chGreen] = (pixels[chGreen] * G)>>8;
	pixels[chBlue ] = (pixels[chBlue ] * B)>>8;
	pixels[chAlpha] = (pixels[chAlpha] * A)>>8;
	pixels += 4;
      }
  }
}

////////////////////////////////////////////////////////
// processGrayImage
//
/////////////////////////////////////////////////////////
void pix_gain :: processGrayImage(imageStruct &image)
{
  int datasize =  image.xsize * image.ysize;
  unsigned char *pixels = image.data;
  if(m_saturate) {
    while (datasize--)
      {
	int gray = static_cast<int>(pixels[chGray] * m_gain[chRed]);
	pixels[chGray] = CLAMP(gray);
	pixels++;
      }
  } else {
    while (datasize--)
      {
	pixels[chGray] = static_cast<int>(pixels[chGray] * m_gain[chRed]);
	pixels++;
      }
  }
}
////////////////////////////////////////////////////////
// do the YUV processing here
//
/////////////////////////////////////////////////////////
void pix_gain :: processYUVImage(imageStruct &image)
{
  int h,w,width;
  long src;
  int y1,y2,u,v;

  short Y=static_cast<short>(m_gain[1] * 255);
  short U=static_cast<short>(m_gain[2] * 255);
  short V=static_cast<short>(m_gain[3] * 255);
  src = 0;
  width = image.xsize/2;
  if(m_saturate) {
    for (h=0; h<image.ysize; h++){
      for(w=0; w<width; w++){
	  
	u = (((image.data[src] - 128) * U)>>8)+128;
	image.data[src] = static_cast<unsigned char>(CLAMP(u));
	  
	y1 = (image.data[src+1] * Y)>>8;
	image.data[src+1] = static_cast<unsigned char>(CLAMP(y1));
	  
	v = (((image.data[src+2] - 128) * V)>>8)+128;
	image.data[src+2] = static_cast<unsigned char>(CLAMP(v));
	  
	y2 = (image.data[src+3] * Y)>>8;
	image.data[src+3] = static_cast<unsigned char>(CLAMP(y2));
	  
	src+=4;
      }
    }
  } else {
    for (h=0; h<image.ysize; h++){
      for(w=0; w<width; w++){
	  
	u = (((image.data[src] - 128) * U)>>8)+128;
	image.data[src] = static_cast<unsigned char>(u);
	  
	y1 = (image.data[src+1] * Y)>>8;
	image.data[src+1] = static_cast<unsigned char>(y1);
	  
	v = (((image.data[src+2] - 128) * V)>>8)+128;
	image.data[src+2] = static_cast<unsigned char>(v);
	  
	y2 = (image.data[src+3] * Y)>>8;
	image.data[src+3] = static_cast<unsigned char>(y2);
	  
	src+=4;
      }
    }
  }
}

#ifdef __MMX__
////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_gain :: processRGBAMMX(imageStruct &image)
{
  _mm_empty();

  short  R = static_cast<int>(256 * m_gain[chRed]);
  short  G = static_cast<int>(256 * m_gain[chGreen]);
  short  B = static_cast<int>(256 * m_gain[chBlue]);
  short  A = static_cast<int>(256 * m_gain[chAlpha]);

  if((R==256)&&(G==256)&&(B==256)&&(B==256)){
    // nothing to do!
    return;
  }
  /* the MMX code goes easily into clipping, 
   * since we are using (short) instead of (int)
   */
  if((R>256)||(G>256)||(B>256)||(B>256)){
    processRGBAImage(image);
    return;
  }

  register int pixsize = (image.ysize * image.xsize)>>1;
#if defined __APPLE__ && defined BYTE_ORDER && defined LITTLE_ENDIAN && (BYTE_ORDER == LITTLE_ENDIAN)
# warning this should be fixed in chRed,...
  register __m64 gain_64 = _mm_setr_pi16(A, R, G, B);
#else
  register __m64 gain_64 = _mm_setr_pi16(R, G, B, A);
#endif
  register __m64*data_p= reinterpret_cast<__m64*>(image.data);
  register __m64 null_64 = _mm_setzero_si64();
  register __m64 a0,a1;

    while(pixsize--) {
      a1 = data_p[0];
      
      a0=_mm_unpacklo_pi8(a1, null_64);
      a1=_mm_unpackhi_pi8(a1, null_64);
      
      a0 = _mm_mullo_pi16(a0, gain_64);
      a1 = _mm_mullo_pi16(a1, gain_64);

      a0 = _mm_srai_pi16(a0, 8);
      a1 = _mm_srai_pi16(a1, 8);
      
      data_p[0]=_mm_packs_pi16(a0, a1);
      data_p++;      
    } 

    _mm_empty();
}
#endif /* __MMX__ */

#ifdef __VEC__
void pix_gain :: processYUVAltivec(imageStruct &image)
{
  /* ignore m_saturate for now...*/
  int h,w,width,height;
  /*altivec code starts */
  width = image.xsize/8;
  height = image.ysize;
  union
  {
    short	elements[8];
    vector	signed short v;
  }shortBuffer;
    
  union
  {
    unsigned long	elements[8];
    vector	unsigned int v;
  }bitBuffer;
    
    
  register vector signed short d, hiImage, loImage, YImage, UVImage;
  vector unsigned char zero = vec_splat_u8(0);
  register vector signed int UVhi,UVlo,Yhi,Ylo;
  register vector signed short c,gain;
  register vector unsigned int bitshift;
  vector unsigned char *inData = (vector unsigned char*) image.data;

    
  shortBuffer.elements[0] = 128;
  shortBuffer.elements[1] = 0;
  shortBuffer.elements[2] = 128;
  shortBuffer.elements[3] = 0;
  shortBuffer.elements[4] = 128;
  shortBuffer.elements[5] = 0;
  shortBuffer.elements[6] = 128;
  shortBuffer.elements[7] = 0;
    
  c = shortBuffer.v;
    
  shortBuffer.elements[0] =static_cast<short> (m_gain[1]*255);
  gain = shortBuffer.v; 
  gain =  vec_splat(gain, 0 );  


  bitBuffer.elements[0] = 8;

  //Load it into the vector unit
  bitshift = bitBuffer.v;
  bitshift = vec_splat(bitshift,0); 
     
  shortBuffer.elements[0] = 128;
   
  //Load it into the vector unit
  d = shortBuffer.v;
  d = (vector signed short)vec_splat((vector signed short)d,0);
    
#ifndef PPC970
  UInt32			prefetchSize = GetPrefetchConstant( 16, 1, 256 );
  vec_dst( inData, prefetchSize, 0 );
#endif
    
  for ( h=0; h<height; h++){
    for (w=0; w<width; w++){
#ifndef PPC970
      vec_dst( inData, prefetchSize, 0 );
#endif
      //interleaved U Y V Y chars
            
      //expand the UInt8's to short's
      hiImage = (vector signed short) vec_mergeh( zero, inData[0] );
      loImage = (vector signed short) vec_mergel( zero, inData[0] );
            
      //vec_subs -128
      hiImage = (vector signed short) vec_sub( hiImage, c );
      loImage = (vector signed short) vec_sub( loImage, c );   
            
      //now vec_mule the UV into two vector ints
      UVhi = vec_mule(gain,hiImage);
      UVlo = vec_mule(gain,loImage);
            
      //now vec_mulo the Y into two vector ints
      Yhi = vec_mulo(gain,hiImage);
      Ylo = vec_mulo(gain,loImage);
            
      //this is where to do the bitshift/divide due to the resolution
      UVhi = vec_sra(UVhi,bitshift);
      UVlo = vec_sra(UVlo,bitshift);
      Yhi = vec_sra(Yhi,bitshift);
      Ylo = vec_sra(Ylo,bitshift);
            
      //pack the UV into a single short vector
      UVImage = vec_packs(UVhi,UVlo);
            
      //pack the Y into a single short vector
      YImage = vec_packs(Yhi,Ylo);
                                            
            
      //vec_adds +128 to U V U V short
      UVImage = vec_adds(UVImage,d);
            
      //vec_mergel + vec_mergeh Y and UV
      hiImage =  vec_mergeh(UVImage,YImage);
      loImage =  vec_mergel(UVImage,YImage);
            
      //pack back to 16 chars
      inData[0] = vec_packsu(hiImage, loImage);

      inData++;
    }
#ifndef PPC970
    vec_dss( 0 );
#endif
  }  /* end of working altivec function */
}
#endif /* __VEC__ */

/////////////////////////////////////////////////////////
// vecGainMess
//
/////////////////////////////////////////////////////////
void pix_gain :: vecGainMess(int argc, t_atom *argv)
{
  if (argc >= 4) m_gain[chAlpha] = atom_getfloat(&argv[3]);
  else if (argc == 3) m_gain[chAlpha] = 1.0;
  else if (argc == 1) m_gain[chRed] = m_gain[chGreen] = m_gain[chBlue] = m_gain[chAlpha] = atom_getfloat(argv);
  else {
    error("not enough gain values");
    return;
  }
  m_gain[chRed] = atom_getfloat(&argv[0]);
  m_gain[chGreen] = atom_getfloat(&argv[1]);
  m_gain[chBlue] = atom_getfloat(&argv[2]);
  setPixModified();
}

/////////////////////////////////////////////////////////
// floatGainMess
//
/////////////////////////////////////////////////////////
void pix_gain :: floatGainMess(float gain)
{
  // assumption that the alpha should be one
  m_gain[chAlpha] = 1.0f;
  m_gain[chRed] = m_gain[chGreen] = m_gain[chBlue] = gain;
  setPixModified();
}
/////////////////////////////////////////////////////////
// floatGainMess
//
/////////////////////////////////////////////////////////
void pix_gain :: saturateMess(int sat)
{
  // assumption that the alpha should be one
  m_saturate=(sat!=0);
  setPixModified();
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_gain :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_gain::vecGainMessCallback),
		  gensym("vec_gain"), A_GIMME, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_gain::floatGainMessCallback),
		  gensym("ft1"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_gain::saturateMessCallback),
		  gensym("saturate"), A_FLOAT, A_NULL);
}
void pix_gain :: vecGainMessCallback(void *data, t_symbol *, int argc, t_atom *argv)
{
  GetMyClass(data)->vecGainMess(argc, argv);
}
void pix_gain :: floatGainMessCallback(void *data, t_floatarg gain)
{
  GetMyClass(data)->floatGainMess(gain);
}
void pix_gain :: saturateMessCallback(void *data, t_floatarg sat)
{
  GetMyClass(data)->saturateMess(static_cast<int>(sat));
}
