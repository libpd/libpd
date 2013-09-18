////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// mark@danks.org
//
// Implementation file
//
//    Copyright (c) 1997-1999 Mark Danks.
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "CPPExtern.h"

#ifdef _WIN32
# include <io.h>
#endif
#ifdef _MSC_VER  /* This is only for Microsoft's compiler, not cygwin, e.g. */
# define snprintf _snprintf
# define vsnprintf _vsnprintf
# define close _close
#endif

#include <stdio.h>
#include <stdarg.h>


void *Obj_header::operator new(size_t, void *location, void *) {
  return(location);
}

t_object * CPPExtern::m_holder=NULL;
char* CPPExtern::m_holdname=NULL;

/////////////////////////////////////////////////////////
//
// CPPExtern
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
CPPExtern :: CPPExtern()
  : x_obj(m_holder),
    m_objectname(NULL),
    m_canvas(NULL),
    m_endpost(true)
{
    m_canvas = canvas_getcurrent();
    if(m_holdname) {
      m_objectname=gensym(m_holdname);
    } else {
      m_objectname=gensym("unknown Gem object");
    }
}
CPPExtern :: CPPExtern(const CPPExtern&org) :
  x_obj(org.x_obj),
  m_objectname(org.m_objectname),
  m_canvas(org.m_canvas),
  m_endpost(true)
{
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
CPPExtern :: ~CPPExtern()
{ }


void CPPExtern :: post(const char*fmt,...) const
{
  char buf[MAXPDSTRING];
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buf, MAXPDSTRING-1, fmt, ap);
  va_end(ap);
  if(m_endpost && NULL!=m_objectname && NULL!=m_objectname->s_name && &s_ != m_objectname){
    ::post("[%s]: %s", m_objectname->s_name, buf);
  } else {
    ::post("%s", buf);
  }
  m_endpost=true;
}
void CPPExtern :: startpost(const char*fmt,...) const
{
  char buf[MAXPDSTRING];
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buf, MAXPDSTRING-1, fmt, ap);
  va_end(ap);
  if(m_endpost && NULL!=m_objectname && NULL!=m_objectname->s_name && &s_ != m_objectname){
    ::startpost("[%s]: %s", m_objectname->s_name, buf);
  } else {
    ::startpost("%s", buf);
  }
  m_endpost=false;
}
void CPPExtern :: endpost(void) const
{
  ::endpost();
  m_endpost=true;
}
void CPPExtern :: verbose(const int level, const char*fmt,...) const
{
  char buf[MAXPDSTRING];
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buf, MAXPDSTRING-1, fmt, ap);
  va_end(ap);
  if(NULL!=m_objectname && NULL!=m_objectname->s_name && &s_ != m_objectname){
    ::verbose(level, "[%s]: %s", m_objectname->s_name, buf);
  } else {
    ::verbose(level, "%s", buf);
  }
}

void CPPExtern :: error(const char*fmt,...) const
{
  char buf[MAXPDSTRING];
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buf, MAXPDSTRING-1, fmt, ap);
  va_end(ap);
  if(NULL!=m_objectname && NULL!=m_objectname->s_name && &s_ != m_objectname){
    char*objname=m_objectname->s_name;
    if(x_obj)
      pd_error(x_obj, "[%s]: %s", objname, buf);
    else if (m_holder)
      pd_error(m_holder, "[%s]: %s", objname, buf);
    else
      ::error("[%s]: %s", objname, buf);
  } else {
    if(x_obj)
      pd_error(x_obj, "%s", buf);
    else if (m_holder)
      pd_error(m_holder, "%s", buf);
    else
      ::error("%s", buf);
  }
}


std::string CPPExtern::findFile(const std::string f, const std::string e) const {
  char buf[MAXPDSTRING], buf2[MAXPDSTRING];
  char*bufptr=0;
  std::string result="";
  int fd=-1;

  t_canvas*canvas=const_cast<t_canvas*>(getCanvas());
  char*filename=const_cast<char*>(f.c_str());
  char*ext=const_cast<char*>(e.c_str());
  
  if ((fd=open_via_path(canvas_getdir(canvas)->s_name, filename, ext, 
                        buf2, &bufptr, MAXPDSTRING, 1))>=0){
    sys_close(fd);
    result=buf2;
    result+="/";
    result+=bufptr;
  } else {
    canvas_makefilename(canvas, filename, buf, MAXPDSTRING);
    result=buf;
  }
  return result;
}

std::string CPPExtern::findFile(const std::string file) const {
  return findFile(file, "");

}
bool CPPExtern :: checkGemVersion(const int major, const int minor) {
  if(!GemVersion::versionCheck(major, minor)) {
    ::error("GEM version mismatch: compiled for %d.%d but we are running %s", 
	    major, minor,
	    GemVersion::versionString());
        return false;
  }
  return true;
}


CPPExtern&CPPExtern::operator=(const CPPExtern&org) {
  x_obj=org.x_obj;
  m_objectname=org.m_objectname;
  m_canvas=org.m_canvas;
  m_endpost=true;
  return *this;
}
