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
//    Copyright (c) 2005 Georg Holzmann <grh@mur.at>
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "textoutline.h"

#if defined FTGL && !defined HAVE_FTGL_FTGL_H
# include "FTGLOutlineFont.h"
#endif

CPPEXTERN_NEW_WITH_GIMME(textoutline);

/////////////////////////////////////////////////////////
//
// textoutline
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
#ifdef FTGL
textoutline :: textoutline(int argc, t_atom *argv)
  : TextBase(argc, argv) {
  fontNameMess(DEFAULT_FONT);
} 
textoutline :: ~textoutline() {
  if(m_font)delete m_font;m_font=NULL;
}
FTFont *textoutline :: makeFont(const char*fontfile){
  if(m_font)delete m_font; m_font=NULL;
  m_font =  new FTGLOutlineFont(fontfile);
  if (m_font->Error()){
    delete m_font;
    m_font = NULL;
  }
  return m_font;
}


#else /* !FTGL */

textoutline :: textoutline(int argc, t_atom *argv)
  : TextBase(argc, argv)
{ }

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
textoutline :: ~textoutline()
{}

#endif /* FTGL */

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void textoutline :: obj_setupCallback(t_class *)
{ }
