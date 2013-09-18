/*
 *  pix_deinterlace.cpp
 *  GEM_darwin
 *
 *  Created by lincoln on 11/18/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#include "pix_deinterlace.h"

CPPEXTERN_NEW(pix_deinterlace);

/////////////////////////////////////////////////////////
//
// pix_2grey
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_deinterlace :: pix_deinterlace() : 
  m_mode(1),
  m_adaptive(10)
{
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
pix_deinterlace :: ~pix_deinterlace()
{ }

void pix_deinterlace :: processRGBAImage(imageStruct &image)
{
int	row, col,field1,field2,field3;
	int temp1, temp2,temp3;
	
	unsigned char *pixels=image.data;

	field1 = 0;
	field2 = image.xsize*4;
	field3 = image.xsize*8;

	if ((m_savedImage.xsize != image.xsize) || (m_savedImage.ysize != image.ysize)){
		m_savedImage.xsize=image.xsize;
		m_savedImage.ysize=image.ysize;
		m_savedImage.setCsizeByFormat(image.format);
		m_savedImage.reallocate();
	}
//	if(saved!=m_savedImage.data)m_savedImage.setBlack();saved=m_savedImage.data;
		
	if (m_mode){
	
	for (row = 0; row < (image.ysize/2)-1; row++){
		
		for (col = 0; col < image.xsize; col++){
			
				
				pixels[field2+chRed] = (pixels[field1+chRed] + pixels[field3+chRed]) / 2;
				
				pixels[field2+chGreen] = (pixels[field1+chGreen] + pixels[field3+chGreen]) / 2;
				
				pixels[field2+chBlue] = (pixels[field1+chBlue] + pixels[field3+chBlue]) / 2;
				
				field1+=4;
				field2+=4;
				field3+=4;
		}
		field1+=image.xsize*4;
		field2+=image.xsize*4;

		field3+=image.xsize*4;
		
	}
	}else{
	for (row = 0; row < (image.ysize/2)-1; row++){
		
		for (col = 0; col < image.xsize; col++){
			
				//temp1 = abs((int)m_savedImage.data[field2 + chRed] - (int)pixels[field2 + chRed]);
				//temp2 = abs((int)m_savedImage.data[field2 + chGreen] - (int)pixels[field2 + chGreen]);
				//temp3 = abs((int)m_savedImage.data[field2 + chBlue] - (int)pixels[field2 + chBlue]);
				temp1 = abs(pixels[field1 + chRed] - pixels[field2 + chRed]);
				temp2 = abs(pixels[field1 + chGreen] - pixels[field2 + chGreen]);
				temp3 = abs(pixels[field1 + chRed] - pixels[field2 + chRed]);
				
				if ((temp1 > m_adaptive) && (temp2 > m_adaptive) && (temp3 > m_adaptive)) {
				
					
			//		diff1 = abs(m_savedImage.data[field1 + chRed] - pixels[field1 + chRed]);
			//		diff2 = abs(m_savedImage.data[field1 + chGreen] - pixels[field1 + chGreen]);
			//		diff3 = abs(m_savedImage.data[field1 + chBlue] - pixels[field1 + chBlue]);
//
			//		if ((diff1 > m_adaptive) && (diff2 > m_adaptive) && (diff3 > m_adaptive)) {
					//	pixels[field2 + chRed] = 255;
					//	pixels[field2 + chGreen] = 255;
					//	pixels[field2 + chBlue] = 255;
						pixels[field2 + chRed] = (	(pixels[field1 + chRed] * 85 + pixels[field3 + chRed] * 85) +
													(pixels[field1 - 4 + chRed] * 85 + pixels[field3 -4 + chRed] * 85)+
													(pixels[field1 + 4 + chRed] * 85 + pixels[field3 + 4 + chRed] * 85)
													) / 512;

						pixels[field2 + chGreen] = ((pixels[field1 + chGreen] * 85 + pixels[field3 + chGreen] * 85) +
													(pixels[field1 - 4 + chGreen] * 85 + pixels[field3 -4 + chGreen] * 85) +
													(pixels[field1 + 4 + chGreen] * 85 + pixels[field3 + 4 + chGreen] * 85)
													) / 512;

						pixels[field2 + chBlue] = (	(pixels[field1 + chBlue] * 85 + pixels[field3 + chBlue] * 85) +
													(pixels[field1 - 4 + chBlue] * 85 + pixels[field3 -4 + chBlue] * 85) +
													(pixels[field1 + 4 + chBlue] * 85 + pixels[field3 + 4 + chBlue] * 85)
													) / 512;
						//pixels[field2 + chGreen] = (pixels[field1 + chGreen] + pixels[field3 + chGreen]) / 2;
						//pixels[field2 + chBlue] = (pixels[field1 + chBlue] + pixels[field3 + chBlue]) / 2;
				
				//	}
				}
				m_savedImage.data[field1 + chRed] = pixels[field1 + chRed];
				m_savedImage.data[field1 + chGreen] = pixels[field1 + chGreen];
				m_savedImage.data[field1 + chBlue] = pixels[field1 + chBlue];
				field1+=4;
				field2+=4; 
				field3+=4;
				
				
		}
		field1+=image.xsize*4;
		field2+=image.xsize*4;
		field3+=image.xsize*4;
	
	}	
	}

}

void pix_deinterlace :: processYUVImage(imageStruct &image)
{

int	row, col,field1,field2,field3;
	int temp1;
	
	unsigned char *pixels=image.data;
	
	field1 = 0;
	field2 = image.xsize*2;
	field3 = image.xsize*4;
	
	if (m_mode){
	
	for (row = 0; row < (image.ysize/2)-1; row++){
		
		for (col = 0; col < image.xsize; col++){
			
				
				pixels[field2] = (pixels[field1] + pixels[field3]) / 2;
				field1++;
				field2++; 
				field3++;
								pixels[field2] = (pixels[field1] + pixels[field3]) / 2;
				field1++;
				field2++;
				field3++;
		}
		field1+=image.xsize;
		field2+=image.xsize;
		field1+=image.xsize;
		field2+=image.xsize;
		field3+=image.xsize;
		field3+=image.xsize;
	}
	}else{
	for (row = 0; row < (image.ysize/2)-1; row++){
		
		for (col = 0; col < image.xsize; col++){
			
				
				temp1 = abs(pixels[field1] - pixels[field2]);
				
				if (temp1 > 10) pixels[field2] = (pixels[field1] + pixels[field3]) / 2;
				
				field1++;
				field2++; 
				field3++;
				//pixels[field2] = pixels[field1];
				//pixels[field2] = (pixels[field1] + pixels[field2]) /2;
				//pixels[field2] = (pixels[field1] + pixels[field2] + pixels[field3]) / 3;
				/*
				temp1 = (pixels[field1] + pixels[field2]) / 2;
				temp2 = (pixels[field2] + pixels[field3]) / 2;
				pixels[field2] = (temp1 + temp2) / 2;*/
				
				temp1 = abs(pixels[field1] - pixels[field2]);
				
				if (temp1 > 10) pixels[field2] = (pixels[field1] + pixels[field3]) / 2;
				
				field1++;
				field2++;
				field3++;
		}
		field1+=image.xsize;
		field2+=image.xsize;
		field1+=image.xsize;
		field2+=image.xsize;
		field3+=image.xsize;
		field3+=image.xsize;
	}	
	}


}
void pix_deinterlace :: processGrayImage(imageStruct &image)
{

  int	row, col,field1,field2,field3;
  int temp1;
  unsigned char *pixels=image.data;
  field1 = 0;
  field2 = image.xsize;
  field3 = image.xsize*2;
  if (m_mode){
    for (row = 0; row < image.ysize-1; row++){
      for (col = 0; col < image.xsize; col++){
        pixels[field2] = (pixels[field1] + pixels[field3]) / 2;
        field1++; field2++; field3++;
        pixels[field2] = (pixels[field1] + pixels[field3]) / 2;
        field1++; field2++; field3++;
      }
      field1+=image.xsize; field1+=image.xsize;
      field2+=image.xsize; field2+=image.xsize; 
      field3+=image.xsize; field3+=image.xsize;
    }
  }else{
    for (row = 0; row < image.ysize-1; row++){
      for (col = 0; col < image.xsize; col++){
        temp1 = abs(pixels[field1] - pixels[field2]);
        if (temp1 > 10) pixels[field2] = (pixels[field1] + pixels[field3]) / 2;
        field1++; field2++; field3++;
        temp1 = abs(pixels[field1] - pixels[field2]);
        if (temp1 > 10) pixels[field2] = (pixels[field1] + pixels[field3]) / 2;
        field1++; field2++; field3++;
      }
      field1+=image.xsize; field1+=image.xsize;
      field2+=image.xsize; field2+=image.xsize;
      field3+=image.xsize; field3+=image.xsize;
    }	
  }
}
#ifdef __VEC__
void pix_deinterlace :: processYUVAltivec(imageStruct &image)
{

	union{
		unsigned char			c[16];
		vector unsigned char	v;
	}charBuffer;

	int	row, col,field1,field2,field3, stride;
	vector unsigned char temp1, temp2,temp3, thresh;
	vector bool char mask, mask2;
	
	vector unsigned char *pixels=(vector unsigned char *)image.data;
	
	//stride = image.xsize/4; //this is to test a crash
	stride = image.xsize/8; //eight pixels per vector
	field1 = 0;
	field2 = stride;
	field3 = stride*2;
	
	temp1 = vec_splat_u8(0);
	
	charBuffer.c[0] = 10;
	thresh = charBuffer.v;
	thresh = vec_splat(thresh,0);
	
	if (m_mode){
	
	for (row = 0; row < (image.ysize/2)-1; row++){
		
		for (col = 0; col < stride; col++){
			
				pixels[field2] = vec_avg(pixels[field1],pixels[field3]);
				
				//pixels[field2] = temp1;
				//pixels[field2] = pixels[field1];
				
				field1++;
				field2++; 
				field3++;
			
			}
			
		field1+=stride;
		field2+=stride;
		field3+=stride;
		}
	}else{
		
		for (row = 0; row < (image.ysize/2)-1; row++){
			
			for (col = 0; col < stride; col++){
				
				//vec_sub should give modulo difference of field1 and field2
				temp2 = vec_subs(pixels[field1],pixels[field2]);
				
				temp3 = vec_subs(pixels[field2],pixels[field1]);
				
				//compare diff to min to get mask
				
				mask = vec_cmpgt(temp2, thresh);
				mask2 = vec_cmpgt(temp3, thresh);
				
				mask = vec_or(mask,mask2);
				
				//vec_avg field1 and field3 
				
				temp3 = vec_avg(pixels[field1],pixels[field3]);
				
				//vec_sel avg and field2 using the mask 
				
				pixels[field2] = vec_sel(pixels[field2],temp3,(vector unsigned char)mask);
					
				field1++;
				field2++; 
				field3++; 
					
					
			}
		field1+=stride;
		field2+=stride;
		field3+=stride;
		}
	}
	
	
}
#endif //__VEC__
/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_deinterlace :: obj_setupCallback(t_class *classPtr)
{ 
	class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_deinterlace::modeMessCallback), gensym("mode"),A_FLOAT,A_NULL);
	class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_deinterlace::adaptiveMessCallback), gensym("adaptive"),A_FLOAT,A_NULL);
}

void pix_deinterlace :: modeMessCallback(void *data,t_floatarg mode)
{
	
	GetMyClass(data)->m_mode = (int)mode;
	
}
void pix_deinterlace :: adaptiveMessCallback(void *data,t_floatarg mode)
{
	
	GetMyClass(data)->m_adaptive = (int)mode;
	
}
