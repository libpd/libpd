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

#include "gemkeyboard.h"

#include "Gem/Event.h"

CPPEXTERN_NEW(gemkeyboard);

/////////////////////////////////////////////////////////
//
// gemkeyboard
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
gemkeyboard :: gemkeyboard()
{
    m_outKeyVal = outlet_new(this->x_obj, 0);

    // register event callback
    setKeyboardCallback(&gemkeyboard::keyboardCallback, this);
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
gemkeyboard :: ~gemkeyboard()
{
    // remove event callback
    removeKeyboardCallback(&gemkeyboard::keyboardCallback, this);

    outlet_free(m_outKeyVal);
}


/////////////////////////////////////////////////////////
// KeyBoardPressed
//
/////////////////////////////////////////////////////////
void gemkeyboard :: KeyBoardPressed(int val, int state)
{
  if (state==0)return;
  outlet_float(m_outKeyVal, static_cast<t_float>(val));
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void gemkeyboard :: obj_setupCallback(t_class *)
{ }
void gemkeyboard :: keyboardCallback(char* w, int x, int y, void *data)
{
  (reinterpret_cast<gemkeyboard*>(data))->KeyBoardPressed(x, y);
}
