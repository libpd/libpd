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

#include "pix_mix.h"

CPPEXTERN_NEW_WITH_GIMME(pix_mix);

/////////////////////////////////////////////////////////
//
// pix_mix
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_mix :: pix_mix(int argc, t_atom*argv)
{ 
  switch (argc){
  case 0:
    imageGain = rightGain = 128;
    break;
  case 1:
    rightGain=CLAMP((float)255.*atom_getfloat(argv));
    imageGain=255-rightGain;
    break;
  default:
    imageGain=CLAMP((float)255.*atom_getfloat(argv));
    rightGain=CLAMP((float)255.*atom_getfloat(argv+1));
  }
  inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("gain"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_mix :: ~pix_mix()
{ }

/////////////////////////////////////////////////////////
// processDualImage
//
/////////////////////////////////////////////////////////
void pix_mix :: processRGBA_RGBA(imageStruct &image, imageStruct &right)
{
  int datasize = image.xsize * image.ysize;
  unsigned char *leftPix = image.data;
  unsigned char *rightPix = right.data;
  // int A,R,G,B;

  int l, r;
  while(datasize--)    {
    l = (leftPix [chRed]   * imageGain)>>8;
    r = (rightPix[chRed]   * rightGain)>>8;
    leftPix[chRed] =   CLAMP_HIGH(l + r);
    l = (leftPix [chGreen] * imageGain)>>8;
    r = (rightPix[chGreen] * rightGain)>>8;
    leftPix[chGreen] = CLAMP_HIGH(l + r);
    l = (leftPix [chBlue]  * imageGain)>>8;
    r = (rightPix[chBlue]  * rightGain)>>8;
    leftPix[chBlue] =  CLAMP_HIGH(l + r);
    leftPix += 4;
    rightPix += 4;
  }
}

/////////////////////////////////////////////////////////
// processDualImage
//
/////////////////////////////////////////////////////////
void pix_mix :: processGray_Gray(imageStruct &image, imageStruct &right)
{
  int datasize = image.xsize * image.ysize;
  unsigned char *leftPix = image.data;
  unsigned char *rightPix = right.data;
  // int A,R,G,B;

  int l, r;
  while(datasize--)    {
    l = ( leftPix[chGray] * imageGain)>>8;
    r = (rightPix[chGray] * rightGain)>>8;
    leftPix[chGray] = CLAMP_HIGH(l + r);
    leftPix ++;
    rightPix++;
  }
}
/////////////////////////////////////////////////////////
// do the YUV processing here
//
/////////////////////////////////////////////////////////
void pix_mix :: processYUV_YUV(imageStruct &image, imageStruct &right){
 int	y1,y2;
 int u,v,u1,v1;
 long width,h,w;
 long src =0;
 
 width = image.xsize/2;
   
 //format is U Y V Y
 for (h=0; h<image.ysize; h++){
   for(w=0; w<width; w++){
     /*  u = (((image.data[src] - 128) * imageGain)>>8)+128;
         u1 = (((right.data[src] - 128) * rightGain)>>8)+128;
         u = u + ((2*u1) -255); */
     u = (image.data[src] - 128) * imageGain;
     u1 = (right.data[src] - 128) * rightGain;
     u = ((u + u1)>>8) + 128;
     image.data[src] = (unsigned char)CLAMP(u);
     
     y1 = ((image.data[src+1] * imageGain) + (right.data[src+1] * rightGain))>>8;
     image.data[src+1] = (unsigned char)CLAMP(y1);
        
     /*v = (((image.data[src+2] - 128) * imageGain)>>8)+128;
       v1 = (((right.data[src+2] - 128) * rightGain)>>8)+128;
       v = v + (2*v1) - 255;*/
     v = (image.data[src+2] - 128) * imageGain;
     
     v1 = (right.data[src+2] - 128) * rightGain;
     v = ((v + v1)>>8) + 128;
     
     image.data[src+2] = (unsigned char)CLAMP(v);

     y2 = ((image.data[src+3] * imageGain) + (right.data[src+3] * rightGain))>>8;
     image.data[src+3] = (unsigned char)CLAMP(y2);

     src += 4;
   }
 }
}

#ifdef __MMX__
void pix_mix :: processRGBA_MMX (imageStruct &image, imageStruct &right){
  int datasize =   image.xsize * image.ysize * image.csize;
  __m64*leftPix =  (__m64*)image.data;
  __m64*rightPix = (__m64*)right.data;

  datasize=datasize/sizeof(__m64)+(datasize%sizeof(__m64)!=0);
  __m64 rGain = _mm_set1_pi16((short)rightGain);
  __m64 lGain = _mm_set1_pi16((short)imageGain);
  __m64 null64 =   _mm_setzero_si64();

  __m64 l1, r1, l2, r2;
  while (datasize--) {
    l1=leftPix [datasize];
    r1=rightPix[datasize];

    l2=_mm_unpackhi_pi8 (l1, null64);
    l1=_mm_unpacklo_pi8 (l1, null64);
    r2=_mm_unpackhi_pi8 (r1, null64);
    r1=_mm_unpacklo_pi8 (r1, null64);

    l1 = _mm_mullo_pi16(l1, lGain);
    l2 = _mm_mullo_pi16(l2, lGain);
    r1 = _mm_mullo_pi16(r1, rGain);
    r2 = _mm_mullo_pi16(r2, rGain);

    l1 = _mm_adds_pu16 (l1, r1);
    l2 = _mm_adds_pu16 (l2, r2);

    l1 = _mm_srli_pi16 (l1, 8);
    l2 = _mm_srli_pi16 (l2, 8);
    l1 = _mm_packs_pu16(l1, l2);
    leftPix[datasize]=l1;
  }
  _mm_empty();
}
void pix_mix :: processYUV_MMX (imageStruct &image, imageStruct &right){
  processRGBA_MMX(image, right);
}
void pix_mix :: processGray_MMX (imageStruct &image, imageStruct &right){
  processRGBA_MMX(image, right);

}
#endif

#ifdef __VEC__
//needs fixing for better IQ
void pix_mix :: processYUV_Altivec (imageStruct &image, imageStruct &right)
{
long h,w, width;
 
    /*altivec code starts */
    width = image.xsize/8;
    union
    {
        //unsigned int	i;
        short	elements[8];
        //vector signed char v;
        vector	signed short v;
    }shortBuffer;
    
        union
    {
        //unsigned int	i;
        unsigned long	elements[8];
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
    
    //vector unsigned char c;
    register vector signed short gainAdd, hiImage, loImage,hiRight,loRight, YImage, UVImage, UVRight, UVTemp, YTemp;
    register vector unsigned char zero = vec_splat_u8(0);
    //vector signed short szero = vec_splat_s16(0);
    register vector unsigned char c,one;
    register vector signed int UVhi,UVlo,Yhi,Ylo;
    register vector signed int UVhiR,UVloR,YhiR,YloR;
    register vector signed short gainSub,gain,gainR,d;
    register vector unsigned int bitshift;
    vector unsigned char *inData = (vector unsigned char*) image.data;
    vector unsigned char *rightData = (vector unsigned char*) right.data;
    register vector unsigned char tempImage;//,tempRight;
    
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
    d = (vector signed short)vec_splat((vector signed short)d,0);
    
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
            hiImage = (vector signed short) vec_mergeh( zero, inData[0] );
            loImage = (vector signed short) vec_mergel( zero, inData[0] );
            
            hiRight = (vector signed short) vec_mergeh( zero, rightData[0] );
            loRight = (vector signed short) vec_mergel( zero, rightData[0] );
            
            //vec_subs -128
            hiImage = (vector signed short) vec_sub( hiImage, gainSub );
            loImage = (vector signed short) vec_sub( loImage, gainSub );   
            
            hiRight = (vector signed short) vec_sub( hiRight, gainSub );
            loRight = (vector signed short) vec_sub( loRight, gainSub );   
            
            
            //now vec_mule the UV into two vector ints
            UVhi = vec_mule(gain,hiImage);
            UVlo = vec_mule(gain,loImage);
            
            UVhiR = vec_mule(gainR,hiRight);
            UVloR = vec_mule(gainR,loRight);
            
            //now vec_mulo the Y into two vector ints
            Yhi = vec_mulo(gain,hiImage);
            Ylo = vec_mulo(gain,loImage);
            
            YhiR = vec_mulo(gainR,hiRight);
            YloR = vec_mulo(gainR,loRight);
            
            
            //this is where to do the bitshift/divide due to the resolution
            //nope  do the add here then proceed?
            
            Yhi = vec_adds(Yhi,YhiR);
            Ylo = vec_adds(Ylo,YloR);
            UVhi = vec_adds(UVhi,UVhiR);
            UVlo = vec_adds(UVlo,UVloR);
            
            UVhi = vec_sra(UVhi,bitshift);
            UVlo = vec_sra(UVlo,bitshift);
            Yhi = vec_sra(Yhi,bitshift);
            Ylo = vec_sra(Ylo,bitshift);
            
            UVhiR = vec_sra(UVhiR,bitshift);
            UVloR = vec_sra(UVloR,bitshift);
            YhiR = vec_sra(YhiR,bitshift);
            YloR = vec_sra(YloR,bitshift);
            
            //pack the UV into a single short vector
            UVImage = vec_packs(UVhi,UVlo);
         //   UVRight = vec_packs(UVhiR,UVloR);

            //pack the Y into a single short vector
            YImage = vec_packs(Yhi,Ylo);
           // YRight = vec_packs(YhiR,YloR);
                                        
            
            //vec_adds +128 to U V U V short
            UVImage = vec_adds(UVImage,gainAdd);
         //   UVRight = vec_adds(UVRight,gainAdd);
            
            //vec_mergel + vec_mergeh Y and UV
            hiImage =  vec_mergeh(UVImage,YImage);
            loImage =  vec_mergel(UVImage,YImage);
           // hiRight =  vec_mergeh(UVRight,YRight);
           // loRight =  vec_mergel(UVRight,YRight);
            
            //pack back to 16 chars and prepare for add
            tempImage = vec_packsu(hiImage, loImage);
           // tempRight = vec_packsu(hiRight, loRight);
        
            //vec_mule UV * 2 to short vector U V U V shorts
            UVImage = (vector signed short)vec_mule(one,tempImage);
           // UVRight = (vector signed short)vec_mule(c,tempRight);
            
            //vec_mulo Y * 1 to short vector Y Y Y Y shorts
            YImage = (vector signed short)vec_mulo(c,tempImage);
           // YRight = (vector signed short)vec_mulo(c,tempRight);

            
            //vel_subs UV - 255
            UVRight = (vector signed short)vec_subs(UVRight, d);
            
            //vec_adds UV
         //   UVTemp = vec_adds(UVImage,UVRight);
            UVTemp = UVImage;
            
            //vec_adds Y
           // YTemp = vec_adds(YImage,YRight);
            YTemp = YImage;
            
            hiImage = vec_mergeh(UVTemp,YTemp);
            loImage = vec_mergel(UVTemp,YTemp);
            
            //vec_mergel + vec_mergeh Y and UV
            inData[0] = vec_packsu(hiImage, loImage);        
        
            inData++;
            rightData++;
        }

    }  /* end of working altivec function */   
# ifndef PPC970
    //stop the cache streams
    vec_dss( 0 );
    vec_dss( 1 );
# endif
}
#endif

/////////////////////////////////////////////////////////
//gain converted from float to int
//
/////////////////////////////////////////////////////////
void pix_mix :: gainMess (float X, float Y)
{
  imageGain = CLAMP(255.f * X);
  rightGain = CLAMP(255.f * Y);
  setPixModified();
}
  
/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_mix :: obj_setupCallback(t_class *classPtr)
{ 
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_mix::gainCallback),
		  gensym("gain"), A_GIMME, A_NULL);//A_DEFFLOAT, A_DEFFLOAT, A_NULL);
}

//void pix_mix :: gainCallback(void *data, t_floatarg X, t_floatarg Y)
void pix_mix :: gainCallback(void *data, t_symbol*s, int argc, t_atom*argv)
{
  if (argc<1)return;
  float X, Y;
  if(argc>1){
    X=atom_getfloat(argv);
    Y=atom_getfloat(argv+1);
  } else {
    Y=atom_getfloat(argv);
    X=1.0-Y;
  }
  GetMyClass(data)->gainMess(X, Y);
}
