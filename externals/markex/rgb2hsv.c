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

static inline float TRI_MAX(float v1, float v2, float v3)
{ if (v1 > v2 && v1 > v3) return(v1);
  if (v2 > v3) return(v2);
  return(v3);
}

static inline float TRI_MIN(float v1, float v2, float v3)
{ if (v1 < v2 && v1 < v3) return(v1);
  if (v2 < v3) return(v2);
  return(v3);
}

/////////////////////////////////////////////////////////
//
// rgb2hsv
//
/////////////////////////////////////////////////////////
// instance structure
static t_class *rgb2hsv_class;

typedef struct _rgb2hsv
{
  t_object    x_obj;	        /* obligatory object header */
  t_outlet    *t_out1;	    /* the outlet */
}t_rgb2hsv;

static void rgb2hsv_float(t_rgb2hsv *x, t_floatarg r, t_floatarg g, t_floatarg b)
{
  t_atom argv[3];

  float h=0, s, v;

  r = FLOAT_CLAMP(r);
  g = FLOAT_CLAMP(g);
  b = FLOAT_CLAMP(b);
  float max = TRI_MAX(r, g, b);
  float min = TRI_MIN(r, g, b);
  v = max;		// the value

  // calculate saturation
  if (max != 0.0f)
    s = (max - min) / max;
  else
    s = 0.0f;

  if (s == 0.0f)
    {
      h = 0.0f;		// hue is undefined if no saturation	
    }
  // chromatic case - calculate hue
  else
    {
      float delta = max - min;
      if (r == max)						// between magenta and cyan
        h = (g - b) / delta;
      else if (g == max)					// between yellow and magenta
        h = 2.0f + (b - r) / delta;
      else if (b == max)					// between cyan and yellow
        h = 4.0f + (r - g) / delta;

      // convert hue to degrees
      h *= 60.0f;
      // make sure hue is nonnegative
      if (h < 0.0)
        h += 360.f;
      // normalize hue
      h /= 360.f;
    }

  SETFLOAT(&argv[0], h);
  SETFLOAT(&argv[1], s);
  SETFLOAT(&argv[2], v);

  outlet_list(x->t_out1, &s_list, 3, argv);
}

static void rgb2hsv_list(t_rgb2hsv *x, t_symbol *s, int argc, t_atom *argv)
{
  if (argc >= 3)
    {
      float r = atom_getfloat(&argv[0]);
      float g = atom_getfloat(&argv[1]);
      float b = atom_getfloat(&argv[2]);
      rgb2hsv_float(x, r, g, b);
    }
}

static void *rgb2hsv_new(void)	// init vals in struct
{
  t_rgb2hsv *x = (t_rgb2hsv *)pd_new(rgb2hsv_class);
  x->t_out1 = outlet_new(&x->x_obj, 0);
  return (x);
}

void rgb2hsv_setup(void)
{
  rgb2hsv_class = class_new(gensym("rgb2hsv"), (t_newmethod)rgb2hsv_new, 0,
                            sizeof(t_rgb2hsv), CLASS_DEFAULT, A_NULL);

  class_addlist(rgb2hsv_class, (t_method)rgb2hsv_list);
}
