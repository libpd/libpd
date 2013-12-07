////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-2000 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    Copyright (c) 2005 Georg Holzmann <grh@mur.at>
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

/*
 * FIXXME: check how font handling behaves with multiple contexts 
 */

#include "TextBase.h"
#include "Gem/Settings.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
# include <io.h>
# define close _close
typedef unsigned __int8 uint8_t;
#else
# include <unistd.h>
#endif

std::string TextBase::DEFAULT_FONT = "vera.ttf";

/////////////////////////////////////////////////////////
//
// TextBase
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
#ifdef FTGL
TextBase :: TextBase(int argc, t_atom *argv)
  :
  m_dist(1), m_valid(0), m_fontSize(20), m_fontDepth(20), m_precision(1.f),
  m_widthJus(CENTER), m_heightJus(MIDDLE), m_depthJus(HALFWAY), 
  m_inlet(NULL),
  m_font(NULL), m_fontname(NULL)
{
  // initial text
  GemSettings::get("font.face", DEFAULT_FONT);
  GemSettings::get("font.size", m_fontSize);


  m_theText.push_back(L"gem");
  makeLineDist();
  if(argc)textMess(argc, argv);

  m_inlet = inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("ft1"));
}

void TextBase :: startRendering(void) {
  if(NULL==m_font) {
    if(m_fontname)
      fontNameMess(m_fontname->s_name);
  }
}


////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void TextBase :: renderLine(const char*line, float dist) {
  float x1=0, y1=0, z1=0, x2=0, y2=0, z2=0;
#if 0
  startpost("renderline: "); {
    const char*c=line;
    while(c) {
      startpost("%c (%x)", c, c);
      c++;
    }
  }
#endif
  m_font->BBox(line, x1, y1, z1, x2, y2, z2); // FTGL

  glPushMatrix();
  glNormal3f(0.0, 0.0, 1.0);
  justifyFont(x1, y1, z1, x2, y2, z2, dist);
  m_font->Render(line);
  glPopMatrix();
}

void TextBase :: renderLine(const wchar_t*line, float dist) {
  float x1=0, y1=0, z1=0, x2=0, y2=0, z2=0;
  m_font->BBox(line, x1, y1, z1, x2, y2, z2); // FTGL

  glPushMatrix();
  glNormal3f(0.0, 0.0, 1.0);
  justifyFont(x1, y1, z1, x2, y2, z2, dist);
  m_font->Render(line);
  glPopMatrix();
}

void TextBase :: render(GemState *)
{
  unsigned int i=0;
  if (m_theText.empty() || !m_font)return;
  // step through the lines
  for(i=0; i<m_theText.size(); i++)
    {
      renderLine(m_theText[i].c_str(), m_lineDist[i]*m_fontSize);
    }
}

////////////////////////////////////////////////////////
// setFontSize
//
////////////////////////////////////////////////////////
void TextBase :: setFontSize(t_float size){
  m_fontSize = size;
  if (!m_font)return;
  if (! m_font->FaceSize(static_cast<int>(m_fontSize)) ) {
    error("unable to set fontsize !");
  }
  setModified();
}
////////////////////////////////////////////////////////
// setPrecision
//
////////////////////////////////////////////////////////
void TextBase :: setPrecision(float prec)
{
  m_precision = prec;
  error("no settable precision for FTGL !");
}

////////////////////////////////////////////////////////
// fontNameMess
//
////////////////////////////////////////////////////////
void TextBase :: fontNameMess(const std::string filename){
  m_valid = 0;
  const char *bufptr=NULL;
  int fd=-1;

  if(filename.empty()){
    error("no font-file specified");
    return;
  }
  std::string fn = findFile(filename);
  bufptr=fn.c_str();

  /* try to open the file */
  FILE*file = fopen(bufptr, "r");
  if (!file) {
    error("cannot find font-file '%s'", bufptr);
    return;
  }
  fclose(file);

  /* now read font */
  if(m_font)delete m_font; m_font=NULL;
  if (makeFont(bufptr)==NULL){
    error("unable to open font '%s'", bufptr);
    return;
  }
  m_fontname=gensym(filename.c_str());

  setFontSize(m_fontSize);
  m_font->Depth(m_fontDepth);
  m_font->CharMap(ft_encoding_unicode);

  setModified();
}

#else /* !FTGL */

TextBase :: TextBase(int argc, t_atom *argv){
  static bool first_time=true;
  if (first_time){
    post("Gem has been compiled without FONT-support !");
    first_time=false;
  }
  m_inlet = inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("ft1"));
}

/////////////////////////////////////////////////////////
// setFontSize
//
/////////////////////////////////////////////////////////
void TextBase :: setFontSize(t_float size)
{}
/////////////////////////////////////////////////////////
// setPrecision
//
/////////////////////////////////////////////////////////
void TextBase :: setPrecision(float prec)
{}

/////////////////////////////////////////////////////////
// fontNameMess
//
/////////////////////////////////////////////////////////
void TextBase :: fontNameMess(const std::string s)
{}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void TextBase :: render(GemState*)
{/* a no-op */ }

#endif /* FTGL */
/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
TextBase :: ~TextBase(){
  /* textbase deletion */
  if(m_inlet)inlet_free(m_inlet);
}

/////////////////////////////////////////////////////////
// setJustification
//
/////////////////////////////////////////////////////////
void TextBase :: setFontSize(){
  setFontSize(m_fontSize);
}

/////////////////////////////////////////////////////////
// setJustification
//
/////////////////////////////////////////////////////////
void TextBase :: setJustification(JustifyWidth wType, JustifyHeight hType, JustifyDepth dType){
  m_widthJus = wType;
  m_heightJus = hType;
  m_depthJus = dType;

  makeLineDist();
}
void TextBase :: setJustification(JustifyWidth wType, JustifyHeight hType)
{
  m_widthJus = wType;
  m_heightJus = hType;

  makeLineDist();
}
void TextBase :: setJustification(JustifyWidth wType)
{
  m_widthJus = wType;
}

void TextBase :: justifyFont(float x1, float y1, float z1,
                             float x2, float y2, float z2, float y_offset)
{
  float width  = 0.f;
  float height = 0.f;
  float depth  = 0.f;

  // Get ascender height (= height of the text)
#ifdef FTGL
  float ascender = m_font->Ascender();
#else
  // we don't have any ascender when not using FTGL
  float ascender = m_fontSize;
#endif

  if (m_widthJus == LEFT)       width = x1;
  else if (m_widthJus == RIGHT) width = x2-x1;
  else if (m_widthJus == CENTER)width = x2 / 2.f;
  else if (m_widthJus == BASEW) width = 0;

  //  if (m_heightJus == BOTTOM)     height = y1;
  //  else if (m_heightJus == TOP)   height = y2-y1;
  //  else if (m_heightJus == MIDDLE)height = y2 / 2.f;
  //  else if (m_heightJus == BASEH) height = 0;

  if (m_heightJus == BOTTOM || m_heightJus == BASEH)
    height = y_offset;
  else if (m_heightJus == TOP)   height = ascender + y_offset;
  else if (m_heightJus == MIDDLE)height = (ascender/2.f) + y_offset;
    
  if (m_depthJus == FRONT)       depth = z1;
  else if (m_depthJus == BACK)   depth = z2-z1;
  else if (m_depthJus == HALFWAY)depth = z2 / 2.f;
  else if (m_depthJus == BASED)  depth = 0;

  glScalef(FONT_SCALE, FONT_SCALE, FONT_SCALE);
  glTranslatef(-width, -height, -depth);
}


/////////////////////////////////////////////////////////
// textMess
//
/////////////////////////////////////////////////////////
void TextBase :: breakLine(wstring line)
{
  // split the string wherever there is a '\n'
  while(line.length()>0){
    size_t pos=line.find('\n');
    
    // if not found, we're done
    if(wstring::npos == pos)break;

    m_theText.push_back(line.substr(0,pos));
    line=line.erase(0,pos+1);
  }
  
  // if there is still a text append it
  if(line.length())
    m_theText.push_back(line);
  makeLineDist();
  setModified();
}

static const uint8_t utf8d[] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

  070,070,070,070,070,070,070,070,070,070,070,070,070,070,070,070,
  050,050,050,050,050,050,050,050,050,050,050,050,050,050,050,050,
  030,030,030,030,030,030,030,030,030,030,030,030,030,030,030,030,
  030,030,030,030,030,030,030,030,030,030,030,030,030,030,030,030,
  204,204,188,188,188,188,188,188,188,188,188,188,188,188,188,188,
  188,188,188,188,188,188,188,188,188,188,188,188,188,188,188,188,
  174,158,158,158,158,158,158,158,158,158,158,158,158,142,126,126,
  111, 95, 95, 95, 79,207,207,207,207,207,207,207,207,207,207,207,

  0,1,1,1,8,7,6,4,5,4,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,1,1,1,1,1,1,1,1,1,1,1,1,
  1,4,4,1,1,1,1,1,1,1,1,1,1,1,1,1,1,4,4,4,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,4,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,8,7,6,4,5,4,3,2,1,1,1,1,

};

static std::wstring toWstring(const char*str) throw(int){
  std::wstring result;

  wchar_t unic = 0;
  uint8_t data, byte, stat = 9;

  int len=0;

  while((byte=*str++)) {
    data = utf8d[ byte ];
    stat = utf8d[ 256 + (stat << 4) + (data >> 4) ];
    byte = (byte ^ (uint8_t)(data << 4));

    unic = (unic << 6) | byte;

    if (!stat) {
      // unic is now a proper code point, we just print it out.
      result+=unic;
      unic = 0;
    }

    if (stat == 1) {
      // the byte is not allowed here; the state would have to
      // be reset to continue meaningful reading of the string

      throw(len);
    }

    len++;
  }

  return result;
}

void TextBase :: textMess(int argc, t_atom *argv)
{
  m_theText.clear();
  if ( argc < 1 ) {return; }

  wstring line = L"";
  int i=0;

  // convert the atom-list into 1 string
  for (i = 0; i < argc; ++i)
    {
      string newtext;
      if (A_FLOAT == argv[i].a_type) {
        char str[MAXPDSTRING];
        char*sp=str;

        atom_string(&argv[i], str, MAXPDSTRING);
        while(*sp) {
          unsigned char c=*sp++;
          line+=c;
        }
      } else {
        char*sp=atom_getsymbol(&argv[i])->s_name;
        try {
          std::wstring ws=toWstring(sp);
          line+=ws;
        } catch (int i) {
            i=0;
          while(*sp) {
            unsigned char c=*sp++;
            line+=c;
          }
        }
      }
      if(argc-1>i)line += L' ';
    }

  breakLine(line);
}

/////////////////////////////////////////////////////////
// line distance, the offset
// of the individual lines
/////////////////////////////////////////////////////////
void TextBase :: makeLineDist()
{
  unsigned int i=0;
  m_lineDist.clear();
  if (m_heightJus == BOTTOM || m_heightJus == BASEH)
    {
      // so the offset will be a simple 
      // [0 1 2 3 ... n] sequence
      for(i=0; i<m_theText.size(); i++)
        m_lineDist.push_back(i);
      return;
    }

  if (m_heightJus == TOP)
    {
      // now in the other direction:
      // [-n ... -2 -1 0]
      signed long j;
      for(j=m_theText.size()-1; j>=0; j--){
        m_lineDist.push_back(-j);
      }
      return;
    }

  // else:
  // calculate the y offset of each line, so
  // that the text will be centered:
  /*lines    y-offset        calculation
    1:   0                 = 0- 0
    2:   -0.5 0.5          = [0 1]   - 0.5
    3:   -1 0 1            = [0 1 2] - 1
    4:   -1.5 -0.5 0.5 1.5 = [0 1 2 3] - 1.5
    5:   -2 -1 0 1 2       = [0 1 2 3 4] - 2
    ...
  */

  float diff = (m_theText.size()-1)*0.5;
  for(i=0; i<m_theText.size(); i++)
    m_lineDist.push_back((i-diff)*m_dist);
}


//-- moocow: modified version of "textMess" for float lists
// this can be used with moocow's pd-string external
// available at http://www.ling.uni-potsdam.de/~moocow/projects/pd/#externs
// it works like this:   a string is represented as a list of ASCII-values
/////////////////////////////////////////////////////////
// stringMess
//
/////////////////////////////////////////////////////////
void TextBase :: stringMess(int argc, t_atom *argv)
{
  m_theText.clear();

  if ( argc < 1 ) { return; }

  int i;
  wstring line = L"";

  for (i = 0; i < argc; i++)    {
    int v=atom_getint(argv+i);
    /* 
     * i experienced crashes in FTGL with indices>65535;
     * since TrueType fonts cannot hold more than 65536 entries
     * we just clamp it...
     * note: at least wikipedia claims that TTF can only hold 2^16 glyphs
     *       i have seen ttf-fonts with (seemingly) more
     */
    if(v<0||v>65535) {
      logpost(NULL, 5, "invalid character %d: using ' ' instead", v);
      v=32;
    }
    line += static_cast<wchar_t>(v);
  }
  line += L'\0';

  breakLine(line);
}
//-- /moocow

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void TextBase :: obj_setupCallback(t_class *classPtr)
{

  class_addlist(classPtr, reinterpret_cast<t_method>(&TextBase::textMessCallback));

  class_addmethod(classPtr, reinterpret_cast<t_method>(&TextBase::textMessCallback),
		  gensym("text"), A_GIMME, A_NULL);
  //-- moocow
  class_addmethod(classPtr, reinterpret_cast<t_method>(&TextBase::stringMessCallback),
		  gensym("string"), A_GIMME, A_NULL);
  //-- /moocow
  class_addmethod(classPtr, reinterpret_cast<t_method>(&TextBase::precisionMessCallback),
		  gensym("precision"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&TextBase::fontNameMessCallback),
		  gensym("font"), A_SYMBOL, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&TextBase::justifyMessCallback),
		  gensym("justify"), A_GIMME, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&TextBase::fontSizeMessCallback),
		  gensym("ft1"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&TextBase::linedistMessCallback),
		  gensym("linedist"), A_FLOAT, A_NULL);
}
void TextBase :: textMessCallback(void *data, t_symbol *, int argc, t_atom *argv)
{
  GetMyClass(data)->textMess(argc, argv);
}

//-- moocow
void TextBase :: stringMessCallback(void *data, t_symbol *, int argc, t_atom *argv)
{
  GetMyClass(data)->stringMess(argc, argv);
}
//-- /moocow

void TextBase :: fontNameMessCallback(void *data, t_symbol *s)
{
  GetMyClass(data)->fontNameMess(s->s_name);
}
void TextBase :: justifyMessCallback(void *data, t_symbol *s, int argc, t_atom*argv)
{
  JustifyWidth  wType=CENTER;
  JustifyHeight hType=MIDDLE;
  JustifyDepth  dType=HALFWAY;
  char c;
  
  switch(argc){
  case 3:
    c=atom_getsymbol(argv+2)->s_name[2];
    switch (c){
    case 'o': case 'O': dType = FRONT; break;
    case 'c': case 'C': dType = BACK; break;
    case 's': case 'S': dType = BASED; break;
    case 'l': case 'L': case 'n': case 'N': dType = HALFWAY; break;
    default:
      GetMyClass(data)->error("invalid depth justification: %s (must be: front|back|halfway|base)", 
	    atom_getsymbol(argv+2)->s_name);
      return;
    }
  case 2:    
    c=atom_getsymbol(argv+1)->s_name[2];
    switch (c){
    case 't': case 'T': hType = BOTTOM; break;
    case 'p': case 'P': hType = TOP; break;
    case 'd': case 'D': case 'n': case 'N': hType = MIDDLE; break;
    case 's': case 'S': hType = BASEH; break;
    default:
      GetMyClass(data)->error("invalid height justification: %s (must be bottom|top|middle|base)", 
	    atom_getsymbol(argv+1)->s_name);
      return;
    }
  case 1:
    c=atom_getsymbol(argv)->s_name[2];
    switch (c){
    case 'f': case 'F': wType = LEFT; break;
    case 'g': case 'G': wType = RIGHT; break;
    case 'n': case 'N': wType = CENTER; break;
    case 's': case 'S': wType = BASEW; break;
    default:
      GetMyClass(data)->error("invalid width justification: %s (must be left|right|center|base)",
	    atom_getsymbol(argv+0)->s_name);
      return;
    }
    break;
  default:
    GetMyClass(data)->error("justification most be \"width [height [depth]]\"");
    return;
  }
  
  switch(argc){
  case 1: GetMyClass(data)->setJustification(wType); break;
  case 2: GetMyClass(data)->setJustification(wType, hType); break;
  case 3: GetMyClass(data)->setJustification(wType, hType, dType); break;
  }
}
void TextBase :: fontSizeMessCallback(void *data, t_floatarg size)
{
  GetMyClass(data)->setFontSize(size);
}
void TextBase :: precisionMessCallback(void *data, t_floatarg prec)
{
  GetMyClass(data)->setPrecision(prec);
}

void TextBase :: linedistMess(float dist)
{
  m_dist = dist;
  makeLineDist();
}
void TextBase :: linedistMessCallback(void *data, t_floatarg dist)
{
  GetMyClass(data)->linedistMess(dist);
}
