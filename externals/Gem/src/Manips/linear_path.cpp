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

#include "linear_path.h"

#include "Utils/Functions.h"

CPPEXTERN_NEW_WITH_GIMME(linear_path);

/////////////////////////////////////////////////////////
//
// separator
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
linear_path :: linear_path(int argc, t_atom *argv)
             : GemPathBase(argc, argv)
{ }

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
linear_path :: ~linear_path()
{ }

/////////////////////////////////////////////////////////
// floatMess
//
/////////////////////////////////////////////////////////
void linear_path :: floatMess(float val)
{
    if (!m_array)
    {
        error("no array");
        return;
    }

    int size;
    t_float *vec;
    if (!garray_getfloatarray(m_array, &size, &vec))
        return;

    if (size % m_numDimens)
    {
        error("size is not a mod of dimensions");
        return;
    }

    float output[64];
    linearFunc(val, output, m_numDimens, size / m_numDimens, vec);

	t_atom argv[64];
    for (int i = 0; i < m_numDimens; i++)
        SETFLOAT((&argv[i]), output[i]);

	outlet_list(m_out1, &s_list, m_numDimens, argv);
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void linear_path :: obj_setupCallback(t_class *)
{ }
