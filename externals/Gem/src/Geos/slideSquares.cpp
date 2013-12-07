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

#include "slideSquares.h"

#include "Gem/State.h"
#include <string.h>
#include "math.h"
#ifdef __APPLE__
#include <Carbon/Carbon.h>
#else
#include <stdlib.h>
#endif

typedef struct {
            GLfloat	X;
            GLfloat	Y;
            GLfloat	U;
            GLfloat	V;
            GLfloat	Speed;
} TSlider;
        
TSlider	Sliders[64];
//int	init = 0;

static inline float ourRand( float Max )
{
   return( (Max * rand()) / RAND_MAX );
}

CPPEXTERN_NEW_WITH_TWO_ARGS(slideSquares, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// slideSquares
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
slideSquares :: slideSquares(t_floatarg width, t_floatarg height)
		   : GemShape(width), m_height(height)
{
    if (m_height == 0.f)
		m_height = 1.f;

    // the height inlet
    m_inletH = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("Ht"));
    slide_init();
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
slideSquares :: ~slideSquares()
{
    inlet_free(m_inletH);
}

/////////////////////////////////////////////////////////
// renderShape
//
/////////////////////////////////////////////////////////
void slideSquares :: renderShape(GemState *state)
{
    int i;
  if(m_drawType==GL_DEFAULT_GEM)m_drawType=GL_QUADS;

    glDisable(GL_DEPTH_TEST);
    //glTranslatef( 0.0f, 0.0f, 0.1f );
    glColor4f( 0.5f, 0.5f, 0.5f, 0.75f );
    glNormal3f(0.0f, 0.0f, 1.0f);
    //glScalef( 1.f, 0.8f, 1.f );
    int curCoord = 0;
    if (GemShape::m_texType && GemShape::m_texNum)
    {
      glBegin(m_drawType);
      for (i=0; i<= 31; i++){
	glTexCoord2f(GemShape::m_texCoords[curCoord].s*Sliders[i].U, GemShape::m_texCoords[curCoord].t*Sliders[i].V ); 
	glVertex3f(m_size*(Sliders[i].X - 0.1), m_height*(Sliders[i].Y - 0.1),  0.0);

	if (GemShape::m_texNum > 1) curCoord = 1;
	glTexCoord2f(GemShape::m_texCoords[curCoord].s*(Sliders[i].U+0.1), GemShape::m_texCoords[curCoord].t*Sliders[i].V ); 
	glVertex3f(m_size*(Sliders[i].X + 0.1), m_height*(Sliders[i].Y - 0.1),  0.0);

	if (GemShape::m_texNum > 2) curCoord = 2;
	glTexCoord2f(GemShape::m_texCoords[curCoord].s*(Sliders[i].U+0.1),GemShape::m_texCoords[curCoord].t*(Sliders[i].V+0.1)); 
	glVertex3f(m_size*(Sliders[i].X + 0.1), m_height*(Sliders[i].Y + 0.1),  0.0);

	if (GemShape::m_texNum > 3) curCoord = 3;
	glTexCoord2f(GemShape::m_texCoords[curCoord].s*Sliders[i].U,GemShape::m_texCoords[curCoord].t*(Sliders[i].V+0.1));
	glVertex3f(m_size*(Sliders[i].X - 0.1), m_height*(Sliders[i].Y + 0.1),  0.0);
                
	Slide( i );
      }
      glEnd();
    }
    else
    {
        glBegin(m_drawType);
            for (i=0; i<= 31; i++){
	    	glTexCoord2f(Sliders[i].U,     Sliders[i].V ); 
                glVertex3f(m_size*(Sliders[i].X - 0.1), m_height*(Sliders[i].Y - 0.1),  0.0);

	        if (GemShape::m_texNum > 1) curCoord = 1;
                    glTexCoord2f( Sliders[i].U+0.1, Sliders[i].V    ); 
                    glVertex3f(m_size*(Sliders[i].X + 0.1), m_height*(Sliders[i].Y - 0.1),  0.0);

	        if (GemShape::m_texNum > 2) curCoord = 2;
                    glTexCoord2f( Sliders[i].U+0.1, Sliders[i].V+0.1 ); 
                    glVertex3f(m_size*(Sliders[i].X + 0.1), m_height*(Sliders[i].Y + 0.1),  0.0);

	        if (GemShape::m_texNum > 3) curCoord = 3;
                    glTexCoord2f(Sliders[i].U,     Sliders[i].V+0.1); 
                    glVertex3f(m_size*(Sliders[i].X - 0.1), m_height*(Sliders[i].Y + 0.1),  0.0);
                
                Slide( i );
            }
        glEnd();
    }
}
void slideSquares :: slide_init()
{
    for (int i=0; i<63; i++) {
        Sliders[i].U = ourRand(1);
        Sliders[i].X = Sliders[i].U*2 -1;
        Sliders[i].Y = ourRand(1) -0.5;
        Sliders[i].V = (Sliders[i].Y/2 + 0.45);
        //Sliders[i].V = (Sliders[i].Y + 0.45);
        Sliders[i].Speed = ourRand(1)/320 + 1/1600;
        
        /*post("sliders[%d].U = %f",i,Sliders[i].U);
        post("sliders[%d].X = %f",i,Sliders[i].X);
        post("sliders[%d].Y = %f",i,Sliders[i].Y);
        post("sliders[%d].V = %f",i,Sliders[i].V);
        post("sliders[%d].speed = %f",i,Sliders[i].Speed); */
    }
}
GLvoid slideSquares :: Slide( int i )
{
    Sliders[i].U = Sliders[i].U + Sliders[i].Speed;
    if (Sliders[i].U > 1) {
        Sliders[i].U =0;
        Sliders[i].X=1;
        Sliders[i].Y = ourRand(1) - 0.5;
        Sliders[i].V = Sliders[i].Y/2 + 0.45;
        //Sliders[i].V = (Sliders[i].Y + 0.45);
        Sliders[i].Speed = ourRand(1)/320 + 1/1600;
    } else
        Sliders[i].X = Sliders[i].U*2 -1;
    /*
    printf("Sliders[0].U = %f\n",Sliders[0].U);
    printf("Sliders[0].X = %f\n",Sliders[0].X);
    printf("Sliders[0].Y = %f\n",Sliders[0].Y);
    printf("Sliders[0].V = %f\n",Sliders[0].V);
    printf("Sliders[0].Speed = %f\n",Sliders[0].Speed);
    */
}

/////////////////////////////////////////////////////////
// heightMess
//
/////////////////////////////////////////////////////////
void slideSquares :: heightMess(float size)
{
    m_height = size;
    setModified();
}

/////////////////////////////////////////////////////////
// typeMess
//
/////////////////////////////////////////////////////////
void slideSquares :: typeMess(t_symbol *type)
{
    if (!strcmp(type->s_name, "line")) 
	    m_drawType = GL_LINE_LOOP;
    else if (!strcmp(type->s_name, "fill")) 
	    m_drawType = GL_QUADS;
    else if (!strcmp(type->s_name, "point"))
	    m_drawType = GL_POINTS;
    else
    {
	    error ("unknown draw style");
	    return;
    }
    setModified();
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void slideSquares :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, reinterpret_cast<t_method>(&slideSquares::heightMessCallback),
    	    gensym("Ht"), A_FLOAT, A_NULL);
}

void slideSquares :: heightMessCallback(void *data, t_floatarg size)
{
    GetMyClass(data)->heightMess(size);
}

