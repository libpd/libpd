/*-----------------------------------------------------------------
  LOG
  GEM - Graphics Environment for Multimedia

  A text

  Copyright (c) 1997-1999 Mark Danks. mark@danks.org
  Copyright (c) Günther Geiger. geiger@epy.co.at
  Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
  Copyright (c) 2005 Georg Holzmann <grh@mur.at>
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

  -----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_BASE_TEXTBASE_H_
#define _INCLUDE__GEM_BASE_TEXTBASE_H_


#ifdef _WIN32
# pragma warning( disable : 4786 )
# pragma warning( disable : 4788 )
#endif

#include "Gem/GemConfig.h"

#include "Base/GemBase.h"

#include <vector>
#include <string>

#ifdef FTGL
# define FONT_SCALE 0.2/3.0
# ifdef HAVE_FTGL_FTGL_H
#  include "FTGL/ftgl.h"
# else
#  include "FTFont.h"
# endif
#else
# define FONT_SCALE 1.0
#endif

using std::vector;
using std::string;
using std::wstring;

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  TextBase
    
  Base class for text objects

  DESCRIPTION

  "ft1" - The font size
  "text" - The text to draw
  
  -----------------------------------------------------------------*/
class GEM_EXTERN TextBase : public GemBase
{
  CPPEXTERN_HEADER(TextBase, GemBase);

    public:

  //////////
  // Constructor with args
  TextBase(int argc, t_atom *argv);
    	
    	
 protected:
  
  //////////
  // Destructor
  virtual ~TextBase();

  //////////
  // Do the rendering
  virtual void	render(GemState*);

  //////////
  // break a string according to '\n'
  virtual void  breakLine(wstring line);

  //-- moocow
  //////////
  // Set the text string from an ASCII list
  virtual void  stringMess(int argc, t_atom *argv);
  //-- /moocow
 
  //////////
  // Set the text string
  virtual void  textMess(int argc, t_atom *argv);

  //////////
  // The font to use
  virtual void  fontNameMess(const std::string filename);

  //////////
  // set line distance
  virtual void linedistMess(float dist);

  //////////
  // Set the font size
  virtual void	setFontSize(float size);
  virtual void  setFontSize();

  //////////
  // Set the precision for rendering
  virtual void	setPrecision(float prec);

  //////////
  // The different types of justification
  enum JustifyWidth { LEFT, RIGHT, CENTER, BASEW };
  enum JustifyHeight { BOTTOM, TOP, MIDDLE, BASEH };
  enum JustifyDepth { FRONT, BACK, HALFWAY, BASED };
	
  //////////
  // Set the justification
  virtual void setJustification(JustifyWidth wType);
  virtual void setJustification(JustifyWidth wType, JustifyHeight hType);
  virtual void setJustification(JustifyWidth wType, JustifyHeight hType, JustifyDepth dType);

  //////////
  // do the justification
  // x1,...,z2 just defines the bounding box of the rendered string.
  // y_offset is the offset of the current line
  virtual void justifyFont(float x1, float y1, float z1,
			   float x2, float y2, float z2, float y_offset=0);


  //-----------------------------------
  // GROUP:	Member variables
  //-----------------------------------
    
  //////////
  // The text to display
  // (one entry for each line)
  vector<wstring> m_theText;

  //////////
  // distance between the lines
  // (1 = 1 line, 0.5 = 0.5 lines, ...)
  float m_dist;

  ///////////
    // vector with the offset
    // of the individual lines
    vector<float> m_lineDist;

    //////////
    // Do we have a valid font?
    int m_valid;
    	
    //////////
    // The font fize
    float		m_fontSize;

    //////////
    // The font depth (only for extruded fonts)
    float		m_fontDepth;

    //////////
    // The rendering precision
    float		m_precision;

    //////////
    // The width justification
    JustifyWidth	m_widthJus;

    //////////
    // The height justification
    JustifyHeight	m_heightJus;

    //////////
    // The depth justification
    JustifyDepth	m_depthJus;

    //////////
    // The inlet
    t_inlet         *m_inlet;


    //////////
    // The default font name
    static std::string DEFAULT_FONT;
 
    //////////
    // The font structure
#ifdef FTGL
    FTFont		*m_font;
    /* this should delete (m_font) if it is notnull and recreate it.
     * a pointer to the new structure is returned (and is set to m_font).
     * if creation fails, the font is cleaned-up and NULL is returned
     */
    virtual FTFont* makeFont(const char*fontname)=0;

    /* this is just handy to reload a font */
    t_symbol* m_fontname;
    /* on starting to render, we reload the font, to make sure it is there 
     * this rids us of having to reload the font by hand everytime the rendering is restarted
     */
    virtual  void startRendering(void);

    /* render one line of the text */
    virtual void renderLine(const char*line,float dist);
    virtual void renderLine(const wchar_t*line,float dist);
#endif
  
 private:
    	    
    ///////////
   // helpers:

   ///////////
   // helper to make the
   // line distance vector
   void makeLineDist();

   //////////
   // Static member functions
   static void 	textMessCallback(void *data, t_symbol *, int argc, t_atom *argv);
   static void  stringMessCallback(void *data, t_symbol *, int argc, t_atom *argv); //--moo
   static void 	fontSizeMessCallback(void *data, t_floatarg size);
   static void  linedistMessCallback(void *data, t_floatarg dist);
   static void 	precisionMessCallback(void *data, t_floatarg prec);
   static void 	fontNameMessCallback(void *data, t_symbol *s);
   static void 	justifyMessCallback(void *data, t_symbol *, int, t_atom*);
};

#endif	// for header file
