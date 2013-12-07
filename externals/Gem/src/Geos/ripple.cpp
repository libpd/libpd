/*
 *  GEM - Graphics Environment for Multimedia
 *
 *  ripple.cpp
 *  gem_darwin
 *
 *  Created by Jamie Tittle on Sun Jan 19 2003.
 *  Copyright (c) 2003 tigital. All rights reserved.
 *    For information on usage and redistribution, and for a DISCLAIMER OF ALL
 *    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
 *
 */

#include "ripple.h"
#include "Gem/State.h"

CPPEXTERN_NEW_WITH_TWO_ARGS(ripple, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// ripple
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
ripple :: ripple( t_floatarg gridX, t_floatarg gridY )
  : GemShape(1.0), 
    m_ctrX(0.f), m_ctrY(0.f),
    m_height(1.0), 
    m_inletH(NULL), m_inletcX(NULL), m_inletcY(NULL),
    m_gridX(0), m_gridY(0),
    m_alreadyInit(false),
    m_sizeX(0.f), m_sizeY(0.f), m_sizeY0(0.f)
             
{
  int gridXi=static_cast<int>(gridX);
  int gridYi=static_cast<int>(gridY);
  m_gridX=(gridXi>0&&gridXi<GRID_MAX_X)?gridXi:GRID_SIZE_X;
  m_gridY=(gridYi>0&&gridXi<GRID_MAX_Y)?gridYi:GRID_SIZE_Y;

  // the height inlet
  m_inletH = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("Ht"));
  m_inletcX = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("cX"));
  m_inletcY = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("cY"));

  m_drawType = GL_POLYGON;
  precalc_ripple_amp();
}

////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
ripple :: ~ripple()
{
  inlet_free(m_inletH);
  inlet_free(m_inletcX);
  inlet_free(m_inletcY);
  m_alreadyInit = false;
}
/////////////////////////////////////////////////////////
// renderShape
//
/////////////////////////////////////////////////////////
void ripple :: renderShape(GemState *state)
{
  int i, j;
  glNormal3f(0.0f, 0.0f, 1.0f);

  glScalef(2.*m_size, 2.*m_size, 2.*m_size);
    
  if (GemShape::m_texType && GemShape::m_texNum>=3)
    {
      if ((m_sizeX  != GemShape::m_texCoords[1].s) ||
          (m_sizeY  != GemShape::m_texCoords[1].t) ||
          (m_sizeY0 != GemShape::m_texCoords[2].t))
        m_alreadyInit = false;
    
      if (!m_alreadyInit)
        {
          m_sizeX  = GemShape::m_texCoords[1].s;
          m_sizeY0 = GemShape::m_texCoords[2].t;
          m_sizeY  = GemShape::m_texCoords[1].t;

          ripple_init();
          precalc_ripple_vector();
          m_alreadyInit = true;
        }
      for (i = 0; i < m_gridX - 1; i++)  {
        for (j = 0; j < m_gridY - 1; j++)  {
          glBegin(m_drawType);
          glTexCoord2fv(m_rippleVertex[i][j].t);
          glVertex2fv(m_rippleVertex[i][j].x);
          glTexCoord2fv(m_rippleVertex[i][j + 1].t);
          glVertex2fv(m_rippleVertex[i][j + 1].x);
          glTexCoord2fv(m_rippleVertex[i + 1][j + 1].t);
          glVertex2fv(m_rippleVertex[i + 1][j + 1].x);
          glTexCoord2fv(m_rippleVertex[i + 1][j].t);
          glVertex2fv(m_rippleVertex[i + 1][j].x);
          glEnd();
        }
      }
      ripple_dynamics();
    }  else  {
    if (!m_alreadyInit)   {
      m_sizeX = 1;
      m_sizeY = 1;
      m_sizeY0= 0;

      ripple_init();
      precalc_ripple_vector();
      m_alreadyInit = true;
    }
    glTranslatef(-.5, -.5, 0.0);
    for (i = 0; i < m_gridX - 1; i++)  {
      for (j = 0; j < m_gridY - 1; j++) {
        glBegin(m_drawType);
        glTexCoord2fv(m_rippleVertex[i][j].t);
        glVertex2fv(m_rippleVertex[i][j].t);
        glTexCoord2fv(m_rippleVertex[i][j + 1].t);
        glVertex2fv(m_rippleVertex[i][j + 1].t);
        glTexCoord2fv(m_rippleVertex[i + 1][j + 1].t);
        glVertex2fv(m_rippleVertex[i + 1][j + 1].t);
        glTexCoord2fv(m_rippleVertex[i + 1][j].t);
        glVertex2fv(m_rippleVertex[i + 1][j].t);
        glEnd();
      }
    }
    glTranslatef(.5, .5, 0.0);
    ripple_dynamics();
  }

  glScalef(.5/m_size, .5/m_size, .5/m_size);
}
/////////////////////////////////////////////////////////
//
//	ripple_init
//	Initialize ripple location and age information.
//
//	Also, precompute the vertex coordinates and the default texture
//	coordinates assigned to them.
/////////////////////////////////////////////////////////

void ripple :: ripple_init()
{
  int i, j;
  glDisable(GL_DEPTH_TEST);

  m_rippleMax = (int)sqrt(m_sizeX * (m_sizeY+m_sizeY0) + m_sizeX * m_sizeX);
  for (i = 0; i < RIPPLE_COUNT; i++)
    {
      m_t[i] = m_rippleMax + RIPPLE_LENGTH;
      m_cx[i] = 0;
      m_cy[i] = 0;
      m_max[i] = 0;
    }

  for (i = 0; i < m_gridX; i++)
    for (j = 0; j < m_gridY; j++)
      {

        m_rippleVertex[i][j].x[0] = (i/(m_gridX - 1.0 ))-0.5;
        m_rippleVertex[i][j].x[1] = (j/(m_gridY - 1.0 ))-0.5;
        m_rippleVertex[i][j].dt[0] = m_sizeX*(i/(m_gridX - 1.0 ));
        m_rippleVertex[i][j].dt[1] = (m_sizeY0-m_sizeY)*(j/(m_gridY - 1.0 ))+m_sizeY;
      }
}

/////////////////////////////////////////////////////////
//	Precompute ripple displacement vectors.
/////////////////////////////////////////////////////////

void ripple :: precalc_ripple_vector()
{
  int i, j, z;
  float x, y, l;

   for (i = 0; i < m_gridX; i++)
    {
      for (j = 0; j < m_gridY; j++)
        {
          x = (float) i/(m_gridX - 1);
          y = (float) j/(m_gridY - 1);
          l = (float) sqrt(x*x + y*y);
          if (l == 0.0)
            {
              x = 0.0f;
              y = 0.0f;
            }
          else
            {
              x /= l;
              y /= l;
            }
          z = (int)(l*m_sizeX*2);
          m_rippleVector[i][j].dx[0] = x*m_sizeX;
          m_rippleVector[i][j].dx[1] = y*(m_sizeY+m_sizeY0);
          m_rippleVector[i][j].r = z;
        }
    }
}

/////////////////////////////////////////////////////////
//	Precompute ripple amplitude decay.
/////////////////////////////////////////////////////////

void ripple :: precalc_ripple_amp()
{
  int i;
  double t;
  double a;

  for (i = 0; i < RIPPLE_LENGTH; i++)
    {
      t = 1.0 - i/(RIPPLE_LENGTH - 1.0);
      a = (-cos(t*2.0*3.1428571*RIPPLE_CYCLES)*0.5 + 0.5)
        *RIPPLE_AMPLITUDE*t*t*t*t*t*t*t*t;
      if (i == 0)
        a = 0.0;
      m_rippleAmp[i].amplitude = a;
    }
}

/////////////////////////////////////////////////////////
//
//	ripple_dynamics
//	Advance one time step and compute new texture coordinates
//	for the next frame of animation.
/////////////////////////////////////////////////////////

void ripple :: ripple_dynamics()
{
  int i, j, k;
  int x, y;
  int mi, mj;
  int r;
  float sx, sy;
  float amp;

  for (i = 0; i < RIPPLE_COUNT; i++) m_t[i] += RIPPLE_STEP;

  for (i = 0; i < m_gridX; i++)
    for (j = 0; j < m_gridY; j++)    {
      m_rippleVertex[i][j].t[0] = m_rippleVertex[i][j].dt[0];
      m_rippleVertex[i][j].t[1] = m_rippleVertex[i][j].dt[1];
      for (k = 0; k < RIPPLE_COUNT; k++)      {
        x = i - m_cx[k];
        y = j - m_cy[k];
        if (x < 0){
          x *= -1;
          sx = -1.0;
        }else
          sx = 1.0;
        if (y < 0){
          y *= -1;
          sy = -1.0;
        }else
          sy = 1.0;
        mi = x;
        mj = y;
        if(mi<0)mi=0;if(mi>=m_gridX)mi=m_gridX-1;
        if(mj<0)mj=0;if(mj>=m_gridY)mj=m_gridY-1;

        r = m_t[k] - m_rippleVector[mi][mj].r;
        if (r < 0)  r = 0;
        if (r > RIPPLE_LENGTH - 1)  r = RIPPLE_LENGTH - 1;

        amp = 1.0 - 1.0*m_t[k]/RIPPLE_LENGTH;
        amp *= amp;
        if (amp < 0.0)  amp = 0.0;
        /* jmz: added m_height */
        m_rippleVertex[i][j].t[0]
          += m_rippleVector[mi][mj].dx[0]*sx*m_rippleAmp[r].amplitude*amp*m_height;
        m_rippleVertex[i][j].t[1]
          += m_rippleVector[mi][mj].dx[1]*sy*m_rippleAmp[r].amplitude*amp*m_height;
      }
    }
}
/////////////////////////////////////////////////////////
//
//	ripple_distance
//
//	Calculate the distance between two points.
//
/////////////////////////////////////////////////////////
float ripple :: ripple_distance(int gx, int gy, int cx, int cy)
{
  return sqrt(1.0*(gx - cx)*(gx - cx) + 1.0*(gy - cy)*(gy - cy));
}

/////////////////////////////////////////////////////////
//
//	ripple_max_distance
//
//	Compute the distance of the given window coordinate
//	to the nearest window corner, in pixels.
/////////////////////////////////////////////////////////
int ripple :: ripple_max_distance(int gx, int gy)
{
  float d;
  float temp_d;

  d = ripple_distance(gx, gy, 0, 0);
  temp_d = ripple_distance(gx, gy, m_gridX, 0);
  if (temp_d > d)
    d = temp_d;
  temp_d = ripple_distance(gx, gy, m_gridX, m_gridY);
  if (temp_d > d)
    d = temp_d;
  temp_d = ripple_distance(gx, gy, 0, m_gridY);
  if (temp_d > d)
    d = temp_d;

  return (int)((d/m_gridX)*m_sizeX + RIPPLE_LENGTH/6);
}
/////////////////////////////////////////////////////////
//	ripple_bang
//
//	Generate a new ripple when the mouse is pressed.  There's
//	a limit on the number of ripples that can be simultaneously
//	generated.
/////////////////////////////////////////////////////////
void ripple :: ripple_bang()
{
  int index = 0;
    
  while (m_t[index] < m_max[index] && index < RIPPLE_COUNT)    index++;
    
  if (index < RIPPLE_COUNT)    {
    m_cx[index] = (int)(1.0*m_ctrX/m_sizeX*m_gridX);
    m_cy[index] = (int)(1.0*m_ctrY/(m_sizeY+m_sizeY0)*m_gridY);
    m_t[index] = 4*RIPPLE_STEP;
    m_max[index] = ripple_max_distance(m_cx[index], m_cy[index]);
  }
}

/////////////////////////////////////////////////////////
// heightMess
//
/////////////////////////////////////////////////////////
void ripple :: heightMess(float height)
{
  m_height = height;
  setModified();
}
/////////////////////////////////////////////////////////
// ctrXMess
//
/////////////////////////////////////////////////////////
void ripple :: ctrXMess(float center)
{
  m_ctrX = (short)center;
  setModified();
}
/////////////////////////////////////////////////////////
// ctrYMess
//
/////////////////////////////////////////////////////////
void ripple :: ctrYMess(float center)
{
  m_ctrY = (short)center;
  setModified();
}

/////////////////////////////////////////////////////////
// typeMess
//
/////////////////////////////////////////////////////////
void ripple :: typeMess(t_symbol *type)
{
  if (!strcmp(type->s_name, "line")) 
    m_drawType = GL_LINE_LOOP;
  else if (!strcmp(type->s_name, "fill")) 
    m_drawType = GL_POLYGON;
  else if (!strcmp(type->s_name, "point"))
    m_drawType = GL_POINTS;
  else if (!strcmp(type->s_name, "default"))
    m_drawType = GL_POLYGON;
  else
    {
	    error ("unknown draw style?");
	    return;
    }
  setModified();
}
/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void ripple :: obj_setupCallback(t_class *classPtr)
{
  class_addbang(classPtr, reinterpret_cast<t_method>(&ripple::bangMessCallback));
  class_addmethod(classPtr, reinterpret_cast<t_method>(&ripple::heightMessCallback),
                  gensym("Ht"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&ripple::ctrXMessCallback),
                  gensym("cX"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&ripple::ctrYMessCallback),
                  gensym("cY"), A_FLOAT, A_NULL);
}

void ripple :: bangMessCallback(void *data)
{
  GetMyClass(data)->ripple_bang();
}
void ripple :: heightMessCallback(void *data, t_floatarg height)
{
  GetMyClass(data)->heightMess(height);
}
void ripple :: ctrXMessCallback(void *data, t_floatarg center)
{
  GetMyClass(data)->ctrXMess(center);
}
void ripple :: ctrYMessCallback(void *data, t_floatarg center)
{
  GetMyClass(data)->ctrYMess(center);
}
