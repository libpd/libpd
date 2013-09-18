/* -----------------------------------------------------------------

GEM - Graphics Environment for Multimedia

Load an digital video (like AVI, Mpeg, Quicktime) into a pix block 
(OS independant parent-class)

Copyright (c) 2010-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PLUGINS_RECORDBASE_H_
#define _INCLUDE__GEM_PLUGINS_RECORDBASE_H_

#include "plugins/record.h"
#include <map>

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  recordBase
    
  parent class for the system- and library-dependent record-loader classes
    
  KEYWORDS
  pix record movie
    
  DESCRIPTION

  -----------------------------------------------------------------*/
namespace gem { namespace plugins {
 class GEM_EXTERN recordBase : public record
{
 protected:

  //////////
  // compress and write the next frame
  /* this is the core-function of this class !!!!
   * when called it returns something depending on success
   * (what? the framenumber and -1 (0?) on failure?)
   */
  virtual bool putFrame(imageStruct*)=0;

  //////////
  // open a movie up
  /* open the record "filename" (think better about URIs ?)
   */
  /* returns TRUE if opening was successfull, FALSE otherwise */
  virtual bool open(const std::string filename);
  //////////
  // close the movie file
  /* stop recording, close the file and clean up temporary things */
  virtual void close(void);

public:
  virtual bool start(const std::string filename, gem::Properties&props);
  virtual void stop (void);
  virtual bool write(imageStruct*);

  /*
   * default implementation: return FALSE
   */
  virtual bool dialog(void);

  /**
   * default implementation: return empty list
   */ 
  virtual std::vector<std::string>getCodecs(void);
  /**
   * default implementation: return empty string
   */ 
  virtual const std::string getCodecDescription(const std::string codecname);
  /**
   * default implementation: return FALSE
   */ 
  virtual bool setCodec(const std::string name);
  /**
   * default implementation: return empty propset
   */ 
  virtual bool enumProperties(gem::Properties&props);

 public:
  
  //////////
  // Constructor
  
  /* initialize the recordloader
   */
  recordBase(void);

  ////////
  // Destructor
  /* free what is apropriate */
  virtual ~recordBase(void);

 protected:
  // map codec-names to codec-descriptions
  std::map<std::string, std::string>m_codecdescriptions;
  // write properties
  gem::Properties m_props;

 private:
  class PIMPL;
  PIMPL*m_pimpl;
 };
}; };

#endif	// for header file
