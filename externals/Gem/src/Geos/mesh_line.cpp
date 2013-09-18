////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-2000 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "mesh_line.h"

#include "Gem/State.h"

CPPEXTERN_NEW_WITH_ONE_ARG(mesh_line, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// mesh_line
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
mesh_line :: mesh_line(t_floatarg sizeX)
        : GemShape(1)
{
  int sizeXi=static_cast<int>(sizeX);
  setGrid(sizeXi);
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
mesh_line :: ~mesh_line()
{ }

/////////////////////////////////////////////////////////
// getTexCoords
//
/////////////////////////////////////////////////////////
void mesh_line :: getTexCoords(void)
{
    for ( int i = 0; i < gridX; ++i)
    {
	  texCoords[i][0][0] = ((xsize*(1.*i)/(gridX-1.)) + xsize0 );
            //post("texCoords[%d][%d] = %f\t%f",i,j,texCoords[i][j][0],texCoords[i][j][1]);
    }
}

/////////////////////////////////////////////////////////
// setSize
//
/////////////////////////////////////////////////////////
void mesh_line :: setGrid( int valueX)
{
	if(valueX>=1) gridX = valueX;
	else gridX = 5;

    getTexCoords();
}

/////////////////////////////////////////////////////////
// renderShape
//
/////////////////////////////////////////////////////////
void mesh_line :: renderShape(GemState *state)
{
	int i;
    GLfloat sizeX = 2. / (gridX-1.);

   if(m_drawType==GL_DEFAULT_GEM)m_drawType=GL_LINE_STRIP;

   glNormal3f(0.0f, 0.0f, 1.0f);
   if (m_drawType == GL_LINE_LOOP)
     m_drawType = GL_LINE_STRIP;

   glNormal3f( 0.0f, 0.0f, 1.0f);

   if (GemShape::m_texType && GemShape::m_texNum>=3)
    {
		if ((xsize0!= GemShape::m_texCoords[0].s) ||
		(xsize != GemShape::m_texCoords[1].s-xsize0))
		alreadyInit = 0;

        if (!alreadyInit)
        {
			xsize0 = GemShape::m_texCoords[0].s;
	    	xsize  = GemShape::m_texCoords[1].s-xsize0;

            setGrid(gridX);
            alreadyInit = 1;
        }


   		glBegin(m_drawType);
        for (int i=0; i<=(gridX-1) ; i++)
        {
            glTexCoord2fv( texCoords[i][0] );
            glVertex3f( m_size * (i*sizeX - 1),0 , 0);
        }
        glEnd();
    }else
    {
        if (!alreadyInit)
        {
            xsize = 1;
			xsize0= 0;

            setGrid( gridX);
            alreadyInit = 1;
        }

        glBegin(m_drawType);
        for ( i = 0; i<=(gridX -1); i++)
        {
    		glTexCoord2fv( texCoords[i][0] );
            glVertex3f( m_size * (i*sizeX -1), 0 , 0 );
        }
        glEnd();
    }
}


/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void mesh_line :: obj_setupCallback(t_class *classPtr)
{
	class_addmethod(classPtr, reinterpret_cast<t_method>(&mesh_line::gridMessCallback),
                  gensym("grid"), A_FLOAT, A_NULL);
}

/////////////////////////////////////////////////////////
// setGrid
//
/////////////////////////////////////////////////////////

void mesh_line :: gridMessCallback(void *data, t_floatarg grid)
{
  int gridi=static_cast<int>(grid);
  GetMyClass(data)->setGrid(gridi);
}



