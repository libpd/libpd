////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-1999 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    Copyright (c) 2002 James Tittle & Chris Clepper
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#ifdef __GNUC__
# warning LATER fix upsidedown
// LATER think about textures without images (or not: use GLSL for that)
#endif

#include "imageVert.h"
#include "Gem/State.h"

CPPEXTERN_NEW(imageVert);

/////////////////////////////////////////////////////////
//
// imageVert
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
imageVert :: imageVert()
  : m_rebuildList(1)
{
  m_dispList = 0;
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
imageVert :: ~imageVert()
{
  // Delete our display list
  if (m_dispList) glDeleteLists(m_dispList, 1);
}

/////////////////////////////////////////////////////////
// processRGBAPix
//
/////////////////////////////////////////////////////////
void imageVert :: processRGBAPix(imageStruct &image, int texture)
{
  float red, green, blue, alpha;
  float red2, green2, blue2, alpha2;
    
  const int ySize = image.ysize;
  const int xSize = image.xsize;
  const int yStride = xSize * image.csize;
  const int xStride = image.csize;

  const float yDiff = 1.f / ySize;
  float yDown = -.5f;
  float yCurrent = yDown + yDiff;
  float yTexDown = 0.f;
  float yTex = yTexDown + yDiff;

  const float xDiff = 1.f / xSize;

  glShadeModel(GL_SMOOTH);

  glNormal3f(0.0f, 0.0f, 1.0f);

  unsigned char *data = image.data + yStride;
  if (texture)   {
    int yCount = ySize;
    while(yCount--)  {
      float xCurrent = -.5f;
      float xTex = 0.f;
      int xCount = xSize;
      glBegin(GL_QUAD_STRIP);
      while(xCount--)   {
	unsigned char *oneDown = data - yStride;
	red   = data[chRed] / 255.f;
	green = data[chGreen] / 255.f;
	blue  = data[chBlue] / 255.f;

	red2   = oneDown[chRed] / 255.f;
	green2 = oneDown[chGreen] / 255.f;
	blue2  = oneDown[chBlue] / 255.f;
    		        
	glTexCoord2f(xTex, yTexDown);
	glVertex3f(xCurrent, yDown, red2 + green2 + blue2);
	glTexCoord2f(xTex, yTex);
	glVertex3f(xCurrent, yCurrent, red + green + blue);

	xCurrent += xDiff;
	xTex += xDiff;
	data += xStride;
      }
      glEnd();
      yDown = yCurrent;
      yCurrent += yDiff;
      yTexDown = yTex;
      yTex += yDiff;
    }
  } else {
    int yCount = ySize;
    while(yCount--) {
      int xCount = xSize;
      float xCurrent = -.5f;

      glBegin(GL_QUAD_STRIP);
      while(xCount--) {
	unsigned char *oneDown = data - yStride;
	red   = data[chRed] / 255.f;
	green = data[chGreen] / 255.f;
	blue  = data[chBlue] / 255.f;
	alpha = data[chAlpha] / 255.f;

	red2   = oneDown[chRed] / 255.f;
	green2 = oneDown[chGreen] / 255.f;
	blue2  = oneDown[chBlue] / 255.f;
	alpha2 = oneDown[chAlpha] / 255.f;
    		        
	glColor4f(red2, green2, blue2, alpha2);
	glVertex3f(xCurrent, yDown, red2 + green2 + blue2);
	glColor4f(red, green, blue, alpha);
	glVertex3f(xCurrent, yCurrent, red + green + blue);

	xCurrent += xDiff;
	data += xStride;
      }
      glEnd();
      yDown = yCurrent;
      yCurrent += yDiff;
    }
  }
}

/////////////////////////////////////////////////////////
// processGrayPix
//
/////////////////////////////////////////////////////////
void imageVert :: processGrayPix(imageStruct &image, int texture)
{
  float gray, gray2;
    
  const int ySize = image.ysize;
  const int xSize = image.xsize;
  const int yStride = xSize * image.csize;
  const int xStride = image.csize;

  const float yDiff = 1.f / ySize;
  float yDown = -.5f;
  float yCurrent = yDown + yDiff;
  float yTexDown = 0.f;
  float yTex = yTexDown + yDiff;

  const float xDiff = 1.f / xSize;

  glShadeModel(GL_SMOOTH);

  glNormal3f(0.0f, 0.0f, 1.0f);

  unsigned char *data = image.data + yStride;
  if (texture) {
    int yCount = ySize;
    while(yCount--) {
      float xCurrent = -.5f;
      float xTex = 0.f;
      int xCount = xSize;
      glBegin(GL_QUAD_STRIP);
                
      while(xCount--) {
	unsigned char *oneDown = data - yStride;
	gray   = data[chGray] / 255.f;
	gray2  = oneDown[chGray] / 255.f;
		    
	glTexCoord2f(xTex, yTexDown);
	glVertex3f(xCurrent, yDown, gray2 + gray2 + gray2);
	glTexCoord2f(xTex, yTex);
	glVertex3f(xCurrent, yCurrent, gray + gray + gray);

	xCurrent += xDiff;
	xTex += xDiff;
	data += xStride;
      }
      glEnd();
      yDown = yCurrent;
      yCurrent += yDiff;
      yTexDown = yTex;
      yTex += yDiff;
    }
  } else {
    int yCount = ySize;
    while(yCount--)  {
      int xCount = xSize;
      float xCurrent = -.5f;
	
      glBegin(GL_QUAD_STRIP);
      while(xCount--)  {
	unsigned char *oneDown = data - yStride;
	gray   = data[chGray] / 255.f;
	gray2  = oneDown[chGray] / 255.f;
    		        
	glColor4f(gray2, gray2, gray2, 1.0f);
	glVertex3f(xCurrent, yDown, gray2 + gray2 + gray2);
	glColor4f(gray, gray, gray, 1.0f);
	glVertex3f(xCurrent, yCurrent, gray + gray + gray);

	xCurrent += xDiff;
	data += xStride;
      }
      glEnd();
      yDown = yCurrent;
      yCurrent += yDiff;
    }
  }
}

/////////////////////////////////////////////////////////
// processYUVPix
//
/////////////////////////////////////////////////////////
void imageVert :: processYUVPix(imageStruct &image, int texture)
{
  error("YUV not yet implemented :-(");
/*  float Y, Y2, U, U2, V, V2;
    
  const int ySize = image.ysize;
  const int xSize = image.xsize;
  const int yStride = xSize * image.csize;
  const int xStride = image.csize;

  const float yDiff = 1.f / ySize;
  float yDown = -.5f;
  float yCurrent = yDown + yDiff;
  float yTexDown = 0.f;
  float yTex = yTexDown + yDiff;

  const float xDiff = 1.f / xSize;

  glShadeModel(GL_SMOOTH);

  glNormal3f(0.0f, 0.0f, 1.0f);

  unsigned char *data = image.data + yStride;
  if (texture)   {
    int yCount = ySize;
    while(yCount--)  {
      float xCurrent = -.5f;
      float xTex = 0.f;
      int xCount = xSize;
      glBegin(GL_QUAD_STRIP);
      while(xCount--)   {
	unsigned char *oneDown = data - yStride;
	Y   = data[chY] / 255.f;
	U = data[chU] / 255.f;
	V  = data[chV] / 255.f;

	Y2   = oneDown[chY] / 255.f;
	U2 = oneDown[chU] / 255.f;
	V2  = oneDown[chV] / 255.f;
    		        
	glTexCoord2f(xTex, yTexDown);
	glVertex3f(xCurrent, yDown, Y2 + U2 + V2);
	glTexCoord2f(xTex, yTex);
	glVertex3f(xCurrent, yCurrent, Y + U + V);

	xCurrent += xDiff;
	xTex += xDiff;
	data += xStride;
      }
      glEnd();
      yDown = yCurrent;
      yCurrent += yDiff;
      yTexDown = yTex;
      yTex += yDiff;
    }
  } else {
    int yCount = ySize;
    while(yCount--) {
      int xCount = xSize;
      float xCurrent = -.5f;
    	    
      glBegin(GL_QUAD_STRIP);
      while(xCount--) {
	unsigned char *oneDown = data - yStride;
	Y   = data[chY] / 255.f;
	U = data[chU] / 255.f;
        V = data[chV] / 255.f;

	Y2   = oneDown[chY] / 255.f;
	U2 = oneDown[chU] / 255.f;
	V2  = oneDown[chV] / 255.f;
    		        
	glColor3f(Y2, U2, V2);
	glVertex3f(xCurrent, yDown, Y2 + U2 + V2);
	glColor3f(Y, U, V);
	glVertex3f(xCurrent, yCurrent, Y + U + V);

	xCurrent += xDiff;
	data += xStride;
      }
      glEnd();
      yDown = yCurrent;
      yCurrent += yDiff;
    }
  }
*/
}
/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void imageVert :: render(GemState *state)
{
  pixBlock*img=NULL;
  int texType=0;
  bool dl = false;

  state->get(GemState::_PIX, img);
  state->get(GemState::_GL_TEX_TYPE, texType);
  state->get(GemState::_GL_DISPLAYLIST, dl);

  // always want to render
  if (!img) return;

  if (img->newimage) m_rebuildList = 1;

  if (!m_dispList){
    m_dispList=glGenLists(1);
    m_rebuildList=1;
  }

  // can we build a display list?
  if (!dl && m_rebuildList)
    {
      glNewList(m_dispList, GL_COMPILE_AND_EXECUTE);
      if (img->image.format == GL_RGBA || img->image.format == GL_BGRA_EXT)	//tigital
	processRGBAPix(img->image, texType);
      else
	processGrayPix(img->image, texType);
      glEndList();
      m_rebuildList = 0;
    }
  // nope, but our current one isn't valid
  else if (m_rebuildList) {
    if (img->image.format == GL_RGBA || img->image.format == GL_BGRA_EXT)	//tigital
      processRGBAPix(img->image, texType);
    else
      processGrayPix(img->image, texType);
  }
  // the display list has already been built
  else glCallList(m_dispList);
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void imageVert :: obj_setupCallback(t_class *)
{ }
