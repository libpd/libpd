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
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "pix_threshold_bernsen.h"

CPPEXTERN_NEW(pix_threshold_bernsen);

/////////////////////////////////////////////////////////
//
// pix_threshold_bernsen
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_threshold_bernsen :: pix_threshold_bernsen():
  m_xtiles(16), m_ytiles(16), m_contrast(10),
  m_minVals(NULL), m_maxVals(NULL)
{
    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("tiles"));
    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("contrast"));
    tilesMess(m_xtiles, m_ytiles);
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_threshold_bernsen :: ~pix_threshold_bernsen()
{
  if(m_minVals) delete[]m_minVals;
  if(m_maxVals) delete[]m_maxVals;
}

/////////////////////////////////////////////////////////
// processGrayImage
//
/////////////////////////////////////////////////////////
void pix_threshold_bernsen :: processGraySub_getMinMax(imageStruct&image,
                                                       int fromX, int toX,
                                                       int fromY, int toY,
                                                       unsigned char*resultMin,
                                                       unsigned char*resultMax)
{
  int min=255;
  int max=0;
  int x, y;
  int linelength = image.xsize*image.csize;

  if(fromX<0)fromX=0;
  if(toX>image.xsize)toX=image.xsize;
  if(fromY<0)fromY=0;
  if(toY>image.ysize)toY=image.ysize;

  for(y=fromY; y<toY; y++){
    // set base to the beginning of the line
    unsigned char* base=image.data + y*linelength + fromX;
    max=min=*base;
    for(x=fromX; x<toX; x++){
      unsigned char value=*base++;
      if(value<min)value=min;
      else if(value>max)value=max;
    }
  }
  *resultMin=min;
  *resultMax=max;
}
inline void processGraySub_threshold(imageStruct&image,
                                     int fromX, int toX,
                                     int fromY, int toY,
                                     unsigned char thresh)
{
  int x, y;
  int linelength = image.xsize*image.csize;

  if(fromX<0)fromX=0;
  if(toX>image.xsize)toX=image.xsize;
  if(fromY<0)fromY=0;
  if(toY>image.ysize)toY=image.ysize;
  for(y=fromY; y<toY; y++){
    // set base to the beginning of the line
    unsigned char* base=image.data + y*linelength + fromX;
    for(x=fromX; x<toX; x++){
      unsigned char value=*base;
      *base++=(value>thresh)*255;
    }
  }
}



void pix_threshold_bernsen :: processGrayImage(imageStruct &image)
{
  int half_xtile=m_xtiles/2;
  int half_ytile=m_ytiles/2;
  int tile_xsize=image.xsize/m_xtiles;
  int tile_ysize=image.ysize/m_ytiles;

  int x, y;

  // get the minimum & maximum of each quarter-sized tile
  for (y=0; y<m_ytiles*2; y++){
    int fromY=image.ysize*y/(m_ytiles*2);
    for (x=0; x<m_xtiles*2; x++){
      int fromX=image.xsize*x/(m_xtiles*2);
      int offset=(2*m_xtiles)*y+x;
      processGraySub_getMinMax(image, 
                               fromX, fromX+half_xtile,
                               fromY, fromY+half_ytile,
                               m_minVals+offset,
                               m_maxVals+offset);
    }
  }
  // get the thresholds for each tile (based on 16 quarter-sized tiles)
#if 0
  for (y=0; y<m_ytiles*2; y++){
    for (x=0; x<m_xtiles*2; x++){
      int offset=(2*m_xtiles)*y+x;
    }
  }
#endif


  for(y=0; y<m_ytiles; y++){
    int fromY=2*y-1;
    int toY  =2*y+3;
    unsigned char mean = 127;

    if(fromY<0)fromY=0;
    if(toY>2*m_ytiles)toY=2*m_ytiles;

    for (x=0; x<m_xtiles; x++){
      int fromX=2*x-1;
      int toX  =2*x+3;
      int i,j;

      unsigned char min=255, max=0;

      if(fromX<0)fromX=0;
      if(toX>2*m_xtiles)toX=2*m_xtiles;

      for(i=fromY; i<toY; i++){
        for(j=fromX; j<toX; j++){
          int offset=(2*m_xtiles)*i+j;

          unsigned char min_=m_minVals[offset];
          unsigned char max_=m_maxVals[offset];
          if(min>min_)min=min_;
          if(max<max_)max=max_;
        }
      }
      if((max-min)>m_contrast)
        mean=(min+max)/2;
      else mean=255;

      processGraySub_threshold(image, 
                               x*tile_xsize,(x+1)*tile_xsize,
                               y*tile_ysize,(y+1)*tile_ysize,
                               mean);
    }
  }
}

/////////////////////////////////////////////////////////
// vecThreshMess
//
/////////////////////////////////////////////////////////
void pix_threshold_bernsen :: tilesMess(int w, int h)
{
  if(m_minVals) delete[]m_minVals;
  if(m_maxVals) delete[]m_maxVals;

  if(w>0)m_xtiles=w;
  if(h>0)m_ytiles=h;
  
  m_minVals=new unsigned char[4*m_xtiles*m_ytiles];
  m_maxVals=new unsigned char[4*m_xtiles*m_ytiles];

  setPixModified();
}
void pix_threshold_bernsen :: contrastMess(int c)
{
  m_contrast=c;
  setPixModified();
}


/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_threshold_bernsen :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_threshold_bernsen::tilesMessCallback),
                    gensym("tiles"), A_FLOAT, A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_threshold_bernsen::contrastMessCallback),
                    gensym("contrast"), A_FLOAT, A_NULL);
}
void pix_threshold_bernsen :: tilesMessCallback(void *data, t_float w, t_float h)
{
  GetMyClass(data)->tilesMess(static_cast<int>(w), static_cast<int>(h));
}
void pix_threshold_bernsen :: contrastMessCallback(void *data, t_float c)
{
  GetMyClass(data)->contrastMess(static_cast<int>(c*255.));
}
