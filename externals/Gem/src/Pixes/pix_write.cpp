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
//    Copyright (c) 2002 James Tittle & Chris Clepper
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "pix_write.h"

#include "Gem/Manager.h"
#include "Gem/Cache.h"
#include "Gem/ImageIO.h"

#include <stdio.h>
#include <string.h>

#ifdef _MSC_VER  /* This is only for Microsoft's compiler, not cygwin, e.g. */
# define snprintf _snprintf
# define vsnprintf _vsnprintf
#endif


CPPEXTERN_NEW_WITH_GIMME(pix_write);

  /////////////////////////////////////////////////////////
//
// pix_write
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_write :: pix_write(int argc, t_atom *argv)
  : m_originalImage(NULL)
{
  m_xoff = m_yoff = 0;
  m_width = m_height = 0;
  if (argc == 4) {
    m_xoff = atom_getint(&argv[0]);
    m_yoff = atom_getint(&argv[1]);
    m_width = atom_getint(&argv[2]);
    m_height = atom_getint(&argv[3]);
  } else if (argc == 2) {
    m_width = atom_getint(&argv[0]);
    m_height = atom_getint(&argv[1]);
  } else if (argc != 0){
    error("needs 0, 2, or 4 values");
    m_xoff = m_yoff = 0;
    m_width = m_height = 128;
  }

  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("vert_pos"));
  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("vert_size"));

  m_automatic = false;
  m_autocount = 0;
  m_filetype=0;
  snprintf(m_pathname, MAXPDSTRING, "gem");

  m_banged = false;


  m_originalImage = new imageStruct();
  m_originalImage->xsize=m_width;
  m_originalImage->ysize=m_height;
  m_originalImage->setCsizeByFormat(GL_RGBA);
  m_originalImage->allocate();
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_write :: ~pix_write()
{
  cleanImage();
}


/////////////////////////////////////////////////////////
// extension checks
//
/////////////////////////////////////////////////////////
bool pix_write :: isRunnable(void) {
  if(GLEW_VERSION_1_1 || GLEW_EXT_texture_object)
    return true;

  error("your system lacks texture support");
  return false;
}


/////////////////////////////////////////////////////////
// writeMess
//
/////////////////////////////////////////////////////////
void pix_write :: doWrite()
{
  int width  = m_width;
  int height = m_height;

  GemMan::getDimen(((m_width >0)?NULL:&width ),
		   ((m_height>0)?NULL:&height));

  m_originalImage->xsize = width;
  m_originalImage->ysize = height;

#ifndef __APPLE__
  m_originalImage->setCsizeByFormat(GL_RGB);
#else
  m_originalImage->setCsizeByFormat(GL_RGBA);
#endif /* APPLE */

  m_originalImage->reallocate();

  /* the orientation is always correct, since we get it from openGL */
  /* if we do need flipping, this must be handled in mem2image() */
  m_originalImage->upsidedown=false;


  glReadPixels(m_xoff, m_yoff, width, height,
               m_originalImage->format, m_originalImage->type, m_originalImage->data);

#if 0 // asynchronous texture fetching idea sketch
/* Enable AGP storage hints */
	glPixelStorei( GL_UNPACK_CLIENT_STORAGE_APPLE, 1 );
	glTextureRangeAPPLE(...);
	glTexParameteri(..., GL_TEXTURE_STORAGE_HINT_APPLE, GL_STORAGE_SHARED_APPLE );
	
	/* Copy from Frame Buffer */
	glCopyTexSubImage2d(...);
	
	/* Flush into AGP */
	glFlush(...);
	
	/* Pull out of AGP */
	glGetTexImage(...);
#endif


  mem2image(m_originalImage, m_filename, m_filetype);
}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void pix_write :: render(GemState *state)
{
  if (m_automatic || m_banged) {
    char *extension;
    if (m_filetype<0)m_filetype=0;
    if (m_filetype==0) {
      extension=(char*)"tif";
    } else {
      extension=(char*)"jpg";
    }
    
    snprintf(m_filename, (size_t)(MAXPDSTRING+10), "%s%05d.%s", m_pathname, m_autocount, extension);
    
    m_autocount++;
    m_banged = false;
    doWrite();
  }
}


/////////////////////////////////////////////////////////
// sizeMess
//
/////////////////////////////////////////////////////////
void pix_write :: sizeMess(int width, int height)
{
  m_width = width;
  m_height = height;
}

/////////////////////////////////////////////////////////
// posMess
//
/////////////////////////////////////////////////////////
void pix_write :: posMess(int x, int y)
{
  m_xoff = x;
  m_yoff = y;
}

void pix_write :: fileMess(int argc, t_atom *argv)
{
  char *extension = (char*)".tif";
  if (argc) {
    if (argv->a_type == A_SYMBOL) {
      atom_string(argv++, m_pathname, MAXPDSTRING);
      argc--;
      snprintf(m_filename, (size_t)(MAXPDSTRING+10), "%s.%s", m_pathname, extension);
    }
    if (argc>0)
      m_filetype = atom_getint(argv);
  }

  m_autocount = 0;
}

/////////////////////////////////////////////////////////
// cleanImage
//
/////////////////////////////////////////////////////////
void pix_write :: cleanImage()
{
  // release previous data
  if (m_originalImage)
    {
      delete m_originalImage;
      m_originalImage = NULL;
    }
}

/////////////////////////////////////////////////////////
// static member functions
//
/////////////////////////////////////////////////////////
void pix_write :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_write::fileMessCallback),
		  gensym("file"), A_GIMME, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_write::autoMessCallback),
		  gensym("auto"), A_FLOAT, A_NULL);
  class_addbang(classPtr, reinterpret_cast<t_method>(&pix_write::bangMessCallback));

  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_write::sizeMessCallback),
		  gensym("vert_size"), A_FLOAT, A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_write::posMessCallback),
		  gensym("vert_pos"), A_FLOAT, A_FLOAT, A_NULL);
}

void pix_write :: fileMessCallback(void *data, t_symbol *s, int argc, t_atom *argv)
{
  GetMyClass(data)->fileMess(argc, argv);
}
void pix_write :: autoMessCallback(void *data, t_floatarg on)
{
  GetMyClass(data)->m_automatic=(on!=0);
}
void pix_write :: bangMessCallback(void *data)
{
  GetMyClass(data)->m_banged=true;
}

void pix_write :: sizeMessCallback(void *data, t_floatarg width, t_floatarg height)
{
  GetMyClass(data)->sizeMess(static_cast<int>(width), static_cast<int>(height));
}
void pix_write :: posMessCallback(void *data, t_floatarg x, t_floatarg y)
{
  GetMyClass(data)->posMess(static_cast<int>(x), static_cast<int>(y));
}
