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

#include "sphere.h"
#include "Gem/State.h"
CPPEXTERN_NEW_WITH_TWO_ARGS(sphere, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// sphere
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
sphere :: sphere(t_floatarg size, t_floatarg slize)
  : GemGluObj(size, slize)
{
      int slice = m_numSlices;
      int stack = m_numSlices;
      
      m_x = new float[slice * stack];
      m_y = new float[slice * stack];
      m_z = new float[slice * stack];

      oldDrawType = m_drawType;

      //init these so the first render always creates the Sphere
      oldStacks = -1;
      oldSlices = -1;
      oldTexture= 0;
    
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
sphere :: ~sphere()
{ }

/////////////////////////////////////////////////////////
// createSphere
//	actually calculates the values of the vertices
//
/////////////////////////////////////////////////////////
void sphere :: createSphere(GemState *state)
{
   // GLdouble radius=m_size;
    GLint slices=m_numSlices;
    GLint stacks=m_numSlices;

    GLfloat rho, drho, theta, dtheta;
    GLfloat s, t, ds, dt;
    GLint i, j, imin, imax;
    GLenum orientation = true; /* GLU_INSIDE; */
    GLfloat nsign = (orientation)?-1.0:1.0;

    GLfloat xsize = 1.0, xsize0 = 0.0;
    GLfloat ysize = 1.0, ysize0 = 0.0;

    TexCoord*texCoords=NULL;
    int texNum=0;
    int texType=0;
    state->get(GemState::_GL_TEX_COORDS, texCoords);
    state->get(GemState::_GL_TEX_TYPE, texType);
    state->get(GemState::_GL_TEX_NUMCOORDS, texNum);


    if(m_drawType==GL_DEFAULT_GEM)m_drawType=GL_FILL;

    if(m_x)delete[]m_x;m_x=NULL;
    if(m_y)delete[]m_y;m_y=NULL;
    if(m_z)delete[]m_z;m_z=NULL;

    if(texType && texNum>=3){
        xsize0 = texCoords[0].s;
        xsize  = texCoords[1].s-xsize0;
        ysize0 = texCoords[1].t;
        ysize  = texCoords[2].t-ysize0;
    }

    drho = M_PI / static_cast<GLfloat>(stacks);
    dtheta = 2.0 * M_PI / static_cast<GLfloat>(slices);

    int src;

    if (m_drawType == GL_FILL) {

        m_x = new float[slices * stacks * 3];
        m_y = new float[slices * stacks * 3];
        m_z = new float[slices * stacks * 3];

        src = 0;
        
        if (!texType) {
            /* draw +Z end as a triangle fan */
            
            for (j = 0; j <= slices; j++) {
                theta = (j == slices) ? 0.0 : j * dtheta;
                m_x[src] = -sin(theta) * sin(drho);
                m_y[src] = cos(theta) * sin(drho);
                m_z[src] = nsign * cos(drho);
                src++;
            }
        }

        ds = 1.0 / slices;
        dt = 1.0 / stacks;
        t = 1.0;			/* because loop now runs from 0 */
        if (texType) {
            imin = 0;
            imax = stacks;
        }
        else {
            imin = 1;
            imax = stacks - 1;
        }

        /* draw intermediate stacks as quad strips */
        for (i = imin; i < imax; i++) {
            rho = i * drho;
            s = 0.0;
            for (j = 0; j <= slices; j++) {
                theta = (j == slices) ? 0.0 : j * dtheta;
                m_x[src] = -sin(theta) * sin(rho);
                m_y[src] = cos(theta) * sin(rho);
                m_z[src] = nsign * cos(rho);
                src++;
                m_x[src] = -sin(theta) * sin(rho + drho);
                m_y[src] = cos(theta) * sin(rho + drho);
                m_z[src] = nsign * cos(rho + drho);
                src++;
                
            }
        }

        if (!texType) {
            /* draw -Z end as a triangle fan */
            
            rho = M_PI - drho;
            s = 1.0;
            t = dt;
            for (j = slices; j >= 0; j--) {
                theta = (j == slices) ? 0.0 : j * dtheta;
                m_x[src] = -sin(theta) * sin(rho);
                m_y[src] = cos(theta) * sin(rho);
                m_z[src] = nsign * cos(rho);
                src++;
            }
        }

    }
    else if (m_drawType == GL_LINE || m_drawType == GLU_SILHOUETTE) {

        //allocate memory - this has twice the vertices as GL_POINT
        m_x = new float[slices * stacks * 2];
        m_y = new float[slices * stacks * 2];
        m_z = new float[slices * stacks * 2];
        
        src = 0;
        
        /* draw stack lines */
        for (i = 1; i < stacks; i++) {	/* stack line at i==stacks-1 was missing here */
            rho = i * drho;
            for (j = 0; j < slices; j++) {
                theta = j * dtheta;
                m_x[src] = cos(theta) * sin(rho);
                m_y[src] = sin(theta) * sin(rho);
                m_z[src] = cos(rho);
                src++;
            }
        }
        /* draw slice lines */
        for (j = 0; j < slices; j++) {
            theta = j * dtheta;
            for (i = 0; i <= stacks; i++) {
                rho = i * drho;
                m_x[src] = cos(theta) * sin(rho);
                m_y[src] = sin(theta) * sin(rho);
                m_z[src] = cos(rho);
                src++;
            }
        }
    }
    
    else if (m_drawType == GL_POINT) {

        //allocate memory - each style has a different number of vertices
        m_x = new float[slices * stacks];
        m_y = new float[slices * stacks];
        m_z = new float[slices * stacks];
        src = 0;

        /* loop over stacks and fill the arrays */
        for (i = 1; i < stacks - 1; i++) {
            rho = i * drho;
            for (j = 0; j < slices; j++) {
                theta = j * dtheta;
                m_x[src] = cos(theta) * sin(rho);
                m_y[src] = sin(theta) * sin(rho);
                m_z[src] = cos(rho);
                src++;
            }
        }
        
    }
    
    
}
/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void sphere :: render(GemState *state)
{
  GLdouble radius=m_size;
  GLint slices=m_numSlices;
  GLint stacks=m_numSlices;

  GLfloat rho, drho, dtheta;
  GLfloat s, t, ds, dt;
  GLint i, j, imin, imax;
  GLenum orientation = true; /* GLU_INSIDE; */
  GLfloat nsign = (orientation)?-1.0:1.0;

  int src=0;

  TexCoord*texCoords=NULL;
  int texType=0;
  int texNum=0;
  bool lighting=false;
  state->get(GemState::_GL_TEX_COORDS, texCoords);
  state->get(GemState::_GL_TEX_TYPE, texType);
  state->get(GemState::_GL_TEX_NUMCOORDS, texNum);
  state->get(GemState::_GL_LIGHTING, lighting);

  GLfloat xsize = 1.0, xsize0 = 0.0;
  GLfloat ysize = 1.0, ysize0 = 0.0;
  if(texType && texNum>=3){
    xsize0 = texCoords[0].s;
    xsize  = texCoords[1].s-xsize0;
    ysize0 = texCoords[1].t;
    ysize  = texCoords[2].t-ysize0;
  }
    
  drho = M_PI / static_cast<GLfloat>(stacks);
  dtheta = 2.0 * M_PI / static_cast<GLfloat>(slices);

  /* texturing: s goes from 0.0/0.25/0.5/0.75/1.0 at +y/+x/-y/-x/+y axis */
  /* t goes from -1.0/+1.0 at z = -radius/+radius (linear along longitudes) */
  /* cannot use triangle fan on texturing (s coord. at top/bottom tip varies) */

  //if anything changed then the geometry is rebuilt
  if (stacks != oldStacks || slices != oldSlices || 
      m_drawType != oldDrawType || texType!=oldTexture){

      //call the sphere creation function to fill the array
      createSphere(state);

      oldStacks = stacks;
      oldSlices = slices;
      oldDrawType = m_drawType;
      oldTexture = texType;
  }
 
  if (m_drawType == GL_FILL) {
    src = 0;
    if (!texType) {
      /* draw +Z end as a triangle fan */
      glBegin(GL_TRIANGLE_FAN);
      glNormal3f(0.0, 0.0, 1.0);
      glVertex3f(0.0, 0.0, nsign * radius);
      for (j = 0; j <= slices; j++) {
			if (lighting)
				glNormal3f(m_x[src] * nsign, m_y[src] * nsign, m_z[src] * nsign);
			glVertex3f(m_x[src] * radius, m_y[src] * radius, m_z[src] * radius);
			src++;
      }
      glEnd();
    }

    ds = 1.0 / slices;
    dt = 1.0 / stacks;
    t = 1.0;			/* because loop now runs from 0 */
    if (texType) {
      imin = 0;
      imax = stacks;
    }
    else {
      imin = 1;
      imax = stacks - 1;
    }

    /* draw intermediate stacks as quad strips */
    for (i = imin; i < imax; i++) {
      glBegin(GL_QUAD_STRIP);
      s = 0.0;
      for (j = 0; j <= slices; j++) {

		if (lighting)
			glNormal3f(m_x[src] * nsign, m_y[src] * nsign, m_z[src] * nsign);
		if(texType)
			glTexCoord2f(s*xsize+xsize0, t*ysize+ysize0);
		glVertex3f(m_x[src] * radius, m_y[src] * radius, m_z[src] * radius);
		src++;
		if (lighting)
			glNormal3f(m_x[src] * nsign, m_y[src] * nsign, m_z[src] * nsign);
		if(texType)
			glTexCoord2f(s*xsize+xsize0, (t - dt)*ysize+ysize0);
		s += ds;
		glVertex3f(m_x[src] * radius, m_y[src] * radius, m_z[src] * radius);
        src++;
      }
      glEnd();
      t -= dt;
    }

    if (!texType) {
      /* draw -Z end as a triangle fan */
      glBegin(GL_TRIANGLE_FAN);
      glNormal3f(0.0, 0.0, -1.0);
      glVertex3f(0.0, 0.0, -radius * nsign);
      s = 1.0;
      t = dt;
      for (j = slices; j >= 0; j--) {
		if (lighting)
			glNormal3f(m_x[src] * nsign, m_y[src] * nsign, m_z[src] * nsign);
		s -= ds;
		glVertex3f(m_x[src] * radius, m_y[src] * radius, m_z[src] * radius);
        src++;
      }
      glEnd();
    }
  }
  else if (m_drawType == GL_LINE || m_drawType == GLU_SILHOUETTE) {
     
      src = 0;
      
      for (i = 1; i < stacks; i++) {	// stack line at i==stacks-1 was missing here
          glBegin(GL_LINE_LOOP);
          for (j = 0; j < slices; j++) {
        
              if (lighting)
                  glNormal3f(m_x[src] * nsign, m_y[src] * nsign, m_z[src] * nsign);
              glVertex3f(m_x[src] * radius, m_y[src] * radius, m_z[src] * radius);
              src++;
          }
          glEnd();
      }

      for (j = 0; j < slices; j++) {
          glBegin(GL_LINE_STRIP);
          for (i = 0; i <= stacks; i++) {

              if (lighting)
                  glNormal3f(m_x[src] * nsign, m_y[src] * nsign, m_z[src] * nsign);
              glVertex3f(m_x[src] * radius, m_y[src] * radius, m_z[src] * radius);
              src++;
          }
          glEnd();
      }   
  }
  
  else if (m_drawType == GL_POINT) {
    /* top and bottom-most points */
    glBegin(GL_POINTS);
    if (lighting)
      glNormal3f(0.0, 0.0, nsign);
    glVertex3d(0.0, 0.0, radius);
    if (lighting)
      glNormal3f(0.0, 0.0, -nsign);
    glVertex3d(0.0, 0.0, -radius);
    
    src = 0;
    
    for (i = 1; i < stacks - 1; i++) {
        rho = i * drho;
        for (j = 0; j < slices; j++) {
            if (lighting)
                glNormal3f(m_x[src] * nsign, m_y[src] * nsign, m_z[src] * nsign);
            glVertex3f(m_x[src] * radius, m_y[src] * radius, m_z[src] * radius);
            src++;
        }
    }
    glEnd();
  }
}
/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void sphere :: obj_setupCallback(t_class *)
{ }

