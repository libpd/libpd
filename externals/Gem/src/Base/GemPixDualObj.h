/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    An object which accepts two pixes.

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    Copyright (c) 2002 James Tittle & Chris Clepper
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_BASE_GEMPIXDUALOBJ_H_
#define _INCLUDE__GEM_BASE_GEMPIXDUALOBJ_H_

#define NEW_DUAL_PIX

#include "Base/GemPixObj.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    GemPixDualObj
    
    An object which accepts two pixes.

DESCRIPTION

    Inlet for a gem - "gem_right"
    
    "gem_right" - The second gem list
   
-----------------------------------------------------------------*/
class GEM_EXTERN GemPixDualObj : public GemPixObj
{
    public:

        //////////
        // Constructor
    	GemPixDualObj();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~GemPixDualObj();

	void render(GemState *state);

    	//////////
	// Derived classes should NOT override this!
	// This makes sure that the images are the same size.
    	// This calls the other process functions based on the input images.
    	virtual void 	processImage(imageStruct &image);

#ifndef NEW_DUAL_PIX
    	//////////
    	// The derived class HAS override this.
    	// This is called whenever a new image comes through and
	//		both of the image structs are RGBA
    	virtual void 	processDualImage(imageStruct &image, imageStruct &right) = 0;
    	
    	//////////
    	// The derived class CAN override this.
    	// This is called whenever a new image comes through and both
    	//		of the image structs are gray8.
		// The default behavior is to output an error.
        virtual void 	processDualGray(imageStruct &image, imageStruct &right);
    	
    	//////////
    	// The derived class CAN override this.
    	// This is called whenever a new image comes through and 
		//		the left image is an RGBA while the right is a gray8.
		// The default behavior is to output an error.
    	virtual void 	processRightGray(imageStruct &image, imageStruct &right);
    	
    	//////////
    	// The derived class CAN override this.
    	// This is called whenever a new image comes through and
		//		the left image is a gray8, the right is an RGBA
		// The default behavior is to output an error.
    	virtual void 	processLeftGray(imageStruct &image, imageStruct &right);
        
    	//////////
    	// The derived class CAN override this.
    	// This is called whenever a new image comes through and both
    	//		of the image structs are YUV.
		// The default behavior is to output an error.
        virtual void 	processDualYUV(imageStruct &image, imageStruct &right);
    	
    	//////////
    	// The derived class CAN override this.
    	// This is called whenever a new image comes through and 
		//		the left image is an RGBA while the right is a YUV.
		// The default behavior is to output an error.
    	virtual void 	processRightYUV(imageStruct &image, imageStruct &right);
    	
    	//////////
    	// The derived class CAN override this.
    	// This is called whenever a new image comes through and
		//		the left image is a YUV, the right is an RGBA
		// The default behavior is to output an error.
    	virtual void 	processLeftYUV(imageStruct &image, imageStruct &right);
#else
    	//////////
    	// The derived class SHOULD override this, if it provides a method for "all" formats
	virtual void processDualImage(imageStruct &left, imageStruct &right);
	// Here come the more specific dual-processors
    	// The derived class SHOULD override these as needed

	/* for simplicity this is done via preprocessor defines:
	 * the functions defined are like : 
	 **  processRGBA_RGBA(left, right);
	 */

#define PROCESS_DUALIMAGE(CS1, CS2)		\
	virtual void process##CS1 ##_##CS2 (imageStruct &left, imageStruct &right){processDualImage(left, right);}
	PROCESS_DUALIMAGE(RGBA, RGBA);
	PROCESS_DUALIMAGE(RGBA, Gray);
	PROCESS_DUALIMAGE(RGBA, YUV );

	PROCESS_DUALIMAGE(Gray, RGBA);
	PROCESS_DUALIMAGE(Gray, Gray);
	PROCESS_DUALIMAGE(Gray, YUV );

	PROCESS_DUALIMAGE(YUV,  RGBA);
	PROCESS_DUALIMAGE(YUV,  Gray);
	PROCESS_DUALIMAGE(YUV,  YUV );
#undef  PROCESS_DUALIMAGE

	/* for simplicity this is done via preprocessor defines:
	 * the functions defined are like : 
	 **  processRGBA_Altivec(left, right);
	 */
#define PROCESS_DUALIMAGE_SIMD(CS1, CS2,_SIMD_EXT)			\
	virtual void process##CS1 ##_##_SIMD_EXT (imageStruct &left, imageStruct &right){ \
	  process##CS1 ##_##CS2 (left, right);}

	PROCESS_DUALIMAGE_SIMD(RGBA, RGBA, MMX);
	PROCESS_DUALIMAGE_SIMD(RGBA, MMX , SSE2);
	PROCESS_DUALIMAGE_SIMD(RGBA, RGBA, Altivec);

	PROCESS_DUALIMAGE_SIMD(YUV , YUV , MMX);
	PROCESS_DUALIMAGE_SIMD(YUV , MMX , SSE2);
	PROCESS_DUALIMAGE_SIMD(YUV , YUV , Altivec);

	PROCESS_DUALIMAGE_SIMD(Gray, Gray, MMX);
	PROCESS_DUALIMAGE_SIMD(Gray, MMX , SSE2);
	PROCESS_DUALIMAGE_SIMD(Gray, Gray, Altivec);
#undef PROCESS_DUALIMAGE_SIMD

#endif                
        //////////
        virtual void	postrender(GemState *);
    	virtual void	stopRendering();
        virtual void	rightstopRendering()	{ ; }
    	virtual void   	rightRender(GemState *state);
    	virtual void   	rightPostrender(GemState *)    	{ ; }
	virtual void	rightStoprender()		{ ; }

    	//////////
    	GemCache    	*m_cacheRight;

    	//////////
    	pixBlock    	*m_pixRight;
        
        int		m_pixRightValid;
        int		org_pixRightValid;

        //////////
        t_inlet         *m_inlet;

    	//////////
    	// creation callback
    	static void 	real_obj_setupCallback(t_class *classPtr)
    	    { GemPixObj::real_obj_setupCallback(classPtr); GemPixDualObj::obj_setupCallback(classPtr); }
    	
  private:
    
     	static inline GemPixDualObj *GetMyClass(void *data) {return((GemPixDualObj *)((Obj_header *)data)->data);}

    	//////////
    	// Static member functions
    	static void 	obj_setupCallback(t_class *classPtr);
    	static void 	gem_rightMessCallback(void *x, t_symbol *s, int argc, t_atom *argv);
};
#endif	// for header file
