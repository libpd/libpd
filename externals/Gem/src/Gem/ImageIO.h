/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    GemPixImageLoad.h
       - code to load in and resize an image
       - part of GEM

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/
 
#ifndef _INCLUDE__GEM_GEM_IMAGEIO_H_
#define _INCLUDE__GEM_GEM_IMAGEIO_H_

#include "Gem/ExportDef.h"

struct imageStruct;

#include <string>


// image2mem() reads an image file into memory
//   and a pointer to an imageStruct
//       NULL = failure
// 
//       format:
//    	  returns either GL_LUMINANCE or GL_RGBA
// 
//   automatically allocates the memory for the user
//
// This can read TIFF, SGI, and JPG images
//
namespace gem {
  class Properties;
  namespace image {
    GEM_EXTERN class load {
    public:
      /**
       * loads an image (given as 'filename') synchronously
       * the function blocks until the image is loaded (in which case it returns TRUE)
       * of the image-loading completely failed (in which case it returns FALSE)
       *
       * the loaded image is stored in 'img'
       * 'props' holds a list of additional image properties discovered during loading
       */
      static bool sync(const std::string filename,
				  imageStruct&img,
				  Properties&props);



      typedef unsigned int id_t;
      static const id_t IMMEDIATE;
      static const id_t INVALID;

      /* the callback used for asynchronous image loading
       * userdata is is the pointer supplied when calling async();
       * id is the ID returned by async()
       * img holds a reference to the newly loaded image
       *  the image is allocated by the loder, but
       *  the callback (you!) is responsible for freeing the image
       *  once it is no more needed
       *  if image loading failed, img is set to NULL
       * props holds a list of additional image properties discovered during loading
       *
       * currently (with Pd being the only RTE),
       * the callback will always be called from within the main thread
       * 
       * the callback might be called directly from within async(),
       * in which case the ID given in the callback and returned by async() 
       * is IMMEDIATE
       */
      typedef void (*callback)(void *userdata, 
			       id_t ID,
			       imageStruct*img,
			       const Properties&props);

      /* loads an image (given as 'filename') asynchronously 
       * image loading is done in a separate thread (if possible);
       * when the image is loaded, the callback 'cb' is called with the new image
       *
       * this function returns an ID which is also passed to the callback function,
       * so the caller can identify a certain request (e.g. if several images have been
       * queued for loading before the 1st one was successfully returned;
       *
       * the image might get loaded (and the cb called) before the call to loadAsync()
       * has finished, in which case IMMEDIATE is returned (and used in the CB)
       *
       * if the image cannot be loaded at all, INVALID is returned
       * (and no callback will ever be called)
       *
       */
      static bool async(callback cb,
				   void*userdata,
				   const std::string filename,
				   id_t&ID
				   );
    
      /* cancels asynchronous loading of an image
       * removes the given ID (as returned by loadAsync()) from the loader queue
       * returns TRUE if item could be removed, or FALSE if no item ID is in the queue
       *
       * there is no point in cancel()ing an IMMEDIATE or ILLEGAL id
       */
      static bool cancel(id_t ID);

      /* load an image in a synchronous way (that is argument compatible with async())
       */ 
      static bool sync(callback cb,
				  void*userdata,
				  const std::string filename,
				  id_t&ID);

      /*
       * deliver all loaded images not delivered yet
       */
      static void poll(void);

      /*
       * set asynch loading to "polling" mode
       * in "polling" mode, the caller has to call 'poll()' manually in order to get any loaded images delivered
       * in "pushing" mode this is done automatically (but might hang with current Pd)
       */
      static bool setPolling(bool);


};};};

/* legacy */
GEM_EXTERN extern imageStruct *image2mem(const char *filename);


// image2mem() reads an image file into memory
//   and a pointer to an imageStruct
//       NULL = failure
// 
//       format:
//    	  returns either GL_LUMINANCE or GL_RGBA
// 
//   automatically allocates the memory for the user
//
// This can write TIFF, JPG and other images (depending on which backends are available
// legacy: type=0 -> TIFF; type>0 -> JPEG and (quality:=type)
//
GEM_EXTERN extern int mem2image(imageStruct *image, const char *filename, const int type);


#endif
