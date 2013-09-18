////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// tigital@mac.com
//
// Implementation file
//
//    Copyright (c) 1997-1999 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    Copyright (c) 2005 James Tittle II. tigital@mac.com
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "pix_vpaint.h"

GLfloat edgeKernel[] = {
    -0.50f, 0.25f, -0.50f,
     0.25f, 1.00f, 0.25f,
     -0.50f, 0.25f, -0.50f};

GLfloat sumMatrix[] = {
    0.33f, 0.33f, 0.33f, 0.0f,
    0.33f, 0.33f, 0.33f, 0.0f,
    0.33f, 0.33f, 0.33f, 0.0f,
    0.00f, 0.00f, 0.00f, 1.0f};

CPPEXTERN_NEW(pix_vpaint);

/////////////////////////////////////////////////////////
//
// pix_vpaint
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_vpaint :: pix_vpaint()
    	  : m_initialized(0), maxPoints(2048), numPoints(0),
		    viewImage(0), useStrokes(1), drawEdges(0), moving(0), m_banged(false)
{
  m_w = m_h = 128;

  m_sizinlet = inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("vert_size"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_vpaint :: ~pix_vpaint()
{
	if (m_initialized)
	{
	  free(points);
	}
  if(m_sizinlet)inlet_free(m_sizinlet);
}


/////////////////////////////////////////////////////////
// extension checks
//
/////////////////////////////////////////////////////////
bool pix_vpaint :: isRunnable(void) {
  /*
   * Test for the required features 
   */
  /*    colorMatrixExtension = isExtensionSupported("GL_SGI_color_matrix");
        blendMinMaxExtension = isExtensionSupported("GL_EXT_blend_minmax");
        canDrawEdges = (colorMatrixExtension && blendMinMaxExtension) ||
        (strncmp((const char *) glGetString(GL_VERSION), "1.2", 3) == 0);
  */
  if(!GLEW_VERSION_1_2) {
    error("openGL-1.2 support missing");
    return false;
  }

  if(!GLEW_EXT_blend_minmax && !GLEW_SGI_color_matrix) {
    error("both color_matrix and blend_minmax extension missing");
    return false;
  }
  
  return true;
}



/////////////////////////////////////////////////////////
// makePoints
//
/////////////////////////////////////////////////////////
void pix_vpaint :: makepoints(void)
{
  GLubyte *bi = (GLubyte *) m_imageStruct.data;
  int i;
  //free(points);
  if (!m_initialized)
    points = (cPoint *) malloc(maxPoints * sizeof(cPoint));

  numPoints = maxPoints;
  if (m_imageStruct.format == GL_YCBCR_422_GEM){
    for (i = 0; i < maxPoints; i++) {
      points[i].x = rand() % m_w>>1;
      points[i].y = rand() % m_h;
      points[i].r = bi[4 * (points[i].y * (m_w>>1) + points[i].x)];
      points[i].g = bi[4 * (points[i].y * (m_w>>1) + points[i].x) + 1];
      points[i].b = bi[4 * (points[i].y * (m_w>>1) + points[i].x) + 2];
    }
  }else{
    for (i = 0; i < maxPoints; i++) {
      points[i].x = rand() % m_w;
      points[i].y = rand() % m_h;
      points[i].r = bi[4 * (points[i].y * m_w + points[i].x) + chRed];
      points[i].g = bi[4 * (points[i].y * m_w + points[i].x) + chGreen];
      points[i].b = bi[4 * (points[i].y * m_w + points[i].x) + chBlue];
    }
  }
}

/////////////////////////////////////////////////////////
// makecone
//
// cone with base at (0,0,0) and top at (0,0,1)
// one unit wide and high
//
/////////////////////////////////////////////////////////
void pix_vpaint :: makecone(void)
{
#define SIDES 20
  int i;
  glNewList(1, GL_COMPILE);
  glBegin(GL_TRIANGLE_FAN);
  glVertex3f(0.f, 0.f, 1.f);
  for (i = 0; i <= SIDES; i++) {
	float s = sinf((2.f * M_PI * i) / SIDES) * 25;
	float c = cosf((2.f * M_PI * i) / SIDES) * 25;
	glVertex3f(s, c, -4.0);
  }
  glEnd();
  glEndList();
}

/////////////////////////////////////////////////////////
// initialize
//
/////////////////////////////////////////////////////////
void pix_vpaint :: init()
{
	m_pbuffer = new PBuffer( m_w, m_h, PBuffer::GEM_PBUFLAG_RGBA | PBuffer::GEM_PBUFLAG_DEPTH );
	m_pbuffer->enable();

    /*
     * Points 
     */
    points = (cPoint *) malloc(maxPoints * sizeof(cPoint));
    makepoints();
    makecone();

    /*
     * Matrices 
     */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -4.0, 4.0);
    glMatrixMode(GL_MODELVIEW);
    glScalef(2.0 / m_w, 2.0 / m_h, 1.0);
    glTranslatef(-m_w / 2.0, -m_h / 2.0, 0.0);

    /*
     * Misc 
     */
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    /*
     * Test for the required features 
     */
    /*    colorMatrixExtension = isExtensionSupported("GL_SGI_color_matrix");
          blendMinMaxExtension = isExtensionSupported("GL_EXT_blend_minmax");
          canDrawEdges = (colorMatrixExtension && blendMinMaxExtension) ||
          (strncmp((const char *) glGetString(GL_VERSION), "1.2", 3) == 0);
    */
    canDrawEdges = 1;
    /*
     * Test for blend extension 
     */
  if (canDrawEdges) {
    GLfloat table[256];
    int i;

    /*
     * Pixel transfer parameters 
     */
    table[0] = 1.0;
    for (i = 1; i < 256; i++)
	    table[i] = 0.0;
    glPixelMapfv(GL_PIXEL_MAP_R_TO_R, 256, table);
    glPixelMapfv(GL_PIXEL_MAP_G_TO_G, 256, table);
    glPixelMapfv(GL_PIXEL_MAP_B_TO_B, 256, table);
  } else {
    error("This OpenGL implementation does not support the color matrix and/or\n");
    error("the min/max blending equations, therefore the option to draw the\n");
    error("Voronoi region edges is unavailable.\n\n");
    error("The required features are available with OpenGL 1.2 or the GL_EXT_blend_minmax\n");
    error("and GL_SGI_color_matrix extensions.\n");
  }
  m_initialized = 1;
  m_pbuffer->disable();
}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void pix_vpaint :: processImage(imageStruct &image)
{
  m_imageStruct = image;
  m_w = m_imageStruct.xsize;
  m_h = m_imageStruct.ysize;

  if (!m_initialized){
    init();
  }
  if ( (m_banged) || ((m_w != m_imageStruct.xsize) && (m_h != m_imageStruct.ysize)) )
  {
    makepoints();
    m_banged=false;
  }
  m_pbuffer->enable();
    
  if (viewImage) {
	glReadBuffer(GL_FRONT);
	glDrawBuffer(GL_FRONT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDrawPixels(m_w, m_h, image.format, GL_UNSIGNED_BYTE, (GLubyte *) m_imageStruct.data);
  } else if (!drawEdges) {
	glDrawBuffer(useStrokes ? GL_BACK : GL_FRONT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/*
	 * Just draw the voronoi regions 
	 */
	for (int i = 0; i < numPoints; i++) {
	    glPushMatrix();
	    glTranslatef(points[i].x, points[i].y, 0.f);
	    glColor3ub(points[i].r, points[i].g, points[i].b);
	    glCallList(1);
	    glColor3f(0.f, 0.f, 0.f);
	    glPopMatrix();
	}

	if (!useStrokes)
	{
      glClear(GL_COLOR_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);
	  glReadBuffer(GL_BACK);
	  glDrawBuffer(GL_BACK);

	  for (int y = 0; y < 3; y++) {
	    for (int x = 0; x < 3; x++) {
		  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		  glPushMatrix();
		  glTranslatef(x - 1, y - 1, 0.0);

		  for (int i = 0; i < numPoints; i++) {
		    glPushMatrix();
		    glTranslatef(points[i].x, points[i].y, 0.f);
		    glColor3ub(points[i].r, points[i].g, points[i].b);
		    glCallList(1);
		    glPopMatrix();
		  }
		  glPopMatrix();

		  glAccum(GL_ACCUM, edgeKernel[3 * y + x]);
	    }
      }
	  glAccum(GL_RETURN, 0.5);

	  /*
	   * Convert to grayscale 
	   */
	  glMatrixMode(GL_COLOR);
	  glLoadMatrixf(sumMatrix);
	  glCopyPixels(0, 0, m_w, m_h, GL_COLOR);
	  glLoadIdentity();
	  glMatrixMode(GL_MODELVIEW);

	  /*
	   * Threshold 
	   */
	  glPixelTransferi(GL_MAP_COLOR, GL_TRUE);
	  glCopyPixels(0, 0, m_w, m_h, GL_COLOR);
	  glPixelTransferi(GL_MAP_COLOR, GL_FALSE);

	  /*
	   * Draw the voronoi regions in the front buffer 
	   */
	  glDrawBuffer(GL_FRONT);
	  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	  for (int i = 0; i < numPoints; i++) {
	    glPushMatrix();
	    glTranslatef(points[i].x, points[i].y, 0.f);
	    glColor3ub(points[i].r, points[i].g, points[i].b);
	    glCallList(1);
	    glColor3f(0.f, 0.f, 0.f);
	    glPopMatrix();
	  }

	  /*
	   * Blend in the edge lines 
	   */
	  glClear(GL_DEPTH_BUFFER_BIT);

    if(GL_EXT_blend_minmax)
      glBlendEquationEXT(GL_MIN_EXT);

	  glEnable(GL_BLEND);

	  glCopyPixels(0, 0, m_w, m_h, GL_COLOR);
	  glDisable(GL_BLEND);
    }
  }
//  glFlush();
  glReadPixels(0,0,m_w, m_h, image.format, image.type, image.data);
  m_pbuffer->disable();
}

/////////////////////////////////////////////////////////
// sizeMess
//
/////////////////////////////////////////////////////////
void pix_vpaint :: sizeMess(int width, int height)
{
	m_w = width;
  m_h = height;
}

/////////////////////////////////////////////////////////
// static member functions
//
/////////////////////////////////////////////////////////
void pix_vpaint :: obj_setupCallback(t_class *classPtr)
{
    class_addbang(classPtr, reinterpret_cast<t_method>(&pix_vpaint::bangMessCallback));
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_vpaint::sizeMessCallback),
    	    gensym("vert_size"), A_FLOAT, A_FLOAT, A_NULL);
}
void pix_vpaint :: sizeMessCallback(void *data, t_floatarg width, t_floatarg height)
{
    GetMyClass(data)->sizeMess((int)width, (int)height);
}
void pix_vpaint :: bangMessCallback(void *data)
{
  GetMyClass(data)->m_banged=true;
}
