////////////////////////////////////////////////////////
//
// pix_hit
//
// hit test over user defined hit_areas...
// 
// Author: Davide Morelli
// http://ww.davidemorelli.it
//
/////////////////////////////////////////////////////////

#ifndef _INCLUDE__GEM_PIXES_PIX_HIT_H_
#define _INCLUDE__GEM_PIXES_PIX_HIT_H_

#include "Base/GemPixObj.h"

#include <stdio.h>

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  pix_hit

  KEYWORDS
  pix
    
  DESCRIPTION

  bangs if there is a minimum number of pixels over a given threshold in a given rectangle
   
  -----------------------------------------------------------------*/
#define NUM_hit_areas 256
#define DEF_THRESHOLD 127
#define DEF_MINIMUM 1
#define DEF_MIN_DISTANCE 0.01

enum areaTypes
{
  rectangle,
  circle,
  line
};

// TODO: this should an union!
typedef struct _hitarea
{
  areaTypes type;
  float x;
  float y;
  float width; // rectangle width or circle radius or second point x
  float height; // rectangle height or second point y
} t_hitarea;

class GEM_EXTERN pix_hit : public GemPixObj
{
  CPPEXTERN_HEADER(pix_hit, GemPixObj);

    public:

  //////////
  // Constructor
  pix_hit(void);
    	
 protected:
    	
  //////////
  // Destructor
  virtual ~pix_hit();

  //////////
  // Do the processing
  virtual void 	processImage(imageStruct &image);

  void threshold(float thresh);
  void minimum(int min);
  void set_min_distance(float min);
  void set_show(int val);
  void del(float min);
  void createRectangle(int n, float x, float y, float w, float h);
  void createCircle(int n, float x, float y, float r);
  void createLine(int n, float x1, float y1, float x2, float y2);
  void move(int n, float x, float y, float w, float h);
  void del(int n);
  unsigned char getGreyValue(GLenum format, unsigned char *data);


  t_hitarea hit_areas[NUM_hit_areas];
  bool area_active[NUM_hit_areas];
  float buffer[NUM_hit_areas];

  unsigned char minimum_threshold;
  short int minimum_pixels;
  float min_distance;
  bool show;

  t_outlet    	*m_hits;
    
 private:
    
  static void	thresholdCallback(void *data, t_floatarg thresh);
  static void	minimumCallback(void *data, t_floatarg min);
  static void	min_distanceCallback(void *data, t_floatarg min);
  static void	deleteCallback(void *data, t_floatarg id);
  static void	createRectangleCallback(void *data, t_symbol *sl, int argc, t_atom *argv);
  static void	createCircleCallback(void *data, t_symbol *sl, int argc, t_atom *argv);
  static void	createLineCallback(void *data, t_symbol *sl, int argc, t_atom *argv);
  static void	moveCallback(void *data, t_symbol *sl, int argc, t_atom *argv);
  static void	showCallback(void *data, t_floatarg val);

};

#endif	// for header file

