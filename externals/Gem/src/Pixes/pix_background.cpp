/*
 *  pix_background.cpp
 *  gem_darwin
 *
 *  Created by chris clepper on Mon Oct 07 2002.
 *  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
 *
 */

#include "pix_background.h"
#include <string.h>

CPPEXTERN_NEW_WITH_GIMME(pix_background);

/////////////////////////////////////////////////////////
//
// pix_background
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_background :: pix_background(int argc, t_atom*argv) :
  m_Yrange(0), m_Urange(0), m_Vrange(0), m_Arange(0), m_reset(1)
{
	inletRange = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("range_n"));

    m_savedImage.xsize=320;
    m_savedImage.ysize=240;
    m_savedImage.setCsizeByFormat(GL_RGBA);
    m_savedImage.reallocate();
    switch(argc) {
    case 4: case 3: case 1:
      rangeNMess(argc, argv);
      break;
    default: break;
    }
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_background :: ~pix_background()
{
  if(inletRange)inlet_free(inletRange);
}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_background :: processRGBAImage(imageStruct &image)
{
  int h,w,hlength;
  long src,pixsize;
  
  src = 0;
  pixsize = image.xsize * image.ysize * image.csize;

  if(m_savedImage.xsize!=image.xsize ||
     m_savedImage.ysize!=image.ysize ||
     m_savedImage.format!=image.format)m_reset=1;

  m_savedImage.xsize=image.xsize;
  m_savedImage.ysize=image.ysize;
  m_savedImage.setCsizeByFormat(image.format);
  m_savedImage.reallocate();

  if (m_reset){
    memcpy(m_savedImage.data,image.data,pixsize);
    m_reset = 0; 
  }

  hlength = image.xsize;

  unsigned char*data =image.data;
  unsigned char*saved=m_savedImage.data;

  for (h=0; h<image.ysize; h++){
    for(w=0; w<hlength; w++){
      if (((data[src+chRed  ] > saved[src+chRed  ] - m_Yrange)&&
	   (data[src+chRed  ] < saved[src+chRed  ] + m_Yrange))&&
	  ((data[src+chGreen] > saved[src+chGreen] - m_Urange)&&
	   (data[src+chGreen] < saved[src+chGreen] + m_Urange))&&
	  ((data[src+chBlue ] > saved[src+chBlue ] - m_Vrange)&&
	   (data[src+chBlue ] < saved[src+chBlue ] + m_Vrange))&&
	  ((data[src+chAlpha] > saved[src+chAlpha] - m_Arange)&&
	   (data[src+chAlpha] < saved[src+chAlpha] + m_Arange)))
	{
	  data[src+chRed] = 0;
	  data[src+chGreen] = 0;
	  data[src+chBlue] = 0;
	  data[src+chAlpha] = 0;
	}
      src+=4;
    }
  }
  m_reset = 0; 
}

void pix_background :: processGrayImage(imageStruct &image)
{
    int i;// h,w,hlength;
  long src,pixsize;
  unsigned char newpix, oldpix, *npixes, *opixes;

  src = 0;
  pixsize = image.xsize * image.ysize * image.csize;
  if(m_savedImage.xsize!=image.xsize ||
     m_savedImage.ysize!=image.ysize ||
     m_savedImage.format!=image.format)m_reset=1;

  m_savedImage.xsize=image.xsize;
  m_savedImage.ysize=image.ysize;
  m_savedImage.setCsizeByFormat(image.format);
  m_savedImage.reallocate();

  if (m_reset){
    memcpy(m_savedImage.data,image.data,pixsize);
    m_reset = 0; 
  }

  npixes=image.data;
  opixes=m_savedImage.data;
  const unsigned char thresh=m_Urange;
  i=pixsize;
  while(i--){
    newpix=*npixes++;
    oldpix=*opixes++;
    if((newpix>oldpix-thresh)&&(newpix<oldpix+thresh))npixes[-1]=0;
  }
  m_reset = 0; 
}
/////////////////////////////////////////////////////////
// do the YUV processing here
//
/////////////////////////////////////////////////////////
void pix_background :: processYUVImage(imageStruct &image)
{
  int h,w,hlength;
  long src,pixsize;

  src = 0;
  pixsize = image.xsize * image.ysize * image.csize;

  if(m_savedImage.xsize!=image.xsize ||
     m_savedImage.ysize!=image.ysize ||
     m_savedImage.format!=image.format)m_reset=1;

  m_savedImage.xsize=image.xsize;
  m_savedImage.ysize=image.ysize;
  m_savedImage.setCsizeByFormat(image.format);
  m_savedImage.reallocate();
  
  if (m_reset){
    memcpy(m_savedImage.data,image.data,pixsize);
    m_reset = 0; 
    // return;
  }

   
  hlength = image.xsize/2;

  unsigned char*data =image.data;
  unsigned char*saved=m_savedImage.data;

  for (h=0; h<image.ysize; h++){
    for(w=0; w<hlength; w++){
          
      if (((data[src] > saved[src] - m_Urange)&&(data[src] < saved[src] + m_Urange))&&
	  ((data[src+1] > saved[src+1] - m_Yrange)&&(data[src+1] < saved[src+1] + m_Yrange))&&
	  ((data[src+2] > saved[src+2] - m_Vrange)&&(data[src+2] < saved[src+2] + m_Vrange)))
	{
	  data[src]   = 128;
	  data[src+1] = 0;
	  data[src+2] = 128;
	  data[src+3] = 0;
	}
      src+=4;
    }
  }
  m_reset = 0; 
}

/////////////////////////////////////////////////////////
// the killer go fast stuff goes in here
//
/////////////////////////////////////////////////////////

#ifdef __MMX__
void pix_background :: processRGBAMMX(imageStruct &image)
{
  long i,pixsize;
  pixsize = image.xsize * image.ysize * image.csize;

  if(m_savedImage.xsize!=image.xsize ||
     m_savedImage.ysize!=image.ysize ||
     m_savedImage.format!=image.format)m_reset=1;

  m_savedImage.xsize=image.xsize;
  m_savedImage.ysize=image.ysize;
  m_savedImage.setCsizeByFormat(image.format);
  m_savedImage.reallocate();

  if (m_reset){
    memcpy(m_savedImage.data,image.data,pixsize);
  }
  m_reset=0;

  i=pixsize/sizeof(__m64)+(pixsize%sizeof(__m64)!=0);

  __m64*data =(__m64*)image.data;
  __m64*saved=(__m64*)m_savedImage.data;

  const __m64 thresh=_mm_set_pi8(m_Yrange, m_Urange, m_Vrange, m_Arange,
				m_Yrange, m_Urange, m_Vrange, m_Arange);
  const __m64 offset=_mm_set_pi8(1, 1, 1, 1, 1, 1, 1, 1);
  __m64 newpix, oldpix, m1;

  while(i--){
    /* 7ops, 3memops */
    /* i have the feeling that this is not faster at all! 
     * even if i have the 3memops + ONLY 1 _mm_subs_pu8() 
     * i am equally slow as the generic code; 
     * adding the other instruction does not change much
     */
    newpix=*data;
    oldpix=*saved++;
    m1    = newpix;
    m1    = _mm_subs_pu8     (m1, oldpix);
    oldpix= _mm_subs_pu8     (oldpix, newpix);
    m1    = _mm_or_si64      (m1, oldpix); // |oldpix-newpix|
    m1    = _mm_adds_pu8     (m1, offset);
    m1    = _mm_subs_pu8     (m1, thresh);
    m1    = _mm_cmpeq_pi32   (m1, _mm_setzero_si64()); // |oldpix-newpix|>thresh
    m1    = _mm_andnot_si64(m1, newpix);

    *data++ = m1; 
  }
  _mm_empty();
}
void pix_background :: processYUVMMX(imageStruct &image)
{
  long pixsize;

  pixsize = image.xsize * image.ysize * image.csize;

  if(m_savedImage.xsize!=image.xsize ||
     m_savedImage.ysize!=image.ysize ||
     m_savedImage.format!=image.format)m_reset=1;

  m_savedImage.xsize=image.xsize;
  m_savedImage.ysize=image.ysize;
  m_savedImage.setCsizeByFormat(image.format);
  m_savedImage.reallocate();
  
  if (m_reset){
    memcpy(m_savedImage.data,image.data,pixsize);
    // return;
  }
  m_reset=0;

  int i=pixsize/sizeof(__m64)+(pixsize%sizeof(__m64)!=0);

  __m64*data =(__m64*)image.data;
  __m64*saved=(__m64*)m_savedImage.data;

  const __m64 thresh=_mm_set_pi8(m_Urange, m_Yrange, m_Vrange, m_Yrange,
			  m_Urange, m_Yrange, m_Vrange, m_Yrange);
  const __m64 offset=_mm_set_pi8(1, 1, 1, 1, 1, 1, 1, 1);
  const __m64 black =_mm_set_pi8((unsigned char)0x00,
				 (unsigned char)0x80,
				 (unsigned char)0x00,
				 (unsigned char)0x80,
				 (unsigned char)0x00,
				 (unsigned char)0x80,
				 (unsigned char)0x00,
				 (unsigned char)0x80);

  __m64 newpix, oldpix, m1;

  while(i--){
    newpix=*data;
    oldpix=*saved++;
    m1    = newpix;
    m1    = _mm_subs_pu8     (m1, oldpix);
    oldpix= _mm_subs_pu8     (oldpix, newpix);
    m1    = _mm_or_si64      (m1, oldpix); // |oldpix-newpix|
    m1    = _mm_adds_pu8     (m1, offset); // to make thresh=0 work correctly
    m1    = _mm_subs_pu8     (m1, thresh);  // m1>thresh -> saturation -> 0
    m1    = _mm_cmpeq_pi32   (m1, _mm_setzero_si64()); // |oldpix-newpix|>thresh

    oldpix= black;
    oldpix= _mm_and_si64     (oldpix, m1);

    m1    = _mm_andnot_si64  (m1, newpix);
    m1    = _mm_or_si64      (m1, oldpix);

    *data++ = m1; 
  }
  _mm_empty();
}

void pix_background :: processGrayMMX(imageStruct &image){
  int i;
  long pixsize;

  pixsize = image.xsize * image.ysize * image.csize;
  if(m_savedImage.xsize!=image.xsize ||
     m_savedImage.ysize!=image.ysize ||
     m_savedImage.format!=image.format)m_reset=1;

  m_savedImage.xsize=image.xsize;
  m_savedImage.ysize=image.ysize;
  m_savedImage.setCsizeByFormat(image.format);
  m_savedImage.reallocate();

  if (m_reset){
    memcpy(m_savedImage.data,image.data,pixsize);
  }
  m_reset=0;
  if(m_Yrange==0)return;  

  __m64*npixes=(__m64*)image.data;
  __m64*opixes=(__m64*)m_savedImage.data;
  __m64 newpix, oldpix, m1;

  unsigned char thresh=m_Yrange-1;
  __m64 thresh8=_mm_set_pi8(thresh,thresh,thresh,thresh,
			  thresh,thresh,thresh,thresh);

  
  i=pixsize/sizeof(__m64)+(pixsize%sizeof(__m64)!=0);
  while(i--){
    newpix=npixes[i];
    oldpix=opixes[i];
    
    m1    = _mm_subs_pu8 (newpix, oldpix);
    oldpix= _mm_subs_pu8 (oldpix, newpix);
    m1    = _mm_or_si64  (m1, oldpix); // |oldpix-newpix|
    m1    = _mm_subs_pu8 (m1, thresh8);
    m1    = _mm_cmpgt_pi8(m1, _mm_setzero_si64()); // |oldpix-newpix|>thresh8
    npixes[i] = _mm_and_si64(m1, newpix);
  }
  _mm_empty();
}
#endif /* __MMX__ */

#ifdef __VEC__
void pix_background :: processYUVAltivec(imageStruct &image)
{
register int h,w,i,j,width;
int pixsize = image.xsize * image.ysize * image.csize;
    h = image.ysize;
    w = image.xsize/8;
    width = image.xsize/8;
    
    //check to see if the buffer isn't 16byte aligned (highly unlikely)
    if (image.ysize*image.xsize % 16 != 0){
        error("image not properly aligned for Altivec - try something SD or HD maybe?");
        return;
        }
    
    union{
        unsigned short		s[8];
        vector unsigned short	v;
    }shortBuffer;

    if(m_savedImage.xsize!=image.xsize ||
       m_savedImage.ysize!=image.ysize ||
       m_savedImage.format!=image.format)m_reset=1;

    m_savedImage.xsize=image.xsize;
    m_savedImage.ysize=image.ysize;
    m_savedImage.setCsizeByFormat(image.format);
    m_savedImage.reallocate();
    
    if (m_reset){
    memcpy(m_savedImage.data,image.data,pixsize);
    m_reset = 0; 
    }
    
    register vector unsigned short	UVres1, Yres1, UVres2, Yres2;//interleave;
    register vector unsigned short	hiImage, loImage;
    register vector unsigned short	Yrange, UVrange, Yblank,UVblank,blank;
    register vector bool short		Ymasklo,Ymaskhi,  UVmaskhi;
    register vector unsigned short	Yhi,Ylo,UVhi,UVlo; 
    register vector unsigned char	one = vec_splat_u8(1);
    register vector unsigned short	sone = vec_splat_u16(1);
    register vector unsigned int			Uhi, Ulo, Vhi, Vlo,Ures,Vres;
    register vector bool int 			Umasklo, Umaskhi, Vmaskhi, Vmasklo;

    vector unsigned char	*inData = (vector unsigned char*) image.data;
    vector unsigned char	*rightData = (vector unsigned char*) m_savedImage.data;
    
    shortBuffer.s[0] =  m_Yrange;
    Yrange = shortBuffer.v;
    Yrange = vec_splat(Yrange,0);
    
    shortBuffer.s[0] = 128;
    shortBuffer.s[1] = 0;
    shortBuffer.s[2] = 128;
    shortBuffer.s[3] = 0;
    shortBuffer.s[4] = 128;
    shortBuffer.s[5] = 0;
    shortBuffer.s[6] = 128;
    shortBuffer.s[7] = 0;
    blank = shortBuffer.v;
    
    shortBuffer.s[0] =  0;
    Yblank = shortBuffer.v;
    Yblank = vec_splat(Yblank,0);
    
    shortBuffer.s[0] =  128;
    UVblank = shortBuffer.v;
    UVblank = vec_splat(UVblank,0);
    
    shortBuffer.s[0] = m_Urange;
    shortBuffer.s[1] = m_Vrange;
    shortBuffer.s[2] = m_Urange;
    shortBuffer.s[3] = m_Vrange;
    shortBuffer.s[4] = m_Urange;
    shortBuffer.s[5] = m_Vrange;
    shortBuffer.s[6] = m_Urange;
    shortBuffer.s[7] = m_Vrange;
    UVrange = shortBuffer.v;
    
    
    //setup the cache prefetch -- A MUST!!!
    UInt32			prefetchSize = GetPrefetchConstant( 16, 1, 256 );
    #ifndef PPC970 
    vec_dst( inData, prefetchSize, 0 );
    vec_dst( rightData, prefetchSize, 1 );
    vec_dst( inData+32, prefetchSize, 2 );
    vec_dst( rightData+32, prefetchSize, 3 );
    #endif //PPC970
    
    for ( i=0; i<h; i++){
        for (j=0; j<w; j++)
        {
        #ifndef PPC970
        //this function is probably memory bound on most G4's -- what else is new?
            vec_dst( inData, prefetchSize, 0 );
            vec_dst( rightData, prefetchSize, 1 );
            vec_dst( inData+32, prefetchSize, 2 );
            vec_dst( rightData+32, prefetchSize, 3 );
        #endif
        //separate the U and V from Y
        UVres1 = (vector unsigned short)vec_mule(one,inData[0]);
        UVres2 = (vector unsigned short)vec_mule(one,rightData[0]);
            
        //vec_mulo Y * 1 to short vector Y Y Y Y shorts
        Yres1 = (vector unsigned short)vec_mulo(one,inData[0]);
        Yres2 = (vector unsigned short)vec_mulo(one,rightData[0]);
        
        Yhi = vec_adds(Yres2,Yrange);
        Ylo = vec_subs(Yres2,Yrange);
        
        //go to ints for comparison
        UVhi = vec_adds(UVres2,UVrange);
        UVlo = vec_subs(UVres2,UVrange);
        
        Uhi = vec_mule(sone,UVhi);
        Ulo = vec_mule(sone,UVlo);
        
        Vhi = vec_mulo(sone,UVhi);
        Vlo = vec_mulo(sone,UVlo);
        
        Ures = vec_mule(sone,UVres1);
         Vres = vec_mulo(sone,UVres1);
         
         Umasklo = vec_cmpgt(Ures,Ulo);
         Umaskhi = vec_cmplt(Ures,Uhi);
         
         Vmasklo = vec_cmpgt(Vres,Vlo);
         Vmaskhi = vec_cmplt(Vres,Vhi);
         
         Umaskhi = vec_and(Umaskhi,Umasklo);
         
         Vmaskhi = vec_and(Vmaskhi,Vmasklo);
         
         Umasklo = vec_and(Umaskhi,Vmaskhi);
         Vmasklo = vec_and(Umaskhi,Vmaskhi);
         
         hiImage = (vector unsigned short)vec_mergeh(Umasklo,Vmasklo);
         loImage = (vector unsigned short)vec_mergel(Umasklo,Vmasklo);
         
         //pack it back down to bool short
         UVmaskhi = (vector bool short)vec_packsu(hiImage,loImage);
         
         Ymasklo = vec_cmpgt(Yres1,Ylo);
         Ymaskhi = vec_cmplt(Yres1,Yhi);
         
         Ymaskhi = vec_and(Ymaskhi,Ymasklo);
         
         Ymaskhi = vec_and(Ymaskhi,UVmaskhi);
         UVmaskhi = vec_and(Ymaskhi,UVmaskhi);
         
         //bitwise comparison and move using the result of the comparison as a mask
         Yres1 = vec_sel(Yres1,Yblank,Ymaskhi);
         
         //UVres1 = vec_sel(UVres1,UVres2,UVmaskhi);
         UVres1 = vec_sel(UVres1,UVblank,UVmaskhi);
         
         //merge the Y and UV back together
         hiImage = vec_mergeh(UVres1,Yres1);
         loImage = vec_mergel(UVres1,Yres1);
         
         //pack it back down to unsigned char to store
         inData[0] = vec_packsu(hiImage,loImage);
         
         inData++;
         rightData++;
        
        }
        #ifndef PPC970
        vec_dss(0);
        vec_dss(1);
        vec_dss(2);
        vec_dss(3);
        #endif
    }
}
#endif //ALTIVEC


void pix_background :: rangeNMess(int argc, t_atom*argv){
  /* normalized values (float)0..1 instead of (int)0..255 */
  unsigned int v=0;
  m_Arange=255;
  switch(argc){
  case 4:  
    m_Arange=CLAMP((float)255.*atom_getfloat(argv+3));
  case 3:
    m_Yrange=CLAMP((float)255.*atom_getfloat(argv));
    m_Urange=CLAMP((float)255.*atom_getfloat(argv+1));
    m_Vrange=CLAMP((float)255.*atom_getfloat(argv+2));
    break;
  case 1:
    v=CLAMP((float)255.*atom_getfloat(argv));
    m_Yrange=v;
    m_Urange=v;
    m_Vrange=v;
    break;
  default:
    error("only 1 or 3 values are allowed as ranges (not %d)", argc);
  }
}


/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_background :: obj_setupCallback(t_class *classPtr)
{
  class_addbang(classPtr, reinterpret_cast<t_method>(&pix_background::resetCallback));
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_background::rangeNCallback),
		  gensym("range_n"), A_GIMME, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_background::rangeCallback),
		  gensym("range"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_background::resetCallback),
		  gensym("reset"), A_NULL);
}


void pix_background :: rangeCallback(void *data, t_floatarg Y, t_floatarg U, t_floatarg V)
{
  GetMyClass(data)->m_Yrange=((int)Y);
  GetMyClass(data)->m_Urange=((int)U);
  GetMyClass(data)->m_Vrange=((int)V);

}

void pix_background :: resetCallback(void *data)
{
  GetMyClass(data)->m_reset=1;
}

void pix_background :: rangeNCallback(void *data, t_symbol*,int argc, t_atom*argv){
  /* normalized values (float)0..1 instead of (int)0..255 */
  GetMyClass(data)->rangeNMess(argc, argv);
}
