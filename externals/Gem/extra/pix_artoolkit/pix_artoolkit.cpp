////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// shigeyuki@pop01.odn.ne.jp
//
// Implementation file
//
//    Copyright (c) 2005-2006 Shigeyuki Hirai.
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////
#include "pix_artoolkit.h"
#include <stdlib.h>
#include <string.h>

CPPEXTERN_NEW(pix_artoolkit)

#ifndef M_PI
# ifdef PI
#  define M_PI PI
# else
#  define M_PI 3.14159265
# endif
#endif

/////////////////////////////////////////////////////////
//
// pix_artoolkit
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_artoolkit :: pix_artoolkit()
#ifdef HAVE_ARTOOLKIT
  :
# ifdef GEM4MAX
GemPixObj(1), 
# endif
m_xsize(320), m_ysize(240), m_thresh(100), 
m_count(0), m_outputMode(OUTPUT_QUATERNION), m_continuous(true), 
m_cparam_name(NULL)
#endif
{
#ifdef GEM4MAX
  m_outMarker = ::listout(this->x_obj);
  m_out1      = ::outlet_new(this->x_obj, 0);
#else
  m_outMarker = outlet_new(this->x_obj, 0);
#endif


#ifdef HAVE_ARTOOLKIT
  static bool firsttime = true;
  if(firsttime) {
    post("ARToolKit support by Shigeyuki Hirai");
    firsttime = false;
  }

  for (int i=0; i<MAX_OBJECTS; i++) {
    m_object[i].patt_name = NULL;
    m_object[i].patt_id = -1;
    m_object[i].model_id = i;
    m_object[i].visible = 0;
    m_object[i].contFlag = false;
    m_object[i].width = 80.0;
    m_object[i].center[0] = 0.0;
    m_object[i].center[1] = 0.0;
  }
# if defined(__APPLE__) || defined(GEM4MAX)
  m_image.setCsizeByFormat(GL_BGRA_EXT);
# else
  m_image.setCsizeByFormat(GL_RGBA);
# endif
#else
  error("compiled without ARToolKit support!");
#endif
}

//////////////////////////////////////////////////////
// Destructor
//
//////////////////////////////////////////////////////
pix_artoolkit :: ~pix_artoolkit()
{
}

#ifdef HAVE_ARTOOLKIT
//////////////////////////////////////////////////////
// processRGBAImage
//
//////////////////////////////////////////////////////
void pix_artoolkit :: processRGBAImage(imageStruct &image)
{
  //	double    		gl_para[16];
  ARMarkerInfo    *marker_info;
  int             marker_num;
  int             i, j, k;

  if (m_xsize != image.xsize || m_ysize != image.ysize) {
    m_xsize = image.xsize;
    m_ysize = image.ysize;
    ::arParamChangeSize(&wparam, m_xsize, m_ysize, &m_cparam);
    ::arInitCparam(&m_cparam);
    ::arParamDisp(&m_cparam);
    //    ::argInit(&m_cparam, 1.0, 0, 0, 0, 0);
    post("ARToolKit: image size was changed (%d, %d)", m_xsize, m_ysize);
  }
  
  if (::arDetectMarker(image.data, m_thresh, &marker_info, &marker_num) < 0) {
    error("ARToolKit: arDetectMarker() error");
    return;
  }
  
  for (i=0; i<MAX_OBJECTS; i++) {
    if (m_object[i].patt_id == -1) continue;
    for (k = -1, j = 0; j < marker_num; j++) {
      if (m_object[i].patt_id == marker_info[j].id) {
        if (k == -1) k = j;
        else if (marker_info[k].cf < marker_info[j].cf) k = j;
      }
      logpost(NULL, 7, "ID: %d (%f, %f)", 
             marker_info[j].id, marker_info[j].pos[0], marker_info[j].pos[1]);
    }
    m_object[i].visible = k;
    if (k == -1) {
      m_object[i].contFlag = false;
    } else if (k >= 0) {
      // get the transformation between the marker and the real camera
      
      if (m_continuous == 0 || m_object[i].contFlag == 0) {
        ::arGetTransMat(&marker_info[k], 
                        m_object[i].center, 
                        m_object[i].width, 
                        m_object[i].trans);
      } else {
        ::arGetTransMatCont(&marker_info[k], 
                            m_object[i].trans, 
                            m_object[i].center, 
                            m_object[i].width, 
                            m_object[i].trans);
      }
      m_object[i].contFlag = true;

      logpost(NULL, 7, "ID(%d), pos(%f, %f), center(%f, %f)", 
             i + 1, 
             marker_info[k].pos[0], marker_info[k].pos[1],
             m_object[i].center[0], m_object[i].center[1]);
      double q[4], p[3], x, y, z, w;
      ::arUtilMat2QuatPos(m_object[i].trans, q, p);

#define NUM_PARAM 8	//ID, positions[3], quaternion[4]
      t_atom ap[MAX_OBJECTS * NUM_PARAM];
      
#ifdef GEM4MAX
      SETLONG(&ap[NUM_PARAM * i + 0], i + 1);	//ID
#else
      SETFLOAT(&ap[NUM_PARAM * i + 0], i + 1); //ID
#endif
      
      SETFLOAT(&ap[NUM_PARAM * i + 1], p[0]);	//positoin.x
      SETFLOAT(&ap[NUM_PARAM * i + 2], p[1]);	//position.y
      SETFLOAT(&ap[NUM_PARAM * i + 3], p[2]);	//position.z
      switch (m_outputMode) {
      case OUTPUT_QUATERNION:
        SETFLOAT(&ap[NUM_PARAM * i + 4], q[0]);	//quaternion.s
        SETFLOAT(&ap[NUM_PARAM * i + 5], q[1]);	//quaternion.t
        SETFLOAT(&ap[NUM_PARAM * i + 6], q[2]);	//quaternion.u
        w = acos(q[3]) * 180. / M_PI;	// 2 * acos(q[3]) * 180. / PI
        SETFLOAT(&ap[NUM_PARAM * i + 7], w);	//
        break;
      case OUTPUT_NORMAL:
        x = m_object[i].trans[0][2];
        y = m_object[i].trans[1][2];
        z = m_object[i].trans[2][2];
        SETFLOAT(&ap[NUM_PARAM * i + 4], x);	//normal.x
        SETFLOAT(&ap[NUM_PARAM * i + 5], y);	//normal.y
        SETFLOAT(&ap[NUM_PARAM * i + 6], z);	//normal.z
        SETFLOAT(&ap[NUM_PARAM * i + 7], 1.0);	//
        break;
      case OUTPUT_EULER:
        break;
      default:
        //				error("pix_artoolkit: illegal output mode");
        break;
      }
      ::outlet_list(this->m_outMarker, 0, NUM_PARAM, &ap[NUM_PARAM * i]);
    }
  }
}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_artoolkit :: processGrayImage(imageStruct &image)
{
  error("requires RGBA images"); return;
  m_image.xsize = image.xsize;
  m_image.ysize = image.ysize;
  m_image.fromGray(image.data);
  image.data   = m_image.data;
  image.notowned = 0;
  image.setCsizeByFormat(m_image.format);
}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_artoolkit :: processYUVImage(imageStruct &image)
{
  error("requires RGBA images"); return;
  m_image.xsize = image.xsize;
  m_image.ysize = image.ysize;
  m_image.fromUYVY(image.data);
  image.data   = m_image.data;
  image.notowned = 0;
  image.setCsizeByFormat(m_image.format);
}

/////////////////////////////////////////////////////////
// loadmarkerMess
//
/////////////////////////////////////////////////////////
void pix_artoolkit :: loadmarkerMess(t_int n, t_symbol *marker_filename)
{
  if  (n > MAX_OBJECTS || n <= 0) {
    error("can't set marker number %d", n);
    return;
  }
  if ((m_object[n - 1].patt_id = ::arLoadPatt(marker_filename->s_name)) < 0) {
    error("ARToolKit: pattern load error (%d) !!", m_object[n - 1].patt_id);
    return;
  }
  post("loaded a marker file (%s) as %d...", marker_filename->s_name, n);
  m_object[n].patt_name = marker_filename;
}

/////////////////////////////////////////////////////////
// objectSizeMess
//
/////////////////////////////////////////////////////////
void pix_artoolkit :: objectSizeMess(t_int n, t_floatarg f)
{
  if (n > MAX_OBJECTS || n <= 0) {
    error("can't set marker number %d", n);
    return;
  }
  m_object[n].width = f;
}

/////////////////////////////////////////////////////////
// loadcparaMess
//
/////////////////////////////////////////////////////////
void pix_artoolkit :: loadcparaMess(t_symbol *cparam_filename)
{
  int err;
  if ((err = ::arParamLoad(cparam_filename->s_name, 1, &wparam)) < 0) {
    error("ARToolKit: Camera parameter load error (%d) !!", err);	//\n
    return;
  }
  post("loaded camera parameter file:(%s)", cparam_filename->s_name);
  ::arParamChangeSize( &wparam, m_xsize, m_ysize, &m_cparam );
  ::arInitCparam( &m_cparam );
  ::arParamDisp( &m_cparam );
  //  ::argInit(&m_cparam, 1.0, 0, 0, 0, 0);
  m_cparam_name = cparam_filename;
}

/////////////////////////////////////////////////////////
// outputmodeMess
//
/////////////////////////////////////////////////////////
void pix_artoolkit :: outputmodeMess(t_int outputMode)
{
  switch (outputMode) {
  case OUTPUT_QUATERNION:
    m_outputMode = OUTPUT_QUATERNION;
    post("ARToolKit: OutputMode->Quartanion");
    break;
  case OUTPUT_NORMAL:
    m_outputMode = OUTPUT_NORMAL;
    post("ARToolKit: OutputMode->Normal");
    break;
  default:
    error("ARToolKit: illegal outputmode");
  }
}

/////////////////////////////////////////////////////////
// continuousMess
//
/////////////////////////////////////////////////////////
void pix_artoolkit :: continuousMess(t_int continuousMode)
{
  if (continuousMode) {
    m_continuous = true;
    post("ARToolKit: Continuous mode: Using arGetTransMatCont.");
  } else {
    m_continuous = false;
    post("ARToolKit: One shot mode: Using arGetTransMat.");
  }
}

/////////////////////////////////////////////////////////
// thresholdMess
//
/////////////////////////////////////////////////////////
void pix_artoolkit :: thresholdMess(t_int threshold)
{
  m_thresh = threshold;
}

/////////////////////////////////////////////////////////
// reset
//
/////////////////////////////////////////////////////////
void pix_artoolkit :: resetMess()
{
  for (int i=0; i<MAX_OBJECTS; i++) {
    m_object[i].patt_name = NULL;
    m_object[i].patt_id = -1;
    m_object[i].model_id = i;
    m_object[i].visible = 0;
    m_object[i].contFlag = false;
    m_object[i].width = 80.0;
    m_object[i].center[0] = 0.0;
    m_object[i].center[1] = 0.0;
  }
}

/////////////////////////////////////////////////////////
// init
//
/////////////////////////////////////////////////////////
void pix_artoolkit :: init()
{
}


/////////////////////////////////////////////////////////
// clearMess
//
/////////////////////////////////////////////////////////
void pix_artoolkit :: clearMess()
{
}
#endif /* HAVE_ARTOOLKIT */

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
#ifdef GEM4MAX
void pix_artoolkit :: obj_setupCallback(void *)
{
  ::addmess((method)&pix_artoolkit::loadmarkerMessCallback, "loadmarker", A_LONG, A_SYMBOL, 0);
  ::addmess((method)&pix_artoolkit::objectSizeMessCallback, "objectsize", A_LONG, A_FLOAT, 0);
  ::addmess((method)&pix_artoolkit::outputmodeMessCallback, "outputmode", A_LONG, 0);
  ::addmess((method)&pix_artoolkit::continuousMessCallback, "continuous", A_LONG, 0);
  ::addmess((method)&pix_artoolkit::thresholdMessCallback, "threshold", A_LONG, 0);
  ::addmess((method)&pix_artoolkit::loadcparaMessCallback, "loadcpara", A_DEFSYM, 0);
  ::addmess((method)&pix_artoolkit::resetMessCallback, "reset", 0);
  ::addmess((method)&pix_artoolkit::clearMessCallback, "clear", 0);
}
void pix_artoolkit :: loadmarkerMessCallback(void *data, t_int n, t_symbol *filename)
{
#ifdef HAVE_ARTOOLKIT
  GetMyClass(data)->loadmarkerMess(n, filename);
#endif /* HAVE_ARTOOLKIT */
}
void pix_artoolkit :: objectSizeMessCallback(void *data, t_int n, t_floatarg f)
{
# ifdef HAVE_ARTOOLKIT
  GetMyClass(data)->objectSizeMess(n, f);
# endif /* HAVE_ARTOOLKIT */
}
void pix_artoolkit :: outputmodeMessCallback(void *data, t_int outputMode)
{
# ifdef HAVE_ARTOOLKIT
  GetMyClass(data)->outputmodeMess((t_int)outputMode);
# endif /* HAVE_ARTOOLKIT */
}
void pix_artoolkit :: continuousMessCallback(void *data, t_int continuousMode)
{
# ifdef HAVE_ARTOOLKIT
  GetMyClass(data)->continuousMess((t_int)continuousMode);
# endif /* HAVE_ARTOOLKIT */
}
void pix_artoolkit :: thresholdMessCallback(void *data, t_int threshold)
{
# ifdef HAVE_ARTOOLKIT
  GetMyClass(data)->thresholdMess((t_int)threshold);	
# endif /* HAVE_ARTOOLKIT */
}
#else
void pix_artoolkit :: obj_setupCallback(t_class *classPtr)
{
#ifndef GEM_INTERNAL
  ::post("pix_artoolkit: (c) 2005-2006 Shigeyuki Hirai");
#endif
  class_addmethod(classPtr, (t_method)&pix_artoolkit::loadmarkerMessCallback, gensym("loadmarker"), A_GIMME, A_NULL);
  class_addmethod(classPtr, (t_method)&pix_artoolkit::objectSizeMessCallback, gensym("objectsize"), A_FLOAT, A_FLOAT, A_NULL);
  class_addmethod(classPtr, (t_method)&pix_artoolkit::outputmodeMessCallback, gensym("outputmode"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, (t_method)&pix_artoolkit::continuousMessCallback, gensym("continuous"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, (t_method)&pix_artoolkit::thresholdMessCallback, gensym("threshold"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, (t_method)&pix_artoolkit::loadcparaMessCallback, gensym("loadcpara"), A_SYMBOL, A_NULL);
  class_addmethod(classPtr, (t_method)&pix_artoolkit::clearMessCallback, gensym("clear"), A_NULL);
}
void pix_artoolkit :: loadmarkerMessCallback(void *data, t_symbol*, int argc, t_atom*argv)
{
# ifdef HAVE_ARTOOLKIT
  if(argc==2)
    GetMyClass(data)->loadmarkerMess(atom_getint(argv), atom_getsymbol(argv+1));
  else
    GetMyClass(data)->error("invalide arguments to loadmarker <#id> <filename>");

# endif /* HAVE_ARTOOLKIT */
}
void pix_artoolkit :: objectSizeMessCallback(void *data, t_floatarg n, t_floatarg f)
{
# ifdef HAVE_ARTOOLKIT
  GetMyClass(data)->objectSizeMess((t_int)n, f);
# endif /* HAVE_ARTOOLKIT */
}
void pix_artoolkit :: outputmodeMessCallback(void *data, t_floatarg outputMode)
{
# ifdef HAVE_ARTOOLKIT
  GetMyClass(data)->outputmodeMess((t_int)outputMode);
# endif /* HAVE_ARTOOLKIT */
}
void pix_artoolkit :: continuousMessCallback(void *data, t_floatarg mode)
{
# ifdef HAVE_ARTOOLKIT
  GetMyClass(data)->continuousMess((t_int)mode);
# endif /* HAVE_ARTOOLKIT */
}
void pix_artoolkit :: thresholdMessCallback(void *data, t_floatarg threshold)
{
# ifdef HAVE_ARTOOLKIT
  GetMyClass(data)->thresholdMess((t_int)threshold);
# endif /* HAVE_ARTOOLKIT */
}
#endif
void pix_artoolkit :: loadcparaMessCallback(void *data, t_symbol *filename)
{
# ifdef HAVE_ARTOOLKIT
  GetMyClass(data)->loadcparaMess(filename);
# endif /* HAVE_ARTOOLKIT */
}
void pix_artoolkit :: resetMessCallback(void *data)
{
# ifdef HAVE_ARTOOLKIT
  GetMyClass(data)->resetMess();
# endif /* HAVE_ARTOOLKIT */
}
void pix_artoolkit :: clearMessCallback(void *data)
{
# ifdef HAVE_ARTOOLKIT
  GetMyClass(data)->clearMess();
# endif /* HAVE_ARTOOLKIT */
}
