////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 2005 IOhannes m zmoelnig. forum::für::umläute. IEM
//    based on reacTIVision by M.Kaltenbrunner and R.Bencina
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "pix_fiducialtrack.h"

#ifdef _WIN32
# ifdef _MSC_VER  /* This is only for Microsoft's compiler, not cygwin, e.g. */
#  define snprintf _snprintf
#  define vsnprintf _vsnprintf
#  define close _close
# endif
# include <io.h>
#else
# include <unistd.h>
#endif

#include <stdio.h>
#include <string.h>

CPPEXTERN_NEW_WITH_ONE_ARG(pix_fiducialtrack,  t_symbol *, A_DEFSYM);

/////////////////////////////////////////////////////////
//
// pix_fiducialtrack
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_fiducialtrack :: pix_fiducialtrack(t_symbol*s) :
  m_width(-1), m_height(-1), initialized(false)
{
  static bool first_time=true;
  if(first_time){
    first_time=false;
    post("\tbased on fidtrack-library (c) R.Bencina\n\tbased on reacTIVision (c) M.Kaltenbrunner, R.Bencina\n\tsee http://www.iua.upf.es/mtg/reacTable/");
  }
  m_infoOut = outlet_new(this->x_obj, &s_list);

  /* so terminate_treeidmap() knows that this is kind of uninitialized */
  memset(&fidtrackerx, 0, sizeof(fidtrackerx));
  memset(&treeidmap,   0, sizeof(treeidmap));

  if((NULL!=s) && (&s_!=s) && (NULL!=s->s_name)){
    treeMess(s);
  } else {
    treeMess(gensym("all.trees"));
  }
}

////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_fiducialtrack :: ~pix_fiducialtrack()
{
  deinit_segmenter();
  outlet_free(m_infoOut);
}
void pix_fiducialtrack::deinit_segmenter() {
  if (initialized){
    terminate_segmenter(&segmenter);
  }
  initialized=false;
}


/////////////////////////////////////////////////////////
// processGrayImage
//
/////////////////////////////////////////////////////////
void pix_fiducialtrack :: processGrayImage(imageStruct &image)
{
  if(image.xsize!=m_width || image.ysize!=m_height)
    deinit_segmenter();

  m_width =image.xsize;
  m_height=image.ysize;

  if(!initialized){
    initialize_segmenter( &segmenter, m_width, m_height, treeidmap.max_adjacencies );
    initialized=true;
  }

  step_segmenter( &segmenter, image.data, m_width, m_height );
  int count = find_fiducialsX( fiducials, MAX_FIDUCIAL_COUNT,  
                               &fidtrackerx , 
                               &segmenter, 
                               m_width, m_height);

  int i;
  for(i=0;i< count;i++) {
    if(fiducials[i].id!=INVALID_FIDUCIAL_ID){
      SETFLOAT((m_outlist+0), (fiducials[i].id));         // id (as in treeidmap)
      SETFLOAT((m_outlist+1), (fiducials[i].x/m_width));  // x (normalized)
      SETFLOAT((m_outlist+2), (fiducials[i].y/m_height)); // y (normalized)
      SETFLOAT((m_outlist+3), (fiducials[i].angle));      // phi (radiant)
      outlet_list(m_infoOut, gensym("list"), 4, m_outlist);
    }
  }
}

/////////////////////////////////////////////////////////
// read a treeidmap from file
//
/////////////////////////////////////////////////////////
void pix_fiducialtrack :: treeMess(t_symbol*s)
{
  if(NULL==s || NULL==s->s_name || &s_==s)return;

  std::string fn = findFile(s->s_name);
  snprintf(m_treefile, MAXPDSTRING, "%s", fn.c_str());

  terminate_fidtrackerX(&fidtrackerx);
  terminate_treeidmap  (&treeidmap);
  deinit_segmenter();
  
  initialize_treeidmap_from_file( &treeidmap, m_treefile );
  initialize_fidtrackerX( &fidtrackerx, &treeidmap, NULL);
  if(treeidmap.max_adjacencies<=0){
    error("could not load TreeIdMap from '%s'", s->s_name);
  }
}
void pix_fiducialtrack :: addMess(t_symbol*s)
{
  error("on-the-fly adding of fiducials not yet implemented!");
#if 0
  std::string fn = findFile(s->s_name);
  m_treefile=fn.c_str();

  initialize_treeidmap_from_file( &treeidmap, m_treefile );
  initialize_fidtrackerX( &fidtrackerx, &treeidmap, NULL);
  deinit_segmenter();
#endif
}
/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_fiducialtrack :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_fiducialtrack::treeMessCallback),
                  gensym("open"), A_SYMBOL, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_fiducialtrack::addMessCallback),
                  gensym("add"), A_SYMBOL, A_NULL);
}
void pix_fiducialtrack :: treeMessCallback(void *data, t_symbol* filename)
{
  GetMyClass(data)->treeMess(filename);
}
void pix_fiducialtrack :: addMessCallback(void *data, t_symbol*s)
{
  GetMyClass(data)->addMess(s);
}
