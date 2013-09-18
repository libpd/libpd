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
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////
#define HELPSYMBOL "scopeXYZ~"

#include "scopeXYZ~.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Gem/State.h"

#define MARK startpost("%s:%d[%s] ", __FILE__, __LINE__, __FUNCTION__); post

CPPEXTERN_NEW_WITH_ONE_ARG(scopeXYZ, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// scopeXYZ
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
scopeXYZ :: scopeXYZ(t_floatarg len)
  : GemShape(), 
    m_drawType(GL_LINE_STRIP),
    m_requestedLength(0), m_realLength(0), m_length(0), 
    m_position(0),
    m_vertices(NULL)
{
  doLengthMess(64);
  lengthMess(static_cast<int>(len));

  /* channels inlet */
  int i;
  for (i=0; i<3; i++)
    inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_signal, &s_signal);
}

////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
scopeXYZ :: ~scopeXYZ()
{
  if(m_vertices)delete[]m_vertices;
}

void scopeXYZ :: doLengthMess(unsigned int L) {
  // this resizes to m_requestedLength if this is set, or to L otherwise
  // actually, resizing is done to the double-size!

  unsigned int length=0;
  if(m_requestedLength>0)
    length=m_requestedLength;
  else if (L>m_requestedLength)
    length=L;

  if(0==length)return; // oops

  m_length=length;

  //post("length=%d\treal=%d\treqested=%d", m_length, m_realLength, m_requestedLength);

  if(m_realLength<length) {
    if(m_vertices)delete[]m_vertices;
    m_realLength=length;

    m_vertices = new t_sample[3* length*2];
    //post("m_vertices: %d*3*2 samples at %x", length, m_vertices);

    unsigned int i;
    for (i = 0; i < 3*length*2; i++)  {
      m_vertices[i]=0.0f;
    }
  }

  m_position%=m_length;
}


void scopeXYZ :: lengthMess(int l)
{
  if(l<=0){
    m_requestedLength=0;
    return;
  }

  m_requestedLength=l;
  doLengthMess();
}

/////////////////////////////////////////////////////////
// renderShape
//
/////////////////////////////////////////////////////////
void scopeXYZ :: renderShape(GemState *state)
{
  t_sample*vertices=m_vertices+3*m_position;
  int count=m_length/2;
  GLenum typ=GL_FLOAT;
  if(sizeof(t_sample)==sizeof(double))
    typ=GL_DOUBLE;

  GLenum drawtype=m_drawType;
  if(drawtype==GL_DEFAULT_GEM)drawtype=GL_LINE_STRIP;
  glNormal3f(0.0f, 0.0f, 1.0f);
  glLineWidth(m_linewidth);

  // activate and specify pointer to vertex array
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, typ, 0, vertices);

  if(GemShape::m_texType) {
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    if(count==GemShape::m_texNum) {
      // in the original code, we used whatever GemShape::m_texCoords were present
      // if we had more vertices, the remaining vs would just use the last texCoord available
      glTexCoordPointer(2, GL_FLOAT, 0, GemShape::m_texCoords);
    } else {
      // in the original code, the texcoords where normalized(!)
      glTexCoordPointer(2, typ, sizeof(t_sample), vertices);
    }
  }

  // draw a cube
  glDrawArrays(drawtype, 0, m_length);

  // deactivate vertex arrays after drawing
  glDisableClientState(GL_VERTEX_ARRAY);

  if(GemShape::m_texType) {
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  }

  glLineWidth(1.0);
}

void scopeXYZ :: bangMess(void){
  unsigned int i;
  t_float*vertL=m_vertices;
  t_float*vertR=m_vertices+m_length;

  post("x\ty\tz\t\tx\ty\tz");
  for(i=0; i<m_length; i++) {
    post("%f\t%f\t%f\t\t%f\t%f\t%f",
         vertL[i*3+0],
         vertL[i*3+1],
         vertL[i*3+2],

         vertR[i*3+0],
         vertR[i*3+1],
         vertR[i*3+2]);
  }
}


/////////////////////////////////////////////////////////
// linewidthMess
//
/////////////////////////////////////////////////////////
void scopeXYZ :: linewidthMess(float linewidth)
{
  m_linewidth = (linewidth < 0.0f) ? 0.0f : linewidth;
  setModified();
}

/////////////////////////////////////////////////////////
// typeMess
//
/////////////////////////////////////////////////////////
void scopeXYZ :: typeMess(t_symbol *type)
{
  char*s=type->s_name;
  char c=*s;
  switch(c){
  case 'd': // default
    m_drawType = GL_DEFAULT_GEM;
    break;
  case 'f': // fill
    m_drawType = GL_POLYGON;
    break;
  case 'p': // point
    m_drawType = GL_POINTS;
    break;
  case 'l': 
    { // line, linestrip
      char c2=s[4];
      switch(c2){
      case 's':
        if(s[5])
          m_drawType = GL_LINE_STRIP;
        else
          m_drawType = GL_LINES;
        break;
      default:
        m_drawType = GL_LINE_LOOP;
        break;
      }
    }
    break;
  case 't':
    { // tri, tristrip, trifan
      char c2=s[3];
      switch(c2){
      case 's':
        m_drawType = GL_TRIANGLE_STRIP;
        break;
      case 'f':
        m_drawType = GL_TRIANGLE_FAN;
        break;
      default:
        m_drawType = GL_TRIANGLES;
        break;
      }
    }
    break;
  case 'q': 
    { // quad, quadstrip
      char c2=s[4];
      switch(c2){
      case 's':
        m_drawType = GL_QUAD_STRIP;
        break;
      default:
        m_drawType = GL_QUADS;
        break;
      }
    }
    break;
  default:
    error ("draw style");
    return;
  }
  setModified();
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void scopeXYZ :: obj_setupCallback(t_class *classPtr)
{
  class_addcreator(reinterpret_cast<t_newmethod>(create_scopeXYZ), 
                   gensym("scopeXYZ~"), A_DEFFLOAT, A_NULL);

  class_addmethod(classPtr, reinterpret_cast<t_method>(&scopeXYZ::linewidthMessCallback),
                  gensym("linewidth"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&scopeXYZ::lengthMessCallback),
                  gensym("length"), A_FLOAT, A_NULL);

  class_addmethod(classPtr, reinterpret_cast<t_method>(&scopeXYZ::dspCallback), 
                  gensym("dsp"), A_NULL);
  class_addmethod(classPtr, nullfn, gensym("signal"), A_NULL);

  class_addbang(classPtr, reinterpret_cast<t_method>(&scopeXYZ::bangCallback));
}
void scopeXYZ :: bangCallback(void *data)
{
  GetMyClass(data)->bangMess();
}

void scopeXYZ :: linewidthMessCallback(void *data, t_floatarg linewidth)
{
  GetMyClass(data)->linewidthMess(linewidth);
}
void scopeXYZ :: lengthMessCallback(void *data, t_floatarg l)
{
  GetMyClass(data)->lengthMess(static_cast<int>(l));
}
void scopeXYZ ::  dspCallback(void *data,t_signal** sp)
{
  GetMyClass(data)->doLengthMess(sp[1]->s_n);
  dsp_add(perform, 5, data, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[1]->s_n);
}

void scopeXYZ :: perform(unsigned int count, t_sample*X, t_sample*Y, t_sample*Z)
{
  int position=m_position;

  t_float*vert=m_vertices+3*position;
  unsigned int i=0;

  // TODO: add some protection against segfaults when bufer is very small
  if(m_length<count)
    count=m_length;
  
  /* fill in the left-side of the double-array */
  for(i=0; i<count; i++) {
    *vert++=*X++;
    *vert++=*Y++;
    *vert++=*Z++;
  }
  /* copy the data over to the right-side of the double-array */
  if((m_position+count)>m_length) {
    // wrap around!
    // 1st copy the lower half
    memcpy(m_vertices+3*(position+m_length),
           m_vertices+3*(position),
           3*(m_length-m_position)*sizeof(t_sample));
    // then the upper half
    memcpy(m_vertices,
           m_vertices+3*(m_length),
           3*(m_position+count-m_length)*sizeof(t_sample));

  } else {
    // ordinary copy
  
    memcpy(m_vertices+3*(position+m_length),
           m_vertices+3*(position),
           3*count*sizeof(t_sample));
  }
  m_position=(m_position+count)%m_length;

  setModified();
}

t_int* scopeXYZ :: perform(t_int* w)
{
  int index=1;
  scopeXYZ *x = GetMyClass(reinterpret_cast<void*>(w[index++]));
  t_sample* in_X = reinterpret_cast<t_sample*>(w[index++]);
  t_sample* in_Y = reinterpret_cast<t_sample*>(w[index++]);
  t_sample* in_Z = reinterpret_cast<t_sample*>(w[index++]);

  t_int n = (w[index++]);

  x->perform(n, in_X, in_Y, in_Z);

  return (w+index);
}
