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

#include "vertex_combine.h"

#include "Gem/State.h"
#include "string.h"
#include "math.h"
CPPEXTERN_NEW(vertex_combine);
 
/////////////////////////////////////////////////////////
//
// vertex_combine
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
vertex_combine :: vertex_combine()
{      
  m_blend = 0.f;
  //hopefully this gets us a right inlet that accepts another gem state
  m_inlet = inlet_new(this->x_obj, &this->x_obj->ob_pd,gensym("gem_state"), gensym("gem_right"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
vertex_combine :: ~vertex_combine()
{ }

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void vertex_combine :: render(GemState *state)
{
  int i,size,srcL,srcS,count,sizeR,ratio,remainder;
    GLfloat *VertexArray;
    float blendL, blendR, ratiof,countf;
    
    VertexArray =state->VertexArray;
    if (state->VertexArray == NULL || state->VertexArraySize <= 0){
        post("no vertex array!");
        return;
    }
    
    if (state->ColorArray == NULL ){
        post("no color array!");
        return;
    }
    
    size = state->VertexArraySize;
    sizeR = m_vertCountR;    
    
    //dumb and dirty x-fade
    blendR = m_blend;
    blendL = fabs(1.f - m_blend);
   
    if (size > sizeR){
        //left is larger
        ratiof = static_cast<float>(size)/sizeR;
        ratio = size / sizeR;
        remainder = size % sizeR;
       // post("float ratio %f:1 int ratio %d:1 remainder %d",ratiof,ratio,remainder);
        i = 0;
        srcL = 0;
        srcS = 0;
        countf = 0.0;
        while (i < sizeR) {
            count = 0;
            while (count < ratio){
                VertexArray[srcL] = (VertexArray[srcL] * blendL) + (m_rightVertexArray[srcS] * blendR);
                VertexArray[srcL+1] = (VertexArray[srcL+1] * blendL) + (m_rightVertexArray[srcS+1] * blendR);
                VertexArray[srcL+2] = (VertexArray[srcL+2] * blendL) + (m_rightVertexArray[srcS+2] * blendR);
                VertexArray[srcL+3] = (VertexArray[srcL+3] * blendL )+ (m_rightVertexArray[srcS+3] * blendR);
                srcL+=4;
                count++;
            }
            i++;
            srcS+=4;
        }
        state->VertexArraySize = size;
    }else{
      ratiof = static_cast<float>(sizeR)/size;
        ratio = sizeR / size;
        remainder = sizeR % size;
        post("float ratio %f:1 int ratio %d:1 remainder %d",ratiof,ratio,remainder);
    }
   
    /* -- this almost works - good for fast and dirty integer ratios
    if (size > sizeR){
        //left is larger
        ratiof = static_cast<float>(size)/sizeR;
        ratio = size / sizeR;
        remainder = size % sizeR;
       // post("float ratio %f:1 int ratio %d:1 remainder %d",ratiof,ratio,remainder);
        i = 0;
        srcL = 0;
        srcS = 0;
        while (i < sizeR) {
            count = 0;
            while (count < ratio){
                VertexArray[srcL] = (VertexArray[srcL] * blendL) + (m_rightVertexArray[srcS] * blendR);
                VertexArray[srcL+1] = (VertexArray[srcL+1] * blendL) + (m_rightVertexArray[srcS+1] * blendR);
                VertexArray[srcL+2] = (VertexArray[srcL+2] * blendL) + (m_rightVertexArray[srcS+2] * blendR);
                VertexArray[srcL+3] = (VertexArray[srcL+3] * blendL )+ (m_rightVertexArray[srcS+3] * blendR);
                srcL+=4;
                count++;
            }
            i++;
            srcS+=4;
        }
        state->VertexArraySize = size;
    }else{
        ratiof = static_cast<float>(sizeR)/size;
        ratio = sizeR / size;
        remainder = sizeR % size;
        post("float ratio %f:1 int ratio %d:1 remainder %d",ratiof,ratio,remainder);
    }
    */
  
}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void vertex_combine :: postrender(GemState *state)
{
//m_vertNum = 0;
}
 
/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void vertex_combine :: rightRender(GemState *state)
{
if (state->VertexArray == NULL || state->VertexArraySize <= 0){
        post("no right vertex array!");
        return;
    }
    
    if (state->ColorArray == NULL ){
        post("no right color array!");
    }

    m_vertCountR = state->VertexArraySize;
    m_rightVertexArray = state->VertexArray;
    m_rightColorArray = state->ColorArray;

    //post("state->VertexArraySize %d", state->VertexArraySize);
    //post("rightRender");
} 
 
 
/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void vertex_combine :: obj_setupCallback(t_class *classPtr)
{        
    class_addmethod(classPtr, reinterpret_cast<t_method>(&vertex_combine::gem_rightMessCallback),
    	    gensym("gem_right"), A_GIMME, A_NULL);
            
    class_addmethod(classPtr, reinterpret_cast<t_method>(&vertex_combine::blendCallback),
    	    gensym("blend"), A_FLOAT, A_NULL);
}
void vertex_combine :: gem_rightMessCallback(void *data, t_symbol *s, int argc, t_atom *argv)
{
  if (argc==1 && argv->a_type==A_FLOAT){
  } else if (argc==2 && argv->a_type==A_POINTER && (argv+1)->a_type==A_POINTER){
    GetMyClass(data)->m_cacheRight = reinterpret_cast<GemCache*>(argv->a_w.w_gpointer);
    GetMyClass(data)->rightRender(reinterpret_cast<GemState*>((argv+1)->a_w.w_gpointer));
  } else GetMyClass(data)->error("wrong righthand arguments....");
}

void vertex_combine :: blendCallback(void *data, t_floatarg x)
{
  GetMyClass(data)->m_blend = x;
}
