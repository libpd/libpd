////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file 
//
//    Copyright (c) 1997-1999 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "plugins/recordBase.h"
#include "Gem/RTE.h"

#include <stdlib.h>

using namespace gem::plugins;

class recordBase :: PIMPL {
public:
  bool running;
  PIMPL(void) :
	running(false)
  {}
};


/////////////////////////////////////////////////////////
//
// recordBase
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////

recordBase :: recordBase() : m_pimpl(new PIMPL())
{}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
recordBase :: ~recordBase()
{
  if(m_pimpl->running) {
	error("record: implementation forgot to call close() - please report a bug!");
  }
  delete m_pimpl;
  m_pimpl=NULL;
}

void recordBase :: close(void)
{}

/////////////////////////////////////////////////////////
// open a file !
//
/////////////////////////////////////////////////////////
bool recordBase :: start(const std::string filename, gem::Properties&props)
{
  if(m_pimpl->running)close();
  m_pimpl->running=false;
  m_props=props;

  m_pimpl->running=open(filename);

  return m_pimpl->running;
}
void recordBase :: stop()
{
  if(m_pimpl->running)
    close();
  m_pimpl->running=false;
}

bool recordBase::write(imageStruct*img) {
  if(!m_pimpl->running)
    return false;
  if(!img) {
    return true;
  }
  m_pimpl->running=putFrame(img);
  return m_pimpl->running;
}

bool recordBase :: open(const std::string filename)
{
  return false;
}

/////////////////////////////////////////////////////////
// set the codec
//
/////////////////////////////////////////////////////////
bool recordBase :: dialog()
{
  return false;
}

/////////////////////////////////////////////////////////
// get number of codecs
//
/////////////////////////////////////////////////////////
std::vector<std::string>recordBase :: getCodecs()
{
  std::vector<std::string>result;
  m_codecdescriptions.clear();
  return result;
}
const std::string recordBase :: getCodecDescription(const std::string name)
{
  std::map<std::string,std::string>::iterator it = m_codecdescriptions.find(name);

  if(it==m_codecdescriptions.end()) {
    return name;
  }

  return it->second;
}


/////////////////////////////////////////////////////////
// set codec by name
//
/////////////////////////////////////////////////////////
bool recordBase :: setCodec(const std::string name)
{
  return false;
}

bool recordBase :: enumProperties(gem::Properties&props) 
{
  props.clear();
  return false;
}
