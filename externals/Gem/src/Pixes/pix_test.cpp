////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-2000 Mark Danks.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "pix_test.h"

CPPEXTERN_NEW(pix_test);

  /////////////////////////////////////////////////////////
//
// pix_test
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_test :: pix_test()
{
  myImage.xsize=myImage.ysize=myImage.csize=1;
  myImage.data = new unsigned char[1];
  csize=2;
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_test :: ~pix_test()
{ }

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_test :: processImage(imageStruct &image)
{
  image.xsize=myImage.xsize=64;
  image.ysize=myImage.ysize=64;
  image.csize=myImage.csize=csize;
  image.format=myImage.format=GL_YUV422_GEM;
  myImage.reallocate();
  int rows=image.xsize;
  int cols=image.xsize;
  unsigned char* data=myImage.data;
  switch (myImage.format){
  case GL_RGBA:
    while(rows--){
      int col=cols/2;
      while(col--){
	data[0]=data[2]=255;
	data[1]=0;  data[3]=255;
	data+=4;
	data[0]=data[1]=data[2]=0;data[3]=255;
	data+=4;
      }
    }
    break;
  case GL_YUV422_GEM:
    post("hallo yuv");
    //    rows/=2;
    int datasize=image.xsize*image.xsize*image.csize;
    while(datasize--)*data++=off;
    break;
    while(rows--){
      int col=cols;
      while(col--){
	*data++=off;
	*data++=128;	
      }
    }
    break;
  }
  image.data=myImage.data;
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_test :: obj_setupCallback(t_class *classPtr)
{
   class_addfloat(classPtr, reinterpret_cast<t_method>(&pix_test::floatMessCallback));    
   class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_test::csizeMessCallback),
		  gensym("csize"), A_FLOAT, A_NULL);
}
void pix_test :: csizeMessCallback(void *data, float n)
{
  GetMyClass(data)->csize=(unsigned char)n;
}
void pix_test :: floatMessCallback(void *data, float n)
{
  GetMyClass(data)->off=(unsigned char)n;
}
