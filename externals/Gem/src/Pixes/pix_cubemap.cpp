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
//    Copyright (c) 2002-2006 James Tittle & Chris Clepper
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "pix_cubemap.h"

#include "Gem/Manager.h"
#include "Gem/Image.h"
#include <string.h>

#ifdef debug
# undef debug
#endif

//#define DEBUG_ME

#ifdef DEBUG_ME
# define debug post
#else
# define debug
#endif

CPPEXTERN_NEW(pix_cubemap);

/////////////////////////////////////////////////////////
//
// pix_cubemap
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_cubemap :: pix_cubemap()
: m_textureOnOff(1),
  m_textureQuality(GL_LINEAR), m_repeat(GL_REPEAT),
  m_didTexture(false), m_rebuildList(0),
  m_textureObj(0),
  m_realTextureObj(0),
  m_oldTexCoords(NULL), m_oldNumCoords(0), m_oldTexture(0),
  m_textureType(GL_TEXTURE_CUBE_MAP),
  m_xRatio(1.), m_yRatio(1.),
  m_env(GL_MODULATE),
  m_texunit(0), 
  m_numTexUnits(0),
  m_map(0)
{
  error("this object is likely to vanish! do not use!!");


  int i=0;

  m_dataSize[0] = m_dataSize[1] = m_dataSize[2] = -1;
  m_buffer.xsize = m_buffer.ysize = m_buffer.csize = -1;
  m_buffer.data = NULL;

  for(i=0; i<6; i++) {
    m_imgIn[i] = NULL;
    m_img[i]   = NULL;  
  }

  // create an inlet to receive external texture IDs
  //  m_imgIn[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("gem_state"), gensym("gem_imageX+"));
  m_imgIn[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("gem_state"), gensym("gem_imageX-"));

  m_imgIn[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("gem_state"), gensym("gem_imageY+"));
  m_imgIn[3] = inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("gem_state"), gensym("gem_imageY-"));

  m_imgIn[4] = inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("gem_state"), gensym("gem_imageZ+"));
  m_imgIn[5] = inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("gem_state"), gensym("gem_imageZ-"));

  // create an outlet to send texture ID
  m_outTexID = outlet_new(this->x_obj, &s_float);
}

////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_cubemap :: ~pix_cubemap()
{
  int i=0;
  for(i=0; i<6; i++) {
    if(m_imgIn[i])
      inlet_free(m_imgIn[i]);
    m_imgIn[i]=NULL;
  }
  if(m_outTexID)outlet_free(m_outTexID);

  m_outTexID=NULL;
}

////////////////////////////////////////////////////////
// setUpTextureState
//
/////////////////////////////////////////////////////////
void pix_cubemap :: setUpTextureState() {
  glPixelStoref(GL_UNPACK_ALIGNMENT, 1);

  glTexParameterf(m_textureType, GL_TEXTURE_MIN_FILTER, m_textureQuality);
  glTexParameterf(m_textureType, GL_TEXTURE_MAG_FILTER, m_textureQuality);
  glTexParameterf(m_textureType, GL_TEXTURE_WRAP_S, m_repeat);
  glTexParameterf(m_textureType, GL_TEXTURE_WRAP_T, m_repeat);

  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, m_env);
}

////////////////////////////////////////////////////////
// extension check
//
/////////////////////////////////////////////////////////
bool pix_cubemap :: isRunnable(void) {
  /* for simplicity's sake, i have dropped support for very old openGL-versions */
  if(!GLEW_VERSION_1_3) {
    error("need at least openGL-1.3 for cube mapping! refusing to work");
    return false;
  }

  m_numTexUnits=0;
  if(GLEW_ARB_multitexture)
    glGetIntegerv( GL_MAX_TEXTURE_UNITS_ARB, &m_numTexUnits );

  return true;
}

void pix_cubemap :: pushTexCoords(GemState*state) {
  state->get(GemState::_GL_TEX_COORDS, m_oldTexCoords);
  state->get(GemState::_GL_TEX_NUMCOORDS, m_oldNumCoords);
  state->get(GemState::_GL_TEX_TYPE, m_oldTexture);
}

void pix_cubemap :: popTexCoords(GemState*state) {
  state->set(GemState::_GL_TEX_COORDS, m_oldTexCoords);
  state->set(GemState::_GL_TEX_NUMCOORDS, m_oldNumCoords);
  state->set(GemState::_GL_TEX_TYPE, m_oldTexture);
}


void pix_cubemap :: sendExtTexture(GLuint texobj, GLfloat xRatio, GLfloat yRatio, GLint texType, GLboolean upsidedown) {
  // send textureID to outlet
  if(texobj){
    t_atom ap[5];
    SETFLOAT(ap,   (t_float)texobj);
    SETFLOAT(ap+1, (t_float)xRatio);
    SETFLOAT(ap+2, (t_float)yRatio);
    SETFLOAT(ap+3, (t_float)texType);
    SETFLOAT(ap+4, (t_float)upsidedown);
    outlet_list(m_outTexID, &s_list, 5, ap);
  }
}

void pix_cubemap :: applyTex(GLint textype, imageStruct*img) {
  //if the texture is a power of two in size then there is no need to subtexture
  if(img) {
    glTexImage2D(textype, 0,
		 img->csize, img->xsize, img->ysize, 
		 0, img->format, img->type, img->data);
    logpost(NULL, 5, "tex:%d\timg=%d %d %d %d %d %d %d %x",
	    textype, 0,
	    GL_RGBA8, img->xsize, img->ysize, 
	    0, img->format, img->type, img->data);
    switch(textype) {
    case(GL_TEXTURE_CUBE_MAP_POSITIVE_X): post("X+"); break;
    case(GL_TEXTURE_CUBE_MAP_NEGATIVE_X): post("X-"); break;
    case(GL_TEXTURE_CUBE_MAP_POSITIVE_Y): post("Y+"); break;
    case(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y): post("Y-"); break;
    case(GL_TEXTURE_CUBE_MAP_POSITIVE_Z): post("Z+"); break;
    case(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z): post("Z-"); break;
    default: post("???");
    }
  }
}

////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void pix_cubemap :: render(GemState *state) {
  m_didTexture=false;
  pushTexCoords(state);

  if(!m_textureOnOff)return;

  /* here comes the work: a new image has to be transfered from main memory to GPU and attached to a texture object */
  if(state) {
    pixBlock*img=NULL;
    state->get(GemState::_PIX, img);
    if(img) {
      if(img->newimage)
        m_img[0]=&img->image;
    }
  }


  if(GLEW_VERSION_1_3) {
    glActiveTexture(GL_TEXTURE0_ARB + m_texunit);
  }
  glEnable(m_textureType);
  glBindTexture(m_textureType, m_textureObj);

  int i=0;
  for(i=0; i<6; i++) {
    applyTex(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, m_img[i]);
    m_img[i]=NULL;
    }

  int mode = GL_NORMAL_MAP;
  switch(m_map) {
  case 0: mode = GL_NORMAL_MAP; break;
  case 1: mode = GL_REFLECTION_MAP; break;
  case 2: mode = GL_SPHERE_MAP; break;
  default: mode=m_map;
  }


  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, mode);
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, mode);
  glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, mode);
  glEnable(GL_TEXTURE_GEN_S);
  glEnable(GL_TEXTURE_GEN_T);
  glEnable(GL_TEXTURE_GEN_R);

  /* cleanup */
  m_rebuildList = 0;
  m_didTexture=true;

  state->set(GemState::_GL_TEX_UNITS, m_numTexUnits);
  state->set(GemState::_GL_TEX_TYPE, 0); // ?

  //  sendExtTexture(m_textureObj, m_xRatio, m_yRatio, m_textureType, upsidedown);
}

void pix_cubemap :: rightImage(int id, GemState *state) {
  if(!state)return;
  if(id<0 || id>=6) {
    error("not a valid image-slot %d", id);
  }
  pixBlock*img=NULL;
  state->get(GemState::_PIX, img);

  if(img) {
    if(img->newimage) {
      m_img[id]=&img->image;
    } else {
      // m_img[id]=NULL;
    }
  }

}


////////////////////////////////////////////////////////
// postrender
//
/////////////////////////////////////////////////////////
void pix_cubemap :: postrender(GemState *state){
  popTexCoords(state);

  if (m_didTexture){
    if(GLEW_VERSION_1_3) {
      glActiveTexture(GL_TEXTURE0_ARB + m_texunit);  //needed?
    }
    glDisable(m_textureType);


    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_GEN_R);
  }

}

////////////////////////////////////////////////////////
// startRendering
//
/////////////////////////////////////////////////////////
void pix_cubemap :: startRendering()
{
  glGenTextures(1, &m_realTextureObj); // this crashes sometimes!!!! (jmz)
  if(GLEW_VERSION_1_3) {
    glActiveTexture(GL_TEXTURE0_ARB + m_texunit);
  }
  glBindTexture(m_textureType, m_realTextureObj);
  m_textureObj=m_realTextureObj;
  setUpTextureState();

  m_dataSize[0] = m_dataSize[1] = m_dataSize[2] = -1;

  if (!m_realTextureObj)	{
    error("Unable to allocate texture object");
    return;
  }
}

////////////////////////////////////////////////////////
// stopRendering
//
/////////////////////////////////////////////////////////
void pix_cubemap :: stopRendering()
{
  if(m_realTextureObj) {
    glDeleteTextures(1, &m_realTextureObj);

    m_realTextureObj = 0;
    m_dataSize[0] = m_dataSize[1] = m_dataSize[2] = -1;
  }
}

////////////////////////////////////////////////////////
// textureOnOff
//
/////////////////////////////////////////////////////////
void pix_cubemap :: textureOnOff(int on)
{
  m_textureOnOff = on;
  setModified();
}

/////////////////////////////////////////////////////////
// textureQuality
//
/////////////////////////////////////////////////////////
void pix_cubemap :: textureQuality(int type)
{
  if (type)
    m_textureQuality = GL_LINEAR;
  else
    m_textureQuality = GL_NEAREST;

  if (m_textureObj) {
    if(GLEW_VERSION_1_3) {
      glActiveTexture(GL_TEXTURE0_ARB + m_texunit);
    }
    glBindTexture(m_textureType, m_textureObj);
    glTexParameterf(m_textureType, GL_TEXTURE_MAG_FILTER, m_textureQuality);
    glTexParameterf(m_textureType, GL_TEXTURE_MIN_FILTER, m_textureQuality);
  }
  setModified();
}

////////////////////////////////////////////////////////
// texture repeat message
//
/////////////////////////////////////////////////////////
void pix_cubemap :: repeatMess(int type)
{
  if (type)
    m_repeat = GL_REPEAT;
  else {
    if(GLEW_EXT_texture_edge_clamp)
      m_repeat = GL_CLAMP_TO_EDGE;
    else
      m_repeat = GL_CLAMP;
  }

  if (m_textureObj) {
    if(GLEW_VERSION_1_1) {
      glBindTexture(m_textureType, m_textureObj);
      glTexParameterf(m_textureType, GL_TEXTURE_WRAP_S, m_repeat);
      glTexParameterf(m_textureType, GL_TEXTURE_WRAP_T, m_repeat);
    } else {
      glBindTextureEXT(m_textureType, m_textureObj);
      glTexParameteri(m_textureType, GL_TEXTURE_WRAP_S, m_repeat);
      glTexParameteri(m_textureType, GL_TEXTURE_WRAP_T, m_repeat);
    }
  }
  setModified();
}

////////////////////////////////////////////////////////
// texture environment mode
//
/////////////////////////////////////////////////////////
void pix_cubemap :: envMess(int num)
{
  switch (num) {
  case 0:
    m_env = GL_REPLACE;
    break;
  case 1:
    m_env = GL_DECAL;
    break;
  case 2:
    m_env = GL_BLEND;
    break;
  case 3:
    m_env = GL_ADD;
    break;
  case 4:
    m_env = GL_COMBINE;
    break;
  default:
    m_env = GL_MODULATE;
  }
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, m_env);
  setModified();
}

////////////////////////////////////////////////////////
// static member functions
//
/////////////////////////////////////////////////////////
void pix_cubemap :: obj_setupCallback(t_class *classPtr)
{
  //  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_cubemap::imageRight_callback), gensym("gem_imageX+"), A_GIMME, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_cubemap::rightImage_callback), gensym("gem_imageX-"), A_GIMME, A_NULL);

  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_cubemap::rightImage_callback), gensym("gem_imageY+"), A_GIMME, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_cubemap::rightImage_callback), gensym("gem_imageY-"), A_GIMME, A_NULL);

  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_cubemap::rightImage_callback), gensym("gem_imageZ+"), A_GIMME, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_cubemap::rightImage_callback), gensym("gem_imageZ-"), A_GIMME, A_NULL);

  class_addfloat(classPtr, reinterpret_cast<t_method>(&pix_cubemap::floatMessCallback));
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_cubemap::textureMessCallback),
                  gensym("quality"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_cubemap::repeatMessCallback),
                  gensym("repeat"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_cubemap::envMessCallback),
                  gensym("env"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_cubemap::texunitCallback),
                  gensym("texunit"), A_FLOAT, A_NULL);

  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_cubemap::mapCallback),
                  gensym("map"), A_FLOAT, A_NULL);
}
void pix_cubemap :: floatMessCallback(void *data, float n)
{
  GetMyClass(data)->textureOnOff((int)n);
}
void pix_cubemap :: textureMessCallback(void *data, t_floatarg quality)
{
  GetMyClass(data)->textureQuality((int)quality);
}
void pix_cubemap :: repeatMessCallback(void *data, t_floatarg repeat)
{
  GetMyClass(data)->repeatMess((int)repeat);
}
void pix_cubemap :: envMessCallback(void *data, t_floatarg num )
{
  GetMyClass(data)->envMess((int) num);
}
void pix_cubemap :: texunitCallback(void *data, t_floatarg unit)
{
  GetMyClass(data)->m_texunit=(int)unit;
}
void pix_cubemap :: mapCallback(void *data, t_floatarg unit)
{
  GetMyClass(data)->m_map=(int)unit;
}

void pix_cubemap :: rightImage_callback(void *data, t_symbol *s, int argc, t_atom *argv)
{
  int id=-1;
  if(gensym("gem_imageX+")==s)id=0;
  if(gensym("gem_imageX-")==s)id=1;
  if(gensym("gem_imageY+")==s)id=2;
  if(gensym("gem_imageY-")==s)id=3;
  if(gensym("gem_imageZ+")==s)id=4;
  if(gensym("gem_imageZ-")==s)id=5;
  if (argc==1 && argv->a_type==A_FLOAT){
  } else if (argc==2 && argv->a_type==A_POINTER && (argv+1)->a_type==A_POINTER){
    GetMyClass(data)->rightImage(id, (GemState *)(argv+1)->a_w.w_gpointer);
  } else {
    GetMyClass(data)->error("wrong righthand arguments...");
    ::error("post: %d", argc);
  }
}
