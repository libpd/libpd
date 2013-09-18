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

#include "pix_snap.h"

#include "Gem/Manager.h"
#include "Gem/Cache.h"
#include "Gem/State.h"

CPPEXTERN_NEW_WITH_GIMME(pix_snap);

/////////////////////////////////////////////////////////
//
// pix_snap
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_snap :: pix_snap(int argc, t_atom *argv)
    	  : m_originalImage(NULL)
{
  m_pixBlock.image = m_imageStruct;
  m_pixBlock.image.data = NULL;
  if (argc == 4){
    m_x = atom_getint(&argv[0]);
    m_y = atom_getint(&argv[1]);
    m_width = atom_getint(&argv[2]);
    m_height = atom_getint(&argv[3]);
  } else if (argc == 2)	{
    m_x = m_y = 0;
    m_width = atom_getint(&argv[0]);
    m_height = atom_getint(&argv[1]);
  } else if (argc == 0)	{
    m_x = m_y = 0;
    m_width = m_height = 128;
  } else {
    error("needs 0, 2, or 4 values");
    m_x = m_y = 0;
    m_width = m_height = 128;
  }
  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("vert_pos"));
  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("vert_size"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_snap :: ~pix_snap()
{
    cleanImage();
}

/////////////////////////////////////////////////////////
// snapMess
//
/////////////////////////////////////////////////////////
void pix_snap :: snapMess()
{
  if(!GLEW_VERSION_1_1 && !GLEW_EXT_texture_object) return;

  if (m_cache&&m_cache->m_magic!=GEMCACHE_MAGIC)
    m_cache=NULL;
        
	if (m_width <= 0 || m_height <= 0)
	{
		error("Illegal size");
		return;
	}
	// do we need to remake the data?
	int makeNew = 0;

    // release previous data
    if (m_originalImage)
    {
		if (m_originalImage->xsize != m_width ||
			m_originalImage->ysize != m_height)
		{
			m_originalImage->clear();
			delete m_originalImage;
			m_originalImage = NULL;
			makeNew = 1;
		}
	}
	else
		makeNew = 1;

    if (makeNew)
	{
		m_originalImage = new imageStruct;
		m_originalImage->xsize = m_width;
		m_originalImage->ysize = m_height;
                /* magic: on __APPLE__ this could actually set to GL_BGRA_EXT ! */
                m_originalImage->setCsizeByFormat(GL_RGBA);
                m_originalImage->upsidedown = 0;

		m_originalImage->allocate(m_originalImage->xsize * m_originalImage->ysize * m_originalImage->csize);
	}

    glFinish();
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glPixelStorei(GL_PACK_ROW_LENGTH, 0);
    glPixelStorei(GL_PACK_SKIP_ROWS, 0);
    glPixelStorei(GL_PACK_SKIP_PIXELS, 0);

    glReadPixels(m_x, m_y, m_width, m_height,
    	    	 m_originalImage->format, m_originalImage->type, m_originalImage->data);    
           
    if (m_cache)
		m_cache->resendImage = 1;

    //post("snapped image"); 
}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void pix_snap :: render(GemState *state)
{
    // if we don't have an image, just return
    if (!m_originalImage)
		return;
    
    // do we need to reload the image?    
    if (m_cache&&m_cache->resendImage)
    {
      m_originalImage->refreshImage(&m_pixBlock.image);
    	m_pixBlock.newimage = 1;
    	m_cache->resendImage = 0;
    }
    
    state->set(GemState::_PIX, &m_pixBlock);
}

/////////////////////////////////////////////////////////
// postrender
//
/////////////////////////////////////////////////////////
void pix_snap :: postrender(GemState *state)
{
    m_pixBlock.newimage = 0;
    state->set(GemState::_PIX, static_cast<pixBlock*>(NULL));
}

/////////////////////////////////////////////////////////
// sizeMess
//
/////////////////////////////////////////////////////////
void pix_snap :: sizeMess(int width, int height)
{
	m_width = width;
    m_height = height;
}

/////////////////////////////////////////////////////////
// posMess
//
/////////////////////////////////////////////////////////
void pix_snap :: posMess(int x, int y)
{
    m_x = x;
    m_y = y;
}

/////////////////////////////////////////////////////////
// cleanImage
//
/////////////////////////////////////////////////////////
void pix_snap :: cleanImage()
{
    // release previous data
    if (m_originalImage)
    {
      delete m_originalImage;
      m_originalImage = NULL;

      m_pixBlock.image.clear();
    }
}

/////////////////////////////////////////////////////////
// static member functions
//
/////////////////////////////////////////////////////////
void pix_snap :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_snap::snapMessCallback),
    	    gensym("snap"), A_NULL);
    class_addbang(classPtr, reinterpret_cast<t_method>(&pix_snap::snapMessCallback));

    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_snap::sizeMessCallback),
    	    gensym("vert_size"), A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_snap::posMessCallback),
    	    gensym("vert_pos"), A_FLOAT, A_FLOAT, A_NULL);
}
void pix_snap :: snapMessCallback(void *data)
{
    GetMyClass(data)->snapMess();
}
void pix_snap :: sizeMessCallback(void *data, t_floatarg width, t_floatarg height)
{
    GetMyClass(data)->sizeMess(static_cast<int>(width), static_cast<int>(height));
}
void pix_snap :: posMessCallback(void *data, t_floatarg x, t_floatarg y)
{
    GetMyClass(data)->posMess(static_cast<int>(x), static_cast<int>(y));
}
