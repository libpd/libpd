////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 2011-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include <iostream>

#include "pix_frei0r.h"
#include "Gem/Exception.h"
#include "Gem/Loaders.h"

#include "Gem/Dylib.h"

#include "Gem/Properties.h"

#include <stdio.h>
#ifdef _WIN32
# include <io.h>
# include <windows.h>
# define snprintf _snprintf
# define close _close

/*
 * Apple used to use CFBundle's to load FF plugins
 * currently this only crashes (on OSX-10.4 and OSX-10.5)
 * we therefore use dlopen() on OSX as well
 */
#elif defined __APPLE__ && 0
# include <mach-o/dyld.h>
# include <unistd.h>
#else
# define DL_OPEN
# include <dlfcn.h>
# include <unistd.h>
#endif /* __APPLE__ */

#include <string.h>


#ifndef HAVE_STRNLEN
#define strnlen f0r_strnlen
static size_t f0r_strnlen(const char* str, size_t maxlen) {
  size_t len=0;
  if(NULL==str)return len;
  while(*str++ && len<maxlen)len++;

  return len;
}
#endif


class pix_frei0r::F0RPlugin {
public:
  bool init(void) {
    if(!f0r_init)return false;

    if(!f0r_get_plugin_info)return false;
    if(!f0r_get_param_info)return false;
    if(!f0r_construct)return false;
    if(!f0r_destruct)return false;
    if(!f0r_set_param_value)return false;
    if(!f0r_get_param_value)return false;
    if(!f0r_deinit)return false;

    int err=0;

    if(f0r_init)
      err=f0r_init();

    f0r_plugin_info_t info;
    f0r_get_plugin_info(&info);
    m_name = info.name;
    m_author = info.author;
    m_type = info.plugin_type;
    switch(m_type) {
    case (F0R_PLUGIN_TYPE_SOURCE):
    case (F0R_PLUGIN_TYPE_FILTER):
      break;
    default:
      ::error("[pix_frei0r] only supports sources/filters, no mixers!");
      return false;
    }

#ifdef __GNUC__
# warning check color type
#endif
    m_color = info.color_model;
#ifdef __GNUC__
# warning check compatibility
#endif
    m_frei0rVersion = info.frei0r_version;
    m_majorVersion = info.major_version;
    m_minorVersion = info.minor_version;
    m_explanation = info.explanation;

    ::post("%s by %s", info.name, info.author);
    ::post("%d:: %s", m_type, info.explanation);


    if(!f0r_update)return false;
    // if(!f0r_update2)return false;

    int numparameters = info.num_params;
    int i=0;
    m_parameterNames.clear();
    m_parameterTypes.clear();
    m_parameter.clear();

    // dummy parameter (so we start at 1)
    m_parameterNames.push_back("");
    m_parameterTypes.push_back(0);

    for(i=0; i<numparameters; i++) {
      f0r_param_info_t pinfo;
      f0r_get_param_info(&pinfo, i);
      m_parameterNames.push_back(pinfo.name);
      m_parameterTypes.push_back(pinfo.type);

      ::post("parm%02d[%s]: %s", i+1, pinfo.name, pinfo.explanation);
    }

    return true;
  }
  void deinit(void) {
    destruct();
    if(f0r_deinit)
      f0r_deinit();
  }

  unsigned int m_width, m_height;

  bool construct(unsigned int width, unsigned int height) {
    destruct();
    m_instance=f0r_construct(width, height);
    m_width=width;
    m_height=height;
    return (m_instance!=NULL);
  }
  void destruct(void) {
    if(m_instance)
      f0r_destruct(m_instance);
    m_instance=NULL;
  }

  f0r_instance_t m_instance;

  std::string m_name;
  std::string m_author;
  int 	m_type;
  int 	m_color;
  int 	m_frei0rVersion;
  int 	m_majorVersion;
  int 	m_minorVersion;
  std::string m_explanation;

  gem::Properties m_parameter;
  std::vector<std::string>m_parameterNames;
  std::vector<int>m_parameterTypes;

typedef int (*t_f0r_init)(void);
typedef void (*t_f0r_get_plugin_info)(f0r_plugin_info_t* pluginInfo);
typedef void (*t_f0r_get_param_info)(f0r_param_info_t* info, int param_index);
typedef f0r_instance_t (*t_f0r_construct)(unsigned int width, unsigned int height);
typedef void (*t_f0r_destruct)(f0r_instance_t instance);
typedef void (*t_f0r_set_param_value)(f0r_instance_t instance, f0r_param_t param, int param_index);
typedef void (*t_f0r_get_param_value)(f0r_instance_t instance, f0r_param_t param, int param_index);
typedef void (*t_f0r_update) (f0r_instance_t instance, double time, const uint32_t* inframe, uint32_t* outframe);
typedef void (*t_f0r_update2)(f0r_instance_t instance, double time, const uint32_t* inframe1, const uint32_t* inframe2, const uint32_t* inframe3, uint32_t* outframe);
typedef int (*t_f0r_deinit)(void);


  t_f0r_init f0r_init;
  t_f0r_get_plugin_info f0r_get_plugin_info;
  t_f0r_get_param_info f0r_get_param_info;
  t_f0r_construct f0r_construct;
  t_f0r_destruct f0r_destruct;
  t_f0r_set_param_value f0r_set_param_value;
  t_f0r_get_param_value f0r_get_param_value;
  t_f0r_update f0r_update;
  t_f0r_update2 f0r_update2;
  t_f0r_deinit f0r_deinit;


  void close(void) {
    destruct();
    int err=f0r_deinit();
#ifdef __GNUC__
# warning what to do with that err?
#endif
  }

  F0RPlugin(std::string name, const t_canvas*parent=NULL) :
    m_width(0), m_height(0),
    m_instance(NULL),
    m_name(""), m_author(""),
    m_type(0), m_color(0),
    m_frei0rVersion(0), m_majorVersion(0), m_minorVersion(0),
    m_explanation(""),
    m_dylib(name)
  {
    f0r_init           =reinterpret_cast<t_f0r_init           >(m_dylib.proc("f0r_init"));
    f0r_get_plugin_info=reinterpret_cast<t_f0r_get_plugin_info>(m_dylib.proc("f0r_get_plugin_info"));
    f0r_get_param_info =reinterpret_cast<t_f0r_get_param_info >(m_dylib.proc("f0r_get_param_info"));
    f0r_construct      =reinterpret_cast<t_f0r_construct      >(m_dylib.proc("f0r_construct"));
    f0r_destruct       =reinterpret_cast<t_f0r_destruct       >(m_dylib.proc("f0r_destruct"));
    f0r_set_param_value=reinterpret_cast<t_f0r_set_param_value>(m_dylib.proc("f0r_set_param_value"));
    f0r_get_param_value=reinterpret_cast<t_f0r_get_param_value>(m_dylib.proc("f0r_get_param_value"));
    f0r_update         =reinterpret_cast<t_f0r_update         >(m_dylib.proc("f0r_update"));
    f0r_update2        =reinterpret_cast<t_f0r_update2        >(m_dylib.proc("f0r_update2"));
    f0r_deinit         =reinterpret_cast<t_f0r_deinit         >(m_dylib.proc("f0r_deinit"));

    if(!init()) {
      deinit();
      throw(GemException("couldn't instantiate frei0r plugin"));
    }
  }

  bool set(unsigned int key, bool value) {
    if(!m_instance)return false;
    f0r_param_bool v=value;
    f0r_set_param_value(m_instance, &v, key);
    return true;
  }
  bool set(unsigned int key, double value) {
    if(!m_instance)return false;
    f0r_param_double v=value;
    f0r_set_param_value(m_instance, &v, key);
    return true;
  }
  bool set(unsigned int key, double x, double y) {
    if(!m_instance)return false;
    f0r_param_position v;
    v.x=x;
    v.y=y;
    f0r_set_param_value(m_instance, &v, key);
    return true;
  }
  bool set(unsigned int key, double r, double g, double b) {
    if(!m_instance)return false;
    f0r_param_color v;
    v.r=r;
    v.g=g;
    v.b=b;
    f0r_set_param_value(m_instance, &v, key);
    return true;
  }
  bool set(unsigned int key, std::string s) {
    if(!m_instance)return false;
    f0r_param_string*v=const_cast<f0r_param_string*>(s.c_str());
    f0r_set_param_value(m_instance, &v, key);
    return true;
  }


  bool process(double time, imageStruct&input, imageStruct&output) {
    if(!m_instance || m_width!=input.xsize || m_height!=input.ysize)
      construct(input.xsize, input.ysize);

    if(!m_instance)return false;

    f0r_update(m_instance, time,
	       reinterpret_cast<const uint32_t*>(input.data),
	       reinterpret_cast<uint32_t*>(output.data));

    return true;
  }

  GemDylib m_dylib;
};


CPPEXTERN_NEW_WITH_ONE_ARG(pix_frei0r,  t_symbol *, A_DEFSYM);

/////////////////////////////////////////////////////////
//
// pix_frei0r
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////

pix_frei0r :: pix_frei0r(t_symbol*s)
  : m_plugin(NULL)
  , m_canopen(false)
{
  //  throw(GemException("Gem has been compiled without Frei0r-support!"));
  int can_rgba=0;
  m_image.setCsizeByFormat(GL_RGBA);

  if(!s || s==&s_) {
    m_canopen=true;
    return;
  }
  char *pluginname = s->s_name;

  m_plugin = new F0RPlugin(pluginname, getCanvas());

  unsigned int numparams = m_plugin->m_parameterNames.size();
  char tempVt[5];

  unsigned int i;
  for(i=1; i<numparams; i++) {
    snprintf(tempVt, 5, "#%d", i);
    tempVt[4]=0;
    unsigned int parmType=0;
    t_symbol*s_inletType;
    parmType=m_plugin->m_parameterTypes[i];

    switch(parmType) {
    case(F0R_PARAM_BOOL):
    case(F0R_PARAM_DOUBLE):
      s_inletType=gensym("float");
    break;
    case(F0R_PARAM_COLOR):
    case(F0R_PARAM_POSITION):
      s_inletType=gensym("list");
    break;
    case(F0R_PARAM_STRING):
      s_inletType=gensym("symbol");
      break;
    default:
      s_inletType=&s_;
    }
    m_inlet.push_back(inlet_new(this->x_obj, &this->x_obj->ob_pd, s_inletType, gensym(tempVt)));
  }
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_frei0r :: ~pix_frei0r()
{
  while(!m_inlet.empty()) {
    t_inlet*in=m_inlet.back();
    if(in)inlet_free(in);
    m_inlet.pop_back();
  }
  closeMess();
}

void pix_frei0r :: closeMess()
{
  if(m_plugin){
    delete m_plugin;
  }
  m_plugin=NULL;
}

void pix_frei0r :: openMess(t_symbol*s)
{
  if(!m_canopen) {
    error("this instance cannot dynamically change the plugin");
    return;
  }

  std::string pluginname = s->s_name;
  if(m_plugin) {
    delete m_plugin;
  }
  m_plugin=NULL;
  try {
    m_plugin = new F0RPlugin(pluginname, getCanvas());
  } catch (GemException&x) {
    error("%s", x.what());
  }

  if(NULL==m_plugin) {
    error("unable to open '%s'", pluginname.c_str());
    return;
  }
}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_frei0r :: processRGBAImage(imageStruct &image)
{
  static double time=0;
  if(!m_plugin)return;

  m_image.xsize=image.xsize;
  m_image.ysize=image.ysize;
  m_image.reallocate();

  m_plugin->process(time, image, m_image);
  time++;

  image.data   = m_image.data;
  image.notowned = true;
  image.setCsizeByFormat(m_image.format);
}

void pix_frei0r :: parmMess(const std::string key, int argc, t_atom *argv){
  if(!m_plugin) {
    error("no plugin present! forgetting parameter....");
    return;
  }
  unsigned int i=0;
  for(i=0; i<m_plugin->m_parameterNames.size(); i++) {
    if(key==m_plugin->m_parameterNames[i]) {
      parmMess(i, argc, argv);
      return;
    }
  }
  error("unknown parameter '%s'", key.c_str());
}


void pix_frei0r :: parmMess(int key, int argc, t_atom *argv){
  unsigned int realkey=0;
  if(!m_plugin) {
    error("no plugin present! forgetting parameter....");
    return;
  }
  if(key<=0) {
    error("parameterIDs must be >0");
    return;
  } else {
    realkey=key-1;
  }
  if(static_cast<unsigned int>(key)>=m_plugin->m_parameterNames.size()) {
    error("parameterID out of bounds");
    return;
  }

  int type=m_plugin->m_parameterTypes[key];

  double r, g, b;
  double x, y;

  const char*name=m_plugin->m_parameterNames[key].c_str();
  switch(type) {
  case(F0R_PARAM_BOOL):
    if(argc!=1) {
      error("param#%02d('%s') is of type BOOL: need exactly 1 argument", key, name);
      return;
    }
    m_plugin->set(realkey, (atom_getfloat(argv)>0.5));
    break;
  case(F0R_PARAM_DOUBLE):
    if(argc!=1) {
      error("param#%02d('%s') is of type DOUBLE: need exactly 1 argument", key, name);
      return;
    }
    m_plugin->set(realkey, static_cast<double>(atom_getfloat(argv)));
    break;
  case(F0R_PARAM_COLOR):
    if(argc!=3) {
      error("param#%02d('%s') is of type COLOR: need exactly 3 arguments", key, name);
      return;
    }
    r=atom_getfloat(argv+0);
    g=atom_getfloat(argv+1);
    b=atom_getfloat(argv+2);
    m_plugin->set(realkey, r, g, b);
    break;
  case(F0R_PARAM_POSITION):
    if(argc!=2) {
      error("param#%02d('%s') is of type POSITION: need exactly 2 arguments", key, name);
      return;
    }
    x=atom_getfloat(argv+0);
    y=atom_getfloat(argv+1);
    m_plugin->set(realkey, x, y);
    break;
  case(F0R_PARAM_STRING):
    if(argc!=1) {
      error("param#%02d('%s') is of type STRING: need exactly 1 argument", key, name);
      return;
    }
    m_plugin->set(realkey, std::string(atom_getsymbol(argv)->s_name));
    break;
  default:
    error("param#%02d('%s') is of UNKNOWN type", key, name);
    break;
  }

}

static const int offset_pix_=strlen("pix_");

static void*frei0r_loader_new(t_symbol*s, int argc, t_atom*argv) {
  ::logpost(NULL, 6, "frei0r_loader: %s",(s?(s->s_name):"<none>"));
  try{	    	    	    	    	    	    	    	\
    Obj_header *obj = new (pd_new(pix_frei0r_class),(void *)NULL) Obj_header;
    char*realname=s->s_name+offset_pix_; /* strip of the leading 'pix_' */
    CPPExtern::m_holder = &obj->pd_obj;
    CPPExtern::m_holdname=s->s_name;
    obj->data = new pix_frei0r(gensym(realname));
    CPPExtern::m_holder = NULL;
    CPPExtern::m_holdname=NULL;
    return(obj);
  } catch (GemException e) {
    ::logpost(NULL, 6, "frei0r_loader: failed!");
    //e.report();
    return NULL;
  }
  return 0;
}
bool pix_frei0r :: loader(t_canvas*canvas, std::string classname) {
  if(strncmp("pix_", classname.c_str(), offset_pix_))
    return false;
  std::string pluginname = classname.substr(offset_pix_);
  pix_frei0r::F0RPlugin*plugin=NULL;
  try {
    plugin=new F0RPlugin(pluginname, canvas);
  } catch (GemException e) {
    ::logpost(NULL, 6, "frei0r_loader: failed!!");
    // e.report();
    return false;
  }

  if(plugin!=NULL) {
    delete plugin;
    class_addcreator(reinterpret_cast<t_newmethod>(frei0r_loader_new), gensym(classname.c_str()), A_GIMME, 0);
    return true;
  }
  return false;
}

static int frei0r_loader(t_canvas *canvas, char *classname) {
  return pix_frei0r::loader(canvas, classname);
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////

void pix_frei0r :: obj_setupCallback(t_class *classPtr)
{
  class_addanything(classPtr, reinterpret_cast<t_method>(&pix_frei0r::parmCallback));
  class_addmethod  (classPtr, reinterpret_cast<t_method>(&pix_frei0r::openCallback), gensym("load"), A_SYMBOL, A_NULL);
  gem_register_loader(frei0r_loader);
}

void pix_frei0r :: parmCallback(void *data, t_symbol*s, int argc, t_atom*argv){
  if('#'==s->s_name[0]) {
    int i = atoi(s->s_name+1);
    GetMyClass(data)->parmMess(i, argc, argv);
  } else {
    GetMyClass(data)->parmMess(std::string(s->s_name), argc, argv);
  }
}

void pix_frei0r :: openCallback(void *data, t_symbol*name){
  GetMyClass(data)->openMess(name);
}
