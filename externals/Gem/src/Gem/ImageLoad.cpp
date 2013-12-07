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
//    Copyright (c) 2002-2003 james tittle/tigital
//
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////
#include "ImageIO.h"
#include "Gem/RTE.h"
#include "Gem/SynchedWorkerThread.h"

#include "plugins/imageloader.h"

namespace gem { namespace image {
  struct PixImageThreadLoader : public gem::thread::SynchedWorkerThread {
    struct InData {
      load::callback cb;
      void*userdata;
      std::string filename;
      InData(load::callback cb_, void*data_, std::string fname) :
        cb(cb_),
        userdata(data_),
        filename(fname) {
      };
    };

    struct OutData {
      load::callback cb;
      void*userdata;
      imageStruct*img;
      gem::Properties props;
      OutData(const InData&in) :
        cb(in.cb),
        userdata(in.userdata),
        img(NULL) { 
      };
    };

    static gem::plugins::imageloader*s_imageloader;
    PixImageThreadLoader(void) :
      SynchedWorkerThread(false)
    { 
      if(NULL==s_imageloader) {
	s_imageloader=gem::plugins::imageloader::getInstance();
      }
      if(!s_imageloader)
	throw(40);

      if(!s_imageloader->isThreadable()) 
        throw(42);
      start();
    }
    virtual ~PixImageThreadLoader(void) {
    }

    virtual void* process(id_t ID, void*data) {
      if(!data){
        //        post("========================================= oops: %d", ID);
        return NULL;
      }

      InData*in=reinterpret_cast<InData*>(data);
      OutData*out=new OutData(*in);
      if(!out) {
        return NULL;
      }
      // DOIT
      out->img=new imageStruct;
      if(!s_imageloader->load(in->filename, *out->img, out->props)) {
        delete out->img;
        out->img=0;
      }
      void*result=reinterpret_cast<void*>(out);
      //post("processing[%d] %p -> %p", ID, data, result);
      return result;
    };

    virtual void done(id_t ID, void*data) {
      OutData*out=reinterpret_cast<OutData*>(data);
      if(out) {
        (*(out->cb))(out->userdata, ID, out->img, out->props);
        delete out;
      } else {
        error("loaded image:%d with no data!", ID);
      }
    };

    virtual bool queue(id_t&ID, load::callback cb, void*userdata, std::string filename) {
      InData *in = new InData(cb, userdata, filename);
      return SynchedWorkerThread::queue(ID, reinterpret_cast<void*>(in));
    };

    static PixImageThreadLoader*getInstance(bool retry=true) {
      static bool didit=false;
      if(!retry && didit)
	return s_instance;
      didit=true;

      if(NULL==s_instance) {
        try {
          s_instance=new PixImageThreadLoader();
        } catch(int i) {
          i=0;
          static bool dunnit=false;
          if(!dunnit) {
            logpost(NULL, 5, "threaded ImageLoading not supported!");
          }
          dunnit=true;
        }

        if(s_instance)
          s_instance->setPolling(true);
      }

      return s_instance;
    };

  private:
    static PixImageThreadLoader*s_instance;
  };
  PixImageThreadLoader*PixImageThreadLoader::s_instance=NULL;
  gem::plugins::imageloader*PixImageThreadLoader::s_imageloader=NULL;


  const load::id_t load::IMMEDIATE= 0;
  const load::id_t load::INVALID  =~0;

  bool load::sync(const std::string filename,
                  imageStruct&result,
                  gem::Properties&props) {
    if(!PixImageThreadLoader::s_imageloader)
      PixImageThreadLoader::s_imageloader=gem::plugins::imageloader::getInstance();
    if((PixImageThreadLoader::s_imageloader) && 
       (PixImageThreadLoader::s_imageloader->load(filename, result, props))) 
      {
	return true;
      }
    return false;
  }
  
  bool load::async(load::callback cb,
                   void*userdata,
                   const std::string filename,
                   id_t&ID) {
    if(NULL==cb) {
      ID=INVALID;
      return false;
    }

    PixImageThreadLoader*threadloader=PixImageThreadLoader::getInstance();

    //post("threadloader %p", threadloader);
    
    if(threadloader) {
      return threadloader->queue(ID, cb, userdata, filename);
    }
    return sync(cb, userdata, filename, ID);
  }

  bool load::sync(load::callback cb,
                  void*userdata,
                  const std::string filename,
                  id_t&ID) {
    if(NULL==cb) {
      ID=INVALID;
      return false;
    }
    imageStruct*result=new imageStruct;
    gem::Properties props;
    if(sync(filename, *result, props)) {
      ID=IMMEDIATE;
      (*cb)(userdata, ID, result, props);
      return true;
    }
    ID=INVALID;
    return false;
  }

  bool load::cancel(id_t ID) {
    PixImageThreadLoader*threadloader=PixImageThreadLoader::getInstance(false);
    if(threadloader) {
      return threadloader->cancel(ID);
    }
    return false;
  }

  bool load::setPolling(bool value) {
    PixImageThreadLoader*threadloader=PixImageThreadLoader::getInstance();
    if(threadloader) {
      return threadloader->setPolling(value);
    }
    return true;
  }
  void load::poll(void) {
    PixImageThreadLoader*threadloader=PixImageThreadLoader::getInstance(false);
    if(threadloader) {
      threadloader->dequeue();
    }
  }


}; // image
}; // gem


/***************************************************************************
 *
 * image2mem - Read in an image in various file formats
 *
 ***************************************************************************/
GEM_EXTERN imageStruct *image2mem(const char *filename)
{
  gem::Properties props;
  imageStruct *img = new imageStruct();
  if(gem::image::load::sync(filename, *img, props))
    return img;
  
  delete img;
  return NULL;
}
