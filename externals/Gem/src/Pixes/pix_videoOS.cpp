////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
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
/*
    this is an attempt at a Linux version of pix_videoOS by Miller Puckette.
    Anyone conversant in c++ will probably howl at this.  I'm uncertain of
    several things.
    
    First, the #includes I threw in pix_videoOS.h may not all be necessary; I
    notice that far fewer are needed for the other OSes.
    
    Second, shouldn't the os-dependent state variables be "private"?  I
    followed the lead of the other os-dependent state variables.  Also,
    I think the indentation is goofy but perhaps there's some reason for it.

    Third, I probably shouldn't be using sprintf to generate filenames; I
    don't know the "modern" c++ way to do this.
    
    Fourth, I don't know why some state variables 
    show up as "arguments" in the pix_videoOS :: pix_videoOS().
     
    This code is written with the "bttv" device in mind, which memory mapes
    images up to 24 bits per pixel.  So we request the whole 24 and don't
    settle for anything of lower quality (nor do we offer anything of higher
    quality; it seems that Gem is limited to 32 bits per pixel including
    alpha.)  We take all video images to be opaque by setting the alpha
    channel to 255.

*/

#include "Gem/GemConfig.h"
#define NO_AUTO_REGISTER_CLASS

#include "pix_videoOS.h"
#include "Gem/Cache.h"

CPPEXTERN_NEW(pix_videoOS);

#define BYTESIN 3

/////////////////////////////////////////////////////////
//
// pix_videoOS
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_videoOS :: pix_videoOS(t_floatarg num)
    	   : m_haveVideo(0), m_swap(1), m_colorSwap(0)
{
  m_pixBlock.image = m_imageStruct;
  m_haveVideo = 0;
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_videoOS :: ~pix_videoOS()
{
}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void pix_videoOS :: render(GemState *state)
{
  if (!m_haveVideo)
    {
      post("no video for this OS");
      return;
    }
}

/////////////////////////////////////////////////////////
// startTransfer
//
/////////////////////////////////////////////////////////
int pix_videoOS :: startTransfer()
{
  post("no video available for this OS");
    if (!m_haveVideo)
    	return(0);

    return(1);
}

/////////////////////////////////////////////////////////
// stopTransfer
//
/////////////////////////////////////////////////////////
int pix_videoOS :: stopTransfer()
{
    if ( !m_haveVideo )
    	return(0);
    
    return(1);
}

/////////////////////////////////////////////////////////
// offsetMess
//
/////////////////////////////////////////////////////////
void pix_videoOS :: offsetMess(int x, int y)
{
  error("offset not supported on this OS");
}

/////////////////////////////////////////////////////////
// cleanPixBlock -- free the pixel buffer memory
//
/////////////////////////////////////////////////////////
void pix_videoOS :: cleanPixBlock()
{
}

/////////////////////////////////////////////////////////
// swapMess
//
/////////////////////////////////////////////////////////
void pix_videoOS :: swapMess(int state)
{
  error("swap not supported on this OS");
}
/////////////////////////////////////////////////////////
// colorspaceMess
//
/////////////////////////////////////////////////////////
void pix_videoOS :: csMess(int format)
{
  error("colorspace not supported on this OS");
}
void pix_videoOS :: csMess(t_symbol*s)
{
  csMess(getPixFormat(s->s_name));
}
/////////////////////////////////////////////////////////
// enumerate devices
//
/////////////////////////////////////////////////////////
void pix_videoOS :: enumerateMess()
{
  error("enumerate not supported on this OS");
}
/////////////////////////////////////////////////////////
// dialog
//
/////////////////////////////////////////////////////////
void pix_videoOS :: dialogMess(int argc, t_atom*argv)
{
  error("dialog not supported on this OS");
}




/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_videoOS :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_videoOS::dimenMessCallback),
    	    gensym("dimen"), A_GIMME, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_videoOS::offsetMessCallback),
    	    gensym("offset"), A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_videoOS::swapMessCallback),
    	    gensym("swap"), A_FLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_videoOS::enumerateMessCallback),
    	    gensym("enumerate"), A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_videoOS::csMessCallback),
    	    gensym("colorspace"), A_DEFSYMBOL, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_videoOS::dialogMessCallback),
    	    gensym("dialog"), A_GIMME, A_NULL);
}
void pix_videoOS :: dimenMessCallback(void *data, t_symbol *s, int ac, t_atom *av)
{
  GetMyClass(data)->dimenMess(static_cast<int>(atom_getfloatarg(0, ac, av)),
			      static_cast<int>(atom_getfloatarg(1, ac, av)),
			      static_cast<int>(atom_getfloatarg(2, ac, av)),
			      static_cast<int>(atom_getfloatarg(3, ac, av)),
			      static_cast<int>(atom_getfloatarg(4, ac, av)),
			      static_cast<int>(atom_getfloatarg(5, ac, av)) );
}
void pix_videoOS :: offsetMessCallback(void *data, t_floatarg x, t_floatarg y)
{
    GetMyClass(data)->offsetMess(static_cast<int>(x), static_cast<int>(y));
}
void pix_videoOS :: swapMessCallback(void *data, t_floatarg state)
{
    GetMyClass(data)->swapMess(static_cast<int>(state));
}
void pix_videoOS :: csMessCallback(void *data, t_symbol*s)
{
  GetMyClass(data)->csMess(s);
}
void pix_videoOS :: enumerateMessCallback(void *data)
{
  GetMyClass(data)->enumerateMess();
}
void pix_videoOS :: dialogMessCallback(void *data, t_symbol*s, int argc, t_atom*argv)
{
  GetMyClass(data)->dialogMess(argc, argv);
}
