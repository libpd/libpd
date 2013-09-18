/*
 *  pix_emboss.cpp
 *  gem_darwin
 *
 *  Created by chris clepper on Mon Oct 07 2002.
 *  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
 *
 */

#include "pix_emboss.h"

CPPEXTERN_NEW(pix_emboss);

/////////////////////////////////////////////////////////
//
// pix_emboss
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_emboss :: pix_emboss()
{
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_emboss :: ~pix_emboss()
{}

/////////////////////////////////////////////////////////
// do the YUV processing here
//
/////////////////////////////////////////////////////////
void pix_emboss :: processYUVImage(imageStruct &image)
{
   long r,c,width, height,src,h,w;
	
   width = image.xsize-1;
   height = image.ysize-1;  
   w = image.xsize*2;
   h = image.ysize*2;
   src =1;
   //   src=w;
   for (r=0; r < height; r++) {
	for (c=0; c < width; c++) {
            image.data[src] = ( image.data[ src-w-2 ] -
                                image.data[ src-w ] -
                                image.data[ src-w+2 ] +
                                image.data[ src -2 ] -
                                image.data[src] -
                                image.data[src + 2] -
                                image.data[src+w-2] - 
                                image.data[src+w] - 
                                image.data[src+w+2] )/4;
            src+=2;
        }
    }
        /*
	for (r=0; r < (width*height); r+= width) {
		for (c=0; c < width; c++) {
                
			image.data[src+1] =(image.data[ r-1 + c-2 ] -	
				      image.data[ r-1 + c ] -	
				      image.data[ r-1 + c+2 ]+ 	
				      image.data[ r+c -2 ] +	
				      image.data[r+c] -		
				      image.data[r+c+2] -		
				      image.data[r+1+c-2] -   	
				      image.data[r+1+c] -    	
				      image.data[r+1+c+2]
				      )/9;
                                      src+=2;
		}
	}*/
   //post("chars:%d",src);
   //post("r:%d",r);
   //post("c:%d",c);
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_emboss :: obj_setupCallback(t_class *classPtr)
{

}
