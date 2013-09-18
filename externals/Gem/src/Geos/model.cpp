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

#include "model.h"
 #include "Gem/State.h"

CPPEXTERN_NEW_WITH_ONE_ARG(model, t_symbol *, A_DEFSYM);

  /////////////////////////////////////////////////////////
//
// model
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
model :: model(t_symbol *filename)
  : m_model(0), m_dispList(0), 
    m_rescaleModel(1), m_smooth(90), m_material(0),
    m_flags(GLM_SMOOTH | GLM_TEXTURE),
    m_group(0),
    m_rebuild(true),
    m_currentH(1.f), m_currentW(1.f),
    m_textype(GLM_TEX_DEFAULT)
{
  // make sure that there are some characters
  if (filename&&filename->s_name&&*filename->s_name) openMess(filename);
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
model :: ~model()
{
  cleanModel();
}

/////////////////////////////////////////////////////////
// cleanModel
//
/////////////////////////////////////////////////////////
void model :: cleanModel()
{
  if (m_dispList)
    {
      // destroy display list
      glDeleteLists(m_dispList, 1);
      m_dispList = 0;
    }
  if(m_model) {
    glmDelete(m_model);
    m_model=NULL;
  }

}

/////////////////////////////////////////////////////////
// materialMess
//
/////////////////////////////////////////////////////////
void model :: materialMess(int material)
{
  if (!m_model) return;
  m_material = material;
  switch (material) {
  case 0:
    m_flags=GLM_SMOOTH | GLM_TEXTURE;
    break;
  default:
    m_flags=GLM_SMOOTH | GLM_TEXTURE | GLM_MATERIAL;
  }
  buildList();
}

/////////////////////////////////////////////////////////
// materialMess
//
/////////////////////////////////////////////////////////
void model :: textureMess(int state)
{
  switch(state) {
  case 0: 
    m_textype=GLM_TEX_LINEAR; 
    break;
  case 1: 
    m_textype=GLM_TEX_SPHEREMAP; 
    break;
  case 2:
    m_textype=GLM_TEX_UV; 
    break;
  default:
    m_textype=GLM_TEX_DEFAULT; 
  }
  m_rebuild=true;
}

/////////////////////////////////////////////////////////
// smoothMess
//
/////////////////////////////////////////////////////////
void model :: smoothMess(t_float fsmooth)
{
  if (!m_model) return;
  if (fsmooth<0.)fsmooth=0.;
  else if (fsmooth>1) fsmooth=1.;
  m_smooth = fsmooth*180.;
  glmVertexNormals(m_model, m_smooth);
  buildList();
}

/////////////////////////////////////////////////////////
// rescaleMess
//
/////////////////////////////////////////////////////////
void model :: reverseMess(int reverse)
{
  if (!m_model) return;
  glmReverseWinding(m_model);
  buildList();
}
/////////////////////////////////////////////////////////
// matrialMess
//
/////////////////////////////////////////////////////////
void model :: rescaleMess(int state)
{
  m_rescaleModel = state;
}

/////////////////////////////////////////////////////////
// matrialMess
//
/////////////////////////////////////////////////////////
void model :: groupMess(int state)
{
  m_group = state;
  buildList();
}


/////////////////////////////////////////////////////////
// openMess
//
/////////////////////////////////////////////////////////
void model :: openMess(t_symbol *filename)
{
  cleanModel();
    
  char buf[MAXPDSTRING];
  canvas_makefilename(const_cast<t_canvas*>(getCanvas()), filename->s_name, buf, MAXPDSTRING);
  // read the object in
  m_model = glmReadOBJ(buf);
  if (!m_model){
      error("unable to read model '%s'", buf);
      return;
  }

  // set the size to -1 to 1
  //
  if (m_rescaleModel)
    glmUnitize(m_model);

  // generate normals if this
  // object doesn't have them.
  //
  glmFacetNormals (m_model);
  glmVertexNormals(m_model, m_smooth);

  glmTexture(m_model, m_textype, m_currentH, m_currentW);
  buildList();
  this->setModified();
}

/////////////////////////////////////////////////////////
// buildList
//
/////////////////////////////////////////////////////////
void model :: buildList()
{
  if (!m_model) return;
  if(!(GLEW_VERSION_1_1)) {
    logpost(NULL, 5, "cannot build display-list now...do you have a window?");
    return;
  }
  if (m_dispList)glDeleteLists(m_dispList, 1);
  //  m_flags = GLM_SMOOTH | GLM_MATERIAL;
  if (!m_group){
    m_dispList = glmList(m_model, m_flags);
  }
  else
  {
    m_dispList = glmListGroup(m_model, m_flags,m_group);
  }
}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void model :: render(GemState *state)
{
  if (state && (m_currentH != state->texCoordX(2) || m_currentW != state->texCoordY(2)))
    {
      m_rebuild=true;
    }
  if(m_rebuild) {
    m_currentH = state->texCoordX(2);
    m_currentW = state->texCoordY(2);
    glmTexture(m_model, m_textype, m_currentH, m_currentW);
    buildList();
    m_rebuild=false;
  }
  if (!m_dispList)return;
  glCallList(m_dispList);
}

void model :: startRendering()
{
  // build a display list
  buildList();
}


/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void model :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr, reinterpret_cast<t_method>(&model::openMessCallback),
		  gensym("open"), A_SYMBOL, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&model::rescaleMessCallback),
		  gensym("rescale"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&model::smoothMessCallback),
		  gensym("smooth"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&model::reverseMessCallback),
		  gensym("revert"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&model::materialMessCallback),
		  gensym("material"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&model::textureMessCallback),
		  gensym("texture"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&model::groupMessCallback),
		  gensym("group"), A_FLOAT, A_NULL);

}
void model :: openMessCallback(void *data, t_symbol *filename)
{
  GetMyClass(data)->openMess(filename);
}
void model :: rescaleMessCallback(void *data, t_floatarg state)
{
  GetMyClass(data)->rescaleMess(static_cast<int>(state));
}
void model :: smoothMessCallback(void *data, t_floatarg smooth)
{
  GetMyClass(data)->smoothMess(smooth);
}
void model :: reverseMessCallback(void *data, t_floatarg state)
{
  GetMyClass(data)->reverseMess(static_cast<int>(state));
}
void model :: textureMessCallback(void *data, t_floatarg state)
{
  GetMyClass(data)->textureMess(static_cast<int>(state));
}
void model :: materialMessCallback(void *data, t_floatarg state)
{
  GetMyClass(data)->materialMess(static_cast<int>(state));
}

void model :: groupMessCallback(void *data, t_floatarg state)
{
  GetMyClass(data)->groupMess(static_cast<int>(state));
}
