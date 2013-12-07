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
#include "multimodel.h"
#include "Gem/State.h"
#include <stdio.h>

CPPEXTERN_NEW_WITH_FOUR_ARGS(multimodel, t_symbol *, A_DEFSYM, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

  multimodel::multiModelCache *multimodel::s_modelCache = NULL;

/////////////////////////////////////////////////////////
//
// multimodel
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
multimodel :: multimodel(t_symbol *filename, t_floatarg baseModel,
			 t_floatarg topModel, t_floatarg skipRate)
  : m_loadedCache(NULL), 
    m_numModels(0), m_curModel(-1), 
    m_rescaleModel(1),
    m_textype(GLM_TEX_DEFAULT),
    m_rebuild(true),
    m_currentH(1.f), m_currentW(1.f)
{
  inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("mdl_num"));

  // make sure that there are some characters
  if (filename->s_name[0]) { 
    int skipRatei=static_cast<int>(skipRate);
    int topModeli=static_cast<int>(topModel);
    int baseModeli=static_cast<int>(baseModel);
    if (skipRatei == 0) {
      if (topModeli == 0)
        openMess(filename, 0, baseModeli, 1);
      else
        openMess(filename, baseModeli, topModeli, 1);
    }
    else openMess(filename, baseModeli, topModeli, skipRatei);
  }
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
multimodel :: ~multimodel()
{
  cleanMultimodel();
}

/////////////////////////////////////////////////////////
// cleanMultimodel
//
/////////////////////////////////////////////////////////
void multimodel :: cleanMultimodel()
{
  if (m_numModels) {
    // decrement the reference count
    m_loadedCache->refCount--;
    
    // If the refCount == 0, then destroy the cache
    if (m_loadedCache->refCount == 0) {
      // find the cache
      multiModelCache *ptr = s_modelCache;
      
      // if the loaded cache is the first cache in the list
      if (m_loadedCache == s_modelCache) {
        s_modelCache = m_loadedCache->next;
        delete m_loadedCache;
      }
      else {
        while (ptr && ptr->next != m_loadedCache) ptr = ptr->next;
        if (!ptr) error("unable to find model cache!");
        else {
          ptr->next = m_loadedCache->next;
          delete m_loadedCache;
        }
      }
    }
    
    m_loadedCache = NULL;
    m_numModels = 0;
  }
}

/////////////////////////////////////////////////////////
// openMess
//
/////////////////////////////////////////////////////////
void multimodel :: openMess(t_symbol *filename, int baseModel, int topModel, int skipRate)
{
  cleanMultimodel();
    
  if (!topModel) {
    error("requires an int for number of models");
    return;
  }
  if (baseModel > topModel) {
    error("top range less than base model");
    return;
  }
  if (skipRate < 1) skipRate = 1;

  // have we already loaded the model?
  multiModelCache *cache = s_modelCache;
  int found = 0;
  while (!found && cache) {
    if (baseModel == cache->baseModel &&
	topModel == cache->topModel &&
	skipRate == cache->skipRate &&
	!strcmp(filename->s_name, cache->modelName)) found = 1;
    else cache = cache->next;
  }
    
  // yep, we have it
  if (found) {
    m_loadedCache = cache;
    m_loadedCache->refCount++;
    m_curModel = 0;
    m_numModels = m_loadedCache->numModels;
    post("loaded models: %s from %d to %d skipping %d",
	 filename->s_name, baseModel, topModel, skipRate);
    return;
  }

  // nope, so create the new cache
  // find the * in the filename    
  char preName[256];
  char postName[256];
    
  int i = 0;
  char *strPtr = filename->s_name;
  while (strPtr[i] && strPtr[i] != '*') {
    preName[i] = strPtr[i];
    i++;
  }
    
  if (!strPtr[i]) {
    error("unable to find * in file name");
    return;
  }

  preName[i] = '\0';    
  strcpy(postName, &(strPtr[i+1]));
    
  // need to figure out how many filenames there are to load
  m_numModels = (topModel + 1 - baseModel) / skipRate;

  // create the new cache
  multiModelCache *newCache = new multiModelCache(filename->s_name);
  newCache->models = new GLint[m_numModels];
  newCache->realmodels = new GLMmodel*[m_numModels];
  newCache->numModels = m_numModels;
  newCache->baseModel = baseModel;
  newCache->topModel = topModel;
  newCache->skipRate = skipRate;

  int realNum = baseModel;
  char bufName[MAXPDSTRING];
  canvas_makefilename(const_cast<t_canvas*>(getCanvas()), preName, bufName, MAXPDSTRING);

  for (i = 0; i < m_numModels; i++, realNum += skipRate) {
    char newName[256];
    sprintf(newName, "%s%d%s", bufName, realNum, postName);
    
    // read the object in
    GLMmodel *m_model = glmReadOBJ(newName);
    if (!m_model) {
      // a load failed, blow away the cache
      newCache->numModels = i;
      delete newCache;
      m_numModels = 0;
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
    glmVertexNormals(m_model, 90); /* SMOOTH */
    
    glmTexture(m_model, m_textype, 1, 1);
    newCache->realmodels[i]=m_model;
  }
  m_curModel = 0;

  m_loadedCache = newCache;
  newCache->refCount++;

  // insert the cache at the end of the linked list
  multiModelCache *ptr = s_modelCache;
    
  if (!ptr) s_modelCache = newCache;
  else {
    while(ptr->next) ptr = ptr->next;
    ptr->next = newCache;
  }

  post("loaded models: %s %s from %d to %d skipping %d",
       bufName, postName, baseModel, topModel, skipRate);
  setModified();
}
/////////////////////////////////////////////////////////
// buildList
//
/////////////////////////////////////////////////////////
void multimodel :: buildList()
{
  int i = m_numModels;
  if (m_numModels && m_loadedCache)
    while(i--)glDeleteLists(m_loadedCache->models[i], 1);

  i=0;
  while(i<m_numModels){
    glmTexture(m_loadedCache->realmodels[i], m_textype, m_currentW, m_currentH);
    m_loadedCache->models[i]=glmList( m_loadedCache->realmodels[i], GLM_SMOOTH | GLM_TEXTURE);

    i++;
  }
}

/////////////////////////////////////////////////////////
// materialMess
//
/////////////////////////////////////////////////////////
void multimodel :: textureMess(int state)
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


void multimodel :: startRendering()
{
  // build a display list
  //  buildList();
  m_rebuild=true;
}
/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void multimodel :: render(GemState *state)
{
  if (!m_numModels || !m_loadedCache) return;
  if (state && (m_currentW != state->texCoordX(2) || m_currentH != state->texCoordY(2)))
    {
      m_rebuild=true;
    }
  if(m_rebuild) {
    m_currentW = state->texCoordX(2);
    m_currentH = state->texCoordY(2);
    buildList();
    m_rebuild=false;
  }
  if (!m_loadedCache->models[m_curModel])return;


  glCallList(m_loadedCache->models[m_curModel]);
}

/////////////////////////////////////////////////////////
// rescaleMess
//
/////////////////////////////////////////////////////////
void multimodel :: rescaleMess(int state)
{
  m_rescaleModel = state;
}

/////////////////////////////////////////////////////////
// changeModel
//
/////////////////////////////////////////////////////////
void multimodel :: changeModel(int modelNum)
{
  if (modelNum >= m_numModels)
    {
      error("selection number too high: %d (max num is %d)", modelNum, m_numModels-1);
      return;
    }
  else if (modelNum < 0)
    {
      error("selection number must be > 0");
      return;
    }
  m_curModel = modelNum;
  //  setModified();
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void multimodel :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr, reinterpret_cast<t_method>(&multimodel::openMessCallback),
		  gensym("open"), A_SYMBOL, A_FLOAT, A_DEFFLOAT, A_DEFFLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&multimodel::changeModelCallback),
		  gensym("mdl_num"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&multimodel::rescaleMessCallback),
		  gensym("rescale"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&multimodel::textureMessCallback),
		  gensym("texture"), A_FLOAT, A_NULL);
}
void multimodel :: openMessCallback(void *data, t_symbol *filename, t_floatarg baseModel,
				    t_floatarg topModel, t_floatarg skipRate)
{
  int skipRatei=static_cast<int>(skipRate);
  int topModeli=static_cast<int>(topModel);
  int baseModeli=static_cast<int>(baseModel);

  if (skipRatei == 0)
    {
      if (topModeli == 0)
	GetMyClass(data)->openMess(filename, 0, topModeli, 0);
      else
	GetMyClass(data)->openMess(filename, baseModeli, topModeli, 0);
    }
  else
    GetMyClass(data)->openMess(filename, baseModeli, topModeli, skipRatei);
}
void multimodel :: changeModelCallback(void *data, t_floatarg modelNum)
{
  GetMyClass(data)->changeModel(static_cast<int>(modelNum));
}
void multimodel :: rescaleMessCallback(void *data, t_floatarg state)
{
  GetMyClass(data)->rescaleMess(static_cast<int>(state));
}
void multimodel :: textureMessCallback(void *data, t_floatarg state)
{
  GetMyClass(data)->textureMess(static_cast<int>(state));
}
