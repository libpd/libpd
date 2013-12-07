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

#include "text2d.h"

#if defined FTGL && !defined HAVE_FTGL_FTGL_H
# include "FTGLPixmapFont.h"
# include "FTGLBitmapFont.h"
#endif

CPPEXTERN_NEW_WITH_GIMME(text2d);

/////////////////////////////////////////////////////////
//
// text2d
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
#ifdef FTGL

text2d :: text2d(int argc, t_atom *argv)
  : TextBase(0,NULL), m_antialias(1), m_afont(NULL)
{
  fontNameMess(DEFAULT_FONT);
} 

text2d :: ~text2d() {
  if(m_font) delete m_font; m_font=NULL;
  if(m_afont)delete m_afont;m_afont=NULL;
}
FTFont *text2d :: makeFont(const char*fontfile){
  if(m_font) delete m_font;  m_font=NULL;
  if(m_afont)delete m_afont; m_afont=NULL;

  m_font =  new FTGLBitmapFont(fontfile);
  if (m_font->Error()){
    delete m_font;
    m_font = NULL;
  }
  m_afont =  new FTGLPixmapFont(fontfile);
  if (m_afont->Error()){
    delete m_afont;
    m_afont = NULL;
  }
  
  return m_font;
}
/////////////////////////////////////////////////////////
// setFontSize
//
/////////////////////////////////////////////////////////
void text2d :: setFontSize(t_float size){
  m_fontSize = size;
  int isize=static_cast<int>(m_fontSize);
  if (m_font)if (! m_font->FaceSize(isize) ) {
    error("GEMtext: unable set fontsize !");
  }
  if (m_afont)if (! m_afont->FaceSize(isize) ) {
    error("GEMtext: unable set fontsize !");
  }
  setModified();
}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void text2d :: render(GemState *)
{
  if (m_theText.empty() || !(m_afont || m_font))return;
  if (m_antialias && !m_afont)m_antialias=0;
  if (!m_antialias && !m_font)m_antialias=1;
  float x1=0, y1=0, z1=0, x2=0, y2=0, z2=0;
  float width=0, height=0, y_offset=0, ascender=0;
  unsigned int i;

  if(m_antialias && m_afont)
  {
    // Get ascender height (= height of the text)
    ascender = m_afont->Ascender();

    // step through the lines
    for(i=0; i<m_theText.size(); i++)
    {
      m_afont->BBox(m_theText[i].c_str(), x1, y1, z1, x2, y2, z2); // FTGL
      y_offset = m_lineDist[i]*m_fontSize;

    if (m_widthJus == LEFT)       width = x1;
    else if (m_widthJus == RIGHT) width = x2-x1;
    else if (m_widthJus == CENTER)width = x2 / 2.f;

      if (m_heightJus == BOTTOM || m_heightJus == BASEH)
        height = y_offset;
      else if (m_heightJus == TOP)   height = ascender + y_offset;
      else if (m_heightJus == MIDDLE)height = (ascender/2.f) + y_offset;

    glPushMatrix();

    glRasterPos2i(0,0);
    glBitmap(0,0,0.0,0.0,-width,-height, NULL);
      m_afont->Render(m_theText[i].c_str());

    glPopMatrix();
    }
  }
  else if (m_font) 
  {
    // Get ascender height (= height of the text)
    ascender = m_font->Ascender();

    // step through the lines
    for(i=0; i<m_theText.size(); i++)
    {
      m_font->BBox(m_theText[i].c_str(), x1, y1, z1, x2, y2, z2); // FTGL
      y_offset = m_lineDist[i]*m_fontSize;

    if (m_widthJus == LEFT)       width = x1;
    else if (m_widthJus == RIGHT) width = x2-x1;
    else if (m_widthJus == CENTER)width = x2 / 2.f;

      if (m_heightJus == BOTTOM || m_heightJus == BASEH)
        height = y_offset;
      else if (m_heightJus == TOP)   height = ascender + y_offset;
      else if (m_heightJus == MIDDLE)height = (ascender/2.f) + y_offset;

    glPushMatrix();

    glRasterPos2i(0,0);
    glBitmap(0,0,0.0,0.0,-width,-height, NULL);
      m_font->Render(m_theText[i].c_str());

    glPopMatrix();
  }
}
}


#else /* !FTGL */

text2d :: text2d(int argc, t_atom *argv)
  : TextBase(argc, argv)
{ }

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
text2d :: ~text2d()
{}

void text2d :: render(GemState*){}

#endif /* FTGL */

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void text2d :: obj_setupCallback(t_class *classPtr )
{ 
  class_addmethod(classPtr, reinterpret_cast<t_method>(&text2d::aliasMessCallback),
		  gensym("alias"), A_FLOAT, A_NULL);
}

void text2d :: aliasMess(int io)
{
  m_antialias = io;
}
void text2d :: aliasMessCallback(void *data, t_floatarg io)
{
  GetMyClass(data)->aliasMess(static_cast<int>(io));
}
