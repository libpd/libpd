////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.at
//
// Implementation file
//
//    
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "Vector.h"

#include <math.h>

#ifdef __ppc__
# include "Functions.h"
# undef sqrt
# define sqrt fast_sqrtf
#endif


// default constructor
CVector3::CVector3() : x(0), y(0), z(0) {}

// This is our constructor that allows us to initialize our data upon creating an instance
CVector3::CVector3(float X, float Y, float Z) : x(X), y(Y), z(Z) {}

// Here we overload the + operator so we can add vectors together 
CVector3 CVector3::operator+(CVector3 vVector) const
{
  // Return the added vectors result.
  return CVector3(vVector.x + x, vVector.y + y, vVector.z + z);
}

// Here we overload the - operator so we can subtract vectors 
CVector3 CVector3::operator-(CVector3 vVector) const
{
  // Return the subtracted vectors result
  return CVector3(x - vVector.x, y - vVector.y, z - vVector.z);
}

// Here we overload the - operator so we can negate a vector
CVector3 CVector3::operator-() const
{
  // Return the subtracted vectors result
  return CVector3(-x, -y, -z);
}


// Here we overload the * operator so we can multiply by scalars
CVector3 CVector3::operator*(float num) const
{
  // Return the scaled vector
  return CVector3(x * num, y * num, z * num);
}
// Here we overload the * operator so we can dot-multiply 
// note: i chose dot-multiplication because this can be done consistently for vectors of any length
//       but i don't know whether this is "the right way" to do
float CVector3::operator*(CVector3 vVector) const
{
  // Return the scaled vector
  return (x*vVector.x+y*vVector.y+z*vVector.z);
}
// cross-multiplication
CVector3 CVector3::cross(CVector3 vVector) const
{
    CVector3 vCross;                                // The vector to hold the cross product
    vCross.x = ((y * vVector.z) - (z * vVector.y)); // Get the X value
    vCross.y = ((z * vVector.x) - (x * vVector.z)); // Get the Y value
    vCross.z = ((x * vVector.y) - (y * vVector.x)); // Get the Z value

    return vCross;                              // Return the cross product
}


// Here we overload the / operator so we can divide by a scalar
CVector3 CVector3::operator/(float num) const
{
  // Return the scale vector
  return CVector3(x / num, y / num, z / num);
}



// here we calculate the absolute-value of the vector
float CVector3::abs() const
{
  return sqrt(x*x+y*y+z*z);
}

// here we calculate the square of the absolute-value of the vector
float CVector3::abs2() const
{
  return (x*x+y*y+z*z);
}

// here we normalize the vector
CVector3 CVector3::normalize() const
{
  CVector3 vNormal;
  float InvMagnitude = 1.f / abs();  // Get the magnitude

  vNormal.x = x*InvMagnitude;              // Divide the vector's X by the magnitude
  vNormal.y = y*InvMagnitude;              // Divide the vector's Y by the magnitude
  vNormal.z = z*InvMagnitude;              // Divide the vector's Z by the magnitude
  
  return vNormal;                         // Return the normal
}
// compares to vectores and resturns true is they are equal (within a certain threshold)
// An epsilon that works fairly well is 0.000001.
bool CVector3::equals(CVector3 vVector, float epsilon) const
{
  return (fabsf(x - vVector.x) < epsilon &&
	  fabsf(y - vVector.y) < epsilon &&
	  fabsf(z - vVector.z) < epsilon);
}
