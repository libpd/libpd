/* -----------------------------------------------------------------

GEM - Graphics Environment for Multimedia

Load an image and return the frame(OS independant parent-class)

Copyright (c) 2011-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PLUGINS_IMAGELOADERBASE_H_
#define _INCLUDE__GEM_PLUGINS_IMAGELOADERBASE_H_

#include "plugins/imageloader.h"


   /*-----------------------------------------------------------------
     -------------------------------------------------------------------
     CLASS
     imageloader
    
     parent class for the system- and library-dependent imageloader classes
    
     KEYWORDS
     pix load an image
    
     DESCRIPTION

     -----------------------------------------------------------------*/
   namespace gem { namespace plugins {
       class GEM_EXTERN imageloaderBase : public imageloader
  {
  public:
  
    //////////
    // Constructor
  
    /* initialize the imageloader
     * set 'threadable' to FALSE if your implementation must NOT be used within
     * threads
     */
    imageloaderBase(bool threadable=true);

    ////////
    // Destructor
    /* free what is apropriate */
    virtual ~imageloaderBase(void);


    virtual bool isThreadable(void) { return m_threadable; }

  protected:
    /* used to store the "set" properties */
    gem::Properties m_properties;

  private:
    bool m_threadable;
  };

};}; // namespace gem

#endif	// for header file
