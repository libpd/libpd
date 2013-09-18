////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// tigital@mac.com
//
// Implementation file
//
//    Copyright (c) 2004-2005 tigital
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "pqtorusknots.h"
#include "Gem/State.h"
#define PI2 3.1415926535897932384626433832795f*2

CPPEXTERN_NEW_WITH_TWO_ARGS(pqtorusknots, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT );

/////////////////////////////////////////////////////////
//
// pqtorusknots
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pqtorusknots :: pqtorusknots(t_floatarg width, t_floatarg q)
  : 
  GemShape(width), m_steps(256), m_facets(16), m_scale(1), m_thickness(0.2),
  m_clumps(12), m_clumpOffset(0), m_clumpScale(0.5), m_uScale(4), m_vScale(64), 
  m_P(2), m_Q(q),
  m_Vertex(NULL), m_Normal(NULL), m_texcoords(NULL), m_Index(NULL), 
  m_Indices(0), m_Vertices(0), 
  m_PrimitiveType(0)
{
  m_Texcoord[0] = m_Texcoord[1] = m_Texcoord[2] = m_Texcoord[3] = 0;

  if (m_P == 0.f) m_P = 1.f;
  if (m_Q == 0.f) m_Q = 0.f;
  m_drawType = GL_TRIANGLE_STRIP;
}

////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pqtorusknots :: ~pqtorusknots()
{
  delete[] m_Vertex; m_Vertex = NULL;
  delete[] m_Normal; m_Normal = NULL;
  delete[] m_Index;  m_Index = NULL;

  m_Indices = 0;  m_PrimitiveType = 0;

  int i,j;
  // clear tex coord arrays, guarding against double deletion
  for (i = 0; i < 4; i++)
    {
      delete[] m_Texcoord[i];
      for (j = i + 1; j < 4; j++)
        {
          if (m_Texcoord[j] == m_Texcoord[i])
            m_Texcoord[j] = NULL;
        }
      m_Texcoord[i] = NULL;
    }
}
////////////////////////////////////////////////////////
// set up vertices
//
/////////////////////////////////////////////////////////
void pqtorusknots :: genVert(){
  if(m_Vertex)   delete[]m_Vertex;    m_Vertex = NULL;
  if(m_Normal)   delete[]m_Normal;    m_Normal = NULL;
  if(m_Index)    delete[]m_Index;     m_Index = NULL;
  if(m_texcoords)delete[]m_texcoords; m_texcoords = NULL;

  int i, j;
  /* formerly this was "m_thickness*=m_scale" 
     which diminuished m_thickness each renderShape-cycle */
  GLfloat thickness = m_thickness * m_scale;

  m_Vertex   = new GLfloat[((m_steps + 1) * (m_facets + 1) + 1) * 3];
  m_Normal   = new GLfloat[((m_steps + 1) * (m_facets + 1) + 1) * 3];
  m_texcoords= new GLfloat[((m_steps + 1) * (m_facets + 1) + 1) * 2];
  m_Index    = new GLuint [ (m_steps + 1) *  m_facets * 2];

  for (i=0; i<4; i++)
    m_Texcoord[i] = m_texcoords;


  m_Indices = (m_steps + 1) * m_facets * 2;
  memset(m_Index, 0, m_Indices*sizeof(GLuint));

  m_Vertices = ((m_steps + 1) * (m_facets + 1) + 1);
  m_PrimitiveType = m_drawType;

  for (j = 0; j < m_facets; j++)
    {
      for (i = 0; i < m_steps + 1; i++)
        {
          m_Index[2 * (i + j * (m_steps + 1)) + 0] = ((j + 1) + i * (m_facets + 1));
          m_Index[2 * (i + j * (m_steps + 1)) + 1] = (j + i * (m_facets + 1));
        }
    }
  for (i = 0; i < m_steps; i++)
    {       

      int offsetF3= 3 * (i * (m_facets + 1) + m_facets);
      int offsetF2= 2 * (i * (m_facets + 1) + m_facets);
      int offset3 = 3 * (i * (m_facets + 1));
  
      float centerpoint[3];
      float Pp = m_P * i * PI2 / m_steps;
      float Qp = m_Q * i * PI2 / m_steps;
      float r = (.5f * (2 + sin(Qp))) * m_scale;
      centerpoint[0] = r * cos(Pp);
      centerpoint[1] = r * cos(Qp);
      centerpoint[2] = r * sin(Pp);
        
      float nextpoint[3];
      Pp = m_P * (i + 1) * PI2 / m_steps;
      Qp = m_Q * (i + 1) * PI2 / m_steps;
      r = (.5f * (2 + sin(Qp))) * m_scale;
      nextpoint[0] = r * cos(Pp);
      nextpoint[1] = r * cos(Qp);
      nextpoint[2] = r * sin(Pp);
        
      float T[3];
      T[0] = nextpoint[0] - centerpoint[0];
      T[1] = nextpoint[1] - centerpoint[1];
      T[2] = nextpoint[2] - centerpoint[2];

      float N[3];
      N[0] = nextpoint[0] + centerpoint[0];
      N[1] = nextpoint[1] + centerpoint[1];
      N[2] = nextpoint[2] + centerpoint[2];


      float B[3];
      B[0] = T[1]*N[2] - T[2]*N[1];
      B[1] = T[2]*N[0] - T[0]*N[2];
      B[2] = T[0]*N[1] - T[1]*N[0];

      N[0] = B[1]*T[2] - B[2]*T[1];
      N[1] = B[2]*T[0] - B[0]*T[2];
      N[2] = B[0]*T[1] - B[1]*T[0];

      float length;
      length = sqrt(B[0] * B[0] + B[1] * B[1] + B[2] * B[2]);
      B[0] /= length;
      B[1] /= length;
      B[2] /= length;

      length = sqrt(N[0] * N[0] + N[1] * N[1] + N[2] * N[2]);
      N[0] /= length;
      N[1] /= length;
      N[2] /= length;

      for (j = 0; j < m_facets; j++)
        {
          float pointx = sin(j * PI2 / m_facets) * thickness * ((sin(m_clumpOffset + m_clumps * i * PI2 / m_steps) * m_clumpScale) + 1);
          float pointy = cos(j * PI2 / m_facets) * thickness * ((cos(m_clumpOffset + m_clumps * i * PI2 / m_steps) * m_clumpScale) + 1);
          int offset3j= 3 * (i * (m_facets + 1) + j);
          int offset2j= 2 * (i * (m_facets + 1) + j);

          m_Vertex[offset3j + 0] = N[0] * pointx + B[0] * pointy + centerpoint[0];
          m_Vertex[offset3j + 1] = N[1] * pointx + B[1] * pointy + centerpoint[1];
          m_Vertex[offset3j + 2] = N[2] * pointx + B[2] * pointy + centerpoint[2];

          m_Normal[offset3j + 0] = m_Vertex[offset3j + 0] - centerpoint[0];
          m_Normal[offset3j + 1] = m_Vertex[offset3j + 1] - centerpoint[1];
          m_Normal[offset3j + 2] = m_Vertex[offset3j + 2] - centerpoint[2];

          float length;
          length = sqrt(m_Normal[offset3j + 0] * m_Normal[offset3j + 0] +
                               m_Normal[offset3j + 1] * m_Normal[offset3j + 1] +
                               m_Normal[offset3j + 2] * m_Normal[offset3j + 2]);
            
          m_Normal[offset3j + 0] /= length;
          m_Normal[offset3j + 1] /= length;
          m_Normal[offset3j + 2] /= length;

          m_texcoords[offset2j + 0] = (static_cast<float>(j) / m_facets) * m_uScale;
          m_texcoords[offset2j + 1] = (static_cast<float>(i) / m_steps ) * m_vScale;
        }
      // create duplicate vertex for sideways wrapping
      // otherwise identical to first vertex in the 'ring' except for the U coordinate
      m_Vertex[offsetF3 + 0] = m_Vertex[offset3 + 0];
      m_Vertex[offsetF3 + 1] = m_Vertex[offset3 + 1];
      m_Vertex[offsetF3 + 2] = m_Vertex[offset3 + 2];

      m_Normal[offsetF3 + 0] = m_Normal[offset3 + 0];
      m_Normal[offsetF3 + 1] = m_Normal[offset3 + 1];
      m_Normal[offsetF3 + 2] = m_Normal[offset3 + 2];

      m_texcoords[offsetF2 + 0] = m_uScale;
      m_texcoords[offsetF2 + 1] = m_texcoords[i * (m_facets + 1) * 2 + 1];

    }
  // create duplicate ring of vertices for longways wrapping
  // otherwise identical to first 'ring' in the knot except for the V coordinate
  for (j = 0; j < m_facets; j++)
    {
      int offset3 = 3 * (m_steps * (m_facets + 1) + j);
      int offset2 = 2 * (m_steps * (m_facets + 1) + j);

      m_Vertex[offset3 + 0] = m_Vertex[j * 3 + 0];
      m_Vertex[offset3 + 1] = m_Vertex[j * 3 + 1];
      m_Vertex[offset3 + 2] = m_Vertex[j * 3 + 2];

      m_Normal[offset3 + 0] = m_Normal[j * 3 + 0];
      m_Normal[offset3 + 1] = m_Normal[j * 3 + 1];
      m_Normal[offset3 + 2] = m_Normal[j * 3 + 2];

      m_texcoords[offset2 + 0] = m_texcoords[j * 2 + 0];
      m_texcoords[offset2 + 1] = m_vScale;
    }
  // finally, there's one vertex that needs to be duplicated due to both U and V coordinate.
  int offsetSF3 = 3 * (m_steps * (m_facets + 1) + m_facets);
  int offsetSF2 = 2 * (m_steps * (m_facets + 1) + m_facets);

  m_Vertex[offsetSF3 + 0] = m_Vertex[0];
  m_Vertex[offsetSF3 + 1] = m_Vertex[1];
  m_Vertex[offsetSF3 + 2] = m_Vertex[2];

  m_Normal[offsetSF3 + 0] = m_Normal[0];
  m_Normal[offsetSF3 + 1] = m_Normal[1];
  m_Normal[offsetSF3 + 2] = m_Normal[2];

  m_texcoords[offsetSF2 + 0] = m_uScale;
  m_texcoords[offsetSF2 + 1] = m_vScale;
}


////////////////////////////////////////////////////////
// renderShape
//
/////////////////////////////////////////////////////////
void pqtorusknots :: renderShape(GemState *state)
{
  if(m_modified)genVert();
  if(!m_Index)return;
  
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);

  glVertexPointer(3, GL_FLOAT, 0, m_Vertex);            
  glNormalPointer(GL_FLOAT, 0, m_Normal);

  if (GemShape::m_texType && GemShape::m_texNum)
    {
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      for (int i = 0; i < 4; i++)
        {
          if (m_Texcoord[i])
            {
              if(GLEW_ARB_multitexture) {
                glClientActiveTextureARB(GL_TEXTURE0_ARB + i);
              }
              glTexCoordPointer(2, GL_FLOAT, 0, m_Texcoord[i]);

            }
        }
    }
  glDrawElements(m_PrimitiveType,m_Indices,GL_UNSIGNED_INT,m_Index);
}
void pqtorusknots :: postrenderShape(GemState *state)
{
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
}

////////////////////////////////////////////////////////
// scaleMess
//
/////////////////////////////////////////////////////////
void pqtorusknots :: scaleMess(float size)
{
  m_scale = size;
  setModified();
}
////////////////////////////////////////////////////////
// stepsMess
//
/////////////////////////////////////////////////////////
void pqtorusknots :: stepsMess(float size)
{
  if (size<0)
    size = 0;
  m_steps = static_cast<GLint>(size);
  setModified();
}
////////////////////////////////////////////////////////
// facetsMess
//
/////////////////////////////////////////////////////////
void pqtorusknots :: facetsMess(float size)
{
  if(size<0)
    size=0;
  m_facets = static_cast<GLint>(size);
  setModified();
}
////////////////////////////////////////////////////////
// thicknessMess
//
/////////////////////////////////////////////////////////
void pqtorusknots :: thickMess(float size)
{
  m_thickness = size;
  setModified();
}
////////////////////////////////////////////////////////
// clumpMess
//
/////////////////////////////////////////////////////////
void pqtorusknots :: clumpMess(float clump, float clumpOffset, float clumpScale)
{
  m_clumps = clump;
  m_clumpOffset = clumpOffset;
  m_clumpScale = clumpScale;
  setModified();
}
////////////////////////////////////////////////////////
// uvScaleMess
//
/////////////////////////////////////////////////////////
void pqtorusknots :: uvScaleMess(float uScale, float vScale)
{
  m_uScale = uScale;
  m_vScale = vScale;
  setModified();
}
////////////////////////////////////////////////////////
// pqMess
//
/////////////////////////////////////////////////////////
void pqtorusknots :: pqMess(float p, float q)
{
  m_P = p;
  m_Q = q;
  setModified();
}

////////////////////////////////////////////////////////
// typeMess
//
/////////////////////////////////////////////////////////
void pqtorusknots :: typeMess(t_symbol *type)
{
  char c=*type->s_name;
  switch(c){
  case 'l': case 'L':   m_drawType = GL_LINE_LOOP; break;
  case 'f': case 'F':   //m_drawType = GL_QUADS; break;
  case 'd': case 'D':   m_drawType = GL_TRIANGLE_STRIP; break;// default
  case 'p': case 'P':   m_drawType = GL_POINTS; break;
  default:
    error ("GEM: square draw style");
    return;
  }
  setModified();
}

////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pqtorusknots :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pqtorusknots::scaleMessCallback),
                  gensym("scale"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pqtorusknots::stepsMessCallback),
                  gensym("steps"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pqtorusknots::facetsMessCallback),
                  gensym("facets"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pqtorusknots::thickMessCallback),
                  gensym("thick"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pqtorusknots::clumpMessCallback),
                  gensym("clump"), A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pqtorusknots::uvScaleMessCallback),
                  gensym("uvScale"), A_FLOAT, A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pqtorusknots::pqMessCallback),
                  gensym("pq"), A_FLOAT, A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pqtorusknots::typeMessCallback),
                  gensym("type"), A_SYMBOL, A_NULL);
}

void pqtorusknots :: scaleMessCallback(void *data, t_floatarg size)
{
  GetMyClass(data)->scaleMess(size);  
}

void pqtorusknots :: stepsMessCallback(void *data, t_floatarg size)
{
  GetMyClass(data)->stepsMess(size);  
}
void pqtorusknots :: facetsMessCallback(void *data, t_floatarg size)
{
  GetMyClass(data)->facetsMess(size);  
}
void pqtorusknots :: thickMessCallback(void *data, t_floatarg size)
{
  GetMyClass(data)->thickMess(size);  
}
void pqtorusknots :: clumpMessCallback(void *data, t_floatarg posX, t_floatarg posY, t_floatarg valforce )
{
  GetMyClass(data)->clumpMess(posX, posY, valforce);
}
void pqtorusknots :: uvScaleMessCallback(void *data, t_floatarg posX, t_floatarg posY)
{
  GetMyClass(data)->uvScaleMess(posX, posY);
}
void pqtorusknots :: pqMessCallback(void *data, t_floatarg posX, t_floatarg posY)
{
  GetMyClass(data)->pqMess(posX, posY);
}
void pqtorusknots :: typeMessCallback(void *data, t_symbol*s)
{
  GetMyClass(data)->typeMess(s);
}
