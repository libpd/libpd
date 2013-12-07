/*
 *  pix_posterize.cpp
 *  gem_darwin
 *  tigital@mac.com
 *
 *  Created by Jamie Tittle on Fri Oct 18 2002.
 *  Copyright (c) 2002 tigital. All rights reserved.
 *
 */
#include "pix_posterize.h"

CPPEXTERN_NEW(pix_posterize);

/////////////////////////////////////////////////////////
//
// pix_posterize
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_posterize :: pix_posterize()
{
    inletF = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("factor"));
    inletL = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("limit"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_posterize :: ~pix_posterize()
{
}

void pix_posterize :: factorMess(float f)
{
  factor = static_cast<int>(f*255.f);
  setPixModified();
}

void pix_posterize :: limitMess(float l)
{
  limit = static_cast<int>(l);
  setPixModified();
}

/////////////////////////////////////////////////////////
// do the YUV processing here
//
/////////////////////////////////////////////////////////
void pix_posterize :: processYUVImage(imageStruct &image)
{
    int h,w;
    long src;
    src = 0;
//format is U Y V Y

    if (factor <= 0 || factor > 255) factor = 1;
    switch (limit) {
        case 1:
	  for (h=0; h<image.ysize; h++){
	    for(w=0; w<image.xsize/2; w++){
	      image.data[src+1] = image.data[src+1] - ( image.data[src+1] % (256/factor));
	      image.data[src+3] = image.data[src+3] - ( image.data[src+3] % (256/factor));
	      if ( image.data[src+1] > 100 && image.data[src+1] < 150) image.data[src+1]=235;
	      if ( image.data[src+3] > 100 && image.data[src+3] < 150) image.data[src+3]=235;
	      image.data[src] = image.data[src] - ( image.data[src] % (256/factor));
	      image.data[src+2] = image.data[src+2] - ( image.data[src+2] % (256/factor));
	      src += 4;
	    }
	  }
	  break;
        case 2:
	  for (h=0; h<image.ysize; h++){
	    for(w=0; w<image.xsize/2; w++){
	      image.data[src+1] = image.data[src+1] - ( image.data[src+1] % (256/factor));
	      image.data[src+3] = image.data[src+3] - ( image.data[src+3] % (256/factor));
	      if ( image.data[src+1] > 100 && image.data[src+1] < 150) image.data[src+1]=235;
	      if ( image.data[src+3] > 100 && image.data[src+3] < 150) image.data[src+3]=235;
	      src += 4;
	    }
	  }
	  break;
        case 3:
	  for (h=0; h<image.ysize; h++){
	    for(w=0; w<image.xsize/2; w++){
	      image.data[src+1] = image.data[src+1] - ( image.data[src+1] % (256/factor));
	      image.data[src+3] = image.data[src+3] - ( image.data[src+3] % (256/factor));
	      if ( image.data[src+1] > 100 && image.data[src+1] < 150) image.data[src+1]=235;
	      if ( image.data[src+3] > 100 && image.data[src+3] < 150) image.data[src+3]=235;
	      image.data[src+3] = image.data[src+3] - ( image.data[src+3] % (256/factor));
	      src += 4;
	    }
	  }
	  break;
        case 4:
	  for (h=0; h<image.ysize; h++){
	    for(w=0; w<image.xsize/2; w++){
	      image.data[src+1] = image.data[src+1] - ( image.data[src+1] % (256/factor));
	      image.data[src+3] = image.data[src+3] - ( image.data[src+3] % (256/factor));
	      if ( image.data[src+1] > 100 && image.data[src+1] < 150) image.data[src+1]=235;
	      if ( image.data[src+3] > 100 && image.data[src+3] < 150) image.data[src+3]=235;
	      image.data[src] = image.data[src] - ( image.data[src] % (256/factor));
	      src += 4;
	    }
	  }
	  break;
        break;
        default:
	  for (h=0; h<image.ysize; h++){
	    for(w=0; w<image.xsize/2; w++){
	      image.data[src+1] = image.data[src+1] - ( image.data[src+1] % (256/factor));
	      image.data[src+3] = image.data[src+3] - ( image.data[src+3] % (256/factor));
	      if ( image.data[src+1] > 100 && image.data[src+1] < 150) image.data[src+1]=235;
	      if ( image.data[src+3] > 100 && image.data[src+3] < 150) image.data[src+3]=235;
	      src += 4;
	    }
	  } 
	  break;                                                                
    }	                    
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_posterize :: obj_setupCallback(t_class *classPtr)
{
class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_posterize::factorMessCallback),
    	    gensym("factor"), A_FLOAT, A_NULL);
            
class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_posterize::limitMessCallback),
    	    gensym("limit"), A_FLOAT, A_NULL);

}

void pix_posterize :: factorMessCallback(void *data, t_floatarg size)
{
    GetMyClass(data)->factorMess(size);
}

void pix_posterize :: limitMessCallback(void *data, t_floatarg size)
{
    GetMyClass(data)->limitMess(size);
}
