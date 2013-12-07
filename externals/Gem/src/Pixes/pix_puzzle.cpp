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

#include "pix_puzzle.h"
#include <string.h>

CPPEXTERN_NEW(pix_puzzle);


unsigned int fastrand()
{
  static int fastrand_val = 0;
  return (fastrand_val=fastrand_val*1103515245+12345);
}


/////////////////////////////////////////////////////////
//
// pix_puzzle
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_puzzle :: pix_puzzle()
{
  myImage.xsize=myImage.ysize=myImage.csize=1;
  blocknum=1;
  myImage.allocate(blocknum);

  m_force = true;

  blockw = 8;
  blockh = 8;

  blockoffset = 0;
  blockpos = 0;

  m_game = false;
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_puzzle :: ~pix_puzzle()
{
  if (blockoffset) delete [] blockoffset;
  if (blockpos)    delete [] blockpos;
  myImage.clear();
}


/////////////////////////////////////////////////////////
//  Puzzle-Buf
//
/////////////////////////////////////////////////////////
void pix_puzzle :: makePuzzleBlocks(int xsize, int ysize, int csize)
{
  int i, x, y;
  if (blockoffset) delete [] blockoffset;
  if (blockpos)    delete [] blockpos;

  blockxsize = xsize / blockw;
  blockysize = ysize / blockh;
  blocknum = blockw * blockh;

  marginw = xsize - blockw*blockxsize;
  marginh = ysize - blockh*blockysize;

  spacepos = blocknum - 1;

  blockoffset = new int[blocknum];
  blockpos = new int[blocknum];

  for(y=0; y<blockh; y++)
    for(x=0; x<blockw; x++) blockoffset[y*blockw+x] = (y*blockysize*xsize + x*blockxsize)*csize;
  for(i=0; i<blocknum; i++) blockpos[i] = i;
}

void pix_puzzle :: shuffle()
{
  int i, a, b, c;

  if (!blockpos) return;
  if (blocknum == 1){
    blockpos[0]=0;
    return; /* nothing to be done for us here */
  }

  for(i=0; i<20*blockw; i++) {
    /* the number of shuffling times is a rule of thumb. */
    a = fastrand()%(blocknum-1);
    b = fastrand()%(blocknum-1);
    if(a == b)
      b = (b+1)%(blocknum-1);
    c = blockpos[a];
    blockpos[a] = blockpos[b];
    blockpos[b] = c;
  }
  setPixModified();
}
/////////////////////////////////////////////////////////
// sizeMess
//
/////////////////////////////////////////////////////////
void pix_puzzle :: sizeMess(int width, int height)
{

  blockw = (width>0)?width:8;
  blockh = (height>0)?height:8;

  m_force=true;
  setPixModified();
}

/////////////////////////////////////////////////////////
// moveMess
//
/////////////////////////////////////////////////////////
void pix_puzzle :: moveMess(int direction)
{
  if (!blockpos)return;
  if (direction==5)m_game=!m_game;
  if (!m_game)return;
  int nextpos, tmp;

  int x = spacepos % blockw;
  int y = spacepos / blockw;

  switch (direction) {
  case 8: // up
    y--;
    break;
  case 2: // down
    y++;
    break;
  case 4: // left
    x++;
    break;
  case 6: // right
    x--;
  default:
    break;
  }

  if (x<0)x=0;
  if (x>=blockw)x=blockw-1;
  if (y<0)y=0;
  if (y>=blockh)y=blockh-1;

  nextpos=y*blockw + x;
  tmp = blockpos[spacepos];
  blockpos[spacepos] = blockpos[nextpos];
  blockpos[nextpos] = tmp;
  spacepos = nextpos;

  setPixModified();
}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_puzzle :: processImage(imageStruct &image)
{
  unsigned char *src = image.data;
  unsigned char *dest;

  int x, y, xx, yy, i;
  unsigned char *p, *q;

  if (m_force || (myImage.xsize*myImage.ysize*myImage.csize != image.xsize*image.ysize*image.csize)){
    int dataSize = image.xsize * image.ysize * image.csize;
    myImage.clear();
    m_force = false;

    myImage.allocate(dataSize);

    makePuzzleBlocks(image.xsize, image.ysize, image.csize);
    shuffle();
  }

  myImage.xsize = image.xsize;
  myImage.ysize = image.ysize;
  myImage.csize = image.csize;
  myImage.type  = image.type;

  dest = myImage.data;

  i=0;
  for (y=0; y<blockh; y++){
    for(x=0; x<blockw; x++) {
      p = &src[blockoffset[blockpos[i]]];
      q = &dest[blockoffset[i]];
      if(m_game && spacepos == i) { // leave one rectangle blank (for the puzzle game)
	for(yy=0; yy<blockysize; yy++) {
	  for(xx=0; xx<blockxsize*image.csize; xx++) {
	    q[xx] = 0;
	  }
	  q += image.xsize*image.csize;
	}
      } else {
	for(yy=0; yy<blockysize; yy++) {
	  for(xx=0; xx<blockxsize*image.csize; xx++) {
	    q[xx] = p[xx];
	  }
	  q += image.xsize*image.csize;
	  p += image.xsize*image.csize;
	}
      }
      i++;
    }
  }

  p = src +  blockw * blockxsize;
  q = dest + blockw * blockxsize;

  if(marginw) {
    for(y=0; y<blockh*blockysize; y++) {
      for(x=0; x<marginw; x++) {
	*q++ = *p++;
      }
      p += image.xsize - marginw;
      q += image.xsize - marginw;
    }
  }

  if(marginh) {
    p = src + (blockh * blockysize) * image.xsize;
    q = dest + (blockh * blockysize) * image.xsize;
    memcpy(p, q, marginh*image.xsize*image.csize);
  }

  image.data=myImage.data;
}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_puzzle :: processYUVImage(imageStruct &image)
{
  unsigned char *src = image.data;
  unsigned char *dest;

  int x, y, xx, yy, i;
  unsigned char *p, *q;

  if (m_force || (myImage.xsize*myImage.ysize*myImage.csize != image.xsize*image.ysize*image.csize)){
    int dataSize = image.xsize * image.ysize * image.csize;
    myImage.clear();
    m_force = false;

    myImage.allocate(dataSize);

    makePuzzleBlocks(image.xsize, image.ysize, image.csize);
    shuffle();
  }

  myImage.xsize = image.xsize;
  myImage.ysize = image.ysize;
  myImage.csize = image.csize;
  myImage.type  = image.type;

  dest = myImage.data;

  i=0;
  for (y=0; y<blockh; y++){
    for(x=0; x<blockw; x++) {
      p = &src[blockoffset[blockpos[i]]];
      q = &dest[blockoffset[i]];
      if(m_game && spacepos == i) { // leave one rectangle blank (for the puzzle game)
	for(yy=0; yy<blockysize; yy++) {
	  for(xx=0; xx<blockxsize*image.csize; xx++) {
	    q[xx] = 0;
	  }
	  q += image.xsize*image.csize;
	}
      } else {
	for(yy=0; yy<blockysize; yy++) {
	  for(xx=0; xx<blockxsize*image.csize; xx++) {
	    q[xx] = p[xx];
	  }
	  q += image.xsize*image.csize;
	  p += image.xsize*image.csize;
	}
      }
      i++;
    }
  }

  p = src +  blockw * blockxsize;
  q = dest + blockw * blockxsize;

  if(marginw) {
    for(y=0; y<blockh*blockysize; y++) {
      for(x=0; x<marginw; x++) {
	*q++ = *p++;
      }
      p += image.xsize - marginw;
      q += image.xsize - marginw;
    }
  }

  if(marginh) {
    p = src + (blockh * blockysize) * image.xsize;
    q = dest + (blockh * blockysize) * image.xsize;
    memcpy(p, q, marginh*image.xsize*image.csize);
  }

  image.data=myImage.data;
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_puzzle :: obj_setupCallback(t_class *classPtr)
{
  class_addbang(classPtr, reinterpret_cast<t_method>(&pix_puzzle::bangMessCallback));

  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_puzzle::sizeMessCallback),
  		  gensym("size"), A_FLOAT, A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_puzzle::moveMessCallback),
  		  gensym("move"), A_FLOAT, A_NULL);
}

void pix_puzzle :: bangMessCallback(void *data)
{
  GetMyClass(data)->shuffle();
}

void pix_puzzle :: sizeMessCallback(void *data, t_floatarg width, t_floatarg height)
{
  GetMyClass(data)->sizeMess((int)width, (int)height);  
}

void pix_puzzle :: moveMessCallback(void *data, t_floatarg state)
{
  GetMyClass(data)->moveMess((int)state);
}
