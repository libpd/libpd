////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-1998 Mark Danks.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    Copyright (c) 2002 James Tittle & Chris Clepper
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////
//
// pix_curve
//
//   IOhannes m zmoelnig
//   mailto:zmoelnig@iem.kug.ac.at
//
//   this code is published under the Gnu GeneralPublicLicense that should be distributed with gem & pd
//
/////////////////////////////////////////////////////////

#include "pix_curve.h"
#include <string.h>
#include <math.h>
#include "RTE/Array.h"

CPPEXTERN_NEW_WITH_GIMME(pix_curve);


  /////////////////////////////////////////////////////////
//
// pix_curve
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_curve :: pix_curve(int argc, t_atom *argv)
{ 
  setMess(argc, argv);
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_curve :: ~pix_curve()
{
}


////////////////////////////////////
// Set Message
//
///////////////////////////////////
void pix_curve :: setMess(int argc, t_atom *argv)
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
t_float* pix_curve :: checkarray(t_symbol *s, int *length)
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


/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_curve :: processRGBAImage(imageStruct &image)
{
  int i=image.xsize*image.ysize;
  unsigned char *base = image.data;
  
  int n_R, n_G, n_B, n_A;

  if (m_mode==0) return;

  gem::RTE::Array tabR=gem::RTE::Array(name_R->s_name);
  gem::RTE::Array tabG=gem::RTE::Array(name_G->s_name);
  gem::RTE::Array tabB=gem::RTE::Array(name_B->s_name);
  gem::RTE::Array tabA=gem::RTE::Array(name_A->s_name);

  n_R=tabR.size();
  n_G=tabG.size();
  n_B=tabB.size();
  n_A=tabA.size();

  switch (m_mode) {
  case 3: // only RGB
    if(! (tabR.isValid() && tabG.isValid() && tabB.isValid()))
       return;
    while (i--) {
      base[chRed  ]=CLAMP(static_cast<int>(tabR[ n_R*base[chRed  ]>>8 ]));
      base[chGreen]=CLAMP(static_cast<int>(tabG[ n_G*base[chGreen]>>8 ]));
      base[chBlue ]=CLAMP(static_cast<int>(tabB[ n_B*base[chBlue ]>>8 ]));

      base+=4;
    }
    break;
  case 4: // RGBA
  case 1: // one table for all
    if(! (tabR.isValid() && tabG.isValid() && tabB.isValid() && tabA.isValid()))
       return;    
    while (i--) {
      base[chRed   ]=CLAMP(static_cast<int>(tabR[ n_R*base[chRed   ]>>8 ]));
      base[chGreen ]=CLAMP(static_cast<int>(tabG[ n_G*base[chGreen ]>>8 ]));
      base[chBlue  ]=CLAMP(static_cast<int>(tabB[ n_B*base[chBlue  ]>>8 ]));
      base[chAlpha ]=CLAMP(static_cast<int>(tabA[ n_A*base[chAlpha]>>8 ]));

      base+=4;
    }
  default:
    break;
  }


}
/////////////////////////////////////////////////////////
// processImage
void pix_curve :: processGrayImage(imageStruct &image)
{
  int i=image.xsize*image.ysize;
  unsigned char *base = image.data;
  
  gem::RTE::Array tab=gem::RTE::Array(name_R->s_name);
  int n = tab.size();

  if(!tab.isValid())return;
  while (i--) {
    base[chGray]=CLAMP(static_cast<int>(tab[ n*base[chGray]>>8 ]));
    base++;
  }
}

void pix_curve :: processYUVImage(imageStruct &image)
{
  int i=image.xsize*image.ysize/2;
  unsigned char *base = image.data;
  
  int n_Y, n_U, n_V;

  if (m_mode==0) return;

  gem::RTE::Array tabY=gem::RTE::Array(name_R->s_name);
  gem::RTE::Array tabU=gem::RTE::Array(name_G->s_name);
  gem::RTE::Array tabV=gem::RTE::Array(name_B->s_name);

  n_Y=tabY.size();
  n_U=tabU.size();
  n_V=tabV.size();

  switch (m_mode) {
  case 3: // YUV
    if(! (tabY.isValid() && tabU.isValid() && tabV.isValid()))
      return;
    while (i--) {
      base[chU ]=CLAMP(static_cast<int>(tabY[ n_U*base[chU ]>>8 ]));
      base[chY0]=CLAMP(static_cast<int>(tabY[ n_Y*base[chY0]>>8 ]));
      base[chV ]=CLAMP(static_cast<int>(tabY[ n_V*base[chV ]>>8 ]));
      base[chY1]=CLAMP(static_cast<int>(tabY[ n_Y*base[chY1]>>8 ]));

      base+=4;
    }
    break;
  case 1: // only Y
    if(! (tabY.isValid()))
      return;

    while (i--) {
      base[chY0]=CLAMP(static_cast<int>(tabY[ n_Y*base[chY0]>>8 ]));
      base[chY1]=CLAMP(static_cast<int>(tabY[ n_Y*base[chY1]>>8 ]));

      base+=4;
    }
  default:
    break;
  }
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_curve :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_curve::setMessCallback),
		  gensym("set"), A_GIMME,0);
}

void pix_curve :: setMessCallback(void *data, t_symbol *s, int argc, t_atom* argv)
{
    GetMyClass(data)->setMess(argc, argv);
}
