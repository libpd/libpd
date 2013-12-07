/* OpenGL pixel buffer
 *
 * Copyright (C) 2003-2004, Alexander Zaprjagaev <frustum@frustum.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#if 0
# define debug error
#else
# define debug
#endif

#include "Gem/GemGL.h"

#ifdef __APPLE__
# include <iostream>
# include <Carbon/Carbon.h>
#else
# include <vector>
#endif

#include "PBuffer.h"
#include "Gem/RTE.h"

#ifdef GLX_VERSION_1_3
struct PBuffer_data {
  Display *display;
	
  GLXPbuffer pbuffer;
  GLXContext context;
	
  GLXPbuffer old_pbuffer;
  GLXContext old_context;
};

/*
 * constructor (linux specific)
 */
PBuffer::PBuffer(int width,int height,int flags) : width(width), height(height), data(NULL)
{	
  Display *display = glXGetCurrentDisplay();
  int screen = DefaultScreen(display);
  GLXContext old_context = glXGetCurrentContext();
	
  std::vector<int> attrib;
  attrib.push_back(GLX_RENDER_TYPE);
  attrib.push_back(GLX_RGBA_BIT);
  attrib.push_back(GLX_DRAWABLE_TYPE);
  attrib.push_back(GLX_PBUFFER_BIT);
  if(flags & GEM_PBUFLAG_RGB || flags & GEM_PBUFLAG_RGBA) {
    attrib.push_back(GLX_RED_SIZE);
    attrib.push_back(flags & GEM_PBUFLAG_FLOAT ? 32 : 8);
    attrib.push_back(GLX_GREEN_SIZE);
    attrib.push_back(flags & GEM_PBUFLAG_FLOAT ? 32 : 8);
    attrib.push_back(GLX_BLUE_SIZE);
    attrib.push_back(flags & GEM_PBUFLAG_FLOAT ? 32 : 8);
    if(flags & GEM_PBUFLAG_RGBA) {
      attrib.push_back(GLX_ALPHA_SIZE);
      attrib.push_back(flags & GEM_PBUFLAG_FLOAT ? 32 : 8);
    }
  }
  if(flags & GEM_PBUFLAG_DEPTH) {
    attrib.push_back(GLX_DEPTH_SIZE);
    attrib.push_back(24);
  }
  if(flags & GEM_PBUFLAG_STENCIL) {
    attrib.push_back(GLX_STENCIL_SIZE);
    attrib.push_back(8);
  }
  if(flags & GEM_PBUFLAG_FLOAT) {
    attrib.push_back(GLX_FLOAT_COMPONENTS_NV);
    attrib.push_back(true);
  }
  if(flags & GEM_PBUFLAG_MULTISAMPLE_2 || flags & GEM_PBUFLAG_MULTISAMPLE_4) {
    attrib.push_back(GLX_SAMPLE_BUFFERS_ARB);
    attrib.push_back(true);
    attrib.push_back(GLX_SAMPLES_ARB);
    attrib.push_back(flags & GEM_PBUFLAG_MULTISAMPLE_2 ? 2 : 4);
  }
  attrib.push_back(0);
	
  std::vector<int> pattrib;
	
  pattrib.push_back(GLX_LARGEST_PBUFFER);
  pattrib.push_back(true);
  pattrib.push_back(GLX_PRESERVED_CONTENTS);
  pattrib.push_back(true);
	
  GLXPbuffer pbuffer;
  GLXContext context;
	
  try {
    int count;
    GLXFBConfig *config;

    if(GLXEW_SGIX_fbconfig && GLXEW_SGIX_pbuffer) {
      debug("using SGIX pbuffers\n");
      pattrib.push_back(0);
			
      config = glXChooseFBConfigSGIX(display,screen,&attrib[0],&count);
      if(!config) throw("glXChooseFBConfigSGIX() failed");
			
      pbuffer = glXCreateGLXPbufferSGIX(display,config[0],width,height,&pattrib[0]);
      if(!pbuffer) throw("glXCreateGLXPbufferSGIX() failed");
			
      context = glXCreateContextWithConfigSGIX(display,config[0],GLX_RGBA_TYPE,old_context,true);
      if(!context) throw("glXCreateContextWithConfigSGIX() failed");
    } else if (NULL!=glXChooseFBConfig) { /* LATER make a better check! */
      debug("using GLX pbuffers");
        pattrib.push_back(GLX_PBUFFER_WIDTH);
        pattrib.push_back(width);
        pattrib.push_back(GLX_PBUFFER_HEIGHT);
        pattrib.push_back(height);
        pattrib.push_back(0);

        config = glXChooseFBConfig(display,screen,&attrib[0],&count);	
        if(!config) throw("glXChooseFBConfig() failed");

        pbuffer = glXCreatePbuffer(display,config[0],&pattrib[0]);
        if(!pbuffer) throw("glXCreatePbuffer() failed");
			
        XVisualInfo *visual = glXGetVisualFromFBConfig(display,config[0]);
        if(!visual) throw("glXGetVisualFromFBConfig() failed");
			
        context = glXCreateContext(display,visual,old_context,true);
        if(!context) throw("glXCreateContext() failed");
    } else {
      throw("your system lacks PBuffer support!");
    }
  }
  catch(const char *err) {
    error("PBuffer::PBuffer(): %s",err);
    pbuffer = glXGetCurrentDrawable();
    context = old_context;
  }
	
  data = new PBuffer_data;
  data->display = display;
	
  data->pbuffer = pbuffer;
  data->context = context;
	
  data->old_pbuffer = glXGetCurrentDrawable();
  data->old_context = old_context;
}

/*
 */
PBuffer::~PBuffer() {
  if(data->context) glXDestroyContext(data->display,data->context);
  if(data->pbuffer) glXDestroyPbuffer(data->display,data->pbuffer);
  delete data;
}

/*
 */
void PBuffer::enable() {
  data->old_pbuffer = glXGetCurrentDrawable();
  data->old_context = glXGetCurrentContext();
	
  if(!glXMakeCurrent(data->display,data->pbuffer,data->context)) {
    error("PBuffer::enable(): glXMakeCurrent() failed");
  }
}

/*
 */
void PBuffer::disable() {
  if(!glXMakeCurrent(data->display,data->old_pbuffer,data->old_context)) {
    error("PBuffer::disable(): glXMakeCurrent() failed");
  }
}
#elif defined __APPLE__
#pragma mark ------AppleCode------
/*
  Apple OSX pBuffer Setup
*/

struct PBuffer_data {
  CGLPBufferObj		pbuffer;
  CGLContextObj		context;
	
  CGLContextObj		old_context;
  CGLPixelFormatObj	pixfmt;
};

#pragma mark ---- Error Reporting ----

// ---------------------------------
float gErrorTime = 0.0;
extern AbsoluteTime gStartTime;
char gErrorMessage[256] = "";
// error reporting as both window message and debugger string
void reportError (char * strError)
{
  //gErrorTime = getElapsedTime ();
  //sprintf (gErrorMessage, "Error: %s (at time: %0.1f secs)", strError, gErrorTime);
  error ("Error: %s", strError);
}

// ---------------------------------

// if error dump cgl errors to debugger string, return error
OSStatus cglReportError (CGLError err)
{
  if (0 != err)
    reportError ((char *) CGLErrorString(err));
  return err;
}

/*
 * constructor (APPLE specific)
 */
PBuffer::PBuffer(int width, int height, int flag) : width(width), height(height)
{
  OSStatus err = noErr;
  CGLPixelFormatAttribute		*att,attrib[64];
  GemCGLint vs, npf;

  // setup offscreen context
  att=attrib;
  *att++=kCGLPFANoRecovery;
  *att++=kCGLPFAAccelerated;
  *att++=kCGLPFAWindow;
  //	*att++=kCGLPFAPBuffer;
  *att++=kCGLPFAColorSize;
  *att++=(CGLPixelFormatAttribute)32;
		
  if (flag & GEM_PBUFLAG_DEPTH){
    *att++=kCGLPFADepthSize;
    //*att++=(CGLPixelFormatAttribute)24;
    *att++=(CGLPixelFormatAttribute)16;
  }
  if (flag & GEM_PBUFLAG_STENCIL){
    *att++=kCGLPFADepthSize;
    *att++=(CGLPixelFormatAttribute)8;
  }
  if (flag & GEM_PBUFLAG_FLOAT){
    //		*att++=kCGLPFADepthSize;
    //		*att++=(CGLPixelFormatAttribute)8;
  }
  //*att++=kCGLPFADoubleBuffer;
  //	*att++=kCGLPFADisplayMask;
  //	*att++=kCGLPFAAllRenderers;
		
  *att=(CGLPixelFormatAttribute)0;
						
  data = new PBuffer_data;
  data->old_context = CGLGetCurrentContext();
  err = CGLGetVirtualScreen(data->old_context, &vs);
  verbose (2, "Target Context (0x%X) Renderer: %s\n",data->old_context, glGetString (GL_RENDERER));
  cglReportError(CGLChoosePixelFormat (attrib, &data->pixfmt, &npf));
	
  cglReportError(CGLCreateContext (data->pixfmt, data->old_context, &(data->context)));
  verbose (2, "pBuffer Context (0x%X) Renderer: %s\n",data->context, glGetString (GL_RENDERER));
	
  /*	if (float_buffer)
    cglReportError( CGLCreatePBuffer ( width, height, GL_TEXTURE_2D, GL_FLOAT, 0, &(data->pbuffer) ) );
    else
       */	
    cglReportError( CGLCreatePBuffer ( width, height, GL_TEXTURE_RECTANGLE_EXT, GL_RGBA, 0, &(data->pbuffer) ));
    cglReportError( CGLSetCurrentContext( data->context ) );
    cglReportError( CGLGetVirtualScreen(data->old_context, &vs) );
    cglReportError( CGLSetPBuffer(data->context, data->pbuffer, 0, 0, vs) );
    verbose (2, "pbuffer (0x%X) Renderer: %s\n",data->pbuffer, glGetString (GL_RENDERER));
}

/*
 */
PBuffer::~PBuffer()
{
  if(data->context) CGLDestroyContext( data->context );
  if(data->pbuffer) CGLDestroyPBuffer( data->pbuffer );
  if(data->pixfmt) CGLDestroyPixelFormat( data->pixfmt );
  CGLSetCurrentContext(NULL);
	
  delete data;
}

/*
 */
void PBuffer::enable()
{
  GemCGLint vs;

  cglReportError (CGLSetCurrentContext (data->context));
  cglReportError (CGLGetVirtualScreen ( data->old_context, &vs ));
  cglReportError( CGLSetPBuffer( data->context, data->pbuffer, 0, 0, vs) );
  debug ("enable Context (0x%X) Renderer: %s\n",CGLGetCurrentContext(), glGetString (GL_RENDERER));
  debug ("pBuffer Context (0x%X) Renderer: %s\n",data->context, glGetString (GL_RENDERER));

  return;
}

/*
 */
void PBuffer::disable() {
  cglReportError ( CGLSetCurrentContext( data->old_context) );
  return;
}
#elif defined _WIN32

struct PBuffer_data {
  HDC hdc;
  HPBUFFERARB pbuffer;
  HGLRC context;
	
  HDC old_hdc;
  HGLRC old_context;
};

#ifndef wglChoosePixelFormatARB
static PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = NULL;
#endif
#ifndef wglCreatePbufferARB
static PFNWGLCREATEPBUFFERARBPROC wglCreatePbufferARB = NULL;
#endif
#ifndef wglGetPbufferDCARB
static PFNWGLGETPBUFFERDCARBPROC wglGetPbufferDCARB = NULL;
#endif
#ifndef wglReleasePbufferDCARB
static PFNWGLRELEASEPBUFFERDCARBPROC wglReleasePbufferDCARB = NULL;
#endif
#ifndef wglDestroyPbufferARB
static PFNWGLDESTROYPBUFFERARBPROC wglDestroyPbufferARB = NULL;
#endif
/*
 * constructor (w32 specific)
 */
PBuffer::PBuffer(int width,int height,int flags) : width(width), height(height) {
	
  HDC old_hdc = wglGetCurrentDC();
  HGLRC old_context = wglGetCurrentContext();
	
  std::vector<int> attrib;
  attrib.push_back(WGL_PIXEL_TYPE_ARB);
  attrib.push_back(WGL_TYPE_RGBA_ARB);
  attrib.push_back(WGL_DRAW_TO_PBUFFER_ARB);
  attrib.push_back(true);
  attrib.push_back(WGL_SUPPORT_OPENGL_ARB);
  attrib.push_back(true);
  if(flags & 	GEM_PBUFLAG_RGB || flags & GEM_PBUFLAG_RGBA) {
    attrib.push_back(WGL_RED_BITS_ARB);
    attrib.push_back(flags & GEM_PBUFLAG_FLOAT ? 32 : 8);
    attrib.push_back(WGL_GREEN_BITS_ARB);
    attrib.push_back(flags & GEM_PBUFLAG_FLOAT ? 32 : 8);
    attrib.push_back(WGL_BLUE_BITS_ARB);
    attrib.push_back(flags & GEM_PBUFLAG_FLOAT ? 32 : 8);
    if(flags & 	GEM_PBUFLAG_RGBA) {
      attrib.push_back(WGL_ALPHA_BITS_ARB);
      attrib.push_back(flags & 	GEM_PBUFLAG_FLOAT ? 32 : 8);
    }
  }
  if(flags & 	GEM_PBUFLAG_DEPTH) {
    attrib.push_back(WGL_DEPTH_BITS_ARB);
    attrib.push_back(24);
  }
  if(flags & 	GEM_PBUFLAG_STENCIL) {
    attrib.push_back(WGL_STENCIL_BITS_ARB);
    attrib.push_back(8);
  }
  if(flags & 	GEM_PBUFLAG_FLOAT) {
    attrib.push_back(WGL_FLOAT_COMPONENTS_NV);
    attrib.push_back(true);
  }
  if(flags & 	GEM_PBUFLAG_MULTISAMPLE_2 || flags & GEM_PBUFLAG_MULTISAMPLE_4) {
    attrib.push_back(WGL_SAMPLE_BUFFERS_ARB);
    attrib.push_back(true);
    attrib.push_back(WGL_SAMPLES_ARB);
    attrib.push_back(flags & 	GEM_PBUFLAG_MULTISAMPLE_2 ? 2 : 4);
  }
  attrib.push_back(0);
	
  HDC hdc;
  HPBUFFERARB pbuffer;
  HGLRC context;
	
  try {
		
    if(!wglChoosePixelFormatARB) wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
    if(!wglChoosePixelFormatARB) throw("wglGetProcAddress(\"wglChoosePixelFormatARB\") failed");
		
    if(!wglCreatePbufferARB) wglCreatePbufferARB = (PFNWGLCREATEPBUFFERARBPROC)wglGetProcAddress("wglCreatePbufferARB");
    if(!wglCreatePbufferARB) throw("wglGetProcAddress(\"wglCreatePbufferARB\") failed");
		
    if(!wglGetPbufferDCARB) wglGetPbufferDCARB = (PFNWGLGETPBUFFERDCARBPROC)wglGetProcAddress("wglGetPbufferDCARB");
    if(!wglGetPbufferDCARB) throw("wglGetProcAddress(\"wglGetPbufferDCARB\") failed");
		
    if(!wglReleasePbufferDCARB) wglReleasePbufferDCARB = (PFNWGLRELEASEPBUFFERDCARBPROC)wglGetProcAddress("wglReleasePbufferDCARB");
    if(!wglReleasePbufferDCARB) throw("wglGetProcAddress(\"wglReleasePbufferDCARB\") failed\n");
		
    if(!wglDestroyPbufferARB) wglDestroyPbufferARB = (PFNWGLDESTROYPBUFFERARBPROC)wglGetProcAddress("wglDestroyPbufferARB");
    if(!wglDestroyPbufferARB) throw("wglGetProcAddress(\"wglDestroyPbufferARB\") failed\n");
		
    int pixelformat;
    unsigned int count;
    wglChoosePixelFormatARB(old_hdc,&attrib[0],NULL,1,&pixelformat,&count);
    if(count == 0) throw("wglChoosePixelFormatARB() failed");
		
    int pattrib[] = { 0 };
		
    pbuffer = wglCreatePbufferARB(old_hdc,pixelformat,width,height,pattrib);
    if(!pbuffer) throw("wglCreatePbufferARB() failed");
		
    hdc = wglGetPbufferDCARB(pbuffer);
    if(!hdc) throw("wglGetPbufferDCARB() failed");
		
    context = wglCreateContext(hdc);
    if(!context) throw("wglCreateContext() failed");
		
    if(!wglShareLists(old_context,context)) throw("wglShareLists() failed");
  }
  catch(const char *err) {
    error("GemPBuffer: %s",err);
    hdc = old_hdc;
    context = old_context;
  }
	
  data = new PBuffer_data;
  data->hdc = hdc;
  data->pbuffer = pbuffer;
  data->context = context;
	
  data->old_hdc = old_hdc;
  data->old_context = old_context;
}

/*
 */
PBuffer::~PBuffer() {
  wglDeleteContext(data->context);
  wglReleasePbufferDCARB(data->pbuffer,data->hdc);
  wglDestroyPbufferARB(data->pbuffer);
  wglMakeCurrent(data->hdc,data->context);
}

/*
 */
void PBuffer::enable() {
  data->old_hdc = wglGetCurrentDC();
  data->old_context = wglGetCurrentContext();
	
  if(!wglMakeCurrent(data->hdc,data->context)) {
    error("PBuffer::disable(): wglMakeCurrent() failed");
  }
}

/*
 */
void PBuffer::disable() {
  if(!wglMakeCurrent(data->old_hdc,data->old_context)) {
    error("PBuffer::disable(): wglMakeCurrent() failed");
  }
}
#else
# error do Pbuffers on your OS !
#endif /* OS */

/* dummy implementations */
PBuffer::PBuffer(const PBuffer&org) : width(org.width), height(org.height), data(NULL) {}
PBuffer&PBuffer::operator=(const PBuffer&org) { return (*this);}
