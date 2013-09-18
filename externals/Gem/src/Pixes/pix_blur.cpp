/*
 *  pix_blur.cpp
 *  gem_darwin
 *
 *  Created by chris clepper on Mon Oct 07 2002.
 *  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
 *
 */

#include "pix_blur.h"
CPPEXTERN_NEW(pix_blur);

/////////////////////////////////////////////////////////
//
// pix_blur
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_blur :: pix_blur()
{	long size,src,i;
    
inletBlur = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("blur"));

m_blur = 0;
m_blurH = 240;
m_blurW = 240;
m_blurBpp = 2;
size = 320 * 240 * 4;
saved = new unsigned int [size];
src=0;
for (i=0;i<size/2;i++)
{
saved[src] = 128;
saved[src+1] = 0;
src += 2;
}
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_blur :: ~pix_blur()
{
delete saved;
}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_blur :: processRGBAImage(imageStruct &image)
{
  int h,w,hlength;
  long src;
  int R,G,B;
  int rightGain,imageGain;
  unsigned char *pixels=image.data;

  src = 0;

  if (m_blurH != image.ysize || m_blurW != image.xsize || m_blurBpp != image.csize) {
    m_blurH = image.ysize;
    m_blurW = image.xsize;
    m_blurBpp = image.csize;
    m_blurSize = m_blurH * m_blurW * m_blurBpp;
    delete saved;
    saved = new unsigned int [m_blurSize];
  }

  rightGain = static_cast<int>(m_blurf * 255.);
  imageGain = static_cast<int>(255. - (m_blurf * 255.));
  hlength = image.xsize;

  
  for (h=0; h<image.ysize; h++){
    for(w=0; w<hlength; w++){
      R = ((pixels[src+chRed] * imageGain)) + ((saved[src+chRed] * rightGain));
      saved[src+chRed] = (unsigned char)CLAMP(R>>8);
      pixels[src+chRed] = saved[src+chRed];
        
      G = ((pixels[src+chGreen] * imageGain)) + ((saved[src+chGreen] * rightGain));
      saved[src+chGreen] = (unsigned char)CLAMP(G>>8);
      pixels[src+chGreen] = saved[src+chGreen];
 
      B = ((pixels[src+chBlue] * imageGain)) + ((saved[src+chBlue] * rightGain));
      saved[src+chBlue] = (unsigned char)CLAMP(B>>8);
      pixels[src+chBlue] = saved[src+chBlue];

      src += 4;
    }
  }
}
void pix_blur :: processGrayImage(imageStruct &image)
{
  long src;
  int Grey;
  int rightGain,imageGain;
  unsigned char *pixels=image.data;

  if (m_blurH != image.ysize || m_blurW != image.xsize || m_blurBpp != image.csize) {
    m_blurH = image.ysize;
    m_blurW = image.xsize;
    m_blurBpp = image.csize;
    m_blurSize = m_blurH * m_blurW * m_blurBpp;
    delete saved;
    saved = new unsigned int [m_blurSize];
  }

  rightGain = static_cast<int>(m_blurf * 255.);
  imageGain = static_cast<int>(255. - (m_blurf * 255.));
  src=m_blurH*m_blurW;
  while(src--){
    Grey = ((pixels[src+chGray] * imageGain)) + ((saved[src+chGray] * rightGain));
    saved[src+chGray] = (unsigned char)CLAMP(Grey>>8);
    pixels[src+chGray] = saved[src+chGray];src--;

    Grey = ((pixels[src+chGray] * imageGain)) + ((saved[src+chGray] * rightGain));
    saved[src+chGray] = (unsigned char)CLAMP(Grey>>8);
    pixels[src+chGray] = saved[src+chGray];src--;

    Grey = ((pixels[src+chGray] * imageGain)) + ((saved[src+chGray] * rightGain));
    saved[src+chGray] = (unsigned char)CLAMP(Grey>>8);
    pixels[src+chGray] = saved[src+chGray];src--;

    Grey = ((pixels[src+chGray] * imageGain)) + ((saved[src+chGray] * rightGain));
    saved[src+chGray] = (unsigned char)CLAMP(Grey>>8);
    pixels[src+chGray] = saved[src+chGray];
  }
  src=0;//m_blurH*m_blurW-4;
  switch(m_blurH*m_blurW%4){
  case 3:
    Grey = ((pixels[src+chGray+3] * imageGain)) + ((saved[src+chGray+3] * rightGain));
    saved[src+chGray+3] = (unsigned char)CLAMP(Grey>>8);
    pixels[src+chGray+3] = saved[src+chGray+3];
  case 2:
    Grey = ((pixels[src+chGray+2] * imageGain)) + ((saved[src+chGray+2] * rightGain));
    saved[src+chGray+2] = (unsigned char)CLAMP(Grey>>8);
    pixels[src+chGray+2] = saved[src+chGray+2];
  case 1:
    Grey = ((pixels[src+chGray+1] * imageGain)) + ((saved[src+chGray+1] * rightGain));
    saved[src+chGray+1] = (unsigned char)CLAMP(Grey>>8);
    pixels[src+chGray+1] = saved[src+chGray+1];
  default:
    break;
  }
}

/////////////////////////////////////////////////////////
// do the YUV processing here
//
/////////////////////////////////////////////////////////
void pix_blur :: processYUVImage(imageStruct &image)
{


if (m_blurH != image.ysize || m_blurW != image.xsize || m_blurBpp != image.csize) {

m_blurH = image.ysize;
m_blurW = image.xsize;
m_blurBpp = image.csize;
m_blurSize = m_blurH * m_blurW * m_blurBpp;
delete saved;
saved = new unsigned int [m_blurSize];
}

     int h,w,hlength;
    long src;

    register int rightGain,imageGain;
    register int y1res,y2res;
src = 0;
rightGain = static_cast<int>(m_blurf * 255.);
imageGain = static_cast<int>(255. - (m_blurf * 255.));
hlength = image.xsize/2;

//unroll this, add register temps and schedule the ops better to remove the data depedencies
for (h=0; h<image.ysize-1; h++){
    for(w=0; w<hlength; w++){
                    //8bit  * 8bit = 16bit
        y1res = (((image.data[src+1] )  * imageGain) + ((saved[src+1] * rightGain)>>8));
       y2res = (((image.data[src+3]  ) * imageGain) + ((saved[src+3]  * rightGain)>>8) );
    
      
      saved[src+1] = y1res;
      y1res = y1res >> 8; //shift to 16bit to store? 
        
       
         
      image.data[src+1] =static_cast<unsigned char>(CLAMP(y1res));
     
     
     saved[src+3] = y2res;
     y2res = y2res >> 8;
        
       
     image.data[src+3] = static_cast<unsigned char>(CLAMP(y2res));
        src+=4; 
   
    }
}
}


/////////////////////////////////////////////////////////
// do the YUV processing here
//
/////////////////////////////////////////////////////////
void pix_blur :: processYUVAltivec(imageStruct &image)
{
#ifdef __VEC__
int h,w,width;
unsigned short rightGain,imageGain;
/*altivec code starts */
    width = image.xsize/8;
    rightGain = (unsigned short)(255. * m_blurf);
    imageGain = (unsigned short) (255. - (255. * m_blurf));
    union
    {
        //unsigned int	i;
        unsigned short	elements[8];
        //vector signed char v;
        vector	unsigned short v;
    }shortBuffer;
    
        union
    {
        //unsigned int	i;
        unsigned int	elements[4];
        //vector signed char v;
        vector	unsigned int v;
    }bitBuffer;
     
        union
    {
        //unsigned int	i;
        unsigned char	elements[16];
        //vector signed char v;
        vector	unsigned char v;
    }charBuffer;
    
    register vector unsigned short gainAdd, hiImage, loImage,hiRight,loRight, YImage, UVImage;     
    vector unsigned char zero = vec_splat_u8(0);
    vector unsigned short sone = vec_splat_u16(1);
    register vector unsigned char c,one;
    register vector unsigned int UVhi,UVlo,Yhi,Ylo;
    register vector unsigned int UVhiR,UVloR,YhiR,YloR;
    register vector unsigned short gainSub,gain,gainR,d;
    register vector unsigned int bitshift;

    vector unsigned char *inData = (vector unsigned char*) image.data;
    vector unsigned char *rightData = (vector unsigned char*) saved;
     
    //Write the pixel (pair) to the transfer buffer
    charBuffer.elements[0] = 2;
    charBuffer.elements[1] = 1;
    charBuffer.elements[2] = 2;
    charBuffer.elements[3] = 1;
    charBuffer.elements[4] = 2;
    charBuffer.elements[5] = 1;
    charBuffer.elements[6] = 2;
    charBuffer.elements[7] = 1;
    charBuffer.elements[8] = 2;
    charBuffer.elements[9] = 1;
    charBuffer.elements[10] = 2;
    charBuffer.elements[11] = 1;
    charBuffer.elements[12] = 2;
    charBuffer.elements[13] = 1;
    charBuffer.elements[14] = 2;
    charBuffer.elements[15] = 1;


    //Load it into the vector unit
    c = charBuffer.v;
    
    one =  vec_splat_u8( 1 );
    
    shortBuffer.elements[0] = 255;
   
    //Load it into the vector unit
    d = shortBuffer.v;
    d = (vector unsigned short)vec_splat((vector unsigned short)d,0);
    
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
    gainAdd = (vector unsigned short)vec_splat((vector unsigned short)gainAdd,0);
#ifndef PPC970
   	UInt32			prefetchSize = GetPrefetchConstant( 16, 1, 256 );
	vec_dst( inData, prefetchSize, 0 );
	vec_dst( rightData, prefetchSize, 1 );
      #endif  
    for ( h=0; h<image.ysize; h++){
        for (w=0; w<width; w++)
        {
        #ifndef PPC970
	vec_dst( inData, prefetchSize, 0 );
        vec_dst( rightData, prefetchSize, 1 );
        #endif
            //interleaved U Y V Y chars
            
            //expand the UInt8's to short's
            hiImage = (vector unsigned short) vec_mergeh( zero, inData[0] );
            loImage = (vector unsigned short) vec_mergel( zero, inData[0] );
            
            hiRight = (vector unsigned short) vec_mergeh( zero, rightData[0] );
            loRight = (vector unsigned short) vec_mergel( zero, rightData[0] );
            
            
            //now vec_mule the UV into two vector ints
            UVhi = vec_mule(sone,hiImage);
            UVlo = vec_mule(sone,loImage);
            
            UVhiR = vec_mule(sone,hiRight);
            UVloR = vec_mule(sone,loRight);
            
            //now vec_mulo the Y into two vector ints
            Yhi = vec_mulo(gain,hiImage);
            Ylo = vec_mulo(gain,loImage);
            
            YhiR = vec_mulo(gainR,hiRight);
            YloR = vec_mulo(gainR,loRight);
            
            
            Yhi = vec_adds(Yhi,YhiR);
            Ylo = vec_adds(Ylo,YloR);
    
            Yhi = vec_sra(Yhi,bitshift);
            Ylo = vec_sra(Ylo,bitshift);
            //pack the UV into a single short vector
            UVImage =  vec_packsu(UVhi,UVlo);

            //pack the Y into a single short vector
            YImage =  vec_packsu(Yhi,Ylo);
                   
            //vec_mergel + vec_mergeh Y and UV
            hiImage =  vec_mergeh(UVImage,YImage);
            loImage =  vec_mergel(UVImage,YImage);
          
            
            //vec_mergel + vec_mergeh Y and UV
            rightData[0] = vec_packsu(hiImage, loImage);
            inData[0] = vec_packsu(hiImage, loImage);        
         
            inData++;
            rightData++;
        }
       }
       #ifndef PPC970
       //stop the cache streams
        vec_dss( 0 );
        vec_dss( 1 );
        #endif
         /* end of working altivec function */   
         
#endif
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_blur :: obj_setupCallback(t_class *classPtr)
{

    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_blur::blurCallback),
		  gensym("blur"), A_DEFFLOAT, A_NULL);
}

void pix_blur :: blurCallback(void *data, t_floatarg value)
{
  GetMyClass(data)->m_blurf=(value);
}

