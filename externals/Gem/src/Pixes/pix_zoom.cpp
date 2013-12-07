////////////////////////////////////////////////////////
//
// GEMi - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-1998 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "pix_zoom.h"

CPPEXTERN_NEW(pix_zoom);

/////////////////////////////////////////////////////////
//
// pix_zoom
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_zoom :: pix_zoom()
{
    zoomMess(1, 1);
    
    // create the new inlet
    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("zoom"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_zoom :: ~pix_zoom()
{ }

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void pix_zoom :: render(GemState *)
{
    glPixelZoom(m_xZoom, m_yZoom);
}

/////////////////////////////////////////////////////////
// postrender
//
/////////////////////////////////////////////////////////
void pix_zoom :: postrender(GemState *)
{
    glPixelZoom(1.0, 1.0);
}

/////////////////////////////////////////////////////////
// zoom_magMess
//
/////////////////////////////////////////////////////////
void pix_zoom :: zoomMess(float xZoom, float yZoom)
{
    m_xZoom = xZoom;
    m_yZoom = yZoom;
    setModified();
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_zoom :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_zoom::zoomMessCallback),
    	    gensym("zoom"), A_FLOAT, A_FLOAT, A_NULL);
}
void pix_zoom :: zoomMessCallback(void *data, t_floatarg xMag, t_floatarg yMag)
{
    GetMyClass(data)->zoomMess((float)xMag, (float)yMag);
}
