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

#include "gemkeyname.h"

#include "Gem/Event.h"

CPPEXTERN_NEW(gemkeyname);

/////////////////////////////////////////////////////////
//
// gemkeyname
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
gemkeyname :: gemkeyname()
{
    m_outKeyState = outlet_new(this->x_obj, 0);
    m_outKeyVal = outlet_new(this->x_obj, 0);

    // register event callback
    setKeyboardCallback(&gemkeyname::keynameCallback, this);
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
gemkeyname :: ~gemkeyname()
{
    // remove event callback
    removeKeyboardCallback(&gemkeyname::keynameCallback, this);

    outlet_free(m_outKeyState);
    outlet_free(m_outKeyVal);
}


/////////////////////////////////////////////////////////
// mouseMotion
//
/////////////////////////////////////////////////////////
void gemkeyname :: KeyNamePressed(char *string, int val, int state)
{
  outlet_symbol(m_outKeyVal, gensym(string));
  outlet_float(m_outKeyState, static_cast<t_float>(state));
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void gemkeyname :: obj_setupCallback(t_class *)
{ }
void gemkeyname :: keynameCallback(char *x, int y, int z, void *data)
{
  (reinterpret_cast<gemkeyname*>(data))->KeyNamePressed(x,y, z);
}
