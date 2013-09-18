////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// Implementation file
//
// Copyright (c) 2002-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//	zmoelnig@iem.kug.ac.at
//  For information on usage and redistribution, and for a DISCLAIMER
//  *  OF ALL WARRANTIES, see the file, "GEM.LICENSE.TERMS"
//
//   
////////////////////////////////////////////////////////

#include "gemlist_info.h"

#include "math.h"

#ifdef __ppc__
#include "Utils/Functions.h"
#undef sqrt
#define sqrt fast_sqrtf
#endif

#define rad2deg -57.2957795132


CPPEXTERN_NEW_WITH_ONE_ARG ( gemlist_info , t_floatarg, A_DEFFLOAT );

/////////////////////////////////////////////////////////
//
// gemlist_info
//
/////////////////////////////////////////////////////////
// Constructor
//
gemlist_info :: gemlist_info	(t_floatarg arg0=0) {
  m_outletRotation = outlet_new(this->x_obj, 0);
  m_outletShear = outlet_new(this->x_obj, 0);
  m_outletScale = outlet_new(this->x_obj, 0);
  m_outletPosition = outlet_new(this->x_obj, 0);
}
/////////////////////////////////////////////////////////
// Destructor
//
gemlist_info :: ~gemlist_info () {
  outlet_free(m_outletScale);
  outlet_free(m_outletPosition);
  outlet_free(m_outletRotation);
  outlet_free(m_outletShear);

}

/////////////////////////////////////////////////////////
// extension check
//
bool gemlist_info :: isRunnable() {
  if(GLEW_VERSION_1_1)
    return true;

  error("your system does not support openGL-1.0 needed for operation");
  return false;
}


/////////////////////////////////////////////////////////
// Render
//
void gemlist_info :: render(GemState *state) {
  float mi[16]={0};
  t_atom alist[12];


  float X, Y, Z, ScaleX, ScaleY, ScaleZ, shearYX, shearZX, shearZY;

  glGetFloatv(GL_MODELVIEW_MATRIX,mi);

  // test de syngularité a effectuer

  // normalisation
  //	for (i=0; i<16; i++) mi[i] /= mi[15];
  // not usefull because I never saw mi[15]!=1; if this change, un-comment this normalisation procedure
  ScaleX = sqrt (mi[0] * mi[0] + mi[4] * mi[4] + mi[8] * mi[8]);
  mi[0] /= ScaleX; // Normalise X
  mi[4] /= ScaleX;
  mi[8] /= ScaleX;

  shearYX = mi[0]*mi[1] + mi[4]*mi[5] + mi[8]*mi[9];

  mi[1] -= shearYX * mi[0]; //make X and Y orthogonal
  mi[5] -= shearYX * mi[4]; 
  mi[9] -= shearYX * mi[8]; 

  ScaleY = sqrt (mi[1] * mi[1] + mi[5] * mi[5] + mi[9] * mi[9]);

  mi[1] /= ScaleY; // Normalise Y
  mi[5] /= ScaleY;
  mi[9] /= ScaleY;
  shearYX /= ScaleY;

  shearZX = mi[0]*mi[2] + mi[4]*mi[6] + mi[8]*mi[10];

  mi[2] -= shearZX * mi[0]; //make X and Z orthogonal
  mi[6] -= shearZX * mi[4]; 
  mi[10] -= shearZX * mi[8]; 

  shearZY = mi[1]*mi[2] + mi[5]*mi[6] + mi[9]*mi[10];

  mi[2] -= shearZY * mi[1]; //make X and Z orthogonal
  mi[6] -= shearZY * mi[5]; 
  mi[10] -= shearZY * mi[9]; 

  ScaleZ = sqrt (mi[2] * mi[2] + mi[6] * mi[6] + mi[10] * mi[10]);

  mi[2] /= ScaleZ; // Normalise Y
  mi[6] /= ScaleZ;
  mi[10] /= ScaleZ;
  shearZX /= ScaleZ;
  shearZY /= ScaleZ;

  // maybee some test could be inserted here.
  // The matrix can only be decomposed if it's determinent is not 0.

  Y = asin(-mi[8]);
  if ( cos(Y) != 0 ) {
    X = atan2(mi[9], mi[10]);
    Z = atan2(mi[4], mi[0]);
  } else {
    X = atan2(mi[1], mi[5]);
    Z = 0;
  }

  X *= rad2deg;
  Y *= rad2deg;
  Z *= rad2deg;

  SETFLOAT(alist+0, ScaleX);
  SETFLOAT(alist+1, ScaleY);
  SETFLOAT(alist+2, ScaleZ);

  SETFLOAT(alist+3, X);
  SETFLOAT(alist+4, Y);
  SETFLOAT(alist+5, Z);

  SETFLOAT(alist+6, mi[12]); 
  SETFLOAT(alist+7, mi[13]);
  SETFLOAT(alist+8, mi[14]);

  SETFLOAT(alist+9, shearYX); 
  SETFLOAT(alist+10, shearZX);
  SETFLOAT(alist+11, shearZY);

  outlet_list (m_outletPosition, &s_list, 3, alist+6);
  outlet_list (m_outletScale, &s_list, 3, alist+0);
  outlet_list (m_outletShear, &s_list, 3, alist+9);
  outlet_list (m_outletRotation, &s_list, 3, alist+3);
}

/////////////////////////////////////////////////////////
// static member functions
//
void gemlist_info :: obj_setupCallback(t_class *classPtr) {
}
