////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-2000 Mark Danks.
//    Copyright (c) GÂžnther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "vertex_quad.h"

#include "Gem/State.h"
#include "string.h"
CPPEXTERN_NEW(vertex_quad);

/////////////////////////////////////////////////////////
//
// vertex_quad
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
vertex_quad :: vertex_quad()
{
    m_blend=0;
    m_VertexArray = new float [16];
    m_ColorArray = new float [16];
    m_TexCoordArray = new float [8];
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
vertex_quad :: ~vertex_quad()
{ }

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void vertex_quad :: render(GemState *state)
{
  
   static GLfloat	quad[16]= {-1.,-1.,0.,1.,
                        1.,-1.,0.,1.,
                        1.,1.,0.,1.,
                        -1.,1.,0.,1.};
                        
    static GLfloat	color[16]=   {1.,1.,1.,1.,
                            1.,1.,1.,1.,
                            1.,1.,1.,1.,
                            1.,1.,1.,1.};
                            
    GLfloat		texcoord[8] = {	0.0,1.0,
                                         1.0,1.0,
                                        1.0,0.0,
                                        0.0,0.0
                                        };
   /*
   coords[3].s = 0.f;		// switched the order of coords on __APPLE__
      coords[3].t = 0.f;		// otherwise we'd be upside down!
      coords[2].s = xRatio;
      coords[2].t = 0.f;
      coords[1].s = xRatio;
      coords[1].t = yRatio;
      coords[0].s = 0.f;
      coords[0].t = yRatio;*/
      
      if (state->numTexCoords){
        texcoord[0] = state->texCoordX(0);
        texcoord[1] = state->texCoordY(0);
        texcoord[2] = state->texCoordX(1);
        texcoord[3] = state->texCoordY(1);
        texcoord[4] = state->texCoordX(2);
        texcoord[5] = state->texCoordY(2);
        texcoord[6] = state->texCoordX(3);
        texcoord[7] = state->texCoordY(3);
      }
      
   
    memcpy(m_VertexArray, quad, sizeof(quad));
    memcpy(m_ColorArray, color, sizeof(color));
    memcpy(m_TexCoordArray, texcoord, sizeof(texcoord));
   //state->VertexArray = 0;
   //state->ColorArray = 0;
   
   //m_VertexArray = quad;
   //m_ColorArray = color;
   
   state->VertexArray = m_VertexArray;
   state->ColorArray = m_ColorArray;
   state->TexCoordArray = m_TexCoordArray;
   
   //let vertex draaw know which arrays to draw
   state->HaveColorArray = 1;
   state->HaveTexCoordArray = 1;
   
        
   //state->VertexArray = quad;
   // state->ColorArray = color;
    state->VertexArrayStride = 4;
    state->VertexArraySize = 4;
      
    //  delete [] color;
    //  delete [] quad;
      
   
   
}
 
/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void vertex_quad :: obj_setupCallback(t_class *classPtr)
{     class_addmethod(classPtr, reinterpret_cast<t_method>(&vertex_quad::blendMessCallback),
    	    gensym("blend"), A_FLOAT, A_NULL);
}

void vertex_quad :: blendMessCallback(void *data, t_floatarg size)
{
  GetMyClass(data)->m_blend=static_cast<int>(size);
}
