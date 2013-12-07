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
  
#include "imageloader.h"
#include "plugins/PluginFactory.h"

#include "Gem/RTE.h"
#include "Gem/Exception.h"

gem::plugins::imageloader :: ~imageloader(void) {}

static gem::PluginFactoryRegistrar::dummy<gem::plugins::imageloader> fac_imageloaderdummy;

namespace gem { namespace plugins {
  class imageloaderMeta : public gem::plugins::imageloader {
  private:
    static imageloaderMeta*s_instance;
    std::vector<gem::plugins::imageloader*>m_loaders;
    std::vector<std::string>m_ids;
    bool m_canThread;
  public:
    imageloaderMeta(void) : m_canThread(true) {
      gem::PluginFactory<gem::plugins::imageloader>::loadPlugins("image");
      std::vector<std::string>available_ids=gem::PluginFactory<gem::plugins::imageloader>::getIDs();

      addLoader(available_ids, "magick");
      addLoader(available_ids);

      if(m_ids.size()>0) {
        startpost("Image loading support:");
        unsigned int i;
        for(i=0; i<m_ids.size(); i++) {
          startpost(" %s", m_ids[i].c_str());
        }
        endpost();
      }


      m_canThread=true;
      unsigned int i;
      for(i=0; i<m_loaders.size(); i++) {
        if(!m_loaders[i]->isThreadable()) {
          m_canThread=false;
          break;
        }
      }
    }
    bool addLoader( std::vector<std::string>available, std::string ID=std::string("")) {
      int count=0;

      std::vector<std::string>id;
      if(!ID.empty()) {
        // if requested 'cid' is in 'available' add it to the list of 'id's
        if(std::find(available.begin(), available.end(), ID)!=available.end()) {
          id.push_back(ID);
        } else {
          // request for an unavailable ID
          logpost(NULL, 6, "backend '%s' unavailable", ID.c_str());
          return false;
        }
      } else {
        // no 'ID' given: add all available IDs
        id=available;
      }

      unsigned int i=0;
      for(i=0; i<id.size(); i++) {
        std::string key=id[i];
        logpost(NULL, 6, "trying to add '%s' as backend", key.c_str());
        if(std::find(m_ids.begin(), m_ids.end(), key)==m_ids.end()) {
          // not yet added, do so now!
          gem::plugins::imageloader*loader=NULL;
          try {
            loader=gem::PluginFactory<gem::plugins::imageloader>::getInstance(key); 
          } catch(GemException x) {
            loader=NULL;
            logpost(NULL, 5, "cannot use image loader plugin '%s': %s", key.c_str(), x.what());
          }
          if(NULL==loader)continue;
          m_ids.push_back(key);
          m_loaders.push_back(loader);
          count++;
          logpost(NULL, 6, "added backend#%d '%s' @ 0x%x", m_loaders.size()-1, key.c_str(), loader);
        }
      }
      return (count>0);
    }

  public:
    virtual ~imageloaderMeta(void) {
      unsigned int i;
      for(i=0; i<m_loaders.size(); i++) {
        delete m_loaders[i];
        m_loaders[i]=NULL;
      }
    }

    virtual bool load(std::string filename, imageStruct&result, gem::Properties&props) {
      unsigned int i;
      for(i=0; i<m_loaders.size(); i++) {
        if(m_loaders[i]->load(filename, result, props))
          return true;
      }
      return false;
    }

    virtual bool isThreadable(void) {
      return m_canThread;
    }
  };  }; };


gem::plugins::imageloader*gem::plugins::imageloader::getInstance(void) {
  gem::plugins::imageloader*result=new imageloaderMeta();
 return result;
}
