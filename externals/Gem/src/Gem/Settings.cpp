////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.at
//
// Implementation file
//
//    Copyright (c) 2009-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
// load settings from a file (or get them via env-variables)
//
/////////////////////////////////////////////////////////

#include "Gem/GemConfig.h"

#include "m_pd.h"

#include "Settings.h"
#include "Files.h"

#include <map>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define GEM_SETTINGS_FILE "gem.conf"
static const char*s_configdir[] = { 
#ifdef __linux__
  "/etc/pd",
  "~/.pd", 
#elif defined __APPLE__
  "/Library/Pd",
  "~/Library/Pd",
#elif defined  _WIN32
  "%CommonProgramFiles%\\Pd",
  "%AppData%\\Pd",
#endif
  0 /* $(pwd)/gem.conf */
};




/* this is ripped from m_imp.h */
struct _gemclass
{
  t_symbol *c_name;                   /* name (mostly for error reporting) */
  t_symbol *c_helpname;               /* name of help file */
  t_symbol *c_externdir;              /* directory extern was loaded from */
  /* ... */ /* the real t_class continues here... */
};
# define t_gemclass struct _gemclass


namespace {
struct PIMPL {
  // dictionary for setting values
  std::map <std::string, t_atom> data;

  virtual t_atom*get(std::string name) {
    std::map<std::string, t_atom>::iterator it=data.find(name);
    if(it==data.end())
      return NULL;
    return &it->second;
  }
  virtual void set(std::string name, t_atom*value) {
    // LATER: we should expand envvariables
    if(value) {
      data[name]= *value;
    } else {
      data.erase(name);
    }
  }

  void set(std::string name, int i) {
    t_atom a;
    SETFLOAT(&a, i);
    set(name, &a);
  }
  void set(std::string name, t_float f) {
    t_atom a;
    SETFLOAT(&a, f);
    set(name, &a);
  }
  void set(std::string name, std::string s) {
    t_atom a;
    SETSYMBOL(&a, gensym(s.c_str()));
    set(name, &a);
  }

  //  std::string expandEnv(std::string , bool bashfilename=false);


  bool open(const char*filename, const char*dirname=NULL) {
    t_binbuf*bb=binbuf_new();
    int r=0;
    if(NULL==filename)
      return false;


    if(dirname) {
      r=binbuf_read(bb, (char*)filename, const_cast<char*>(gem::files::expandEnv(dirname, true).c_str()), 1);
      if(0==r)logpost(NULL, 5, "found Gem-settings '%s' in '%s'", filename, dirname);
    } else {
      r=binbuf_read_via_path(bb, (char*)filename, (char*)".", 1);
      if(0==r)logpost(NULL, 5, "found Gem-settings '%s'", filename);
    }

    if(r){
      binbuf_free(bb);
      return false;
    }

    int ac=binbuf_getnatom(bb);
    t_atom*av=binbuf_getvec(bb);

    std::string s;
    t_atom*a=NULL;
    int state=0; /* 0=(next is ID); 1=(next is value); 2=(next is ignored) */

    while(ac--) {
      if (av->a_type == A_SEMI) {
        // done
        if(!s.empty()) {
          set(s, a);
        }
        state=0;
        s.clear();
      } else {
        switch (state) {
        case 0:
          s=atom_getsymbol(av)->s_name;
          state=1;
          break;
        case 1:
          a=av;
          state=2;
          break;
        default:
          break;
        }
      }
      av++;
    }

    binbuf_free(bb);
    return true;
  }

  void print(void) {
    std::map <std::string , t_atom>::iterator it;
    for(it = data.begin(); 
        it != data.end();
        it++)
      {
        if(!it->first.empty()) {
          startpost("key ['%s']: '", it->first.c_str());
          postatom(1, &it->second);
	  
          post("'");
        }
      }
  }



  PIMPL(void)
  {
    int i=0;
#ifdef GEM_DEFAULT_FONT
    set("font.face", GEM_DEFAULT_FONT);
#endif

    setEnv("settings.file", "GEM_SETTINGS");
    t_atom*a=NULL;
    a=get("settings.file");
    if(a) {
      std::string s=atom_getsymbol(a)->s_name;
      open(gem::files::expandEnv(s.c_str(), true).c_str(), ".");
    } else {
      while(s_configdir[i]) {
	open(GEM_SETTINGS_FILE, s_configdir[i]);
	i++;
      }
      open(GEM_SETTINGS_FILE, ".");
    }

    /* legacy settings via environmental variables */
    setEnv("texture.rectangle", "GEM_RECTANGLE_TEXTURE");
    setEnv("singlecontext", "GEM_SINGLE_CONTEXT"); // hmm, what's a better new name for this?
    setEnv("font.face", "GEM_DEFAULT_FONT");


    t_gemclass *c = (t_gemclass*)class_new(gensym("Gem"), 0, 0, 0, 0, A_NULL);
    set("gem.path", c->c_externdir->s_name);

    //    print();
  }

  ~PIMPL(void) {
    
  }

  void setEnv(std::string key, const std::string env) {
    if(env.empty())return;
    if(key.empty())return;

    char*result=getenv(env.c_str());
    if(NULL==result) {
      return;
    }

    t_atom a;
    errno=0;

    /* try integer */
    long l=strtol(result, NULL, 0);
    if(0==errno) {
      SETFLOAT(&a, l);
      set(key, &a);
    }

    /* try float */
    double d=strtod(result, NULL);
    if(0==errno) {
      SETFLOAT(&a, d);
      set(key, &a);
    }

    /* try symbol */
    SETSYMBOL(&a, gensym(result));
    set(key, &a);

    // we ignore lists and other complex things for now
  }
};
static PIMPL*settings=NULL;
};



/* GemSettings: the public API */


/* public static functions */
void GemSettings::init() {
  if(settings)return;
  settings=new PIMPL(); 
}
void GemSettings::print() {
  if(!settings)return;
  settings->print();
}
void GemSettings::save() {
  if(!settings)return;
  post("GemSettings: save not yet implemented!");
}



t_atom*GemSettings::get(const std::string s) {
  if(NULL==settings) init();
  return settings->get(s.c_str());
}
void GemSettings::set(const std::string s, t_atom*v) {
  settings->set(s.c_str(), v);
}


void GemSettings::get(const std::string key, int&value) {
  t_atom*a=get(key);
  if(a && A_FLOAT==a->a_type) {
    value=atom_getint(a);
  }
}
void GemSettings::get(const std::string key, t_float&value) {
  t_atom*a=get(key);
  if(a && A_FLOAT==a->a_type) {
    value=atom_getfloat(a);
  }
}

void GemSettings::get(const std::string key, std::string&value) {
  t_atom*a=get(key);
  if(a) {
    value=atom_getsymbol(a)->s_name;
  }
}


std::vector<std::string>GemSettings::keys(void) {
  std::vector<std::string>result;
  if(NULL==settings) init();
  if(NULL!=settings) {
    std::map<std::string, t_atom>::iterator it=settings->data.begin();
    while(settings->data.end() != it) {
      result.push_back(it->first);
    }
  }
  return result;
}
