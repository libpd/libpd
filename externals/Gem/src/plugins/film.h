/* -----------------------------------------------------------------

GEM - Graphics Environment for Multimedia

Load an digital video (like AVI, Mpeg, Quicktime) into a pix block 
(OS independant parent-class)

Copyright (c) 1997-1999 Mark Danks. mark@danks.org
Copyright (c) Günther Geiger. geiger@epy.co.at
Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PLUGINS_FILM_H_
#define _INCLUDE__GEM_PLUGINS_FILM_H_

#include "Gem/ExportDef.h"
#include "Gem/GemGL.h"
#include <string>

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  film
    
  parent class for the system- and library-dependent film-loader classes
    
  KEYWORDS
  pix film movie
    
  DESCRIPTION

  -----------------------------------------------------------------*/
  
struct pixBlock;
namespace gem {
  class Properties;
}
namespace gem { namespace plugins {
class GEM_EXTERN film
{
 public:

  //////////
  // returns an instance wrapping all plugins or NULL
  // if NULL is returned, you might still try your luck with manually accessing the 
  // PluginFactory
  static film*getInstance(void);

  /////////
  // dtor must be virtual
  virtual ~film(void);

  //////////
  // open a movie up
  /* open the film "filename" (think better about URIs ?)
   *
   * try to open the film with the requested properties
   *
   * about properties:
   *  requestprops: are properties that can change the behaviour of how the 
   *                film is opened; examples are "colorspace" (e.g. GL_RGBA) or 
   *                "streaming" (rather than random frame access)
   *                the backend need not implement any of the properties
   *
   *  resultprops: give feedback about the opened film
   *               if the film could not be opened, the content is undefined
   *               if the film was successfully opened, following properties should be set
   *         if a property can not be determined (e.g. variable fps), it should be set unset
   *
   *   
   * discussion: should the colourspace be only a hint or should we force it
   * (evt. by converting the actual cs by hand to the desired one)
   * more discussion: i guess the cs should really be forced somehow by [pix_film]
   * now i don't know, whether the cs-conversion should be done by [pix_film] itself or
   * rather by the film*-classes. 
   * but i guess film* makes more sense, because then, [pix_film] doesn't have to know
   * anything about the internal cs of the decoder
   */
  /* returns TRUE if loading was successfull, FALSE otherwise */
  virtual bool open(const std::string, 
		    const gem::Properties&requestprops) = 0;

  /* some error codes */
  enum errCode { SUCCESS = 0,
		 FAILURE = 1,
		 DONTKNOW= 2 };

  //////////
  // Change which image to display
  /* this is the second core function of this class:
   * most decoding-libraries can set the frame-number on a random-access basis.
   * some cannot, then this might do nothing
   * you could also switch between various tracks of a file (if the format supports it)
   * specifying trackNum as -1 means "same track as before"
   */
  virtual errCode changeImage(int imgNum, int trackNum=-1) = 0;

  //////////
  // get the next frame
  /* this is the core-function of this class !!!!
   * when called it returns the current frame in the *pixBlock structure
   * dev: you can use "m_image" for this (and "return &m_image;")
   * if the image cannot be read, returns 0
   * dev: you probably want to set the whole meta-information
   *      (xsize,ysize,csize,format) over again
   *      if you are smart and the colour-space is fine, just point 
   * if this is a "new" frame (e.g. freshly decoded),
   * pixblock.newimage should be set to 1
   */
  virtual pixBlock* getFrame(void) = 0;

  //////////
  // close the movie file
  /* close the file and clean up temporary things */
  virtual void close(void) = 0;


  ////////
  // returns true if instance can be used in thread
  virtual bool isThreadable(void) = 0;

  /**
   * list all properties the currently opened film supports
   * if no film is opened, this returns generic backend properties 
   * which can be different from media specific properties
   * after calling, "readable" will hold a list of all properties that can be read
   * and "writeable" will hold a list of all properties that can be set
   * if the enumeration fails, this returns <code>false</code>
   */

  virtual bool enumProperties(gem::Properties&readable,
			      gem::Properties&writeable) = 0;

  /**
   * set a number of properties (as defined by "props")
   * the "props" may hold properties not supported by the currently opened media,
   *  which is legal; in this case the superfluous properties are simply ignored
   * this function MAY modify the props; 
   * namely one-shot properties should be removed from the props
   *
   * examples: "colorspace" GL_RGBA
   *           "auto"       1 
   */
  virtual void setProperties(gem::Properties&props) = 0;

  /**
   * get the current value of the given properties from the media
   * if props holds properties that can not be read for the media, they are set to UNSET 
   *
   *               "width" (width of each frame in pixels)
   *               "height" (height of each frame in pixels)
   *               "fps" (frames per second)
   *               "frames" (framecount)
   */
  virtual void getProperties(gem::Properties&props) = 0;
};

};}; // namespace gem::plugins


/**
 * \fn REGISTER_FILMFACTORY(const char *id, Class filmClass)
 * registers a new class "filmClass" with the film-factory
 *
 * \param id a symbolic (const char*) ID for the given class
 * \param filmClass a class derived from "film"
 */
#define REGISTER_FILMFACTORY(id, TYP) static gem::PluginFactoryRegistrar::registrar<TYP, gem::plugins::film> fac_film_ ## TYP (id)

#endif	// for header file
