////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// mark@danks.org
//
//    Copyright (c) 1997-1999 Mark Danks.
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "LICENSE.txt" in this distribution.
//
/////////////////////////////////////////////////////////

#include "m_pd.h"
#include <math.h>

static inline float FLOAT_CLAMP(float x) { return((x > 1.f) ? 1.f : ( (x < 0.f) ? 0.f : x)); }

/////////////////////////////////////////////////////////
//
// hsv2rgb
//
/////////////////////////////////////////////////////////
// instance structure
static t_class *hsv2rgb_class;

typedef struct _hsv2rgb
{
  t_object    x_obj;	        // obligatory object header
  t_outlet    *t_out1;	    // the outlet
}t_hsv2rgb;

static void hsv2rgb_float(t_hsv2rgb *x, t_floatarg h, t_floatarg s, t_floatarg v)
{
  t_atom argv[3];
  float r=0, g=0, b=0;

  h = FLOAT_CLAMP(h);
  s = FLOAT_CLAMP(s);
  v = FLOAT_CLAMP(v);

  // convert hue to degrees
  h *= 360.f;
	
  if (s == 0.0)		// black and white
    {
      r = g = b = v;
    }
  else
    {
      if (h == 360.0)			// 360 == 0 degrees
        h = 0.0f;
      h /= 60.0f;				// hue is now [0, 6]
      {
        int i = (int)floor(h);
        float f = h - i;		// f is the fractional part of h
        float p = v * (1 - s);
        float q = v * (1 - s * f);
        float t = v * (1 - s * (1 - f));
        
        switch (i)
          {
          case 0:
            r = v;
            g = t;
            b = p;
            break;
          case 1:
            r = q;
            g = v;
            b = p;
            break;
          case 2:
            r = p;
            g = v;
            b = t;
            break;
          case 3:
            r = p;
            g = q;
          b = v;
          break;
          case 4:
            r = t;
            g = p;
            b = v;
            break;
          case 5:
            r = v;
            g = p;
            b = q;
            break;
          }
      }
    }
  SETFLOAT(&argv[0], r);
  SETFLOAT(&argv[1], g);
  SETFLOAT(&argv[2], b);
  outlet_list(x->t_out1, &s_list, 3, argv);
}

static void hsv2rgb_list(t_hsv2rgb *x, t_symbol *sym, int argc, t_atom *argv)
{
  if (argc >= 3)
    {
      float h = atom_getfloat(&argv[0]);
      float s = atom_getfloat(&argv[1]);
      float v = atom_getfloat(&argv[2]);
      hsv2rgb_float(x, h, s, v);
    }
}

static void *hsv2rgb_new(void)	// init vals in struct
{
  t_hsv2rgb *x = (t_hsv2rgb *)pd_new(hsv2rgb_class);
  x->t_out1 = outlet_new(&x->x_obj, 0);
  return (x);
}

void hsv2rgb_setup(void)
{
  hsv2rgb_class = class_new(gensym("hsv2rgb"), (t_newmethod)hsv2rgb_new, 0,
                            sizeof(t_hsv2rgb), CLASS_DEFAULT, A_NULL);
  class_addlist(hsv2rgb_class, (t_method)hsv2rgb_list);
}

