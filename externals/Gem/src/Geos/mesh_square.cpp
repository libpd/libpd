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

#include "mesh_square.h"

#include "Gem/State.h"

CPPEXTERN_NEW_WITH_TWO_ARGS(mesh_square, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// mesh_square
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
mesh_square :: mesh_square(t_floatarg sizeX, t_floatarg sizeY)
        : GemShape(1)
{
  int sizeXi=static_cast<int>(sizeX);
  int sizeYi=static_cast<int>(sizeY);
  setSize(sizeXi,sizeYi);
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
mesh_square :: ~mesh_square()
{ }

/////////////////////////////////////////////////////////
// getTexCoords
//
/////////////////////////////////////////////////////////
void mesh_square :: getTexCoords(void)
{
    for ( int i = 0; i < gridX; ++i)
    {
        for ( int j = 0; j < gridY; ++j)
        {
	  texCoords[i][j][0] = ((xsize*(1.*i)/(gridX-1.)) + xsize0 );
	  texCoords[i][j][1] = ((ysize*(1.*j)/(gridY-1.)) + ysize0 );
            //post("texCoords[%d][%d] = %f\t%f",i,j,texCoords[i][j][0],texCoords[i][j][1]);
        }
    }
}

/////////////////////////////////////////////////////////
// setSize
//
/////////////////////////////////////////////////////////
void mesh_square :: setSize( int valueX, int valueY )
{
	if(valueX>1) gridX = valueX;
	else gridX = 5;

	if(valueY>1) gridY = valueY;
	else gridY = gridX;

    getTexCoords();
}

void mesh_square :: setGridX( int valueX )
{
  setSize(valueX, gridY);
}
void mesh_square :: setGridY( int valueY )
{
  setSize(gridX, valueY);
}

/////////////////////////////////////////////////////////
// renderShape
//
/////////////////////////////////////////////////////////
void mesh_square :: renderShape(GemState *state)
{
	int i,j;
    GLfloat sizeX = 2. / (gridX-1.);
    GLfloat sizeY = 2. / (gridY-1.);

   if(m_drawType==GL_DEFAULT_GEM)m_drawType=GL_TRIANGLE_STRIP;

   glNormal3f(0.0f, 0.0f, 1.0f);
   if (m_drawType == GL_LINE_LOOP)
     m_drawType = GL_LINES;

   glNormal3f( 0.0f, 0.0f, 1.0f);

   if (GemShape::m_texType && GemShape::m_texNum>=3)
    {
		if ((xsize0!= GemShape::m_texCoords[0].s) ||
		(xsize != GemShape::m_texCoords[1].s-xsize0) ||
		(ysize0!= GemShape::m_texCoords[1].t) ||
		(ysize != GemShape::m_texCoords[2].t-ysize0))
		alreadyInit = 0;

        if (!alreadyInit)
        {
			xsize0 = GemShape::m_texCoords[0].s;
	    	xsize  = GemShape::m_texCoords[1].s-xsize0;
	    	ysize0 = GemShape::m_texCoords[1].t;
	    	ysize  = GemShape::m_texCoords[2].t-ysize0;

            setSize( gridX, gridY );
            alreadyInit = 1;
        }

        for (int i=0; i<(gridX-1) ; i++)
        {
    		glBegin(m_drawType);
            for (int j = 0; j < gridY ; j++)
            {
                glTexCoord2fv( texCoords[i][j] );
                glVertex3f( m_size * (i*sizeX - 1), m_size * (j*sizeY -1) , 0);

                glTexCoord2fv( texCoords[i+1][j] );
                glVertex3f(  m_size * ((i+1)*sizeX - 1), m_size * (j*sizeY -1), 0);
            }
            glEnd();
        }
    }else
    {
        if (!alreadyInit)
        {
            xsize = 1;
            ysize = 1;
			ysize0= 0;
			xsize0= 0;

            setSize( gridX, gridY);
            alreadyInit = 1;
        }

        for ( i = 0; i<(gridX -1); i++)
        {
            glBegin(m_drawType);
            for ( j = 0; j < gridY  ; j++)
            {
				glTexCoord2fv( texCoords[i][j] );
                glVertex3f( m_size * (i*sizeX -1), m_size * (j*sizeY -1), 0 );
            
                glTexCoord2fv( texCoords[i+1][j] );
                glVertex3f( m_size * ((i+1)*sizeX -1), m_size * (j*sizeY -1), 0 );
            }
            glEnd();
        }
    }
}


/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void mesh_square :: obj_setupCallback(t_class *classPtr)
{
	class_addmethod(classPtr, reinterpret_cast<t_method>(&mesh_square::gridMessCallback),
                  gensym("grid"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&mesh_square::gridXMessCallback),
                  gensym("gridX"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&mesh_square::gridYMessCallback),
                  gensym("gridY"), A_FLOAT, A_NULL);
}

/////////////////////////////////////////////////////////
// setGrid
//
/////////////////////////////////////////////////////////

void mesh_square :: gridMessCallback(void *data, t_floatarg grid)
{
  int gridi=static_cast<int>(grid);
  GetMyClass(data)->setSize(gridi,gridi);
}

void mesh_square :: gridXMessCallback(void *data, t_floatarg grid)
{
  int gridi=static_cast<int>(grid);
  GetMyClass(data)->setGridX(gridi);
}

void mesh_square :: gridYMessCallback(void *data, t_floatarg grid)
{
  int gridi=static_cast<int>(grid);
  GetMyClass(data)->setGridY(gridi);
}


