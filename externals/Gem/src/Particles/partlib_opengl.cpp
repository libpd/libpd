// opengl.cpp
//
// Copyright 1998 by David K. McAllister
//
// This file implements the API calls that draw particle groups in OpenGL.
//
// (l) 3004:forum::für::umläute:2003 modified for Gem

#include "partlib_general.h"

#include "Gem/GemGL.h"

// Emit OpenGL calls to draw the particles. These are drawn with
// whatever primitive type the user specified(GL_POINTS, for
// example). The color and radius are set per primitive, by default.
// For GL_LINES, the other vertex of the line is the velocity vector.
// XXX const_size is ignored.
PARTICLEDLL_API void pDrawGroupp(int primitive, bool const_size, bool const_color)
{
	_ParticleState &_ps = _GetPState();

	// Get a pointer to the particles in gp memory
	ParticleGroup *pg = _ps.pgrp;

	if(pg == NULL)
		return; // ERROR
	
	if(pg->p_count < 1)
		return;
	
	if(primitive == GL_POINTS)
	{
		glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
		glEnableClientState(GL_VERTEX_ARRAY);
		if(!const_color)
		{
			glEnableClientState(GL_COLOR_ARRAY);
			glColorPointer(4, GL_FLOAT, sizeof(Particle), &pg->list[0].color);
		}
		
		glVertexPointer(3, GL_FLOAT, sizeof(Particle), &pg->list[0].pos);
		glDrawArrays((GLenum)primitive, 0, pg->p_count);
		glPopClientAttrib();
		// XXX For E&S
		glDisableClientState(GL_COLOR_ARRAY);
	}
	else
	{
		// Assume GL_LINES
		glBegin((GLenum)primitive);
		
		if(!const_color)
		{
			for(int i = 0; i < pg->p_count; i++)
			{
				Particle &m = pg->list[i];
				
				// Warning: this depends on alpha following color in the Particle struct.
				glColor4fv((GLfloat *)&m.color);
				glVertex3fv((GLfloat *)&m.pos);
				
				// For lines, make a tail with the velocity vector's direction and
				// a length of radius.
				pVector tail = m.pos - m.vel;			
				glVertex3fv((GLfloat *)&tail);
			}
		}
		else
		{
			for(int i = 0; i < pg->p_count; i++)
			{
				Particle &m = pg->list[i];
				glVertex3fv((GLfloat *)&m.pos);
				
				// For lines, make a tail with the velocity vector's direction and
				// a length of radius.
				pVector tail = m.pos - m.vel;			
				glVertex3fv((GLfloat *)&tail);
			}
		}
		glEnd();
	}
}

PARTICLEDLL_API void pDrawGroupl(int dlist, bool const_size, bool const_color, bool const_rotation)
{
	_ParticleState &_ps = _GetPState();

	// Get a pointer to the particles in gp memory
	ParticleGroup *pg = _ps.pgrp;
	if(pg == NULL)
		return; // ERROR
	
	if(pg->p_count < 1)
		return;

	//if(const_color)
	//	glColor4fv((GLfloat *)&pg->list[0].color);

	for(int i = 0; i < pg->p_count; i++)
	{
		Particle &m = pg->list[i];

		glPushMatrix();
		glTranslatef(m.pos.x, m.pos.y, m.pos.z);

		if(!const_size)
			glScalef(m.size.x, m.size.y, m.size.z);
		else
			glScalef(pg->list[i].size.x, pg->list[i].size.y, pg->list[i].size.z);

		// Expensive! A sqrt, cross prod and acos. Yow.
		if(!const_rotation)
		{
			pVector vN(m.vel);
			vN.normalize();
			pVector voN(m.velB);
			voN.normalize();

			pVector biN;
			if(voN.x == vN.x && voN.y == vN.y && voN.z == vN.z)
				biN = pVector(0, 1, 0);
			else
				biN = vN ^ voN;
			biN.normalize();

			pVector N(vN ^ biN);

			double M[16];
			M[0] = vN.x;  M[4] = biN.x;  M[8] = N.x;  M[12] = 0;
			M[1] = vN.y;  M[5] = biN.y;  M[9] = N.y;  M[13] = 0;
			M[2] = vN.z;  M[6] = biN.z;  M[10] = N.z; M[14] = 0;
			M[3] = 0;     M[7] = 0;      M[11] = 0;   M[15] = 1;
			glMultMatrixd(M);
		}

		// Warning: this depends on alpha following color in the Particle struct.
		if(!const_color)
			glColor4fv((GLfloat *)&m.color);

		glCallList(dlist);

		glPopMatrix();
	}
}
