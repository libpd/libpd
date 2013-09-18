#include "PluginFactory.h"
#include "Gem/Settings.h"
#include "Gem/Files.h"
#include "Gem/Dylib.h"
#include "Gem/RTE.h"

#include "sstream"

using namespace gem;

class gem::BasePluginFactory::Pimpl {
  friend class BasePluginFactory;
  Pimpl(void) {
    
  }

  ~Pimpl(void) {

  }

  std::vector<std::string>p_loaded;

  std::map<std::string, void*>p_ctors;
};


gem::BasePluginFactory::BasePluginFactory(void) : m_pimpl(new Pimpl) {

}
gem::BasePluginFactory::~BasePluginFactory(void) {
  delete m_pimpl;  m_pimpl=NULL;
}

int gem::BasePluginFactory::doLoadPlugins(std::string basename, std::string path) {
  if(path.empty()){
    GemSettings::get("gem.path", path);
  }
  if(!path.empty()){
    path=path+std::string("/");
  }
  //std::cerr << "load plugins '" << basename << "' in '" << path << "'" << std::endl;

  std::string pattern = path+std::string("gem_") + basename+std::string("*")+GemDylib::getDefaultExtension();
  //std::cerr << "pattern : " << pattern << std::endl;

  unsigned int count=0;

  std::vector<std::string>files=gem::files::getFilenameListing(pattern);
  unsigned int i=0;

  for(i=0; i<files.size(); i++) {
    GemDylib*dll=NULL;
    const std::string f=files[i];
    // check whether this file has already been loaded
    // LATER make checks more sophisticated (like checking file-handles)
    bool alreadyloaded=false;
    unsigned int j;
    for(j=0; j<m_pimpl->p_loaded.size(); j++)
      if(f == m_pimpl->p_loaded[j]) {
	alreadyloaded=true;
	//std::cerr << "not reloading '"<<f<<"'"<<std::endl;
	break;
      }
    if(alreadyloaded)continue;

    //std::cerr << "dylib loading file '" << f << "'!" << std::endl;
    dll=NULL;
    try {
      dll=new GemDylib(f, "");
    } catch (GemException x) {
        // oops, on w32 this might simply be because getFilenameListing() stripped the path
        // so let's try again, with Path added...
        if(f.find(path) == f.npos) {
            try {
                std::string f1=path;
                f1+=f;
                dll=new GemDylib(f1, "");
            } catch (GemException x1) {
                // giving up
                //std::cerr << "library loading returned: " << x1.what() << std::endl;
                dll=NULL;
            }
        } else {
            //std::cerr << "library loading returned: " << x.what() << std::endl;
            dll=NULL;
        }
    }
    if(dll){ // loading succeeded
        try {
            m_pimpl->p_loaded.push_back(f);
            count++;
        } catch (GemException x) {
            //std::cerr << "plugin loading returned: " << x.what() << std::endl;
        }
    }

  }

  return count;
}

std::vector<std::string>gem::BasePluginFactory::get() {
  std::vector<std::string>result;
  if(m_pimpl) {
    std::map<std::string, void*>::iterator iter = m_pimpl->p_ctors.begin();
    for(; iter != m_pimpl->p_ctors.end(); ++iter) {
      if(NULL!=iter->second)
        result.push_back(iter->first);
    }
  }
  return result;
}

void*gem::BasePluginFactory::get(std::string id) {
  void*ctor=NULL;
  if(m_pimpl)
    ctor=m_pimpl->p_ctors[id];
  return ctor;
}

void gem::BasePluginFactory::set(std::string id, void*ptr) {
  if(m_pimpl)
    m_pimpl->p_ctors[id]=ptr;
}
