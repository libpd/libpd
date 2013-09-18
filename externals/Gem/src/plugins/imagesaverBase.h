/* -----------------------------------------------------------------

GEM - Graphics Environment for Multimedia

Load an image and return the frame(OS independant parent-class)

Copyright (c) 2011-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PLUGINS_IMAGESAVERBASE_H_
#define _INCLUDE__GEM_PLUGINS_IMAGESAVERBASE_H_


#include "plugins/imagesaver.h"


   /*-----------------------------------------------------------------
     -------------------------------------------------------------------
     CLASS
     imagesaver
    
     parent class for the system- and library-dependent imagesaver classes
    
     KEYWORDS
     save a pix to disk
    
     DESCRIPTION

     -----------------------------------------------------------------*/
   namespace gem { namespace plugins {
       class GEM_EXTERN imagesaverBase : public imagesaver 
  {
  public:
  
    //////////
    // Constructor
  
    /* initialize the imagesaver
     * set 'threadable' to FALSE if your implementation must NOT be used within
     * threads
     */
    imagesaverBase(bool threadable=true);

    ////////
    // Destructor
    /* free what is apropriate */
    virtual ~imagesaverBase(void);


    virtual float estimateSave( const imageStruct&img, const std::string&filename, const std::string&mimetype, const gem::Properties&props);
    
    /**
     * get writing capabilities of this backend (informative)
     * 
     * list all (known) mimetypes and properties this backend supports for writing
     *  both can be empty, if they are not known when requested
     * if only some properties/mimetypes are explicitely known (but it is likely that more are supported), 
     * it is generally better, to list the few rather than nothing
     */
    virtual void getWriteCapabilities(std::vector<std::string>&mimetypes, gem::Properties&props);
    
    /* returns TRUE, if it is save to use this backend from multple threads
     */
    virtual bool isThreadable(void) { return m_threadable; }
       
  private:
    bool m_threadable;
  };

}; }; // namespace gem

#endif	// for header file
