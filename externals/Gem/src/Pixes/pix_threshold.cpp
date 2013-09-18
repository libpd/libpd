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

#include "pix_threshold.h"

CPPEXTERN_NEW(pix_threshold);

/////////////////////////////////////////////////////////
//
// pix_threshold
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_threshold :: pix_threshold()
{
    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("ft1"));
    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("vec_thresh"));
    m_thresh[chRed] = m_thresh[chGreen] = m_thresh[chBlue] = m_thresh[chAlpha] = 0;
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_threshold :: ~pix_threshold()
{ }

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_threshold :: processRGBAImage(imageStruct &image)
{
    int datasize = image.xsize * image.ysize;

    unsigned char *base = image.data;

    while(datasize--)
    {
		if (base[chRed] < m_thresh[chRed]) base[chRed] = 0;
		if (base[chGreen] < m_thresh[chGreen]) base[chGreen] = 0;
		if (base[chBlue] < m_thresh[chBlue]) base[chBlue] = 0;
		if (base[chAlpha] < m_thresh[chAlpha]) base[chAlpha] = 0;
		base += 4;
    }    
}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_threshold :: processYUVImage(imageStruct &image)
{
    int datasize = (image.xsize/2) * image.ysize;

    unsigned char *base = image.data;

    while(datasize--)
    {
		//if (base[0] < m_thresh[1]) base[0] = 0; //u
		if (base[1] < m_Y) base[1] = 0;//y1
		//if (base[2] < m_thresh[2]) base[2] = 0;//v
		if (base[3] < m_Y) base[3] = 0;//y2
		base += 4;
    }  
}

#ifdef __VEC__
/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_threshold :: processYUVAltivec(imageStruct &image)
{
    int datasize = (image.xsize/8) * image.ysize;
	
	vector unsigned char thresh, in;	
	vector bool char	mask;
    unsigned char *base = image.data;

	union{
		unsigned char			c[16];
		vector unsigned char	v;
	}charBuf;

	charBuf.c[0] = 0;
	charBuf.c[1] = m_Y;
	charBuf.c[2] = 0;
	charBuf.c[3] = m_Y;
	charBuf.c[4] = 0;
	charBuf.c[5] = m_Y;
	charBuf.c[6] = 0;
	charBuf.c[7] = m_Y;
	charBuf.c[8] = 0;
	charBuf.c[9] = m_Y;
	charBuf.c[10] = 0;
	charBuf.c[11] = m_Y;
	charBuf.c[12] = 0;
	charBuf.c[13] = m_Y;
	charBuf.c[14] = 0;
	charBuf.c[15] = m_Y;
	
	thresh = charBuf.v;

    while(datasize--)
    {
		//if (base[0] < m_thresh[1]) base[0] = 0; //u
		//if (base[1] < m_Y) base[1] = 0;//y1
		//if (base[2] < m_thresh[2]) base[2] = 0;//v
		//if (base[3] < m_Y) base[3] = 0;//y2
		//base += 4;
		in = vec_ld(0,base);
		mask = vec_cmpgt(in,thresh);
		in = vec_and(in,mask);
		vec_st(in,0,base);
		
		base += 16;
    }    
}
#endif //Altivec	  

/////////////////////////////////////////////////////////
// processGrayImage
//
/////////////////////////////////////////////////////////
void pix_threshold :: processGrayImage(imageStruct &image)
{
    int datasize = image.xsize * image.ysize;

    unsigned char *base = image.data;

    while(datasize--)
    {
		if (base[chGray] < m_thresh[chRed]) base[chGray] = 0;
		base++;
    }    
}

/////////////////////////////////////////////////////////
// vecThreshMess
//
/////////////////////////////////////////////////////////
void pix_threshold :: vecThreshMess(int argc, t_atom *argv)
{
    if (argc >= 4)
    {
    	m_thresh[chAlpha] = CLAMP(atom_getfloat(&argv[3]) * 255);
    }
    else if (argc == 3) m_thresh[3] = 0;
    else
    {
    	error("not enough threshold values");
    	return;
    }
    
    m_thresh[chRed] = CLAMP(atom_getfloat(&argv[0]) * 255);
    m_thresh[chGreen] = CLAMP(atom_getfloat(&argv[1]) * 255);
    m_thresh[chBlue] = CLAMP(atom_getfloat(&argv[2]) * 255);
    m_Y = CLAMP(atom_getfloat(&argv[0]) * 255);
    setPixModified();
}

/////////////////////////////////////////////////////////
// floatThreshMess
//
/////////////////////////////////////////////////////////
void pix_threshold :: floatThreshMess(float thresh)
{
    m_thresh[chRed] = m_thresh[chGreen] = m_thresh[chBlue] = m_Y = CLAMP(thresh * 255);
    // assumption that the alpha threshold should be zero
    m_thresh[chAlpha] = 0;
    setPixModified();
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_threshold :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_threshold::vecThreshMessCallback),
    	    gensym("vec_thresh"), A_GIMME, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_threshold::floatThreshMessCallback),
    	    gensym("ft1"), A_FLOAT, A_NULL);
}
void pix_threshold :: vecThreshMessCallback(void *data, t_symbol *, int argc, t_atom *argv)
{
    GetMyClass(data)->vecThreshMess(argc, argv);
}
void pix_threshold :: floatThreshMessCallback(void *data, t_floatarg thresh)
{
    GetMyClass(data)->floatThreshMess((float)thresh);
}
