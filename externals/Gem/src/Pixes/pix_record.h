/*
  -----------------------------------------------------------------

  GEM - Graphics Environment for Multimedia

  write a pix block into a digital video (like AVI, Mpeg, Quicktime) 
  (OS independant parent-class)

  Copyright (c) 2005-2010 Chris Clepper
  Copyright (c) 2009-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

  ----------------------------------------------------------------
*/

#if 1
//this will record movies
#ifndef _INCLUDE__GEM_PIXES_PIX_RECORD_H_
#define _INCLUDE__GEM_PIXES_PIX_RECORD_H_

#include "Base/GemBase.h"
#include "Gem/Image.h"

#include "plugins/record.h"

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  pix_record
    
  Writes a pix to disk
    
  KEYWORDS
  pix
    
  DESCRIPTION

  "file" - filename to write to
  "bang" - do write now
  "auto 0/1" - stop/start writing automatically
    
  -----------------------------------------------------------------*/
class GEM_EXTERN pix_record : public GemBase
{
  CPPEXTERN_HEADER(pix_record, GemBase);

    public:

  //////////
  // Constructor
  pix_record(int argc, t_atom *argv);
    	
 protected:
    	
  //////////
  // Destructor
  virtual ~pix_record();
	
  virtual void	stopRecording();
  virtual void	startRecording();
    	
  //////////
  // Do the rendering
  virtual void 	render(GemState *state);

  //////////
  // Clear the dirty flag on the pixBlock
  virtual void 	postrender(GemState *state) {};

  //////////
  // Set the filename and filetype
  std::string m_filename;
  virtual void	fileMess(t_symbol*s,int argc, t_atom *argv);
 
  //////////
  // turn recording on/off
  virtual void	recordMess(bool on);

  ////////
  // call up compression dialog
  virtual void	dialogMess();
		
  virtual void	getCodecList();
  std::string m_codec;
  virtual void	codecMess(t_atom *argv);

  //////////
  // Manual writing
  bool            m_banged;
    	
  //////////
  // Automatic writing
  bool            m_automatic;
	
  //////////
  // a outlet for information like #frames
  t_outlet     *m_outNumFrames;
  // another outlet for extra information (like list of codecs...)
  t_outlet     *m_outInfo;
  
  int m_currentFrame; //keep track of the number of frames
  
  //////////
  //
  int m_maxFrames;
  
  gem::Properties m_props;
  virtual void	enumPropertiesMess(void);
  virtual void	setPropertiesMess(t_symbol*,int argc, t_atom*argv);
  virtual void	clearPropertiesMess(void);	
  
 private:
  bool m_recording;
  gem::plugins::record *m_handle;
  std::vector<std::string>m_ids;
  std::vector<gem::plugins::record*>m_handles;
  std::vector<gem::plugins::record*>m_allhandles;
  virtual bool addHandle(std::vector<std::string>available_ids, std::string id=std::string(""));
  //////////
  // static member functions
  void 	autoMess(bool on);
  void 	bangMess();
  static void 	codecMessCallback(void *data, t_symbol *s, int argc, t_atom *argv);

  static void 	minMessCallback(void *data, t_floatarg min);
  static void 	maxMessCallback(void *data, t_floatarg max);

  class PIMPL;
  PIMPL*m_pimpl;
};
#endif	// for header file
#endif //removes pix_record
