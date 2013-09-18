// p_vector.h - yet another vector class.
//
// Copyright 1997 by Jonathan P. Leech
// Modifications Copyright 1997-1999 by David K. McAllister
//
// A simple 3D float vector class for internal use by the particle systems.

#ifndef particle_vector_h
#define particle_vector_h

#include "Utils/GemMath.h"

class pVector
{
public:
	float x, y, z;
	
	inline pVector(float ax, float ay, float az) : x(ax), y(ay), z(az)
	{
		//x = ax; y = ay; z = az;
	}
	
	inline pVector() {}
	
	inline float length() const
	{
		return sqrtf(x*x+y*y+z*z);
	}
	
	inline float length2() const
	{
		return (x*x+y*y+z*z);
	}
	
	inline float normalize()
	{
		float onel = 1.0f / sqrtf(x*x+y*y+z*z);
		x *= onel;
		y *= onel;
		z *= onel;
		
		return onel;
	}
	
	inline float operator*(const pVector &a) const
	{
		return x*a.x + y*a.y + z*a.z;
	}
	
	inline pVector operator*(const float s) const
	{
		return pVector(x*s, y*s, z*s);
	}
	
	inline pVector operator/(const float s) const
	{
		float invs = 1.0f / s;
		return pVector(x*invs, y*invs, z*invs);
	}
	
	inline pVector operator+(const pVector& a) const
	{
		return pVector(x+a.x, y+a.y, z+a.z);
	}
	
	inline pVector operator-(const pVector& a) const
	{
		return pVector(x-a.x, y-a.y, z-a.z);
	}
	
	inline pVector operator-()
	{
		x = -x;
		y = -y;
		z = -z;
		return *this;
	}
	
	inline pVector& operator+=(const pVector& a)
	{
		x += a.x;
		y += a.y;
		z += a.z;
		return *this;
	}
	
	inline pVector& operator-=(const pVector& a)
	{
		x -= a.x;
		y -= a.y;
		z -= a.z;
		return *this;
	}
	
	inline pVector& operator*=(const float a)
	{
		x *= a;
		y *= a;
		z *= a;
		return *this;
	}
	
	inline pVector& operator/=(const float a)
	{
		float b = 1.0f / a;
		x *= b;
		y *= b;
		z *= b;
		return *this;
	}
	
	inline pVector& operator=(const pVector& a)
	{
		x = a.x;
		y = a.y;
		z = a.z;
		return *this;
	}
	
	inline pVector operator^(const pVector& b) const
	{
		return pVector(
			y*b.z-z*b.y,
			z*b.x-x*b.z,
			x*b.y-y*b.x);
	}
};

#endif
