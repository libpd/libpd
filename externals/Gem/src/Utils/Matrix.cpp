////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-1999 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "Matrix.h"

#include <assert.h>

#include <math.h>
#include <stdio.h>

#ifdef __ppc__
#include <ppc_intrinsics.h>
#undef sqrt
#define sqrt fast_sqrtf
inline float fast_sqrtf(float x)
{
	register float est = (float)__frsqrte(x);
	return x * 0.5f * est * __fnmsubs(est * est, x, 3.0f);
}
#endif

static const float PI = 3.141592f;
static const float DEGREES_TO_RADIANS = 0.017453f;

/////////////////////////////////////////////////////////
//
// Matrix
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
Matrix :: Matrix()
{
	identity();
}

/////////////////////////////////////////////////////////
// identity
//
/////////////////////////////////////////////////////////
void Matrix :: identity()
{
	mat[0][1] = mat[0][2] = mat[0][3] =
	mat[1][0] = mat[1][2] = mat[1][3] =
	mat[2][0] = mat[2][1] = mat[2][3] =
	mat[3][0] = mat[3][1] = mat[3][2] = 0.0f;

	mat[0][0] = mat[1][1] = mat[2][2] = mat[3][3] = 1.0f;
}

/////////////////////////////////////////////////////////
// scale
//
/////////////////////////////////////////////////////////
void Matrix :: scale(float x, float y, float z)
{
	mat[0][0] *= x;
	mat[0][1] *= x;
	mat[0][2] *= x;
	mat[1][0] *= y;
	mat[1][1] *= y;
	mat[1][2] *= y;
	mat[2][0] *= z;
	mat[2][1] *= z;
	mat[2][2] *= z;
}

/////////////////////////////////////////////////////////
// multiply
//
/////////////////////////////////////////////////////////
void Matrix :: multiply(Matrix *matrix)
{
	Matrix tmp;

	assert(matrix);

	tmp = *this;

	mat[0][0] = tmp.mat[0][0] * matrix->mat[0][0] + tmp.mat[0][1] * matrix->mat[1][0] + tmp.mat[0][2] * matrix->mat[2][0] + tmp.mat[0][3] * matrix->mat[3][0];
	mat[1][0] = tmp.mat[1][0] * matrix->mat[0][0] + tmp.mat[1][1] * matrix->mat[1][0] + tmp.mat[1][2] * matrix->mat[2][0] + tmp.mat[1][3] * matrix->mat[3][0];
	mat[2][0] = tmp.mat[2][0] * matrix->mat[0][0] + tmp.mat[2][1] * matrix->mat[1][0] + tmp.mat[2][2] * matrix->mat[2][0] + tmp.mat[2][3] * matrix->mat[3][0];
	mat[3][0] = tmp.mat[3][0] * matrix->mat[0][0] + tmp.mat[3][1] * matrix->mat[1][0] + tmp.mat[3][2] * matrix->mat[2][0] + tmp.mat[3][3] * matrix->mat[3][0];

	mat[0][1] = tmp.mat[0][0] * matrix->mat[0][1] + tmp.mat[0][1] * matrix->mat[1][1] + tmp.mat[0][2] * matrix->mat[2][1] + tmp.mat[0][3] * matrix->mat[3][1];
	mat[1][1] = tmp.mat[1][0] * matrix->mat[0][1] + tmp.mat[1][1] * matrix->mat[1][1] + tmp.mat[1][2] * matrix->mat[2][1] + tmp.mat[1][3] * matrix->mat[3][1];
	mat[2][1] = tmp.mat[2][0] * matrix->mat[0][1] + tmp.mat[2][1] * matrix->mat[1][1] + tmp.mat[2][2] * matrix->mat[2][1] + tmp.mat[2][3] * matrix->mat[3][1];
	mat[3][1] = tmp.mat[3][0] * matrix->mat[0][1] + tmp.mat[3][1] * matrix->mat[1][1] + tmp.mat[3][2] * matrix->mat[2][1] + tmp.mat[3][3] * matrix->mat[3][1];

	mat[0][2] = tmp.mat[0][0] * matrix->mat[0][2] + tmp.mat[0][1] * matrix->mat[1][2] + tmp.mat[0][2] * matrix->mat[2][2] + tmp.mat[0][3] * matrix->mat[3][2];
	mat[1][2] = tmp.mat[1][0] * matrix->mat[0][2] + tmp.mat[1][1] * matrix->mat[1][2] + tmp.mat[1][2] * matrix->mat[2][2] + tmp.mat[1][3] * matrix->mat[3][2];
	mat[2][2] = tmp.mat[2][0] * matrix->mat[0][2] + tmp.mat[2][1] * matrix->mat[1][2] + tmp.mat[2][2] * matrix->mat[2][2] + tmp.mat[2][3] * matrix->mat[3][2];
	mat[3][2] = tmp.mat[3][0] * matrix->mat[0][2] + tmp.mat[3][1] * matrix->mat[1][2] + tmp.mat[3][2] * matrix->mat[2][2] + tmp.mat[3][3] * matrix->mat[3][2];

	mat[0][3] = tmp.mat[0][0] * matrix->mat[0][3] + tmp.mat[0][1] * matrix->mat[1][3] + tmp.mat[0][2] * matrix->mat[2][3] + tmp.mat[0][3] * matrix->mat[3][3];
	mat[1][3] = tmp.mat[1][0] * matrix->mat[0][3] + tmp.mat[1][1] * matrix->mat[1][3] + tmp.mat[1][2] * matrix->mat[2][3] + tmp.mat[1][3] * matrix->mat[3][3];
	mat[2][3] = tmp.mat[2][0] * matrix->mat[0][3] + tmp.mat[2][1] * matrix->mat[1][3] + tmp.mat[2][2] * matrix->mat[2][3] + tmp.mat[2][3] * matrix->mat[3][3];
	mat[3][3] = tmp.mat[3][0] * matrix->mat[0][3] + tmp.mat[3][1] * matrix->mat[1][3] + tmp.mat[3][2] * matrix->mat[2][3] + tmp.mat[3][3] * matrix->mat[3][3];
}

/////////////////////////////////////////////////////////
// translate
//
/////////////////////////////////////////////////////////
void Matrix :: translate(float X, float Y, float Z)
{
	mat[0][3] += X * mat[0][0] + Y * mat[0][1] + Z * mat[0][2];
	mat[1][3] += X * mat[1][0] + Y * mat[1][1] + Z * mat[1][2];
	mat[2][3] += X * mat[2][0] + Y * mat[2][1] + Z * mat[2][2];
}

/////////////////////////////////////////////////////////
// rotateX
//
/////////////////////////////////////////////////////////
void Matrix :: rotateX(float degrees)
{
	Matrix tmp(*this);
	float s, c;

	c = (float)cos(DEGREES_TO_RADIANS * degrees);
	s = (float)sin(DEGREES_TO_RADIANS * degrees);

	mat[0][1] = c * tmp.mat[0][1] + s * tmp.mat[0][2];
	mat[1][1] = c * tmp.mat[1][1] + s * tmp.mat[1][2];
	mat[2][1] = c * tmp.mat[2][1] + s * tmp.mat[2][2];
	mat[0][2] = c * tmp.mat[0][2] - s * tmp.mat[0][1];
	mat[1][2] = c * tmp.mat[1][2] - s * tmp.mat[1][1];
	mat[2][2] = c * tmp.mat[2][2] - s * tmp.mat[2][1];
}

/////////////////////////////////////////////////////////
// rotateY
//
/////////////////////////////////////////////////////////
void Matrix :: rotateY(float degrees)
{
	Matrix tmp(*this);
	float s, c;

	c = (float)cos(DEGREES_TO_RADIANS * degrees);
	s = (float)sin(DEGREES_TO_RADIANS * degrees);

	mat[0][2] = c * tmp.mat[0][2] + s * tmp.mat[0][0];
	mat[1][2] = c * tmp.mat[1][2] + s * tmp.mat[1][0];
	mat[2][2] = c * tmp.mat[2][2] + s * tmp.mat[2][0];

	mat[0][0] = c * tmp.mat[0][0] - s * tmp.mat[0][2];
	mat[1][0] = c * tmp.mat[1][0] - s * tmp.mat[1][2];
	mat[2][0] = c * tmp.mat[2][0] - s * tmp.mat[2][2];
}

/////////////////////////////////////////////////////////
// rotateZ
//
/////////////////////////////////////////////////////////
void Matrix :: rotateZ(float degrees)
{
	Matrix tmp(*this);
	float s, c;

	c = (float)cos(DEGREES_TO_RADIANS * degrees);
	s = (float)sin(DEGREES_TO_RADIANS * degrees);

	mat[0][0] = c * tmp.mat[0][0] + s * tmp.mat[0][1];
	mat[1][0] = c * tmp.mat[1][0] + s * tmp.mat[1][1];
	mat[2][0] = c * tmp.mat[2][0] + s * tmp.mat[2][1];

	mat[0][1] = c * tmp.mat[0][1] - s * tmp.mat[0][0];
	mat[1][1] = c * tmp.mat[1][1] - s * tmp.mat[1][0];
	mat[2][1] = c * tmp.mat[2][1] - s * tmp.mat[2][0];
}

/////////////////////////////////////////////////////////
// transform
//
/////////////////////////////////////////////////////////
void Matrix::transform(float srcX, float srcY, float srcZ, float *dstX, float *dstY, float *dstZ) const
{
	*dstX = srcX * mat[0][0] + srcY * mat[0][1] + srcZ * mat[0][2] + mat[0][3];
	*dstY = srcX * mat[1][0] + srcY * mat[1][1] + srcZ * mat[1][2] + mat[1][3];
	*dstZ = srcX * mat[2][0] + srcY * mat[2][1] + srcZ * mat[2][2] + mat[2][3];
}

/////////////////////////////////////////////////////////
// generateNormal
//
/////////////////////////////////////////////////////////
void Matrix :: generateNormal(const float *v1, const float *v2, const float *v3, float *dst)
{
	float v1v2[3];
	float v2v3[3];
	for (int i = 0; i < 3; i++)
	{
		v1v2[i] = v1[i] - v2[i];
		v2v3[i] = v2[i] - v3[i];
	}
	
	// cross product
	dst[0] = (v1v2[1] * v2v3[2]) - (v1v2[2] * v2v3[1]);
	dst[1] = (v1v2[2] * v2v3[0]) - (v1v2[0] * v2v3[2]);
	dst[2] = (v1v2[0] * v2v3[1]) - (v1v2[1] * v2v3[0]);

	// normalize
	float mag = (float)sqrt(dst[0] * dst[0] + dst[1] * dst[1] + dst[2] * dst[2]);
	//assert( mag != 0.0f );
	if ( mag != 0.0f )
	{
		dst[0] *= 1.f/mag;
		dst[1] *= 1.f/mag;
		dst[2] *= 1.f/mag;
	}
}

