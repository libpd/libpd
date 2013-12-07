////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
//    Copyright (c) 1997-2000 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "part_render.h"

#include <string.h>


#include "papi.h"

CPPEXTERN_NEW(part_render);

/////////////////////////////////////////////////////////
//
// part_render
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
part_render :: part_render()
{
  m_colorize=true;
  m_sizing  =true;
  m_number=1000;
  m_colors = new GLfloat[m_number*4];
  m_sizes  = new GLfloat[m_number*3];
  m_pos    = new GLfloat[m_number*3];
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
part_render :: ~part_render()
{ }

/////////////////////////////////////////////////////////
// renderParticles
//
/////////////////////////////////////////////////////////
void part_render :: renderParticles(GemState *state)
{
  if (m_tickTime > 0.f)    {
    pMove();
  }
  //	pDrawGroupp();
  int cnt = pGetGroupCount();
  if(cnt < 1)return;
  if (cnt>m_number){
    if(m_colors)delete[]m_colors;
    if(m_sizes) delete[]m_sizes;
    if(m_pos)   delete[]m_pos;
    m_number = cnt;

    m_colors = new GLfloat[m_number * 4];
    m_sizes  = new GLfloat[m_number * 3];
    m_pos    = new GLfloat[m_number * 3];
  }
  GLfloat *position = m_pos;
  GLfloat *color = m_colorize ?  m_colors : NULL;
  GLfloat *size = m_sizing ? m_sizes : NULL;
  pGetParticles(0, cnt, position, color, NULL, size);
  for(int i = 0; i < cnt; i++)	{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(position[0], position[1], position[2]);
    position+=3;
    if(color!=NULL){
      glColor4fv((GLfloat *)&color[i*4]);
      //post("%d color: %f %f %f", i, color[0], color[1], color[2], color[3]);
    }
    if(size!=NULL){
      glScalef(size[0], size[1], size[2]);
      //      post("%d size: %f %f %f", i, size[0], size[1], size[2]);
      size+=3;
    }
    if(i<(cnt-1)){
      continueRender(state);
      glMatrixMode(GL_MODELVIEW);
      glPopMatrix();
    }
  }
}
void part_render :: postrender(GemState*){
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

/////////////////////////////////////////////////////////
// typeMess
void part_render :: colorMess(int state)
{
  m_colorize=!(!state);
}
void part_render :: sizeMess(int state)
{
  m_sizing=!(!state);
}


/////////////////////////////////////////////////////////
// static member functions
//
/////////////////////////////////////////////////////////
void part_render :: obj_setupCallback(t_class *classPtr)
{
   class_addmethod(classPtr, reinterpret_cast<t_method>(&part_render::colorMessCallback),
    	    gensym("colorize"), A_FLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&part_render::sizeMessCallback),
    	    gensym("size"), A_FLOAT, A_NULL);
}
void part_render :: colorMessCallback(void *data,  t_floatarg f)
{
    GetMyClass(data)->colorMess((int)f);
}
void part_render :: sizeMessCallback(void *data,  t_floatarg f)
{
    GetMyClass(data)->sizeMess((int)f);
}
