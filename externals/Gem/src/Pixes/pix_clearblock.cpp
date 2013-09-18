////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-1998 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "pix_clearblock.h"

#include "Gem/Cache.h"
#include "Gem/State.h"


CPPEXTERN_NEW(pix_clearblock);

/////////////////////////////////////////////////////////
//
// pix_clearblock
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_clearblock :: pix_clearblock()
                : m_oldcache(NULL)
{ }

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_clearblock :: ~pix_clearblock()
{
    if (m_oldcache) stopRendering();
}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void pix_clearblock :: render(GemState *state)
{
  // the cache and image should have been set
  if (!m_oldcache || m_oldcache->m_magic!=GEMCACHE_MAGIC){
    // our cache has not been set or has been destroyed
    startRendering(); // i'm not sure, whether this breaks the default behaviour
    return;
  }
  state->set(GemState::_PIX, static_cast<pixBlock*>(NULL));
}

/////////////////////////////////////////////////////////
// postrender
//
/////////////////////////////////////////////////////////
void pix_clearblock :: postrender(GemState *state)
{
#ifdef __GNUC__
# warning shant we reset to the original image?
#endif
    state->set(GemState::_PIX, static_cast<pixBlock*>(NULL)); 
}

/////////////////////////////////////////////////////////
// startRendering
//
/////////////////////////////////////////////////////////
void pix_clearblock :: startRendering()
{
  if (m_cache && m_cache->m_magic!=GEMCACHE_MAGIC)
    m_cache=NULL;

  m_oldcache = m_cache;

  if(m_oldcache)
    m_cache = new GemCache(m_oldcache->m_parent);
}

/////////////////////////////////////////////////////////
// stopRendering
//
/////////////////////////////////////////////////////////
void pix_clearblock :: stopRendering()
{
    if (m_cache) delete m_cache;
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_clearblock :: obj_setupCallback(t_class *)
{ }
