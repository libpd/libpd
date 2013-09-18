/*-----------------------------------------------------------------

GEM - Graphics Environment for Multimedia

Base Class for Video Capture Plugins

Copyright (c) 2010-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PLUGINS_VIDEOBASE_H_
#define _INCLUDE__GEM_PLUGINS_VIDEOBASE_H_

#include "plugins/video.h"
#include "Gem/Image.h"

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
	video
    
	a OS-indendent parent-class for grabbing video-frames
	
  KEYWORDS
	pix, capture
    
  -----------------------------------------------------------------*/
namespace gem { namespace plugins {
 class GEM_EXTERN videoBase : public video {
  public:
    /**
     * returns TRUE if the object can be used in a thread or FALSE otherwise
     * if a backend implements threading itself, it should return FALSE
     * in order to prevent double threading
     */
    virtual bool isThreadable(void);

    /**
     * reset the backend, possibly re-enumerating devices
     * returns TRUE if reset was successfull
     */
    virtual bool          reset(void);

 protected:
    /**
     * open the video-device
     * the exact video device is either the default one or has to be set prior with setDevice()
     * the device need not start streaming yet
     * this comes in handy when determining the correct backend for a certain device
     * \return TRUE when we successfully opened the device and can startTransfer immediately
     */
    virtual bool           openDevice(gem::Properties&props) = 0;

    /**
     * close the video device, freeing all ressources
     * once the device has been closed it should be useable by other applications,...
     * this get's called when switching to another backend or when deleting the object
     */
    virtual void          closeDevice(void) = 0;

    /**
     * Start up the video device (called on startRendering)
     * \return FALSE is something failed, TRUE otherwise
     */
    virtual bool	    	startTransfer(void) = 0;

    /**
     * Stop the video device (called on stopRendering)
     * \return TRUE if a transfer was going on, FALSE if the transfer was already stopped
     */
    virtual bool	   	stopTransfer(void) = 0;


    /**
     * Stops the video device and if it was running restarts it
     * \return the return code of startTransfer()
     */
    virtual bool	   	restartTransfer(void);

 public:
    /** 
     * get the next frame (called when rendering)
     * grab the next frame from the device
     * if no new frame is available, this should set the "newimage" flag to false
     * \return the new frame or NULL on error
     */
    virtual pixBlock *getFrame(void);

    /**
     * release a frame (after use)
     * this gets called once for each frame retrieved via getFrame()
     * if you are using DMA or the like, now is the time to release the ressource
     */
    virtual void releaseFrame(void);


    /** turn on/off "asynchronous"-grabbing
     * default is "true"
     * "asynchronous" means, that the device is constantly grabbing, and grabFrame() returns the current frame
     * non-"continous" means, that the device will only issue a new grab when a frame has read
     *   (thus potentially reducing the CPU-load to what is needed, at the cost of slightly outdated images
     * returns: the old state
     */
    virtual bool grabAsynchronous(bool);

    //////////////////////
    // device settings

    /**
     * enumerate known devices
     * \return a list of device names (if they can be enumerated)
     */
    virtual std::vector<std::string>enumerate(void);

    /**
     * set the device to be opened next time
     * the ID provided should match an index in the list returned by enumerate()
     * after the device has been set, the caller(!) has to restart 
     * (close() the current handle, try open() with the new settings)
     * the default implementation (which you normally shouldn't need to override)
     * will simply set m_devicenum and clear m_devicename
     */
    virtual bool	    	setDevice(int ID);

    /**
     * set the device to be opened next time
     * the list returned by enumerate() provides a set of valid names to use here
     * depending on the backend, other names might be possible as well (e.g. IP-cameras)
     * after the device has been set, the caller(!) has to restart 
     * (close() the current handle, try open() with the new settings)
     * the default implementation (which you normally shouldn't need to override)
     * will simply set m_devicename and clear m_devicenum
     */
    virtual bool	    	setDevice(const std::string);

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
				gem::Properties&writeable);

    /**
     * set a number of properties (as defined by "props")
     * the "props" may hold properties not supported by the currently opened device,
     *  which is legal; in this case the superfluous properties are simply ignored
     * this function MAY modify the props; 
     * namely one-shot properties (e.g. "do-white-balance-now") 
     *     should be removed from the props
     */
    virtual void setProperties(gem::Properties&props);

    /**
     * get the current value of the given properties from the device
     * if props holds properties that can not be read from the device, they are set to UNSET 
     */
    virtual void getProperties(gem::Properties&props);


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
    virtual bool	    	dialog(std::vector<std::string>names=std::vector<std::string>());
    /**
     * enumerate list of possible dialogs (if any)
     */
    virtual std::vector<std::string>dialogs(void);

    /**
     * Set the preferred colorspace (of the frames returned by getFrame()
     * \return FALSE if the colorspace cannot be set (e.g. while grabbing is active)
     */
    virtual bool	    	setColor(int);
  
    //////////
    // Constructor
    // if numlocks>0 we will use a thread to capture the image create <numlocks> mutexes
    // 
    videoBase(const std::string name);
  
    //////////
    // Destructor
    virtual ~videoBase(void);

    //! open the device (calls openDevice())
    virtual bool open(gem::Properties&props);
    //! close the device (calls closeDevice())
    virtual void close(void);
    //! start the transmission (calls startTransfer())
    virtual bool start(void);
    //! stop the transmission (calls stopTransfer())
    virtual bool stop(void);

  protected:

    /**
     * in the constructor, you ought to call provide() for each generic backend your plugin is capable of
     * e.g. the DirectShow plugin will call: provide("analog"); provide("dv");
     * everything you add here, will be automaticalyl enumerated with the provides() methods
     */
    void provide(const std::string);

    //! grab a frame (work-horse)
    /* this will be run in a separate thread (if threading is enabled)
     * makes the data accessible in the "m_image" struct!
     * access to m_image MUST be protected by lock() and unlock()
     * this should not be used anymore!
     */
    virtual bool grabFrame(void){/* lock(); m_image.image.data=NULL; unlock(); */ return false; };

    /***************
     ** THREADING
     ** these are here for legacy reasons
     ** DO NOT USE THEM IN NEW CODE
     */

    /* starts the thread that will call grabFrame() (if threads are requested in the ctor) */
    bool startThread(void);
    /* stops the thread; waits at most "timeout" microseconds; if 0 waits forever; if -1 waits for time specified in ctor */
    bool stopThread(int timeout=-1);

    /* locks the mutex #<id>; 
     * if the mutex does not exist (e.g. no threading), this will simply return
     * the default mutex #0 is locked by default in the getFrame() to protect the m_image ressource
     */
    void lock(unsigned int id=0);
    /* unlocks the mutex #<id>; 
     * if the mutex does not exist (e.g. no threading), this will simply return
     * the default mutex #0 is locked by default in the getFrame() to protect the m_image ressource
     */
    void unlock(unsigned int id=0);

    /* sleep a selected time in usec
     * convenience wrapper around select()
     */
    void usleep(unsigned long usec);

    //////////
    // Constructor
    // if numlocks>0 we will use a thread to capture the image create <numlocks> mutexes
    // 
    videoBase(const std::string name, unsigned int numlocks);

    /*
     * THREADING
     ************** */

  protected:
    //! indicates valid transfer (automatically set in start()/stop())
    bool m_capturing;
    //! indicates valid device (automatically set in open()/close())
    bool m_haveVideo;
    //! a place to store the image with grabFrame()
    pixBlock m_image;
  
    unsigned int m_width;
    unsigned int m_height;
    int m_reqFormat;

    /* specify either devicename XOR devicenum */
    /* if !m_devicename.empty() 
     *  then use m_devicename
     * elif m_devicenum>=0
     *  use m_devicenum (preferrable as index in the list returned by enumerate()
     * else
     *  try open a "default" device
     */
    std::string m_devicename;
    int m_devicenum;

  public:
    // for pix_video: query whether this backend provides access to this class of devices
    // (e.g. "dv")
    virtual bool provides(const std::string);
    // get a list of all provided devices
    virtual std::vector<std::string>provides(void);

    // get's the name of the backend (e.g. "v4l")
    virtual const std::string getName(void);


  private:
    class PIMPL;
    PIMPL*m_pimpl;
};
};}; // namespace

#endif	// for header file
