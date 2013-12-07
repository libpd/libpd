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

#include "pix_multiimage.h"

#include <stdio.h>

#include "Gem/Cache.h"
#include "Gem/State.h"
#include "Gem/ImageIO.h"

CPPEXTERN_NEW_WITH_FOUR_ARGS(pix_multiimage, t_symbol *, A_DEFSYM, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

pix_multiimage::multiImageCache *pix_multiimage::s_imageCache = NULL;

/////////////////////////////////////////////////////////
//
// pix_multiimage
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_multiimage :: pix_multiimage(t_symbol *filename, t_floatarg baseImage, t_floatarg topImage, t_floatarg skipRate)
    	    	: m_numImages(0), m_curImage(-1), m_loadedCache(NULL)
{
  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("img_num"));
  m_pixBlock.image = m_imageStruct;

  // make sure that there are some characters
  if (filename->s_name[0]) { 
    if (skipRate == 0)  {
      if (topImage == 0)
	openMess(filename, 0, (int)baseImage, 1);
      else
	openMess(filename, (int)baseImage, (int)topImage, 1);
    }else openMess(filename, (int)baseImage, (int)topImage, (int)skipRate);
  }
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_multiimage :: ~pix_multiimage()
{
    cleanImages();
}

/////////////////////////////////////////////////////////
// openMess
//
/////////////////////////////////////////////////////////
void pix_multiimage :: openMess(t_symbol *filename, int baseImage, int topImage, int skipRate)
{
    cleanImages();

    if (m_cache&&m_cache->m_magic!=GEMCACHE_MAGIC)
      m_cache=NULL;   
    /*
    if (!topImage)
    {
    	error("requires an int for number of images");
        return;
    }
    */
    if (baseImage > topImage)
    {
        error("Top range less than base image");
        return;
    }
    if (skipRate < 1) skipRate = 1;

    // have we already loaded the image?
    multiImageCache *cache = s_imageCache;
    int found = 0;
    while (!found && cache)
    {
        if (baseImage == cache->baseImage &&
            topImage == cache->topImage &&
            skipRate == cache->skipRate &&
            !strcmp(filename->s_name, cache->imageName)) found = 1;
        else cache = cache->next;
    }
    
    // yep, we have it
    if (found)
    {
        m_loadedCache = cache;
        m_loadedCache->refCount++;
        m_curImage = 0;
        m_numImages = m_loadedCache->numImages;
        m_loadedCache->images[m_curImage]->copy2Image(&m_pixBlock.image);
        m_pixBlock.newimage = 1;
        if (m_cache) m_cache->resendImage = 1;
        return;
    }

    // nope, so create the new cache
    // find the * in the filename    
    char preName[256];
    char postName[256];
    
    int i = 0;
    char *strPtr = filename->s_name;
    while (strPtr[i] && strPtr[i] != '*')
    {
    	preName[i] = strPtr[i];
    	i++;
    }
    
    if (!strPtr[i])
    {
    	error("Unable to find * in file name");
    	return;
    }

    preName[i] = '\0';    
    strcpy(postName, &(strPtr[i+1]));
    
    // need to figure out how many filenames there are to load
    m_numImages = (topImage + 1 - baseImage) / skipRate;

    // create the new cache
    multiImageCache *newCache = new multiImageCache(filename->s_name);
    newCache->images = new imageStruct*[m_numImages];
    newCache->numImages = m_numImages;
    newCache->baseImage = baseImage;
    newCache->topImage = topImage;
    newCache->skipRate = skipRate;

    int realNum = baseImage;
    char bufName[MAXPDSTRING];
    canvas_makefilename(const_cast<t_canvas*>(getCanvas()), preName, bufName, MAXPDSTRING);

    // allocate texture bindings for OpenGL
    newCache->textBind = new unsigned int[m_numImages];

    for (i = 0; i < m_numImages; i++, realNum += skipRate)
    {
        char newName[MAXPDSTRING];
	    sprintf(newName, "%s%d%s", bufName, realNum, postName);
		newCache->textBind[i] = 0;
        if ( !(newCache->images[i] = image2mem(newName)) )
	    {
            // a load failed, blow away the cache
            newCache->numImages = i;
            delete newCache;
    	    m_numImages = 0;
	        return;
	    }
    }

    m_curImage = 0;
    newCache->images[m_curImage]->copy2Image(&m_pixBlock.image);
    m_pixBlock.newimage = 1;
    if (m_cache) m_cache->resendImage = 1;

    m_loadedCache = newCache;
    newCache->refCount++;

    // insert the cache at the end of the linked list
    multiImageCache *ptr = s_imageCache;
    
    if (!ptr) s_imageCache = newCache;
    else
    {
        while(ptr->next) ptr = ptr->next;
        ptr->next = newCache;
    }

    post("loaded images: %s %s from %d to %d skipping %d",
                bufName, postName, baseImage, topImage, skipRate);
}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void pix_multiimage :: render(GemState *state)
{
    // if we don't have an image, just return
    if (!m_numImages) return;
    
    // do we need to reload the image?    
    if (m_cache->resendImage)
    {
      m_loadedCache->images[m_curImage]->refreshImage(&m_pixBlock.image);
    	m_pixBlock.newimage = 1;
    	m_cache->resendImage = 0;
    }
    
    state->set(GemState::_PIX, &m_pixBlock);
}

/////////////////////////////////////////////////////////
// postrender
//
/////////////////////////////////////////////////////////
void pix_multiimage :: postrender(GemState *state)
{
  m_pixBlock.newimage = 0;
  state->set(GemState::_PIX, static_cast<pixBlock*>(NULL));
}

/////////////////////////////////////////////////////////
// startRendering
//
/////////////////////////////////////////////////////////
void pix_multiimage :: startRendering()
{
    if (!m_numImages) return;

    m_loadedCache->images[m_curImage]->refreshImage(&m_pixBlock.image);
    m_pixBlock.newimage = 1;
}

/////////////////////////////////////////////////////////
// changeImage
//
/////////////////////////////////////////////////////////
void pix_multiimage :: changeImage(int imgNum)
{
  if (m_cache&&m_cache->m_magic!=GEMCACHE_MAGIC)
    m_cache=NULL;

    if (imgNum >= m_numImages)
    {
    	error("selection number too high: %d (max num is %d)", imgNum, m_numImages);
    	return;
    }
    else if (imgNum < 0)
    {
        error("selection number must be > 0");
        return;
    }
    m_curImage = imgNum;
    if (m_cache) m_cache->resendImage = 1;
}

/////////////////////////////////////////////////////////
// cleanImages
//
/////////////////////////////////////////////////////////
void pix_multiimage :: cleanImages()
{
    if (m_numImages)
    {
        // decrement the reference count
        m_loadedCache->refCount--;

        // If the refCount == 0, then destroy the cache
        if (m_loadedCache->refCount == 0)
        {
            // find the cache
            multiImageCache *ptr = s_imageCache;

            // if the loaded cache is the first cache in the list
            if (m_loadedCache == s_imageCache)
            {
                s_imageCache = m_loadedCache->next;
                delete m_loadedCache;
            }
            else
            {
                while (ptr && ptr->next != m_loadedCache) ptr = ptr->next;
                if (!ptr) error("Unable to find image cache!");
                else
                {
                    ptr->next = m_loadedCache->next;
                    delete m_loadedCache;
                }
            }
        }

	    m_loadedCache = NULL;
    	m_numImages = 0;
    	m_pixBlock.image.clear();
        m_pixBlock.image.data = NULL;
    }
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_multiimage :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_multiimage::openMessCallback),
    	    gensym("open"), A_SYMBOL, A_FLOAT, A_DEFFLOAT, A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_multiimage::changeImageCallback),
    	    gensym("img_num"), A_FLOAT, A_NULL);
}
void pix_multiimage :: openMessCallback(void *data, t_symbol *filename, t_floatarg baseImage,
                                        t_floatarg topImage, t_floatarg skipRate)
{
    if ((int)skipRate == 0)
    {
        if ((int)topImage == 0)
			GetMyClass(data)->openMess(filename, 0, (int)baseImage, 0);
        else
			GetMyClass(data)->openMess(filename, (int)baseImage, (int)topImage, 0);
    }
    else
		GetMyClass(data)->openMess(filename, (int)baseImage, (int)topImage, (int)skipRate);
}
void pix_multiimage :: changeImageCallback(void *data, t_floatarg imgNum)
{
    GetMyClass(data)->changeImage((int)imgNum);
}
