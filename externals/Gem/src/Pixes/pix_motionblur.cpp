/////////////////////////////////////////////////////////
//  pix_motionblur.cpp
//  gem_darwin
//
//  Created by chris clepper on Mon Oct 07 2002.
//  Copyright (c) 2002.  All rights reserved.
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////


#include "pix_motionblur.h"
CPPEXTERN_NEW(pix_motionblur);
   
/////////////////////////////////////////////////////////
//
// pix_motionblur
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_motionblur :: pix_motionblur()
{    
  inletmotionblur = inlet_new(this->x_obj, &this->x_obj->ob_pd, 
			      &s_float,
			      gensym("motionblur"));

  m_blur0 = 256;
  m_blur1 = 0;

  m_savedImage.xsize=320;
  m_savedImage.ysize=240;
  m_savedImage.setCsizeByFormat(GL_RGBA);
  m_savedImage.reallocate();
  m_savedImage.setBlack();
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_motionblur :: ~pix_motionblur()
{}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_motionblur :: processRGBAImage(imageStruct &image)
{
  int h,w,height,width;
  long src=0;
  register int R,R1,G,G1,B,B1; //too many for x86?  i really don't know or care
  int rightGain,imageGain;
  unsigned char *pixels=image.data;
  unsigned char *saved = m_savedImage.data;

  m_savedImage.xsize=image.xsize;
  m_savedImage.ysize=image.ysize;
  m_savedImage.setCsizeByFormat(image.format);
  m_savedImage.reallocate();
  if(saved!=m_savedImage.data)m_savedImage.setBlack();saved=m_savedImage.data;

  rightGain = m_blur1; 
  imageGain = m_blur0;
  height = image.ysize;
  width = image.xsize;

  for (h=0; h<height; h++){
    for(w=0; w<width; w++){
      R  = pixels[src+chRed];
      R1 = saved [src+chRed];
      G  = pixels[src+chGreen];
      G1 = saved [src+chGreen];
      B  = pixels[src+chBlue];
      B1 = saved [src+chBlue];
        
      R  = R * imageGain;
      R1 = R1 * rightGain;
      G  = G * imageGain;
      G1 = G1 * rightGain;
      B  = B * imageGain;
      B1 = B1 * rightGain;

      R = R + R1;
      G = G + G1;
      B = B + B1;

      R1 = R>>8;
      G1 = G>>8;
      B1 = B>>8;

      saved[src+chRed]   = (unsigned char)R1;
      saved[src+chGreen] = (unsigned char)G1;
      saved[src+chBlue]  = (unsigned char)B1;

      pixels[src+chRed]   = (unsigned char)R1;
      pixels[src+chGreen] = (unsigned char)G1;
      pixels[src+chBlue]  = (unsigned char)B1;
        
      src += 4;
    }
  }
}
void pix_motionblur :: processGrayImage(imageStruct &image)
{
  int h,w,height,width;
  long src;
  register int G, G1; //too many for x86?  i really don't know or care
  int rightGain,imageGain;
  unsigned char *pixels=image.data;
  int Gray;

  src = 0;
  Gray=chGray;

  unsigned char *saved = m_savedImage.data;

  m_savedImage.xsize=image.xsize;
  m_savedImage.ysize=image.ysize;
  m_savedImage.setCsizeByFormat(image.format);
  m_savedImage.reallocate();
  if(saved!=m_savedImage.data)m_savedImage.setBlack();saved=m_savedImage.data;

  rightGain = m_blur1;
  imageGain = m_blur0;
  height = image.ysize;
  width = image.xsize;

  for (h=0; h<height; h++){
    for(w=0; w<width; w++){
      G = pixels[src+chGray];
      G1 = saved[src+chGray];
      G = G * imageGain;
      G1 = G1 * rightGain;

      G = G + G1;
      G1 = G>>8;
      saved[src+chGray] = (unsigned char)G1;
      pixels[src+chGray] = (unsigned char)G1;
      src ++;
    }
  }
}

/////////////////////////////////////////////////////////
// do the YUV processing here
// -- note this is scheduled for PPC, 
//    if you use another CPU write another function for it
/////////////////////////////////////////////////////////
void pix_motionblur :: processYUVImage(imageStruct &image)
{

  unsigned char *saved = m_savedImage.data;

  m_savedImage.xsize=image.xsize;
  m_savedImage.ysize=image.ysize;
  m_savedImage.setCsizeByFormat(image.format);
  m_savedImage.reallocate();
  if(saved!=m_savedImage.data)m_savedImage.setBlack();saved=m_savedImage.data;

  int h,w,hlength;
  register long src,dst;

  register int rightGain,imageGain;
  register int y1,y1a,y2,y2a,y1res,y2res,u,u1,v,v1;
  register int loadU,loadV,loadY1, loadY2,loadU1,loadV1,loadY1a, loadY2a;
    
  src = 0;
  dst = 0;

  loadU = image.data[src];
  loadU1 = saved[src]; 
  loadY1 = image.data[src+1] ;
  loadY1a = saved[src+1];
   
  loadV = image.data[src+2];
  loadV1 = saved[src+2]; 
  loadY2 = image.data[src+3];
  loadY2a = saved[src+3] ;
  src += 4;

  rightGain = m_blur1;
  imageGain = m_blur0;
  hlength = image.xsize/2;

  //unroll this, add register temps and schedule the ops better to remove the data depedencies

  // JMZ: i am not sure whether i really understand what is going on here
  for (h=0; h<image.ysize-1; h++){
    for(w=0; w<hlength; w++){
      u  = loadU - 128;
      u1 = loadU1 >> 8;
      v = loadV - 128;
      v1 = loadV1>>8;
       
      y1  = loadY1 * imageGain;
      y1a = loadY1a * rightGain; 
      y2 = loadY2 * imageGain;
      y2a = loadY2a  * rightGain; 
      u *= imageGain;
      u1 *= rightGain;
      
      v *= imageGain;
      v1 *= rightGain;

      loadU = static_cast<int>(image.data[src]); 
      loadU1 = static_cast<int>(saved[src]); 
      loadY1 = static_cast<int>(image.data[src+1]);
      loadY1a = static_cast<int>(saved[src+1]);

      loadV = static_cast<int>(image.data[src+2]);
      loadV1 = static_cast<int>(saved[src+2]); 
      loadY2 = static_cast<int>(image.data[src+3]);
      loadY2a = static_cast<int>(saved[src+3]);
      
      y1a = y1a>>8;
      y2a = y2a>>8;
 
      u += u1; 
      v += v1;
      saved[dst] = u;
      saved[dst+2] = v;
      u = u>>8; 
      v = v>>8;
      u += 128;
      v += 128;

      y1res = y1 + y1a;
      y2res = y2 + y2a;
 

      saved[dst+1] = y1res;
      saved[dst+3] = y2res;
      
      y1res = y1res >> 8; //shift to 16bit to store? 

      y2res = y2res >> 8;

      image.data[dst] = (unsigned char)u;
      image.data[dst+2] = (unsigned char)v;
      image.data[dst+1] =(unsigned char)y1res;
      image.data[dst+3] = (unsigned char)y2res;
      src+=4;dst+=4;
    }
  }
}

#ifdef __MMX__
/* do the processing for all colourspaces */
void pix_motionblur :: processMMX(imageStruct &image)
{
  m_savedImage.xsize=image.xsize;
  m_savedImage.ysize=image.ysize;
  m_savedImage.setCsizeByFormat(image.format);
  m_savedImage.reallocate();

  int pixsize=image.ysize*image.xsize*image.csize;
  pixsize=pixsize/sizeof(__m64)+(pixsize%sizeof(__m64)!=0);

  __m64*pixels=(__m64*)image.data;
  __m64*old=(__m64*)m_savedImage.data;

  __m64 newGain = _mm_set1_pi16(static_cast<short>(m_blur0));
  __m64 oldGain = _mm_set1_pi16(static_cast<short>(m_blur1));
  __m64 null64 = _mm_setzero_si64();

  __m64 newpix1, newpix2, oldpix1, oldpix2;

  while(pixsize--){
    newpix1=pixels[pixsize];
    oldpix1=old[pixsize];

    newpix2 = _mm_unpackhi_pi8(newpix1, null64);
    newpix1 = _mm_unpacklo_pi8(newpix1, null64);
    oldpix2 = _mm_unpackhi_pi8(oldpix1, null64);
    oldpix1 = _mm_unpacklo_pi8(oldpix1, null64);

    newpix1 = _mm_mullo_pi16(newpix1, newGain);
    newpix2 = _mm_mullo_pi16(newpix2, newGain);
    oldpix1 = _mm_mullo_pi16(oldpix1, oldGain);
    oldpix2 = _mm_mullo_pi16(oldpix2, oldGain);

    newpix1 = _mm_adds_pu16 (newpix1, oldpix1);
    newpix2 = _mm_adds_pu16 (newpix2, oldpix2);

    newpix1 = _mm_srli_pi16(newpix1, 8);
    newpix2 = _mm_srli_pi16(newpix2, 8);
    newpix1 = _mm_packs_pu16(newpix1, newpix2);
    pixels[pixsize]=newpix1;
    old   [pixsize]=newpix1;
  }
  _mm_empty();
}
/* call the main MMX-function */
void pix_motionblur :: processRGBAMMX(imageStruct &image)
{  processMMX(image); }
void pix_motionblur :: processYUVMMX(imageStruct &image)
{  processMMX(image); }
void pix_motionblur :: processGrayMMX(imageStruct &image)
{  processMMX(image); }
#endif

#ifdef __VEC__
/* start of optimized motionblur */
void pix_motionblur :: processYUVAltivec(imageStruct &image)
{
  int h,w,width;
  signed short rightGain,imageGain;
  unsigned char *saved = m_savedImage.data;

  m_savedImage.xsize=image.xsize;
  m_savedImage.ysize=image.ysize;
  m_savedImage.setCsizeByFormat(image.format);
  m_savedImage.reallocate();
  if(saved!=m_savedImage.data)m_savedImage.setBlack();saved=m_savedImage.data;

  width = image.xsize/8;
  /*
    // hmm: why does it read 235 ?
  rightGain = (signed short)(235. * m_motionblur);
  imageGain = (signed short) (255. - (235. * m_motionblur));
  */
  rightGain = m_blur1;
  imageGain = m_blur0;

  union
  {
    signed short	elements[8];
    vector	signed short v;
  }shortBuffer;
  
  union
  {
    unsigned int	elements[4];
    vector	unsigned int v;
  }bitBuffer;
  
  register vector signed short gainAdd, hiImage, loImage,hiRight,loRight, YImage, UVImage; 
  // register vector signed short loadhiImage, loadloImage,loadhiRight,loadloRight;
  register vector unsigned char loadImage, loadRight;
  register vector unsigned char zero = vec_splat_u8(0);
  register vector signed int UVhi,UVlo,Yhi,Ylo;
  register vector signed int UVhiR,UVloR,YhiR,YloR;
  register vector signed short gainSub,gain,gainR;//,d;
  register vector unsigned int bitshift;
  vector unsigned char *inData = (vector unsigned char*) image.data;
  vector unsigned char *rightData = (vector unsigned char*) saved;

    
  shortBuffer.elements[0] = 128;
  shortBuffer.elements[1] = 0;
  shortBuffer.elements[2] = 128;
  shortBuffer.elements[3] = 0;
  shortBuffer.elements[4] = 128;
  shortBuffer.elements[5] = 0;
  shortBuffer.elements[6] = 128;
  shortBuffer.elements[7] = 0;
  
  gainSub = shortBuffer.v;
     
  shortBuffer.elements[0] = imageGain;
  gain = shortBuffer.v; 
  gain =  vec_splat(gain, 0 );  
  
  shortBuffer.elements[0] = rightGain;
  gainR = shortBuffer.v; 
  gainR =  vec_splat(gainR, 0 ); 

  bitBuffer.elements[0] = 8;
  
  //Load it into the vector unit
  bitshift = bitBuffer.v;
  bitshift = vec_splat(bitshift,0); 
     
  shortBuffer.elements[0] = 128;
  
  //Load it into the vector unit
  gainAdd = shortBuffer.v;
  gainAdd = (vector signed short)vec_splat((vector signed short)gainAdd,0);
  
#ifndef PPC970
  UInt32			prefetchSize = GetPrefetchConstant( 16, 1, 256 );
  vec_dst( inData, prefetchSize, 0 );
  vec_dst( rightData, prefetchSize, 1 );
  vec_dst( inData+32, prefetchSize, 2 );
  vec_dst( rightData+32, prefetchSize, 3 );
#endif    
    
  loadImage = inData[0];
  loadRight = rightData[0];
     
  for ( h=0; h<image.ysize; h++){
    for (w=0; w<width; w++)
      {
#ifndef PPC970
	vec_dst( inData, prefetchSize, 0 );
        vec_dst( rightData, prefetchSize, 1 );
        vec_dst( inData+32, prefetchSize, 2 );
        vec_dst( rightData+32, prefetchSize, 3 );
#endif
	//interleaved U Y V Y chars
            
	hiImage = (vector signed short) vec_mergeh( zero, loadImage );
	loImage = (vector signed short) vec_mergel( zero, loadImage );
            
	hiRight = (vector signed short) vec_mergeh( zero, loadRight );
	loRight = (vector signed short) vec_mergel( zero, loadRight );
            
	//hoist that load!!
	loadImage = inData[1];
	loadRight = rightData[1];
            
	//subtract 128 from UV
            
	hiImage = vec_subs(hiImage,gainSub);
	loImage = vec_subs(loImage,gainSub);
            
	hiRight = vec_subs(hiRight,gainSub);
	loRight = vec_subs(loRight,gainSub);
            
	//now vec_mule the UV into two vector ints
	//change sone to gain
	UVhi = vec_mule(gain,hiImage);
	UVlo = vec_mule(gain,loImage);
            
	UVhiR = vec_mule(gainR,hiRight);
	UVloR = vec_mule(gainR,loRight);
            
	//now vec_mulo the Y into two vector ints
	Yhi = vec_mulo(gain,hiImage);
	Ylo = vec_mulo(gain,loImage);
            
	YhiR = vec_mulo(gainR,hiRight);
	YloR = vec_mulo(gainR,loRight);
            
             
	//this is where to do the add and bitshift due to the resolution
	//add UV
	UVhi = vec_adds(UVhi,UVhiR);
	UVlo = vec_adds(UVlo,UVloR);
        
	Yhi = vec_adds(Yhi,YhiR);
	Ylo = vec_adds(Ylo,YloR);
            
	//bitshift UV
	UVhi = vec_sra(UVhi,bitshift);
	UVlo = vec_sra(UVlo,bitshift);
            
	Yhi = vec_sra(Yhi,bitshift);
	Ylo = vec_sra(Ylo,bitshift);
                        
	//pack the UV into a single short vector
	UVImage =  vec_packs(UVhi,UVlo);

	//pack the Y into a single short vector
	YImage =  vec_packs(Yhi,Ylo);
                   
	//vec_mergel + vec_mergeh Y and UV
	hiImage =  vec_mergeh(UVImage,YImage);
	loImage =  vec_mergel(UVImage,YImage);
            
	//add 128 offset back
	hiImage = vec_adds(hiImage,gainSub);
	loImage = vec_adds(loImage,gainSub);
            
	//vec_mergel + vec_mergeh Y and UV
	rightData[0] = (vector unsigned char)vec_packsu(hiImage, loImage);
	inData[0] = (vector unsigned char)vec_packsu(hiImage, loImage);        
         
	inData++;
	rightData++;
      }
  }  
#ifndef PPC970
  //stop the cache streams
  vec_dss( 0 );
  vec_dss( 1 );
  vec_dss( 2 );
  vec_dss( 3 );
#endif
           
         
}/* end of working altivec function */ 
#endif /* ALTIVEC */

void pix_motionblur :: motionblurMessage(int argc, t_atom*argv){
  switch(argc){
  case 1:
    m_blur1=static_cast<int>(256.f*atom_getfloat(argv));
    if(m_blur1<0)m_blur1=0;if(m_blur1>256)m_blur1=256;
    m_blur0=256-m_blur1;
    break;
  case 2:
    m_blur1=static_cast<int>(256.f*atom_getfloat(argv));
    if(m_blur1<0)m_blur1=0;if(m_blur1>256)m_blur1=256;
    m_blur0=static_cast<int>(256.f*atom_getfloat(argv+1));
    if(m_blur0<0)m_blur0=0;if(m_blur0>256)m_blur0=256;
    break;
  default:
    error("specify 1 or 2 values");
  }
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_motionblur :: obj_setupCallback(t_class *classPtr)
{

  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_motionblur::motionblurCallback),
		  gensym("motionblur"), A_GIMME, A_NULL);
}

void pix_motionblur :: motionblurCallback(void *data, t_symbol*, int argc, t_atom*argv)
{
  GetMyClass(data)->motionblurMessage(argc, argv);
}
