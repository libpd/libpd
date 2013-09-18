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
#include <iostream>

/////////////////////////////////////////////////////////
//
// pix_histo
//
//   IOhannes m zmoelnig
//   mailto:zmoelnig@iem.kug.ac.at
//
//   this code is published under the Gnu GeneralPublicLicense that should be distributed with gem & pd
//
/////////////////////////////////////////////////////////

#include "pix_histo.h"
#include <string.h>
#include <math.h>

#include "RTE/Array.h"
#include "Gem/PixConvert.h"

CPPEXTERN_NEW_WITH_GIMME(pix_histo);


  /////////////////////////////////////////////////////////
//
// pix_histo
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_histo :: pix_histo(int argc, t_atom *argv)
{ 
  setMess(argc, argv);
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_histo :: ~pix_histo()
{
}


////////////////////////////////////
// Set Message
//
///////////////////////////////////
void pix_histo :: setMess(int argc, t_atom *argv)
{
  t_atom *ap=argv;
  int n=argc;

  if (!(argc==1 || argc==3 || argc==4)) {
    error("only 1, 3 or 4 arguments are allowed");
    m_mode=0;
    return;
  }

  while(n--){
    if (ap->a_type != A_SYMBOL) {
      error("only symbolic table-names are accepted");
      return;
    }
    ap++;
  }

  m_mode=3;

  ap=argv;
  switch (argc) {
  case 1:
    name_R=name_G=name_B=name_A=atom_getsymbol(ap);
    m_mode=1;
    break;
  case 4:
    name_A=atom_getsymbol(ap+3);
    m_mode=4;
  default:
    name_R=atom_getsymbol(ap);
    name_G=atom_getsymbol(ap+1);
    name_B=atom_getsymbol(ap+2);
  }
  setPixModified();
}


///////////////
// check if array exists and whether it is a floatarray
//
///////////////
t_float* pix_histo :: checkarray(t_symbol *s, int *length)
{
    t_garray *a;
    t_float  *fp;
    *length = 0;

    if (!(a = (t_garray *)pd_findbyclass(s, garray_class)))
    {
    	if (*s->s_name) error("%s: no such array", s->s_name);
    	fp = 0;
    }
    else if (!garray_getfloatarray(a, length, &fp))
    {
    	error("%s: bad template for tabwrite~", s->s_name);
    	fp = 0;
    }

    if (*length==0){
      error("table %s is zero-lengthed", s->s_name);
      fp=0;
    }
    return fp;
}

///////////////
// update graphs
//
///////////////
void pix_histo :: update_graphs(void)
{
  t_garray *a;

  switch (m_mode) {
  case 4:
    if ((a = (t_garray *)pd_findbyclass(name_A, garray_class)))
      garray_redraw(a);
  case 3:
    if ((a = (t_garray *)pd_findbyclass(name_G, garray_class)))
      garray_redraw(a);
    if ((a = (t_garray *)pd_findbyclass(name_B, garray_class)))
      garray_redraw(a);
  case 1:
    if ((a = (t_garray *)pd_findbyclass(name_R, garray_class)))
      garray_redraw(a);
  default:
    break;
  }
}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_histo :: processRGBAImage(imageStruct &image)
{
  int size=image.xsize*image.ysize;
  unsigned char *base = image.data;
  
  int n_R=0, n_G=0, n_B=0, n_A=0;

  t_float incr=0.;

  gem::RTE::Array tabR=gem::RTE::Array(name_R->s_name);
  gem::RTE::Array tabG=gem::RTE::Array(name_G->s_name);
  gem::RTE::Array tabB=gem::RTE::Array(name_B->s_name);
  gem::RTE::Array tabA=gem::RTE::Array(name_A->s_name);

  if (m_mode==0) return;
  switch (m_mode) {
  case 4:
    if(!tabA.isValid())return;
    n_A=tabA.size();
    tabA.set(0);

  case 3:
    if(!tabB.isValid())return;
    n_B=tabB.size();
    tabB.set(0);

    if(!tabG.isValid())return;
    n_G=tabG.size();
    tabG.set(0);

  case 1:
    if(!tabR.isValid())return;
    n_R=tabR.size();
    tabR.set(0);

  default:
    break;
  }

  incr = 1./size;

  switch (m_mode) {
  case 1: // RGB->grey
    while (size--) {
      
#if 1
      const unsigned int grey =((base[chRed]  *RGB2GRAY_RED+
                                 base[chGreen]*RGB2GRAY_GREEN+
                                 base[chBlue] *RGB2GRAY_BLUE)
                                >>8)+RGB2GRAY_OFFSET;
      const unsigned int index=(n_R*grey)>>8;
      tabR[index]+=incr;
#else
      float grey = (base[chRed] * 0.3086f + base[chGreen] * 0.6094f + base[chBlue] * 0.0820f)/255.f;
      tabR[static_cast<int>(n_R*grey)]+=incr;
#endif
      base+=4;
    }
    break;
  case 3: // RGB
    while (size--) {
      tabR[(n_R*base[chRed  ])>>8]+=incr;
      tabG[(n_G*base[chGreen])>>8]+=incr;
      tabB[(n_B*base[chBlue ])>>8]+=incr;

      base+=4;
    }
    break;
  case 4: // RGBA
    while (size--) {
      tabR[(n_R*base[chRed  ])>>8]+=incr;
      tabG[(n_G*base[chGreen])>>8]+=incr;
      tabB[(n_B*base[chBlue ])>>8]+=incr;
      tabA[(n_A*base[chAlpha])>>8]+=incr;

      base+=4;
    }
  default:
    break;
  }

  update_graphs();
}

void pix_histo :: processYUVImage(imageStruct &image)
{
  int size=image.xsize*image.ysize;
  unsigned char *base = image.data;
  
  int n_Y=0, n_U=0, n_V=0;

  gem::RTE::Array tabY=gem::RTE::Array(name_R->s_name);
  gem::RTE::Array tabU=gem::RTE::Array(name_G->s_name);
  gem::RTE::Array tabV=gem::RTE::Array(name_B->s_name);

  if (m_mode==0) return;
  switch (m_mode) {
  case 3:
    if(!tabU.isValid())return;
    n_U=tabU.size();
    tabU.set(0);

    if(!tabV.isValid())return;
    n_V=tabV.size();
    tabV.set(0);

  case 1:
    if(!tabY.isValid())return;
    n_Y=tabY.size();
    tabY.set(0);
  default:
    break;
  }

  t_float incrY = 1./size;
  t_float incrUV = incrY *2.f;

  size/=2;
  switch (m_mode) {
  case 1: // RGB->grey
    while (size--) {
      tabY[(n_Y*base[chY0])>>8]+=incrY;
      tabY[(n_Y*base[chY1])>>8]+=incrY;

      //      *(tab_Y+((n_Y*base[chY0])>>8))+=incrY;
      //      *(tab_Y+((n_Y*base[chY1])>>8))+=incrY;
      base+=4;
    }
    break;
  case 3: // RGB
    while (size--) {
      tabU[(n_U*base[chU ])>>8]+=incrUV;
      tabY[(n_Y*base[chY0])>>8]+=incrY;
      tabV[(n_V*base[chV ])>>8]+=incrUV;
      tabY[(n_Y*base[chY1])>>8]+=incrY;
      base+=4;
    }
    break;
  default:
    break;
  }
  update_graphs();
}

void pix_histo :: processGrayImage(imageStruct &image)
{
  int size=image.xsize*image.ysize;
  t_float incr= 1./size;

  unsigned char *base = image.data;

  if (m_mode==0) return;

  gem::RTE::Array tab=gem::RTE::Array(name_A->s_name);
  int n=tab.size();

  if(!tab.isValid())
    return;

  tab.set(0);

  while (size--) {
    tab[(n*base[chGray])>>8]+=incr;
    base++;
  }

  update_graphs();
}
/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_histo :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_histo::setMessCallback),
		  gensym("set"), A_GIMME,0);
}

void pix_histo :: setMessCallback(void *data, t_symbol *s, int argc, t_atom* argv)
{
    GetMyClass(data)->setMess(argc, argv);
}
