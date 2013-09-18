/* -----------------------------------------------------------------

GEM - Graphics Environment for Multimedia

Load an image and return the frame(OS independant parent-class)

Copyright (c) 2011-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PLUGINS_IMAGEBASE_H_
#define _INCLUDE__GEM_PLUGINS_IMAGEBASE_H_

#include "plugins/imageloader.h"
#include "plugins/imagesaver.h"

   /*-----------------------------------------------------------------
     -------------------------------------------------------------------
     CLASS
     imageBase
    
     parent class for the system- and library-dependent imageBase-loader classes
     this class should only be used for plugin implementations
     the plugin-host should use the imageBaseloader/imageBasesaver classes resp.
    
     KEYWORDS
     pix load an image
    
     DESCRIPTION

     -----------------------------------------------------------------*/
namespace gem { namespace plugins {
class GEM_EXTERN imageBase : public imageloader, public imagesaver
  {
  public:
  
    //////////
    // Constructor
  
    /* initialize the image class (set 'threadable' to FALSE if this object must
     * NOT be used within a threaded context
     */
    imageBase(bool threadable=true);

    ////////
    // Destructor
    /* free what is apropriate */
    virtual ~imageBase(void);


    virtual bool isThreadable(void);
    virtual void getWriteCapabilities(std::vector<std::string>&mimetypes, gem::Properties&props);

    virtual float estimateSave( const imageStruct&img, 
				const std::string&filename, 
				const std::string&mimetype, 
				const gem::Properties&props);

    /**
     * list all properties this backend supports
     * after calling, "readable" will hold a list of all properties that can be read
     * and "writeable" will hold a list of all properties that can be set
     * if the enumeration fails, this returns <code>false</code>
     */
    virtual bool enumProperties(gem::Properties&readable,
                                gem::Properties&writeable);

    /**
     * set a number of properties (as defined by "props")
     * the "props" may hold properties not supported by the currently opened device,
     *  which is legal; in this case the superfluous properties are simply ignored
     * this function MAY modify the props; 
     */
    virtual void setProperties(gem::Properties&props);

    /**
     * get the current value of the given properties from the device
     * if props holds properties that can not be read from the device, they are set to UNSET 
     */
    virtual void getProperties(gem::Properties&props);

  protected:
    /* used to store the "set" properties */
    gem::Properties m_properties;
  private:
    bool m_threadable;
  };

};}; // namespace gem


/**
 * \fn REGISTER_IMAGEFACTORY(const char *id, Class imageClass)
 * registers a new class "imageClass" with the image-factory
 *
 * \param id a symbolic (const char*) ID for the given class
 * \param imageClass a class derived from "image"
 */
#define REGISTER_IMAGEFACTORY(id, TYP) REGISTER_IMAGELOADERFACTORY(id, TYP); REGISTER_IMAGESAVERFACTORY(id, TYP)


#endif	// for header file
