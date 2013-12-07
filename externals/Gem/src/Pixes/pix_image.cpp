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
#include "Gem/GemConfig.h"
#include "pix_image.h"

#include "Gem/State.h"

#ifdef _WIN32
# include <io.h>
# define close _close
# define snprintf _snprintf
#endif

#if defined(__unix__) || defined(__APPLE__) 
# include <unistd.h>
# include <strings.h>
#endif

#include <stdio.h>
#include "Gem/Cache.h"

CPPEXTERN_NEW_WITH_ONE_ARG(pix_image, t_symbol *, A_DEFSYM);



/////////////////////////////////////////////////////////
//
// pix_image
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_image :: pix_image(t_symbol *filename) :
  m_wantThread(true),
  m_loadedImage(NULL),
  m_id(gem::image::load::INVALID),
  m_infoOut(gem::RTE::Outlet(this))
{
  if(filename!=&s_)openMess(filename->s_name);
  gem::image::load::poll();
}

////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_image :: ~pix_image()
{
  gem::image::load::cancel(m_id);
  cleanImage();
  m_id=gem::image::load::INVALID;
}

void pix_image :: threadMess(bool onoff)
{
  m_wantThread=onoff;
}

/////////////////////////////////////////////////////////
// openMess
//
/////////////////////////////////////////////////////////
void pix_image :: openMess(std::string filename)
{
  if(filename.empty())return;

  gem::image::load::cancel(m_id);
 
  m_filename = findFile(filename);

  gem::image::load::callback cb = loadCallback;
  void*userdata=reinterpret_cast<void*>(this);

  m_id = gem::image::load::INVALID;

  bool success=false;
  if(m_wantThread) {
    success=gem::image::load::async(cb, userdata, m_filename, m_id);
  } else {
    success=gem::image::load:: sync(cb, userdata, m_filename, m_id);
  }
  if(gem::image::load::INVALID == m_id)
    success=false;

  std::vector<gem::any>atoms;
  gem::any value;

  if(success) {
    if(gem::image::load::IMMEDIATE!=m_id) {
      logpost(NULL, 5, "loading image '%s' with ID:%d", m_filename.c_str(), m_id);
      atoms.push_back(value=std::string("defer"));
      atoms.push_back(value=(int)m_id);
    } else {
      atoms.push_back(value=std::string("success"));
    }
  } else {
    error("loading of '%s' failed", m_filename.c_str());
    atoms.push_back(value=std::string("fail"));
  }
  atoms.push_back(value=m_filename);
  m_infoOut.send("load", atoms);
}


void    pix_image:: loaded(const gem::image::load::id_t ID, 
			   imageStruct*img,
			   const gem::Properties&props) {
  std::vector<gem::any>atoms;
  gem::any value;

  if(ID!=m_id || ID == gem::image::load::INVALID) {
    atoms.push_back(value=std::string("discard"));
    if(ID!=gem::image::load::INVALID)
      atoms.push_back(value=(int)ID);
    logpost(NULL, 5, "discarding image with ID %d", ID);
    m_infoOut.send("load", atoms);
    return;
  }

  cleanImage();
  if(img) {
    m_loadedImage=img;
    m_loadedImage->copy2Image(&m_pixBlock.image);
    m_pixBlock.newimage = 1;
    logpost(NULL, 4, "loaded image '%s'", m_filename.c_str());
    atoms.push_back(value=std::string("success"));
  } else {
    error("failed to load image '%s'", m_filename.c_str());
    atoms.push_back(value=std::string("fail"));
  }
  atoms.push_back(value=(int)ID);
  if(gem::image::load::IMMEDIATE!=m_id) {
    m_infoOut.send("load", atoms);
  }
}
void    pix_image:: loadCallback(void*data,
				 gem::image::load::id_t ID, 
				 imageStruct*img,
				 const gem::Properties&props) {
  pix_image*me=reinterpret_cast<pix_image*>(data);
  me->loaded(ID, img, props);
}



    	    	

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void pix_image :: render(GemState *state)
{
  gem::image::load::poll();

  // if we don't have an image, just return
  if (!m_loadedImage){
    return;
  }

  // do we need to reload the image?    
  if (m_cache&&m_cache->resendImage)
    {
      m_loadedImage->refreshImage(&m_pixBlock.image);
      m_pixBlock.newimage = 1;
      m_cache->resendImage = 0;
    }
  state->set(GemState::_PIX, &m_pixBlock);
}

/////////////////////////////////////////////////////////
// postrender
//
/////////////////////////////////////////////////////////
void pix_image :: postrender(GemState *state)
{
  m_pixBlock.newimage = 0;
  state->set(GemState::_PIX, static_cast<pixBlock*>(NULL));
}

/////////////////////////////////////////////////////////
// startRendering
//
/////////////////////////////////////////////////////////
void pix_image :: startRendering()
{
  if (!m_loadedImage) return;
  m_loadedImage->refreshImage(&m_pixBlock.image);
  m_pixBlock.newimage = 1;
}

/////////////////////////////////////////////////////////
// cleanImage
//
/////////////////////////////////////////////////////////
void pix_image :: cleanImage()
{
  // release previous data
  if (m_loadedImage)
    {
      delete m_loadedImage;
      m_loadedImage = NULL;
      m_pixBlock.image.clear();
      m_pixBlock.image.data = NULL;
    }
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_image :: obj_setupCallback(t_class *classPtr)
{
  CPPEXTERN_MSG1(classPtr, "open", openMess, std::string);
  CPPEXTERN_MSG1(classPtr, "thread", threadMess, bool);
}
