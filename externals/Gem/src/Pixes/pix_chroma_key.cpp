/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia
    pix_chroma_key.cpp
    gem_darwin
  
    Created by chris clepper on Mon Oct 07 2002.
    Copyright (c) 2002-2006 cgc. All rights reserved.
 
    Copyright (c) 1997-2000 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#include "pix_chroma_key.h"

CPPEXTERN_NEW(pix_chroma_key);

/////////////////////////////////////////////////////////
//
// pix_chroma_key
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_chroma_key :: pix_chroma_key()
{
  m_direction = 1;
  m_mode=1;
  m_Yrange = m_Vrange = m_Vrange = m_Yvalue = m_Uvalue = m_Vvalue = 0;
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_chroma_key :: ~pix_chroma_key()
{}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_chroma_key :: processRGBA_RGBA(imageStruct &image, imageStruct &right)
{
  long src;
  int datasize = image.xsize * image.ysize;
  unsigned char *leftPix = image.data;
  unsigned char *rightPix = right.data;
  unsigned char Ghi,Glo,Bhi,Blo,Rhi,Rlo;
  src =0;

  Rhi = CLAMP(m_Yvalue + m_Yrange); 
  Rlo = CLAMP(m_Yvalue - m_Yrange);
  Ghi = CLAMP(m_Uvalue + m_Urange); 
  Glo = CLAMP(m_Uvalue - m_Urange);
  Bhi = CLAMP(m_Vvalue + m_Vrange); 
  Blo = CLAMP(m_Vvalue - m_Vrange);

  if (m_direction) {    
    while(datasize--){        
      if ((leftPix[chBlue] < Bhi)&&(leftPix[chBlue] > Blo)&&
	  (leftPix[chRed]  < Rhi)&&(leftPix[chRed]  > Rlo)&&
	  (leftPix[chGreen]< Ghi)&&(leftPix[chGreen]> Glo))
	{
	  leftPix[chRed]   = rightPix[chRed];
	  leftPix[chGreen] = rightPix[chGreen];
	  leftPix[chBlue]  = rightPix[chBlue];
	}
      leftPix+=4;
      rightPix+=4;
    }
  } else { //this needs help
    while(datasize--){
      if (!((leftPix[chBlue] < Bhi)&&(leftPix[chBlue] > Blo)&&
            (leftPix[chRed]  < Rhi)&&(leftPix[chRed]  > Rlo)&&
            (leftPix[chGreen]< Ghi)&&(leftPix[chGreen]> Glo)))
	{
	  leftPix[chRed]   = rightPix[chRed];
	  leftPix[chGreen] = rightPix[chGreen];
	  leftPix[chBlue]  = rightPix[chBlue];
	}
      leftPix+=4;
      rightPix+=4;
    }
  }
}


/////////////////////////////////////////////////////////
// do the YUV processing here
//
/////////////////////////////////////////////////////////
void pix_chroma_key :: processYUV_YUV(imageStruct &image, imageStruct &right)
{
   long src,h,w,xsize;
   unsigned char Uhi,Ulo,Vhi,Vlo,Yhi,Ylo;
   src =0;
   bool change1,change2,change3,change4;

   Yhi = CLAMP(m_Yvalue + m_Yrange); 
   Ylo = CLAMP(m_Yvalue - m_Yrange);
   Uhi = CLAMP(m_Uvalue + m_Urange); 
   Ulo = CLAMP(m_Uvalue - m_Urange);
   Vhi = CLAMP(m_Vvalue + m_Vrange); 
   Vlo = CLAMP(m_Vvalue - m_Vrange);
   //format is U Y V Y
   xsize = image.xsize/2;
   if (m_mode){
     if (m_direction) {
       for (h=0; h<image.ysize; h++){
	 for(w=0; w<xsize; w++){
	   /*   if (
		((image.data[src] < Uhi)&&(image.data[src] > Ulo))&&
		((image.data[src+1] < Yhi)&&(image.data[src+1] > Ylo)||(image.data[src+3] < Yhi)&&(image.data[src+3] > Ylo))&&
		((image.data[src+2] < Vhi)&&(image.data[src+2] > Vlo)))
                {
                image.data[src] = right.data[src];
                image.data[src+1] = right.data[src+1];
		
                image.data[src+2] = right.data[src+2];
                image.data[src+3] = right.data[src+3];
                } */
	   if (
	       ((image.data[src] < Uhi)&&(image.data[src] > Ulo))&&
	       ((image.data[src+1] < Yhi)&&(image.data[src+1] > Ylo))&&
	       ((image.data[src+2] < Vhi)&&(image.data[src+2] > Vlo))
	       )
	     {
	       image.data[src] = right.data[src];
	       image.data[src+1] = right.data[src+1];
	     }
	   if (
	       ((image.data[src] < Uhi)&&(image.data[src] > Ulo))&&
	       ((image.data[src+2] < Vhi)&&(image.data[src+2] > Vlo))&&
	       ((image.data[src+3] < Yhi)&&(image.data[src+3] > Ylo))
	       )   
	     {
	       image.data[src+2] = right.data[src+2];
	       image.data[src+3] = right.data[src+3];
	     }    
	   src+=4;
	 }
       }
     } else { //this needs help
       for (h=0; h<image.ysize; h++){
	 for(w=0; w<xsize; w++){
	   if (!((image.data[src] < Uhi)&&(image.data[src] > Ulo)&&
		 (image.data[src+1] < Yhi)&&(image.data[src+1] > Ylo)&&
		 (image.data[src+2] < Vhi)&&(image.data[src+2] > Vlo)))
	     {
	       image.data[src] = right.data[src];
	       image.data[src+1] = right.data[src+1];
	       image.data[src+2] = right.data[src+2];
	       image.data[src+3] = right.data[src+3];
	     }
	   src+=4;
	 }
       }   
     } 
   }else{
     /**/
     //this mode does interpolation between Y values if one of the pair lies outside the range
     // could this also be done even if both lie in the range too??
     if (m_direction) {
       for (h=0; h<image.ysize; h++){
	 for(w=0; w<image.xsize/4; w++){
	   change1 = 0;
	   change2 = 0;  
	   change3 = 0;
	   change4 = 0;  
	   if ((image.data[src] < Uhi)&&(image.data[src] > Ulo)&&
	       (image.data[src+1] < Yhi)&&(image.data[src+1] > Ylo)&&
	       (image.data[src+2] < Vhi)&&(image.data[src+2] > Vlo)){
	     change1 = 1;
	   }

	   if ((image.data[src] < Uhi)&&(image.data[src] > Ulo)&&
	       (image.data[src+3] < Yhi)&&(image.data[src+3] > Ylo)&&
	       (image.data[src+2] < Vhi)&&(image.data[src+2] > Vlo)){
	     change2 = 1;
	   }

	   if ((image.data[src+4] < Uhi)&&(image.data[src+4] > Ulo)&&
	       (image.data[src+5] < Yhi)&&(image.data[src+5] > Ylo)&&
	       (image.data[src+6] < Vhi)&&(image.data[src+6] > Vlo)){
	     change3 = 1;
	   }

	   if ((image.data[src+4] < Uhi)&&(image.data[src+4] > Ulo)&&
	       (image.data[src+7] < Yhi)&&(image.data[src+7] > Ylo)&&
	       (image.data[src+6] < Vhi)&&(image.data[src+6] > Vlo)){
	     change4 = 1;
	   }   

	   if (change1 && change2 && change3 && change4){
	     image.data[src] = right.data[src];
	     image.data[src+1] = right.data[src+1];
	     image.data[src+2] = right.data[src+2];
	     image.data[src+3] = right.data[src+3];
	     image.data[src+4] = right.data[src+4];
	     image.data[src+5] = right.data[src+5];
	     image.data[src+6] = right.data[src+6];
	     image.data[src+7] = right.data[src+7];
	   }else{
	     if(change1 || change2 || change3 || change4){
	       int temp1,temp2;
	       image.data[src] = right.data[src];
	       temp1 = ((image.data[src+1] * 32) + (image.data[src+3] * 32) + (image.data[src+5] * 32) + (image.data[src+7] * 32))>>8;
	       temp2 = ((right.data[src+1] * 32) + (right.data[src+3] * 32)+ (right.data[src+5] * 32) + (right.data[src+7] * 32))>>8;

	       //   temp1 = ((image.data[src+1] * 255) + (right.data[src+1] * 0))>>8;
	       image.data[src+1] = CLAMP(temp1 + temp2) ;
	       image.data[src+2] = right.data[src+2];
                
	       //  temp2 = ((image.data[src+3] * 192) + (right.data[src+3] * 64))>>8;
	       image.data[src+3] = CLAMP(temp1 + temp2);
	       image.data[src+4] = right.data[src+4];

	       //  temp1 = ((image.data[src+5] * 128) + (right.data[src+5] * 128))>>8;
	       image.data[src+5] = CLAMP(temp1 + temp2);
	       image.data[src+6] = right.data[src+6];

	       //  temp2 = ((image.data[src+7] * 64) + (right.data[src+7] * 192))>>8;
	       image.data[src+7] = CLAMP(temp1 + temp2);                
	       change1 = 0; change2 = 0;change3 = 0; change4 = 0;
	     }  
	     //  }else{
	     // }
	   }    
	   src+=8;
	 }
       }    
     }else{
       for (h=0; h<image.ysize; h++){
	 for(w=0; w<image.xsize/2; w++){
	   if (!((image.data[src] < Uhi)&&(image.data[src] > Ulo)&&
		 (image.data[src+1] < Yhi)&&(image.data[src+1] > Ylo))){
	     image.data[src] = right.data[src];
	     image.data[src+1] = right.data[src+1];
	   }
	   if (!((image.data[src+3] < Yhi)&&(image.data[src+3] > Ylo)&&
		 (image.data[src+2] < Vhi)&&(image.data[src+2] > Vlo))){
	     image.data[src+2] = right.data[src+2];
	     image.data[src+3] = right.data[src+3];   
	   }
	   src+=4;
	 }
       }
     }
   }
}

#ifdef __MMX__
void pix_chroma_key :: processRGBA_MMX(imageStruct &image, imageStruct &right)
{
  int datasize = image.xsize * image.ysize * image.csize;
  datasize=datasize/sizeof(__m64)+(datasize%sizeof(__m64)!=0);

  __m64 *leftPix =  (__m64*)image.data;
  __m64 *rightPix = (__m64*)right.data;


  const __m64 hi=_mm_setr_pi8(CLAMP(m_Yvalue + m_Yrange), 
			CLAMP(m_Uvalue + m_Urange),
			CLAMP(m_Vvalue + m_Vrange),
			(unsigned char)0xFF,
			CLAMP(m_Yvalue + m_Yrange), 
			CLAMP(m_Uvalue + m_Urange),
			CLAMP(m_Vvalue + m_Vrange),
			(unsigned char)0xFF);
  const __m64 lo=_mm_setr_pi8(CLAMP(m_Yvalue - m_Yrange), 
			CLAMP(m_Uvalue - m_Urange),
			CLAMP(m_Vvalue - m_Vrange),
			(unsigned char)0x00,
			CLAMP(m_Yvalue - m_Yrange), 
			CLAMP(m_Uvalue - m_Urange),
			CLAMP(m_Vvalue - m_Vrange),
			(unsigned char)0x00);

  const __m64 null64=_mm_setzero_si64();

  __m64 r, l, b0, b1;

  if (m_direction) {    
    while(datasize--){
      l=leftPix [datasize];
      r=rightPix[datasize];

      b0=_mm_subs_pu8   (lo, l);
      b1=_mm_subs_pu8   (l, hi);
      b0=_mm_cmpeq_pi32 (b0, null64);
      b1=_mm_cmpeq_pi32 (b1, null64);

      b0=_mm_and_si64   (b0, b1);

      b1=_mm_and_si64   (b0, r);
      b0=_mm_andnot_si64(b0, l);

      leftPix[datasize]=_mm_or_si64(b0, b1);
    }
  } else {
    while(datasize--){
      l=leftPix [datasize];
      r=rightPix[datasize];

      b0=_mm_subs_pu8   (lo, l);
      b1=_mm_subs_pu8   (l, hi);
      b0=_mm_cmpeq_pi32 (b0, null64);
      b1=_mm_cmpeq_pi32 (b1, null64);

      b0=_mm_and_si64   (b0, b1);

      b1=_mm_and_si64   (b0, l);
      b0=_mm_andnot_si64(b0, r);

      leftPix[datasize]=_mm_or_si64(b0, b1);
    }
  }
  _mm_empty();
}

void pix_chroma_key :: processYUV_MMX(imageStruct &image, imageStruct &right)
{
  int datasize = image.xsize * image.ysize * image.csize;
  datasize=datasize/sizeof(__m64)+(datasize%sizeof(__m64)!=0);

  __m64 *leftPix =  (__m64*)image.data;
  __m64 *rightPix = (__m64*)right.data;

  // no m_mode yet (does it make any sense at all ?)

  const __m64 hi=_mm_setr_pi8(CLAMP(m_Uvalue + m_Urange), 
			      CLAMP(m_Yvalue + m_Yrange),
			      CLAMP(m_Vvalue + m_Vrange),
			      CLAMP(m_Yvalue + m_Yrange),
			      CLAMP(m_Uvalue + m_Urange), 
			      CLAMP(m_Yvalue + m_Yrange),
			      CLAMP(m_Vvalue + m_Vrange),
			      CLAMP(m_Yvalue + m_Yrange));
  const __m64 lo=_mm_setr_pi8(CLAMP(m_Uvalue - m_Urange), 
			      CLAMP(m_Yvalue - m_Yrange),
			      CLAMP(m_Vvalue - m_Vrange),
			      CLAMP(m_Yvalue - m_Yrange),
			      CLAMP(m_Uvalue - m_Urange), 
			      CLAMP(m_Yvalue - m_Yrange),
			      CLAMP(m_Vvalue - m_Vrange),
			      CLAMP(m_Yvalue - m_Yrange));

  const __m64 null64=_mm_setzero_si64();

  __m64 r, l, b0, b1;

  if (m_direction) {    
    while(datasize--){
      l=leftPix [datasize];
      r=rightPix[datasize];

      b0=_mm_subs_pu8   (lo, l);
      b1=_mm_subs_pu8   (l, hi);
      b0=_mm_cmpeq_pi32 (b0, null64);
      b1=_mm_cmpeq_pi32 (b1, null64);

      b0=_mm_and_si64   (b0, b1);

      b1=_mm_and_si64   (b0, r);
      b0=_mm_andnot_si64(b0, l);

      leftPix[datasize]=_mm_or_si64(b0, b1);
    }
  } else {
    while(datasize--){
      l=leftPix [datasize];
      r=rightPix[datasize];

      b0=_mm_subs_pu8(lo, l);
      b1=_mm_subs_pu8(l, hi);
      b0=_mm_cmpeq_pi32(b0, null64);
      b1=_mm_cmpeq_pi32(b1, null64);

      b0=_mm_and_si64 (b0, b1);

      b1=_mm_and_si64   (b0, l);
      b0=_mm_andnot_si64(b0, r);

      leftPix[datasize]=_mm_or_si64(b0, b1);
    }
  }
  _mm_empty();
}

void pix_chroma_key :: processGray_MMX(imageStruct &image, imageStruct &right)
{
  int datasize = image.xsize * image.ysize * image.csize;
  datasize=datasize/sizeof(__m64)+(datasize%sizeof(__m64)!=0);

  __m64 *leftPix =  (__m64*)image.data;
  __m64 *rightPix = (__m64*)right.data;

  // no m_mode yet (does it make any sense at all ?)

  const __m64 hi=_mm_setr_pi8(CLAMP(m_Yvalue + m_Yrange), 
			      CLAMP(m_Yvalue + m_Yrange),
			      CLAMP(m_Yvalue + m_Yrange),
			      CLAMP(m_Yvalue + m_Yrange),
			      CLAMP(m_Yvalue + m_Yrange), 
			      CLAMP(m_Yvalue + m_Yrange),
			      CLAMP(m_Yvalue + m_Yrange),
			      CLAMP(m_Yvalue + m_Yrange));
  const __m64 lo=_mm_setr_pi8(CLAMP(m_Yvalue - m_Yrange), 
			      CLAMP(m_Yvalue - m_Yrange),
			      CLAMP(m_Yvalue - m_Yrange),
			      CLAMP(m_Yvalue - m_Yrange),
			      CLAMP(m_Yvalue - m_Yrange), 
			      CLAMP(m_Yvalue - m_Yrange),
			      CLAMP(m_Yvalue - m_Yrange),
			      CLAMP(m_Yvalue - m_Yrange));

  const __m64 null64=_mm_setzero_si64();

  __m64 r, l, b0, b1;

  if (m_direction) {    
    while(datasize--){
      l=leftPix [datasize];
      r=rightPix[datasize];

      b0=_mm_subs_pu8   (lo, l);
      b1=_mm_subs_pu8   (l, hi);
      b0=_mm_cmpeq_pi32 (b0, null64);
      b1=_mm_cmpeq_pi32 (b1, null64);

      b0=_mm_and_si64   (b0, b1);

      b1=_mm_and_si64   (b0, r);
      b0=_mm_andnot_si64(b0, l);

      leftPix[datasize]=_mm_or_si64(b0, b1);
    }
  } else {
    while(datasize--){
      l=leftPix [datasize];
      r=rightPix[datasize];

      b0=_mm_subs_pu8(lo, l);
      b1=_mm_subs_pu8(l, hi);
      b0=_mm_cmpeq_pi32(b0, null64);
      b1=_mm_cmpeq_pi32(b1, null64);

      b0=_mm_and_si64 (b0, b1);

      b1=_mm_and_si64   (b0, l);
      b0=_mm_andnot_si64(b0, r);

      leftPix[datasize]=_mm_or_si64(b0, b1);
    }
  }
  _mm_empty();
}

#endif

/////////////////////////////////////////////////////////
// the killer go fast stuff goes in here
//
/////////////////////////////////////////////////////////
#ifdef __VEC__
void pix_chroma_key :: processYUV_Altivec(imageStruct &image, imageStruct &right)
{
register int h,w,i,j,width;

    h = image.ysize;
    w = image.xsize/8;
    width = image.xsize/8;
    
    //check to see if the buffer isn't 16byte aligned (highly unlikely)
    if (image.ysize*image.xsize % 16 != 0){
        error("image not properly aligned for Altivec");
        return;
        }

    union{
        unsigned short		s[8];
        vector unsigned short	v;
    }shortBuffer;

    union{
        unsigned int		i[4];
        vector unsigned int	v;
    }longBuffer;

    register vector unsigned short	UVres1, Yres1, UVres2, Yres2;//interleave;
    register vector unsigned short	hiImage, loImage;
    register vector bool short		Ymasklo,Ymaskhi, UVmaskhi;//UVmasklo, Vmaskhi, Vmasklo;
    register vector unsigned short	Yhi,Ylo;//UVhi,UVlo;Vhi,Vlo;
    register vector unsigned char	one = vec_splat_u8(1);
    register vector unsigned short	sone = vec_splat_u16(1);
    register vector unsigned int			Uhi, Ulo, Vhi, Vlo,Ures,Vres;
    register vector bool int 			Umasklo, Umaskhi, Vmaskhi, Vmasklo;

    vector unsigned char	*inData = (vector unsigned char*) image.data;
    vector unsigned char	*rightData = (vector unsigned char*) right.data;
   
    shortBuffer.s[0] = CLAMP(m_Yvalue + m_Yrange);
    Yhi = shortBuffer.v;
    Yhi = vec_splat(Yhi,0);
    
    shortBuffer.s[0] = CLAMP(m_Yvalue - m_Yrange);
    Ylo = shortBuffer.v;
    Ylo = vec_splat(Ylo,0);
    
    longBuffer.i[0] = CLAMP(m_Uvalue + m_Urange);
    longBuffer.i[1] = CLAMP(m_Uvalue + m_Urange);
    longBuffer.i[2] = CLAMP(m_Uvalue + m_Urange);
    longBuffer.i[3] = CLAMP(m_Uvalue + m_Urange);
    
    
    Uhi = longBuffer.v;
    
    longBuffer.i[0] = CLAMP(m_Uvalue - m_Urange);
    longBuffer.i[1] = CLAMP(m_Uvalue - m_Urange);
    longBuffer.i[2] = CLAMP(m_Uvalue - m_Urange);
    longBuffer.i[3] = CLAMP(m_Uvalue - m_Urange);
    
    
    Ulo = longBuffer.v;
    
    longBuffer.i[0] = CLAMP(m_Vvalue + m_Vrange);
    longBuffer.i[1] = CLAMP(m_Vvalue + m_Vrange);
    longBuffer.i[2] = CLAMP(m_Vvalue + m_Vrange);
    longBuffer.i[3] = CLAMP(m_Vvalue + m_Vrange);
    
    
    Vhi = longBuffer.v;
    
    longBuffer.i[0] = CLAMP(m_Vvalue - m_Vrange);
    longBuffer.i[1] = CLAMP(m_Vvalue - m_Vrange);
    longBuffer.i[2] = CLAMP(m_Vvalue - m_Vrange);
    longBuffer.i[3] = CLAMP(m_Vvalue - m_Vrange);
    
    
    Vlo = longBuffer.v;
    #ifndef PPC970
    //setup the cache prefetch -- A MUST!!!
    UInt32			prefetchSize = GetPrefetchConstant( 16, 1, 256 );
    vec_dst( inData, prefetchSize, 0 );
    vec_dst( rightData, prefetchSize, 1 );
    #endif
    if (m_direction) {
    
    for ( i=0; i<h; i++){
        for (j=0; j<w; j++)
        {
        #ifndef PPC970
        //this function is probably memory bound on most G4's -- what else is new?
        vec_dst( inData, prefetchSize, 0 );
        vec_dst( rightData, prefetchSize, 1 );
        #endif
        

        //separate the U and V from Y
        UVres1 = (vector unsigned short)vec_mule(one,inData[0]);
        UVres2 = (vector unsigned short)vec_mule(one,rightData[0]);
        
        //vec_mulo Y * 1 to short vector Y Y Y Y shorts
        Yres1 = (vector unsigned short)vec_mulo(one,inData[0]);
        Yres2 = (vector unsigned short)vec_mulo(one,rightData[0]);
                        
         //compare the Y values   
         Ymasklo = vec_cmpgt(Yres1,Ylo);
         Ymaskhi = vec_cmplt(Yres1,Yhi);
         
         Ymaskhi = vec_and(Ymaskhi,Ymasklo);
         /*
         UVmasklo = vec_cmpgt(Yres1,UVlo);
         UVmaskhi = vec_cmplt(Yres1,UVhi);
         
         UVmaskhi = vec_and(UVmaskhi,UVmasklo);
         */
         
         //so it seems the only way to do this is separate int vecs for U and V
         //more ops but still about 325% faster than scalar - so who cares?
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
         
         Ymaskhi = vec_and(Ymaskhi,UVmaskhi);
         UVmaskhi = vec_and(Ymaskhi,UVmaskhi);
         //bitwise comparison and move using the result of the comparison as a mask
         Yres1 = vec_sel(Yres1,Yres2,Ymaskhi);
         
         //UVres1 = vec_sel(UVres1,UVres2,UVmaskhi);
         UVres1 = vec_sel(UVres1,UVres2,UVmaskhi);
         
         //merge the Y and UV back together
         hiImage = vec_mergeh(UVres1,Yres1);
         loImage = vec_mergel(UVres1,Yres1);
         
         //pack it back down to unsigned char to store
         inData[0] = vec_packsu(hiImage,loImage);
         
            inData++;
            rightData++;
         
        }
        #ifndef PPC970
        vec_dss(1);
        vec_dss(0);
        #endif
    }
    }else{
    /*
    for ( i=0; i<h; i++){
        for (j=0; j<w; j++)
        {
        
        vec_dst( inData, prefetchSize, 0 );
        vec_dst( rightData, prefetchSize, 1 );

        
        UVres1 = (vector unsigned short)vec_mule(one,inData[0]);
        UVres2 = (vector unsigned short)vec_mule(one,rightData[0]);
            
        //vec_mulo Y * 1 to short vector Y Y Y Y shorts
        Yres1 = (vector unsigned short)vec_mulo(one,inData[0]);
        Yres2 = (vector unsigned short)vec_mulo(one,rightData[0]);
            
         Ymask1 = vec_cmplt(Yres1,Yres2);
         
         Yres1 = vec_sel(Yres2,Yres1,Ymask1);
         
         UVres1 = vec_sel(UVres2,UVres1,Ymask1);
         
         hiImage = vec_mergeh(UVres1,Yres1);
         loImage = vec_mergel(UVres1,Yres1);
         
         inData[0] = vec_packsu(hiImage,loImage);
         
            inData++;
            rightData++;
        
        }
        vec_dss(1);
        vec_dss(0);
	}
    */
    }
}
#endif


/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_chroma_key :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_chroma_key::directionCallback),
		  gensym("direction"), A_DEFFLOAT, A_NULL);
                  
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_chroma_key::modeCallback),
		  gensym("mode"), A_DEFFLOAT, A_NULL);
                  
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_chroma_key::rangeCallback),
		  gensym("range"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_NULL);
                  
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_chroma_key::valueCallback),
		  gensym("value"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_NULL);
}

void pix_chroma_key :: directionCallback(void *data, t_floatarg state)
{
  GetMyClass(data)->m_direction=!(!(int)state);
}

void pix_chroma_key :: modeCallback(void *data, t_floatarg state)
{
  GetMyClass(data)->m_mode=!(!(int)state);
}

//make separate ranges for Y U V
void pix_chroma_key :: rangeCallback(void *data, t_floatarg Yval, t_floatarg Uval,t_floatarg Vval)
{
  if(fabs(Yval)<=1.0 && fabs(Uval)<=1.0 && fabs(Vval)<=1.0) {
    GetMyClass(data)->m_Yrange=((unsigned char)255*Yval);
    GetMyClass(data)->m_Urange=((unsigned char)255*Uval);
    GetMyClass(data)->m_Vrange=((unsigned char)255*Vval);
  } else {
    GetMyClass(data)->post("using deprecated un-normalized ranges (0..255): consider using (0..1) instead!");
    GetMyClass(data)->m_Yrange=((unsigned char)Yval);
    GetMyClass(data)->m_Urange=((unsigned char)Uval);
    GetMyClass(data)->m_Vrange=((unsigned char)Vval);
  }
}

void pix_chroma_key :: valueCallback(void *data, t_floatarg Yval, t_floatarg Uval, t_floatarg Vval)
{
  if(fabs(Yval)<=1.0 && fabs(Uval)<=1.0 && fabs(Vval)<=1.0) {
    GetMyClass(data)->m_Yvalue=((unsigned char)255*Yval);
    GetMyClass(data)->m_Uvalue=((unsigned char)255*Uval);
    GetMyClass(data)->m_Vvalue=((unsigned char)255*Vval);
  } else {
    GetMyClass(data)->post("using deprecated un-normalized values (0..255): consider using (0..1) instead!");
    GetMyClass(data)->m_Yvalue=((unsigned char)Yval);
    GetMyClass(data)->m_Uvalue=((unsigned char)Uval);
    GetMyClass(data)->m_Vvalue=((unsigned char)Vval);
  }
}
