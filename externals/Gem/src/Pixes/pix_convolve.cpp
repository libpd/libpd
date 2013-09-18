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

#include "pix_convolve.h"

CPPEXTERN_NEW_WITH_TWO_ARGS(pix_convolve, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// pix_convolve
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_convolve :: pix_convolve(t_floatarg fRow, t_floatarg fCol)
  : m_imatrix(NULL)
{
  int row = static_cast<int>(fRow);
  int col = static_cast<int>(fCol);

    if (!row || !col )
    {
    	error("matrix must have some dimension");
    	return;
    }
    
    if (!(row % 2) || !(col % 2) )
    {
    	error("matrix must have odd dimensions");
    	return;
    }
    
    m_rows = row;
    m_cols = col;
    m_irange = 255;
    m_imatrix = new signed short[m_rows * m_cols];

    // zero out the matrix
    int i;
    for (i = 0; i < m_cols * m_rows; i++) m_imatrix[i] = 0;
    // insert a one for the default center value (identity matrix)
    m_imatrix[ ((m_cols / 2 + 1) * m_rows) + (m_rows / 2 + 1) ] = 255;
    
    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("ft1"));
    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("matrix"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_convolve :: ~pix_convolve()
{
    if (m_imatrix)delete [] m_imatrix;
}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_convolve :: calculateRGBA3x3(imageStruct &image,imageStruct &tempImg)
{
  int i;
  int j;
//  int k;
  int xsize =  tempImg.xsize;
  int ysize =  tempImg.ysize;
  int size = xsize*ysize - xsize-1;
  int csize = tempImg.csize;

  int* src = (int*) tempImg.data;
  int* dest = (int*)image.data;
  
 
//unroll this to do R G B in one pass?? (too many registers?)
  i = xsize;
  int* val1 = 0;
  int* val2 = src+i-xsize;
  int* val3 = src+i-xsize+1;
  int* val4 = src+i-1;
  int* val5 = src+i;
  int* val6 = src+i+1;
  int* val7 = src+i+xsize-1;
  int* val8 = src+i+xsize;
  int* val9 = src+i+xsize+1;
  int res;
  for (i=xsize+1;i<size;i++) {
    val1 = val2;
    val2 = val3;
    val3 = src+i-xsize+1;
    val4 = val5;
    val5 = val6;
    val6 = src+i+1;
    val7 = val8;
    val8 = val9;
    val9 = src+i+xsize+1;
    if (i%xsize == 0 || i%xsize == xsize-1) continue;
    #ifndef __APPLE__
    for (j=0;j<3;j++) 
    #else
    for (j=1;j<4;j++)
    #endif
    {
      //      res =  m_imatrix[0]*(int)((unsigned char*)val1)[j];
      res =  m_imatrix[0]*static_cast<int>(reinterpret_cast<unsigned char*>(val1)[j]);
      res += m_imatrix[1]*static_cast<int>(reinterpret_cast<unsigned char*>(val2)[j]);
      res += m_imatrix[2]*static_cast<int>(reinterpret_cast<unsigned char*>(val3)[j]);
      res += m_imatrix[3]*static_cast<int>(reinterpret_cast<unsigned char*>(val4)[j]);
      res += m_imatrix[4]*static_cast<int>(reinterpret_cast<unsigned char*>(val5)[j]);
      res += m_imatrix[5]*static_cast<int>(reinterpret_cast<unsigned char*>(val6)[j]);
      res += m_imatrix[6]*static_cast<int>(reinterpret_cast<unsigned char*>(val7)[j]);
      res += m_imatrix[7]*static_cast<int>(reinterpret_cast<unsigned char*>(val8)[j]);
      res += m_imatrix[8]*static_cast<int>(reinterpret_cast<unsigned char*>(val9)[j]);
      res*=m_irange;
      res>>=16;
      ((unsigned char*)dest)[i*csize+j] = CLAMP(res);
    }

  }

  
}

void pix_convolve :: processRGBAImage(imageStruct &image)
{
    image.copy2Image(&tempImg);
    int initX = m_rows / 2;
    int initY = m_cols / 2;
    int maxX = tempImg.xsize - initX;
    int maxY = tempImg.ysize - initY;
    int xTimesc = tempImg.xsize * tempImg.csize;
    int initOffset = initY * xTimesc + initX * tempImg.csize;
    const int csize = tempImg.csize;

    if (m_rows == 3 && m_cols == 3) {
      calculateRGBA3x3(image,tempImg);
      return;
    }

    for (int y = initY; y < maxY; y++)
    {
        int realY = y * xTimesc;
        int offsetY = realY - initOffset;

    	for (int x = initX; x < maxX; x++)
    	{
    	    int realPos = x * csize + realY;
            int offsetXY = x * csize + offsetY;

    	    // skip the alpha value
	    for (int c = 1; c < csize; c++)
    	    {
    		    int new_val = 0;
		    int offsetXYC = offsetXY + c;
    		    for (int matY = 0; matY < m_cols; matY++)
    		    {
    		        int offsetXYCMat = matY * xTimesc + offsetXYC;
    		        int realMatY = matY * m_rows;
    	    	    for (int matX = 0; matX < m_rows; matX++)
    	    	    {
                        new_val += (tempImg.data[offsetXYCMat + matX * csize] *
                                        m_imatrix[realMatY + matX])>>8;
    	    	    }
    		    }
                    image.data[realPos + c] = CLAMP(new_val);  
		    //removes insult from injury ??
		    // we do not use the m_irange anymore ...  remove it ??

    	    }
    	}
    }

}

void pix_convolve :: processGrayImage(imageStruct &image)
{
  const int csize=image.csize;
    image.copy2Image(&tempImg);
    int initX = m_rows / 2;
    int initY = m_cols / 2;
    int maxX = tempImg.xsize - initX;
    int maxY = tempImg.ysize - initY;
    int xTimesc = tempImg.xsize * csize;
    int initOffset = initY * xTimesc + initX * csize;

    for (int y = initY; y < maxY; y++)    {
      int realY = y * xTimesc;
      int offsetY = realY - initOffset;

      for (int x = initX; x < maxX; x++)    	{
	int offsetXY = x + offsetY;

	int new_val = 0;
	int offsetXYC = offsetXY;
	for (int matY = 0; matY < m_cols; matY++)   {
	  int offsetXYCMat = matY * xTimesc + offsetXYC;
	  int realMatY = matY * m_rows;
	  for (int matX = 0; matX < m_rows; matX++)     {
	    new_val += (tempImg.data[offsetXYCMat + matX] *
			m_imatrix[realMatY + matX])>>8;
	  }
	}
	image.data[x+realY] = CLAMP(new_val);  
	//removes insult from injury ??
	// we do not use the m_irange anymore ...  remove it ??
      }
    }
}

void pix_convolve :: processYUVImage(imageStruct &image)
{
  image.copy2Image(&tempImg);
  //float range = 1;
    int initX = m_rows / 2;
    int initY = m_cols / 2;
    int maxX = tempImg.xsize - initX;
    int maxY = tempImg.ysize - initY;
    int xTimesc = tempImg.xsize * tempImg.csize;
    int initOffset = initY * xTimesc + initX * tempImg.csize;
    
 //   calculate3x3YUV(image,tempImg);
 
//quick fix for Intel 3x3YUV problems
#ifdef __BIG_ENDIAN__
    if (m_rows == 3 && m_cols == 3) {
      calculate3x3YUV(image,tempImg);
      return;
    }
#endif
    if (m_chroma) {
    for (int y = initY; y < maxY; y++)   {
        int realY = y * xTimesc;
        int offsetY = realY - initOffset;

    	for (int x = initX; x < maxX; x++)
    	{
    	    int realPos = x * tempImg.csize + realY;
            int offsetXY = x * tempImg.csize + offsetY;

    	    // skip the UV
    	    for (int c = 1; c < 3; c+=2)
    	    {
    		    int new_val = 0;
                int offsetXYC = offsetXY + c;
    		    for (int matY = 0; matY < m_cols; matY++)
    		    {
    		        int offsetXYCMat = matY * xTimesc + offsetXYC;
    		        int realMatY = matY * m_rows;
    	    	    for (int matX = 0; matX < m_rows; matX++)
    	    	    {
                      new_val += (tempImg.data[offsetXYCMat + matX * tempImg.csize] *
                                        m_imatrix[realMatY + matX])>>8;
    	    	    }
    		    }
                   image.data[realPos + c] = CLAMP(new_val);
                   // image.data[realPos + c-1] = 128;  //remove the U+V
    	    }
    	}
    }
    }else{
    for (int y = initY; y < maxY; y++)
    {
        int realY = y * xTimesc;
        int offsetY = realY - initOffset;

    	for (int x = initX; x < maxX; x++)
    	{
    	    int realPos = x * tempImg.csize + realY;
            int offsetXY = x * tempImg.csize + offsetY;

    	    // skip the UV
    	    for (int c = 1; c < 3; c+=2)
    	    {
    		    int new_val = 0;
                int offsetXYC = offsetXY + c;
    		    for (int matY = 0; matY < m_cols; matY++)
    		    {
    		        int offsetXYCMat = matY * xTimesc + offsetXYC;
    		        int realMatY = matY * m_rows;
    	    	    for (int matX = 0; matX < m_rows; matX++)
    	    	    {
                      new_val += (tempImg.data[offsetXYCMat + matX * tempImg.csize] *
                                        m_imatrix[realMatY + matX])>>8;
    	    	    }
    		    }
                   image.data[realPos + c] = CLAMP(new_val);
                    image.data[realPos + c-1] = 128;  //remove the U+V
    	    }
    	}
    }
    }
   
}

//make two functions - one for chroma one without
void pix_convolve :: calculate3x3YUV(imageStruct &image,imageStruct &tempImg)
{

#ifdef __VEC__
calculate3x3YUVAltivec(image,tempImg);
return;
#else

  int i;
  int j;
  int k;
  int xsize =  tempImg.xsize -1;
  int ysize =  tempImg.ysize -1;
  int size = xsize*ysize - xsize-1;
  int length;

  short* src = (short*) tempImg.data;
  short* dest = (short*)image.data;
  register int mat1,mat2,mat3,mat4,mat5,mat6,mat7,mat8,mat9;
  register int res1,res2,res3,res4,res5,res6,res7,res8,res9;
  register int range;
  
  mat1 = m_imatrix[0];
  mat2 = m_imatrix[1];
  mat3 = m_imatrix[2];
  mat4 = m_imatrix[3];
  mat5 = m_imatrix[4];
  mat6 = m_imatrix[5];
  mat7 = m_imatrix[6];
  mat8 = m_imatrix[7];
  mat9 = m_imatrix[8]; 
  range =m_irange;
 
if (m_chroma){
  i = xsize;
 
#ifdef i386
  register unsigned char val1 = 0;  
  register unsigned char val2 = src[i-xsize+1]; 
  register unsigned char val3 = src[i-xsize+3];
  register unsigned char val4 = src[i-1];
  register unsigned char val5 = src[i+1];
  register unsigned char val6 = src[i+3];
  register unsigned char val7 = src[i+xsize-1];
  register unsigned char val8 = src[i+xsize+1];
  register unsigned char val9 = src[i+xsize+3];
#else 
  register unsigned char val1 = 0;  
  register unsigned char val2 = src[i-xsize+1]; 
  register unsigned char val3 = src[i-xsize+3];
  register unsigned char val4 = src[i-1];
  register unsigned char val5 = src[i+1];
  register unsigned char val6 = src[i+3];
  register unsigned char val7 = src[i+xsize-1];
  register unsigned char val8 = src[i+xsize+1];
  register unsigned char val9 = src[i+xsize+3];
#endif  

  //unroll this 2x to fill the registers? (matrix*y1*y2= 9*9*9 =27)
  //messed up looking on x86
i=xsize+2;

length = size /2;

	  for (k=1;k<ysize;k++) {
        for (j=1;j<xsize;j++) {
  //load furthest value first...the rest should be in cache
    
            val7 = val8;
            val8 = val9;
            val9 = src[i+xsize+3]; //this will come from main mem
            val1 = val2;
            val2 = val3;
            val3 = src[i-xsize+3]; //should be in cache from previous pass
            val4 = val5;
            val5 = val6;
            val6 = src[i+3];
    
            //unroll??
            res1 = mat1*static_cast<int>(val1);
            res2 = mat2*static_cast<int>(val2);
            res3 = mat3*static_cast<int>(val3);
            res4 = mat4*static_cast<int>(val4);
            res5 = mat5*static_cast<int>(val5);
            res6 = mat6*static_cast<int>(val6);
            res7 = mat7*static_cast<int>(val7);
            res8 = mat8*static_cast<int>(val8);
            res9 = mat9*static_cast<int>(val9);
            
            
            res1 += res2 + res3;
            res4 += res5 + res6;
            res7 += res8 + res9;
            res1 += res4 + res7;
        
            res1*=range;
            res1>>=16;
            ((unsigned char*)dest)[i*2+1] = CLAMP(res1);
            i++;
    
        }
    i=k*tempImg.xsize;
  } 
  }else{
   
  i = xsize;
  //make these temp register vars rather than pointers?
  
  short* val1 = 0;  
  short* val2 = src+i-xsize; //val2 = src[i-xsize];
  short* val3 = src+i-xsize+1; //val3 = src[i-xsize+1];
  short* val4 = src+i-1; //val4 = src[i-1];
  short* val5 = src+i; //val5 = src[i];
  short* val6 = src+i+1; //val6 = src[i+1];
  short* val7 = src+i+xsize-1; //val7 = src[i+xsize-1];
  short* val8 = src+i+xsize; //val8 = src[i+xsize];
  short* val9 = src+i+xsize+1; //val9 = src[i+xsize+1];
  /*
  register short* val1 = 0;  
  register short* val2 = src+i-xsize; //val2 = src[i-xsize];
  register short* val3 = src+i-xsize+1; //val3 = src[i-xsize+1];
  register short* val4 = src+i-1; //val4 = src[i-1];
  register short* val5 = src+i; //val5 = src[i];
  register short* val6 = src+i+1; //val6 = src[i+1];
  register short* val7 = src+i+xsize-1; //val7 = src[i+xsize-1];
  register short* val8 = src+i+xsize; //val8 = src[i+xsize];
  register short* val9 = src+i+xsize+1; //val9 = src[i+xsize+1];*/
  //int res; 
 // for (i=xsize+1;i<size;i++) {
   for (k=1;k<ysize;k++) {
        for (j=1;j<xsize;j++) {
    val1 = val2;
    val2 = val3;
    val3 = src+i-xsize+1;
    val4 = val5;
    val5 = val6;
    val6 = src+i+1;
    val7 = val8;
    val8 = val9;
    val9 = src+i+xsize+1; 
    
   /* if (i%xsize == 0 || i%xsize == xsize-1) continue;
    #ifndef __APPLE__
    for (j=0;j<3;j++) 
    #else
    for (j=1;j<3;j+=2)
    #endif
    { */
    
      res1 = mat1*static_cast<int>(reinterpret_cast<unsigned char*>(val1)[j]);
      res2 = mat2*static_cast<int>(reinterpret_cast<unsigned char*>(val2)[j]);
      res3 = mat3*static_cast<int>(reinterpret_cast<unsigned char*>(val3)[j]);
      res4 = mat4*static_cast<int>(reinterpret_cast<unsigned char*>(val4)[j]);
      res5 = mat5*static_cast<int>(reinterpret_cast<unsigned char*>(val5)[j]);
      res6 = mat6*static_cast<int>(reinterpret_cast<unsigned char*>(val6)[j]);
      res7 = mat7*static_cast<int>(reinterpret_cast<unsigned char*>(val7)[j]);
      res8 = mat8*static_cast<int>(reinterpret_cast<unsigned char*>(val8)[j]);
      res9 = mat9*static_cast<int>(reinterpret_cast<unsigned char*>(val9)[j]);
      res1 += res2 + res3;
      res4 += res5 + res6;
      res7 += res8 + res9;
      res1 += res4 + res7;
      res1*=range;
      res1>>=16;
     // ((unsigned char*)dest)[i*2] = 128;
     // ((unsigned char*)dest)[i*2+2] = 128;
      ((unsigned char*)dest)[i*2+1] = CLAMP(res1);
   // }
     ((unsigned char*)dest)[i*2] = 128;
     // ((unsigned char*)dest)[i*2+2] = 128;
      i++;
      }
    i=k*tempImg.xsize;
  }
  }
#endif
}

//too many temps for all the registers - reuse some
void pix_convolve :: calculate3x3YUVAltivec(imageStruct &image,imageStruct &tempImg)
{
 #ifdef __VEC__
 int h,w,width,i;
 int xsize =  (tempImg.xsize)*2;
 
   width = (tempImg.xsize)/8;
   //format is U Y V Y
  
    union
    {
        short	elements[8];
        vector	signed short v;
    }shortBuffer;
    
    union
    {
        unsigned int	elements[4];
        vector	unsigned int v;
    }intBuffer;
    
    vector unsigned char one;
    vector signed short mat1,mat2,mat3,mat4,mat5,mat6,mat7,mat8,mat9; 
    vector unsigned char  val1,val2,val3,val4,val5,val6,val7,val8,val9;
    register vector signed int  res1,res2,res3,res4,res5,res6,res7,res8,res9;
    vector signed int  yhi,ylo;
    register vector signed int  res1a,res2a,res3a,res4a,res5a,res6a,res7a,res8a,res9a;
    vector unsigned int bitshift;
    register vector signed short y1,y2,y3,y4,y5,y6,y7,y8,y9,yres,uvres,hiImage,loImage;
    vector signed short range,uvnone,uv128;
    unsigned char *dst =  (unsigned char*) image.data;
    unsigned char *src =  (unsigned char*) tempImg.data;
   

    one =  vec_splat_u8( 1 );
    
    intBuffer.elements[0] = 8;
    //Load it into the vector unit
    bitshift = intBuffer.v;
    bitshift = (vector unsigned int)vec_splat((vector unsigned int)bitshift,0);
      
     shortBuffer.elements[0] = m_irange;
    range = shortBuffer.v;
    range = (vector signed short)vec_splat((vector signed short)range, 0); 
    
     shortBuffer.elements[0] = 128;
    uvnone = shortBuffer.v;
    uvnone = (vector signed short)vec_splat((vector signed short)uvnone, 0); 
      
    //load the matrix values into vectors 
    shortBuffer.elements[0] = m_imatrix[0];
    mat1 = shortBuffer.v;
    mat1 = (vector signed short)vec_splat((vector signed short)mat1,0);
    
    shortBuffer.elements[0] = m_imatrix[1];
    mat2 = shortBuffer.v;
    mat2 = (vector signed short)vec_splat((vector signed short)mat2,0);
    
    shortBuffer.elements[0] = m_imatrix[2];
    mat3 = shortBuffer.v;
    mat3 = (vector signed short)vec_splat((vector signed short)mat3,0);
    
    shortBuffer.elements[0] = m_imatrix[3];
    mat4 = shortBuffer.v;
    mat4 = (vector signed short)vec_splat((vector signed short)mat4,0);
    
    shortBuffer.elements[0] = m_imatrix[4];
    mat5 = shortBuffer.v;
    mat5 = (vector signed short)vec_splat((vector signed short)mat5,0);
    
    shortBuffer.elements[0] = m_imatrix[5];
    mat6 = shortBuffer.v;
    mat6 = (vector signed short)vec_splat((vector signed short)mat6,0);
    
    shortBuffer.elements[0] = m_imatrix[6];
    mat7 = shortBuffer.v;
    mat7 = (vector signed short)vec_splat((vector signed short)mat7,0);
    
    shortBuffer.elements[0] = m_imatrix[7];
    mat8 = shortBuffer.v;
    mat8 = (vector signed short)vec_splat((vector signed short)mat8,0);
    
    shortBuffer.elements[0] = m_imatrix[8];
    mat9 = shortBuffer.v;
    mat9 = (vector signed short)vec_splat((vector signed short)mat9,0);
    
    shortBuffer.elements[0] = 128;
    uv128 = shortBuffer.v;
    uv128 = (vector signed short)vec_splat((vector signed short)uv128,0);
    #ifndef PPC970 
    UInt32			prefetchSize = GetPrefetchConstant( 16, 1, 256 );
    vec_dst( src, prefetchSize, 0 );
    vec_dst( dst, prefetchSize, 0 );
      #endif   
 
    i = xsize*2;

//need to treat the first rows as a special case for accuracy and keep it from crashing
//or just skip the first 2 rows ;)
 
    for ( h=2; h<image.ysize-1; h++){
   // i+=2; //this gets rid of the echoes but kills the vertical edge-detects???
   i+=8;
        for (w=0; w<width-1; w++)
        {
        #ifndef PPC970
            vec_dst( src, prefetchSize, 0 );
            vec_dst( dst, prefetchSize, 1 );    
     #endif
            
            val1 = vec_ld(0,src+(i-xsize-2));//this might crash?
            val2 = vec_ld(0,src+(i-xsize)); 
            val3 = vec_ld(0,src+(i-xsize+2)); 
            val4 = vec_ld(0,src+(i-2)); 
            val5 = vec_ld(0,src+i);
            val6 = vec_ld(0,src+(i+2)); 
            val7 = vec_ld(0,src+(i+xsize-2)); 
            val8 = vec_ld(0,src+(i+xsize)); 
            val9 = vec_ld(0,src+(i+xsize+2));
            
            //extract the Y for processing
            y1 = (vector signed short)vec_mulo((vector unsigned char)one,(vector unsigned char)val1);
            y2 = (vector signed short)vec_mulo((vector unsigned char)one,(vector unsigned char)val2);
            y3 = (vector signed short)vec_mulo((vector unsigned char)one,(vector unsigned char)val3);
            y4 = (vector signed short)vec_mulo((vector unsigned char)one,(vector unsigned char)val4);
            y5 = (vector signed short)vec_mulo((vector unsigned char)one,(vector unsigned char)val5);
            y6 = (vector signed short)vec_mulo((vector unsigned char)one,(vector unsigned char)val6);
            y7 = (vector signed short)vec_mulo((vector unsigned char)one,(vector unsigned char)val7);
            y8 = (vector signed short)vec_mulo((vector unsigned char)one,(vector unsigned char)val8);
            y9 = (vector signed short)vec_mulo((vector unsigned char)one,(vector unsigned char)val9);
            
            uvres = (vector signed short)vec_mule((vector unsigned char)one,(vector unsigned char)val5);
            
            //mult the Y by the matrix coefficient
            res1 = vec_mulo(mat1,y1);
            res2 = vec_mulo(mat2,y2);
            res3 = vec_mulo(mat3,y3);
            res4 = vec_mulo(mat4,y4);
            res5 = vec_mulo(mat5,y5);
            res6 = vec_mulo(mat6,y6);
            res7 = vec_mulo(mat7,y7);
            res8 = vec_mulo(mat8,y8);
            res9 = vec_mulo(mat9,y9);
            
            res1a = vec_mule(mat1,y1);
            res2a = vec_mule(mat2,y2);
            res3a = vec_mule(mat3,y3);
            res4a = vec_mule(mat4,y4);
            res5a = vec_mule(mat5,y5);
            res6a = vec_mule(mat6,y6);
            res7a = vec_mule(mat7,y7);
            res8a = vec_mule(mat8,y8);
            res9a = vec_mule(mat9,y9);
            
            //sum the results - these are only 1 cycle ops so no dependency issues
            res1 = vec_adds(res1,res2); //1+2
            res3 = vec_adds(res3,res4);//3+4
            res5 = vec_adds(res5,res6);//5+6
            res7 = vec_adds(res7,res8);//7+8
            res1 = vec_adds(res1,res3);//(1+2)+(3+4)
            res7 = vec_adds(res7,res9);//7+8+9
            res1 = vec_adds(res1,res5);//(1+2)+(3+4)+(5+6)
            res1 = vec_adds(res1,res7);//(1+2)+(3+4)+(5+6)+(7+8+9)
            
            res1a = vec_adds(res1a,res2a); //1+2
            res3a = vec_adds(res3a,res4a);//3+4
            res5a = vec_adds(res5a,res6a);//5+6
            res7a = vec_adds(res7a,res8a);//7+8
            res1a = vec_adds(res1a,res3a);//(1+2)+(3+4)
            res7a = vec_adds(res7a,res9a);//7+8+9
            res1a = vec_adds(res1a,res5a);//(1+2)+(3+4)+(5+6)
            res1a = vec_adds(res1a,res7a);//(1+2)+(3+4)+(5+6)+(7+8+9)
            
            
            //do the bitshift on the results here??
            res1 = vec_sra(res1,bitshift);
            res1a = vec_sra(res1a,bitshift); 
                        
            //pack back to one short vector??
            yhi = vec_mergeh(res1a,res1);
            ylo = vec_mergel(res1a,res1);
           

            yres = vec_packs(yhi,ylo);
            
            
            //combine with the UV
            //vec_mergel + vec_mergeh Y and UV
            hiImage =  vec_mergeh(uvres,yres);
            loImage =  vec_mergel(uvres,yres);
            
          val1 = vec_packsu(hiImage,loImage);
          vec_st(val1,0,dst+i);
           i+=16;
           
        }
        i = h * xsize;
        #ifndef PPC970
        vec_dss( 0 );
        vec_dss( 1 );
    #endif
}  /*end of working altivec function */


#endif
}

/////////////////////////////////////////////////////////
// rangeMess
//
/////////////////////////////////////////////////////////
void pix_convolve :: rangeMess(float range)
{
    m_irange = (int)(range*255.f);
    setPixModified();
}

/////////////////////////////////////////////////////////
// matrixMess
//
/////////////////////////////////////////////////////////
void pix_convolve :: matrixMess(int argc, t_atom *argv)
{
    if (argc != m_cols * m_rows)
    {
    	error("matrix size not correct");
    	return;
    }

    int i;
    for (i = 0; i < argc; i++) m_imatrix[i] = (int)(atom_getfloat(&argv[i])*255.);


    setPixModified();
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_convolve :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_convolve::matrixMessCallback),
    	    gensym("matrix"), A_GIMME, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_convolve::rangeMessCallback),
    	    gensym("ft1"), A_FLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_convolve::chromaMessCallback),
    	    gensym("chroma"), A_FLOAT, A_NULL);
}
void pix_convolve :: matrixMessCallback(void *data, t_symbol *, int argc, t_atom *argv)
{
    GetMyClass(data)->matrixMess(argc, argv);
}
void pix_convolve :: rangeMessCallback(void *data, t_floatarg range)
{
    GetMyClass(data)->rangeMess(range);
}

void pix_convolve :: chromaMessCallback(void *data, t_floatarg value)
{
    GetMyClass(data)->m_chroma=static_cast<int>(value);
}
