/*-----------------------------------------------------------------
  LOG
  GEM - Graphics Environment for Multimedia

  Load multiple images

  Copyright (c) 1997-1999 Mark Danks. mark@danks.org
  Copyright (c) Günther Geiger. geiger@epy.co.at
  Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

  -----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_GEOS_MULTIMODEL_H_
#define _INCLUDE__GEM_GEOS_MULTIMODEL_H_

#include "Base/GemBase.h"
#include <string.h>
#include "model_loader.h"

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  multimodel
    
  Load multiple models

  DESCRIPTION

  Inlet for a list - "multimodel"

  "open" - the multimodel to set the object to
    
  -----------------------------------------------------------------*/
class GEM_EXTERN multimodel : public GemBase
{
  CPPEXTERN_HEADER(multimodel, GemBase);

    public:

  //////////
  // Constructor
  multimodel(t_symbol *filename, t_floatarg baseModel, t_floatarg topModel, t_floatarg skipRate);
    	
  class multiModelCache
    {
    public:
                
      multiModelCache(const char *_modelName)
        : refCount(0), next(NULL), models(NULL),
        numModels(0), baseModel(0), topModel(0),
        skipRate(0)
          { modelName = strdup(_modelName); }
        ~multiModelCache()
          { delete modelName;
            for (int i = 0; i < numModels; i++) {
              glmDelete(realmodels[i]);
              glDeleteLists(models[i], 1);
            }
            delete [] models;
          }
        int                 refCount;
        multiModelCache     *next;
        GLint               *models;
        GLMmodel            **realmodels;
        int                 numModels;
        char                *modelName;
        int                 baseModel;
        int                 topModel;
        int                 skipRate;
    };
  
  //////////
  static multiModelCache      *s_modelCache;

 protected:
    	
  //////////
  // Destructor
  virtual ~multimodel();

  //////////
  // When an open is received
  virtual void	openMess(t_symbol *filename, int baseModel, int topModel, int skipRate);
    	
  //////////
  void	    	cleanMultimodel();

  //////////
  virtual void	buildList();
    	
  //////////
  virtual void	startRendering();

  //////////
  virtual void	render(GemState *state);

  //////////
  // Change which model to display
  void	    	changeModel(int modelNum);
    
  //////////
  // Set the rescale state
  void	    	rescaleMess(int state);
    
  //////////
  multiModelCache *m_loadedCache;
    	
  //-----------------------------------
  // GROUP:	Model data
  //-----------------------------------
    
  //////////
  // The number of loaded models
  int 	    	m_numModels;

  //////////
  // The current model
  int 	    	m_curModel;

  //////////
  // Rescale the models when loaded?
  int 	    	m_rescaleModel;


  //////////
  // Which texture type (linear, spheric)
  virtual void	textureMess(int state);
  glmtexture_t m_textype; 

  bool    m_rebuild;
  float		m_currentH, m_currentW;

 private:
    
  //////////
  // static member functions
  static void 	openMessCallback(void *data, t_symbol *filename, t_floatarg baseModel, t_floatarg topModel, t_floatarg skipRate);
  static void 	changeModelCallback(void *data, t_floatarg modelNum);
  static void 	rescaleMessCallback(void *data, t_floatarg modelNum);
	static void   textureMessCallback(void *data, t_floatarg);
};

#endif	// for header file
