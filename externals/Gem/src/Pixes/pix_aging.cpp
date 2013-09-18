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
//    Copyright (c) 2002 James Tittle & Chris Clepper
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
// this is based on EffecTV by Fukuchi Kentarou
// * AgingTV - film-aging effect.
// * Copyright (C) 2001 FUKUCHI Kentarou
//
/////////////////////////////////////////////////////////

#include "pix_aging.h"

static unsigned int fastrand_val;
static unsigned int fastrand()
{
  //	return (fastrand_val=fastrand_val*1103515245+12345+2);
  return(fastrand_val=fastrand_val*435898247+382842987);
}

static int dx[8] = { 1, 1, 0,-1,-1,-1, 0, 1};
static int dy[8] = { 0,-1,-1,-1, 0, 1, 1, 1};

CPPEXTERN_NEW(pix_aging);

/////////////////////////////////////////////////////////
//
// pix_aging
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_aging :: pix_aging()
{
  m_coloraging=1;
  m_scratching=1;
  m_pits=1;
  m_dusts=1;

  m_scratchlines=20;
  m_scratch = new t_scratch[m_scratchlines];
  m_areascale=2;
  m_pitinterval=10;
  m_dustinterval = 0;
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_aging :: ~pix_aging()
{ }

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_aging :: processImage(imageStruct &image)
{
  unsigned char *pixes = image.data;
  unsigned char *p;
  int count = image.ysize * image.xsize * image.csize;
  unsigned char a;
  int width = image.xsize * image.csize;
  int height= image.ysize;
 
  int n;
  
  if (m_coloraging){
    n=count;
    while(n--){
      a = (*pixes & 0xfc)>>2;
      //      *pixes++ += 0x18 + ((fastrand()>>8)&0x10) - a;
      *pixes++ += 0x28 + fastrand()%8 - a;
    } 
  }
  if (m_scratching){
   int i, y, y1, y2;
    for(i=0; i<m_scratchlines; i++) {
      if(m_scratch[i].life) {
	m_scratch[i].x = m_scratch[i].x + m_scratch[i].dx*image.csize;
	if(m_scratch[i].x < 0 || m_scratch[i].x > width*256) {
	  m_scratch[i].life = 0;
	  goto break_scratch;
	}
	//	int offset=(m_scratch[i].x>>8);
	int offset = m_scratch[i].x/256;
	offset -= offset%image.csize;
#if 1
	pixes = image.data + offset;
#else
	pixes = image.data + (m_scratch[i].x>>8);
#endif
	if(m_scratch[i].init) {
	  y1 = m_scratch[i].init;
	  m_scratch[i].init = 0;
	} else {
	  y1 = 0;
	}
	m_scratch[i].life--;
	if(m_scratch[i].life) {
	  y2 = height;
	} else {
	  y2 = fastrand() % height+1;
	}
	for(y=y1; y<y2; y++) {
#if 0
	  a = *pixes & 0xfefeff;
	  a += 0x202020;
	  b = a & 0x1010100;
	  *pixes = a | (b - (b>>8));
#endif
	  pixes[chRed]=0xc0;
	  pixes[chGreen]=0xc0;
	  pixes[chBlue]=0xc0;
	
	  pixes += width;
	}
      } else {
	if((fastrand()&0xf0000000) == 0) {
	  m_scratch[i].life = 2 + (fastrand()>>27);
	  m_scratch[i].x = fastrand() % (width * 256);
	  m_scratch[i].dx = ((int)fastrand())>>23;
	  m_scratch[i].init = (fastrand() % (height-1))+1;
	}
      }
    break_scratch:
      ;
    }
  }
  if (m_pits){ /* PROBLEMS here */
    int i, j;
    int pnum, size, pnumscale;
    int x, y;
    pixes = image.data;
    pnumscale = m_areascale * 2;
    if(m_pitinterval) {
      pnum = pnumscale + (fastrand()%pnumscale);
      m_pitinterval--;
    } else {
      pnum = fastrand()%pnumscale;
      if((fastrand()&0xf8000000) == 0) {
	m_pitinterval = (fastrand()>>28) + 20;
      }
    }
    for(i=0; i<pnum; i++) {
      x = fastrand()%(width-1);
      y = fastrand()%(height-1);
      size = fastrand()>>28;
      for(j=0; j<size; j++) {
	x = x + fastrand()%3-1;
	y = y + fastrand()%3-1;
	if(x<=0 || x>=(width-1)) goto break_pit;
	if(y<=0 || y>=(height-1)) goto  break_pit;
	p = pixes + y*width + x;
	p[chRed]=p[chGreen]=p[chBlue]=0xc0;
      }
    break_pit:
      ;
    }
  }
  if (m_dusts){
    int i, j;
    int dnum;
    int d, len;
    int x, y;
    pixes = image.data;
    
    if(m_dustinterval == 0) {
      if((fastrand()&0xf0000000) == 0) {
	m_dustinterval = fastrand()>>29;
      }
      return;
    }
    
    dnum = m_areascale*4 + (fastrand()>>27);
    for(i=0; i<dnum; i++) {
      x = fastrand()%width;
      y = fastrand()%height;
      d = fastrand()>>29;
      len = fastrand()%m_areascale + 5;
      for(j=0; j<len; j++) {
	d = (d + fastrand()%3 - 1) & 7;
	y += dy[d];
	x += dx[d];
	if(x<=0 || x>=(width-1)) goto break_dust;
	if(y<=0 || y>=(height-1)) goto break_dust;
	p = pixes + y*width + x;
  	p[chRed]=p[chGreen]=p[chBlue]=0x10;
      }
    break_dust:
      ;
    }
    m_dustinterval--;
  } 
}

void pix_aging :: scratchMess(int scratchlines)
{
  if (scratchlines>0){
    m_scratching=1;
    m_scratchlines=scratchlines;
    if(m_scratch)delete [] m_scratch;
    m_scratch = new t_scratch[m_scratchlines];
  } else m_scratching=0;
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_aging :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_aging::colorMessCallback),
		   gensym("coloraging"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_aging::scratchMessCallback),
		  gensym("scratch"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_aging::dustMessCallback),
		   gensym("dust"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_aging::pitsMessCallback),
		  gensym("pits"), A_FLOAT, A_NULL);
}


void pix_aging :: colorMessCallback(void *data, t_floatarg state)
{
  GetMyClass(data)->m_coloraging=((int)state!=0);
  GetMyClass(data)->setPixModified();
}
void pix_aging :: scratchMessCallback(void *data, t_floatarg state)
{
  GetMyClass(data)->scratchMess((int)state);  
  GetMyClass(data)->setPixModified();
}
void pix_aging :: dustMessCallback(void *data, t_floatarg state)
{
  GetMyClass(data)->m_dusts=((int)state!=0);
  GetMyClass(data)->setPixModified();
}
void pix_aging :: pitsMessCallback(void *data, t_floatarg state)
{
  GetMyClass(data)->m_pits=((int)state!=0);
  GetMyClass(data)->setPixModified();
}
