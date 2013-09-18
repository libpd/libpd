/*
 *  pix_contrast.h
 *  GEM_darwin
 *
 *  Created by cgc on 8/23/05.
 *  Copyright 2005 cgc. All rights reserved.
 *
 */

#ifndef _INCLUDE__GEM_PIXES_PIX_CONTRAST_H_
#define _INCLUDE__GEM_PIXES_PIX_CONTRAST_H_

#include "Base/GemPixObj.h"


class GEM_EXTERN pix_contrast : public GemPixObj
{
    CPPEXTERN_HEADER(pix_contrast, GemPixObj);

    public:

	    //////////
	    // Constructor
    	pix_contrast();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_contrast();

		virtual void	processRGBAImage(imageStruct &image);

		virtual void	processGrayImage(imageStruct &image);

	#ifdef __VEC__
		virtual void 	processYUVAltivec(imageStruct &image);
	#endif //__VEC__
		virtual void 	processYUVImage(imageStruct &image);
		
		void	contrastMess(float contrast);
		void	saturationMess(float contrast);
		
		float	m_contrast;
		float	m_saturation;

                t_inlet*m_inSat, *m_inCon;

 private:
    
    	//////////
    	// Static member functions
    	static void 	contrastMessCallback(void *data, t_floatarg contrast);
		static void 	saturationMessCallback(void *data, t_floatarg contrast);

};
#endif
