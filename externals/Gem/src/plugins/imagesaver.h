/* -----------------------------------------------------------------

GEM - Graphics Environment for Multimedia

Load an image and return the frame(OS independant interface)

Copyright (c) 2011-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PLUGINS_IMAGESAVER_H_
#define _INCLUDE__GEM_PLUGINS_IMAGESAVER_H_

#include "Gem/Image.h"
#include "Gem/Properties.h"

#include <string>


/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  imagesaver
    
  interface for the system- and library-dependent imagesaver classes
    
  KEYWORDS
  save a pix to disk
    
  DESCRIPTION

  -----------------------------------------------------------------*/
namespace gem { namespace plugins {
    class GEM_EXTERN imagesaver
    {
    public:

      //////////
      // returns an instance wrapping all plugins or NULL
      // if NULL is returned, you might still try your luck with manually accessing the 
      // PluginFactory
      static imagesaver*getInstance(void);

      ////////
      // dtor must be virtual
      virtual ~imagesaver(void);

      /* save the image 'img' under the filename 'filename', respecting as many 'props' as possible
       *
       * returns TRUE if saving was successfull, FALSE otherwise */
      virtual bool save(const imageStruct&img, const std::string&filename, const std::string&mimetype, const gem::Properties&props) = 0;

      /* estimate how 'well' we could save the 'img'
       *  this is used to rate the different backends for a given image
       *
       *  e.g. if the user requests saving of an image as <filename>, virtually all backends will have a way to to as requested
       *        however, if filename was "bla.jpg", a TIFF-backend might save as a TIFF-image with a .jpg extension, 
       *        which is probably not what the user expected (esp. if there _is_ a JPEG-backend, which for whatever reasons 
       *        would only have been called after the TIFF-backend)
       *
       * the solution is quite simple: each backend is first asked, how well it could save a given image according to properties
       *  the backend that returns the highest value, will be chosen first; if it fails to save the image 
       *  (returning FALSE in the save() function), the backend with the next higher rating will be chosen and so on
       *
       * 
       * mimetype and properties are the main factors for rating; 
       *                'mimetype' (string): mimetype of the image; e.g. 'image/jpeg' means 'write the image as JPEG'
       *                                     if not empty, the mimetype will override all other ways to set the output format (like filename)
       *                                     even though we only expect mimetypes of type 'image/*', the prefix ('image/') is mandatory
       * a predefined properties (for legacy reasons) is:
       *                'quality'  (float) : for lossy formats, this is the quality (in percent)
       *
       * expected return values:
       *     <=0: 'USE ME IF YOU MUST (but rather not)'
       *          0 is returned, if the backend expects to be able to save the given image under the given 
       *          filename to disk, but it will ignore all properties (including the mimetype!) and will 
       *          ignore all file extensions
       *          it is hoped that '0' is never the winner (for any feasible format)
       *         example: saves a TIFF-image as /tmp/foo.doc
       *     100: 'YES'
       *          100 is returned, if the plugin knows how to handle the given 'mimetype' property
       *          if 'mimetype' is empty and the plugin has performed an heuristic based on the filename
       *          to determine that the user wants a format that is provided by this very plugin, it can return 100 as well.
       *          however, if 'mimetype' and file extension contradict each other, 'mimetype' wins!
       *     100+: 'YES, ABSOLUTELY'
       *          every additional property that can be applied, gains an extra point
       *         example: both the JPG and the JPEG2K backend provide saving of jpegs, but only JPG can set the quality
       *            the user requests: filename=img.jpg,mimetype='image/jpeg',quality=20 
       *            JPG returns 101, whereas JPEG2K returns 100, so JPG wins and writes
       *    0..50: the backend knows how to handle some of the properties (but it has no clue about the output format requested
       *         example: filename=img.tif,mimetype='image/tiff',quality=20 
       *            JPG knows how to handle the 'quality' property, but not the 'mimetype', so it scores 1 point
       *            TIFF knows how to handle the 'mimetype' but not the 'quality', so it scores 100 points
       */
      virtual float estimateSave( const imageStruct&img, const std::string&filename, const std::string&mimetype, const gem::Properties&props) = 0;
    
      /**
       * get writing capabilities of this backend (informative)
       * 
       * list all (known) mimetypes and properties this backend supports for writing
       *  both can be empty, if they are not known when requested
       * if only some properties/mimetypes are explicitely known (but it is likely that more are supported), 
       * it is generally better, to list the few rather than nothing
       */
      virtual void getWriteCapabilities(std::vector<std::string>&mimetypes, gem::Properties&props) = 0;
    
      /* returns TRUE, if it is save to use this backend from multple threads
       */
      virtual bool isThreadable(void) = 0;
    };

  }; }; // namespace gem


/**
 * \fn REGISTER_IMAGESAVERFACTORY(const char *id, Class imagesaverClass)
 * registers a new class "imagesaverClass" with the imagesaver-factory
 *
 * \param id a symbolic (const char*) ID for the given class
 * \param imagesaverClass a class derived from "imagesaver"
 */
#define REGISTER_IMAGESAVERFACTORY(id, TYP) static gem::PluginFactoryRegistrar::registrar<TYP, gem::plugins::imagesaver> fac_imagesaver_ ## TYP (id)

#endif	// for header file
