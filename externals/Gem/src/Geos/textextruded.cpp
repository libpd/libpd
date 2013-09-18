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

#include "textextruded.h"

#if defined FTGL && !defined HAVE_FTGL_FTGL_H
# include "FTGLExtrdFont.h"
#endif

CPPEXTERN_NEW_WITH_GIMME(textextruded);

/////////////////////////////////////////////////////////
//
// textextruded
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
#ifdef FTGL
textextruded :: textextruded(int argc, t_atom *argv)
  : TextBase(argc, argv) {
  fontNameMess(DEFAULT_FONT);
} 
textextruded :: ~textextruded() {
  if(m_font)delete m_font;m_font=NULL;
}
FTFont *textextruded :: makeFont(const char*fontfile){
  if(m_font)delete m_font; m_font=NULL;
  m_font =  new FTGLExtrdFont(fontfile);
  if (m_font->Error()){
    delete m_font;
    m_font = NULL;
  }
  return m_font;
}

/////////////////////////////////////////////////////////
// setPrecision
//
/////////////////////////////////////////////////////////
void textextruded :: setDepth(float prec)
{
  m_fontDepth = prec;
  if(!m_font)return;
  m_font->Depth(m_fontDepth);
  setFontSize();
  setModified();
}
#else

textextruded :: textextruded(int argc, t_atom *argv)
  : TextBase(argc, argv)
{
  error("FTGL-support is needed for extruded fonts!");
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
textextruded :: ~textextruded()
{}

/////////////////////////////////////////////////////////
// setPrecision
//
/////////////////////////////////////////////////////////
void textextruded :: setDepth(float prec)
{}
#endif /* FTGL */

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void textextruded :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr, reinterpret_cast<t_method>(&textextruded::depthMessCallback),
		  gensym("depth"), A_FLOAT, A_NULL);  
}
void textextruded :: depthMessCallback(void *data, t_floatarg depth)
{
  GetMyClass(data)->setDepth(depth);
}
