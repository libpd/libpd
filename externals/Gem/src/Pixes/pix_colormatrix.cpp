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

#include "pix_colormatrix.h"

CPPEXTERN_NEW(pix_colormatrix);

/////////////////////////////////////////////////////////
//
// pix_colormatrix
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_colormatrix :: pix_colormatrix()
{
    // zero out the matrix
    for (int i = 0; i < 16; i++) m_matrix[i] = 0.0;
    // insert ones allow the diagonal
    m_matrix[0] = 1.;
    m_matrix[5] = 1.;
    m_matrix[10] = 1.;
    m_matrix[15] = 1.;
    
    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("matrix"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_colormatrix :: ~pix_colormatrix()
{ }

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_colormatrix :: processRGBAImage(imageStruct &image)
{
    unsigned char *base = image.data;
	int count = image.ysize * image.xsize;

    while (count--)
    {
        float red   = base[chRed];
        float green = base[chGreen];
        float blue  = base[chBlue];
        float alpha = base[chAlpha];

    	float r = m_matrix[0] * red + m_matrix[4] * green + m_matrix[8] * blue + m_matrix[12] * alpha;
    	float g = m_matrix[1] * red + m_matrix[5] * green + m_matrix[9] * blue + m_matrix[13] * alpha;
    	float b = m_matrix[2] * red + m_matrix[6] * green + m_matrix[10] * blue + m_matrix[14] * alpha;
    	float a = m_matrix[3] * red + m_matrix[7] * green + m_matrix[11] * blue + m_matrix[15] * alpha;
    	base[chRed] = CLAMP(r);
    	base[chGreen] = CLAMP(g);
    	base[chBlue] = CLAMP(b);
    	base[chAlpha] = CLAMP(a);
		base += 4;
	}
}

/////////////////////////////////////////////////////////
// matrixMess
//
/////////////////////////////////////////////////////////
void pix_colormatrix :: matrixMess(int argc, t_atom *argv)
{
    // 4x4 matrix
    if (argc == 16)
    {
        for (int i = 0; i < 16; i++) m_matrix[i] = atom_getfloat(&argv[i]);
    }
    // 3x3 matrix
    else if (argc == 9)
    {
        // default other values to zero, except alpha
        for (int i = 0, j = 0; i < 9; i+=3, j+=4)
        {
            m_matrix[j] = atom_getfloat(&argv[i]);
            m_matrix[j+1] = atom_getfloat(&argv[i+1]);
            m_matrix[j+2] = atom_getfloat(&argv[i+2]);
            m_matrix[j+3] = 0.;
        }
        m_matrix[12] = m_matrix[13] = m_matrix[14] = 0.;
        m_matrix[15] = 1.;
    }
    else
    {
    	error("GEM: color matrix size not correct");
    	return;
    }
    
    setPixModified();
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_colormatrix :: obj_setupCallback(t_class *classPtr)
{
  class_addcreator(reinterpret_cast<t_newmethod>(create_pix_colormatrix), 
		   gensym("pix_colourmatrix"), A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_colormatrix::matrixMessCallback),
    	    gensym("matrix"), A_GIMME, A_NULL);
}
void pix_colormatrix :: matrixMessCallback(void *data, t_symbol *, int argc, t_atom *argv)
{
    GetMyClass(data)->matrixMess(argc, argv);
}
