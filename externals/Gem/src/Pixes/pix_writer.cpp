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


#include "pix_writer.h"
#ifdef _MSC_VER  /* This is only for Microsoft's compiler, not cygwin, e.g. */
# define snprintf _snprintf
# define vsnprintf _vsnprintf
#endif

#include "Gem/Manager.h"
#include "Gem/ImageIO.h"

#include <stdio.h>

CPPEXTERN_NEW_WITH_GIMME(pix_writer);

  /////////////////////////////////////////////////////////
//
// pix_writer
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_writer :: pix_writer(int argc, t_atom *argv)
  : m_banged(false), 
    m_automatic(false), m_autocount(0),
    m_filetype(0)
{
  snprintf(m_pathname, MAXPDSTRING, "gem");
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_writer :: ~pix_writer()
{
}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void pix_writer :: processImage(imageStruct &image)
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

    mem2image(&image, m_filename, m_filetype);

    m_autocount++;
    m_banged = false;
  }
}


void pix_writer :: fileMess(int argc, t_atom *argv)
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
// static member functions
//
/////////////////////////////////////////////////////////
void pix_writer :: obj_setupCallback(t_class *classPtr)
{
  class_addbang(classPtr, reinterpret_cast<t_method>(&pix_writer::bangMessCallback));

  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_writer::fileMessCallback),
		  gensym("file"), A_GIMME, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_writer::autoMessCallback),
		  gensym("auto"), A_FLOAT, A_NULL);
}

void pix_writer :: fileMessCallback(void *data, t_symbol *s, int argc, t_atom *argv)
{
  GetMyClass(data)->fileMess(argc, argv);
}
void pix_writer :: autoMessCallback(void *data, t_floatarg on)
{
  GetMyClass(data)->m_automatic=(on!=0);
}
void pix_writer :: bangMessCallback(void *data)
{
  GetMyClass(data)->m_banged=true;
}

