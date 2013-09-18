/*-----------------------------------------------------------------

GEM - Graphics Environment for Multimedia

Base Class for Video Capture Plugins

Copyright (c) 2010-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PLUGINS_VIDEO_H_
#define _INCLUDE__GEM_PLUGINS_VIDEO_H_

#include "Gem/Properties.h"
#include <vector>
#include <string>

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  video
    
  a OS-indendent interface for grabbing video-frames
	
  KEYWORDS
  pix, capture
    
  -----------------------------------------------------------------*/
struct pixBlock;
namespace gem { namespace plugins {
    class GEM_EXTERN video {
    public:

      //////////
      // returns an instance wrapping all plugins or NULL
      // if NULL is returned, you might still try your luck with manually accessing the 
      // PluginFactory
      static video*getInstance(void);

      ////////
      // dtor must be virtual
      virtual ~video(void);

      //////////////////////
      // device settings

      /**
       * enumerate known devices
       * \return a list of device names (if they can be enumerated)
       */
      virtual std::vector<std::string>enumerate(void) = 0;

      /**
       * set the device to be opened next time
       * the ID provided should match an index in the list returned by enumerate()
       * after the device has been set, the caller(!) has to restart 
       * (close() the current handle, try open() with the new settings)
       * the default implementation (which you normally shouldn't need to override)
       * will simply set m_devicenum and clear m_devicename
       */
      virtual bool	    	setDevice(int ID) = 0;

      /**
       * set the device to be opened next time
       * the list returned by enumerate() provides a set of valid names to use here
       * depending on the backend, other names might be possible as well (e.g. IP-cameras)
       * after the device has been set, the caller(!) has to restart 
       * (close() the current handle, try open() with the new settings)
       * the default implementation (which you normally shouldn't need to override)
       * will simply set m_devicename and clear m_devicenum
       */
      virtual bool	    	setDevice(const std::string) = 0;


      //! open the device (calls openDevice())
      virtual bool open(gem::Properties&props) = 0;
      //! start the transmission (calls startTransfer())
      virtual bool start(void) = 0;

      /** 
       * get the next frame (called when rendering)
       * grab the next frame from the device
       * if no new frame is available, this should set the "newimage" flag to false
       * \return the new frame or NULL on error
       */
      virtual pixBlock *getFrame(void) = 0;

      /**
       * release a frame (after use)
       * this gets called once for each frame retrieved via getFrame()
       * if you are using DMA or the like, now is the time to release the ressource
       */
      virtual void releaseFrame(void) = 0;


      //! stop the transmission (calls stopTransfer())
      virtual bool stop(void) = 0;
      //! close the device (calls closeDevice())
      virtual void close(void) = 0;

      /**
       * reset the backend, possibly re-enumerating devices
       * returns TRUE if reset was successfull
       */
      virtual bool          reset(void) = 0;


      /**
       * list all properties the currently opened device supports
       * after calling, "readable" will hold a list of all properties that can be read
       * and "writeable" will hold a list of all properties that can be set
       * if the enumeration fails, this returns <code>false</code>
       *
       * the backend has to provide the names for the properties
       *  these are defined by default, and need not be enumerated!
       *    "width"            "dimen" message   (float)
       *    "height"           "dimen" message   (float)
       *       "leftmargin"   ("dimen" message)  (float)
       *       "rightmargin"  ("dimen" message)  (float)
       *       "toptmargin"   ("dimen" message)  (float)
       *       "bottommargin" ("dimen" message)  (float)
       *    "channel"          "channel" message (float)
       *    "frequency"        "channel" message (float)
       *    "norm"             "norm" message    (string)
       *    "quality"          "quality" message (float)
       */
      virtual bool enumProperties(gem::Properties&readable,
				  gem::Properties&writeable) = 0;

      /**
       * set a number of properties (as defined by "props")
       * the "props" may hold properties not supported by the currently opened device,
       *  which is legal; in this case the superfluous properties are simply ignored
       * this function MAY modify the props; 
       * namely one-shot properties (e.g. "do-white-balance-now") 
       *     should be removed from the props
       */
      virtual void setProperties(gem::Properties&props) = 0;

      /**
       * get the current value of the given properties from the device
       * if props holds properties that can not be read from the device, they are set to UNSET 
       */
      virtual void getProperties(gem::Properties&props) = 0;


      /**
       * call a system-specific configuration dialog
       * if your system provides a GUI for configuring the device, here is the time to open it
       * of several dialogs are available (for different properties), the user can specify which one
       * they want with the string list
       * if the list is empty, provide sane defaults (e.g. ALL dialogs)
       * if the system does not support dialogs, return FALSE
       * if the system does support dialogs and the user has specified which one they want, 
       * return TRUE if at least one dialog could be handled
       */
      virtual bool	    	dialog(std::vector<std::string>names=std::vector<std::string>()) = 0;
      /**
       * enumerate list of possible dialogs (if any)
       */
      virtual std::vector<std::string>dialogs(void) = 0;





      /**
       * returns TRUE if the object can be used in a thread or FALSE otherwise
       * if a backend implements threading itself, it should return FALSE
       * in order to prevent double threading
       */
      virtual bool isThreadable(void) = 0;


      /** turn on/off "asynchronous"-grabbing
       * default is "true"
       * "asynchronous" means, that the device is constantly grabbing, and grabFrame() returns the current frame
       * non-"continous" means, that the device will only issue a new grab when a frame has read
       *   (thus potentially reducing the CPU-load to what is needed, at the cost of slightly outdated images
       * returns: the old state
       */
      virtual bool grabAsynchronous(bool) = 0;

      /**
       * Set the preferred colorspace (of the frames returned by getFrame()
       * \return FALSE if the colorspace cannot be set (e.g. while grabbing is active)
       */
      virtual bool setColor(int) = 0;
  

      // meta information about the plugin

      // for pix_video: query whether this backend provides access to this class of devices
      // (e.g. "dv")
      virtual bool provides(const std::string) = 0;
      // get a list of all provided devices
      virtual std::vector<std::string>provides(void) = 0;

      // get's the name of the backend (e.g. "v4l")
      virtual const std::string getName(void) = 0;
    };
  };}; // namespace

/* 
 * factory code:
 * to use these macros, you have to include "plugins/PluginFactory.h"
 */


/**
 * \fn REGISTER_VIDEOFACTORY(const char *id, Class videoClass)
 * registers a new class "videoClass" with the video-factory
 *
 * \param id a symbolic (const char*) ID for the given class
 * \param videoClass a class derived from "video"
 */
#define REGISTER_VIDEOFACTORY(id, TYP) static gem::PluginFactoryRegistrar::registrar<TYP, gem::plugins::video> fac_video_ ## TYP (id)

#endif	// for header file
