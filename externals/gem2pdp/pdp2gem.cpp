/*
 *  pdp2gem : pdp to gem bridge
 *
 *  Holds the contents of a PDP packet and introduce it in the GEM rendering chain
 *
 *  Copyright (c) 2003-2005 Yves Degoyon
 *  Copyright (c) 2004-2005 James Tittle
 *
 */


#include "pdp2gem.h"
#include "Base/GemState.h"
#include "Base/GemVersion.h"
#include "yuv.h"

CPPEXTERN_NEW_WITH_ONE_ARG(pdp2gem, t_symbol *, A_DEFSYM)

pdp2gem :: pdp2gem(t_symbol *colorspace)
		 : m_colorspace(GL_YUV422_GEM), m_data(NULL), m_xsize(0), m_ysize(0),
		 m_mutex(NULL), m_packet0(-1), m_dropped(0), m_pdpdata(NULL)
{
  
  // csMess allows us to select what colorspace to convert
  //  the YV12 pixels into while handing it over to GEM
  csMess(colorspace->s_name);

  // initialize the pix block data
  m_pixBlock.image.data = NULL;
  m_pixBlock.image.xsize = 0;
  m_pixBlock.image.ysize = 0;

  if ( m_colorspace == GL_RGB ){
  m_pixBlock.image.csize = 3;
  m_pixBlock.image.format = GL_RGB;
  m_pixBlock.image.type = GL_UNSIGNED_BYTE;
	m_format = GL_RGB;  
	m_csize = 3;
  }else if ( m_colorspace == GL_YUV422_GEM ){
	m_pixBlock.image.csize = 2;
	m_pixBlock.image.format = GL_YUV422_GEM;
#ifdef __APPLE__
	m_pixBlock.image.type = GL_UNSIGNED_SHORT_8_8_REV_APPLE;
#else
	m_pixBlock.image.type = GL_UNSIGNED_BYTE;
#endif
	m_format = GL_YUV422_GEM;
	m_csize = 2;
  }else if ( m_colorspace == GL_RGBA || m_colorspace == GL_BGRA_EXT){
	m_pixBlock.image.csize = 4;
	m_csize = 4;
#ifdef __APPLE__
	m_pixBlock.image.format = GL_BGRA_EXT;
	m_format = GL_BGRA_EXT;
#else
	m_pixBlock.image.format = GL_RGBA;
	m_format = GL_RGBA;
#endif
	m_pixBlock.image.type = GL_UNSIGNED_BYTE;
	post("pdp2gem:  RGBA conversion not yet implemented");
  }else if ( m_colorspace == GL_LUMINANCE ){
	post("pdp2gem:  Gray conversion not yet implemented");
	m_pixBlock.image.csize = 1;
	m_pixBlock.image.format = GL_LUMINANCE;
	m_pixBlock.image.type = GL_UNSIGNED_BYTE;
	m_format = GL_RGB;
	m_csize = 1;
  }else{
	post("pdp2gem:  needs to know what colorspace to import to:  YUV, RGB, RGBA, or Gray");
  }

  m_mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
  if ( pthread_mutex_init(m_mutex, NULL) < 0 )
  {
       perror("pdp2gem : couldn't create mutex");
  }
  else
  {
       post("pdp2gem : created mutex");
  }
}

pdp2gem :: ~pdp2gem()
{
  if ( m_mutex ) 
  {
    pthread_mutex_destroy(m_mutex);
    free(m_mutex);
    post("pdp2gem : destroyed mutex");
  }

  deleteBuffer();
  colorspace = NULL;
}

void pdp2gem :: deleteBuffer()
{
  if ( m_data )
  {
    delete [] m_data;
  }
  m_pixBlock.image.data=NULL;
}

void pdp2gem :: createBuffer()
{
  const int neededXSize = m_xsize;
  const int neededYSize = m_ysize;
  int dataSize;

  deleteBuffer();

  m_pixBlock.image.xsize = neededXSize;
  m_pixBlock.image.ysize = neededYSize;
  m_pixBlock.image.csize = m_csize;
  m_pixBlock.image.format= m_format;
  m_pixBlock.image.upsidedown=1;

  // +4 from MPEG 
  dataSize = (m_pixBlock.image.xsize * m_pixBlock.image.ysize * m_pixBlock.image.csize)+4; 
  m_data = new unsigned char[dataSize];
  memset(m_data, 0, dataSize);

  m_pixBlock.image.data = m_data;

  post("pdp2gem : created buffer %d x %d", m_xsize, m_ysize );
}

void pdp2gem :: pdpMess(t_symbol *action, int pcktno)
{
 t_int psize;
  short int *pY, *pY2, *pU, *pV;
 unsigned char r,g,b;
  unsigned char y,y2,u,v;
 t_int cpt, px, py;

  if (action == gensym("register_ro"))  
  {
	m_dropped = pdp_packet_copy_ro_or_drop((int*)&m_packet0, pcktno);
  }
  // post("pdp2gem : got pdp packet #%d : dropped : %d", pcktno, m_dropped );

  m_header = pdp_packet_header(m_packet0);

  if ((action == gensym("process")) && (-1 != m_packet0) && (!m_dropped))
  {
    if ( PDP_IMAGE == m_header->type )
    {
	  if (pdp_packet_header(m_packet0)->info.image.encoding == PDP_IMAGE_YV12 
	  				&& (m_colorspace == GL_RGB || m_colorspace == GL_BGR_EXT) )
	  {
		if ( ( m_xsize != (int)m_header->info.image.width ) ||
			 ( m_ysize != (int)m_header->info.image.height ) )
		{
		  m_xsize = m_header->info.image.width;
		  m_ysize = m_header->info.image.height;
		  createBuffer();
		}
		m_pdpdata = (short int *)pdp_packet_data(m_packet0);

		// lock mutex
		pthread_mutex_lock(m_mutex);
		
		// convert pixels from yv12 to RGB/BGR
		m_pixBlock.image.fromYV12(m_pdpdata);
		
		// unlock mutex
		pthread_mutex_unlock(m_mutex);

		// free PDP packet
		pdp_packet_mark_unused(m_packet0);
		m_packet0 = -1;
		
	} else if (pdp_packet_header(m_packet0)->info.image.encoding == PDP_IMAGE_YV12 
	  				&& (m_colorspace == GL_RGBA || m_colorspace == GL_BGRA_EXT) )
	{
		if ( ( m_xsize != (int)m_header->info.image.width ) ||
			 ( m_ysize != (int)m_header->info.image.height ) )
		{
		  m_xsize = m_header->info.image.width;
		  m_ysize = m_header->info.image.height;
		  createBuffer();
		}
		m_pdpdata = (short int *)pdp_packet_data(m_packet0);

		// lock mutex
		pthread_mutex_lock(m_mutex);
		
		// convert pixels from yv12 to RGBA/BGRA
		m_pixBlock.image.fromYV12(m_pdpdata);
		
		// unlock mutex
		pthread_mutex_unlock(m_mutex);

		// free PDP packet
		pdp_packet_mark_unused(m_packet0);
		m_packet0 = -1;
		
	} else if (pdp_packet_header(m_packet0)->info.image.encoding == PDP_IMAGE_YV12 
									&& m_colorspace == GL_YUV422_GEM)
	{
		  if ( ( m_xsize != (int)m_header->info.image.width ) ||
			 ( m_ysize != (int)m_header->info.image.height ) )
		  {
		    m_xsize = m_header->info.image.width;
		    m_ysize = m_header->info.image.height;
		    createBuffer();
		  }
		m_pdpdata = (short int *)pdp_packet_data(m_packet0);

 		// lock mutex
 		pthread_mutex_lock(m_mutex);
 		
 		// convert pixels from yv12 to uyvy
 		m_pixBlock.image.fromYV12(m_pdpdata);

		// unlock mutex
		pthread_mutex_unlock(m_mutex);

		// free PDP packet
		pdp_packet_mark_unused(m_packet0);
		m_packet0 = -1;       
	  }
	  else
	  {
		post("pdp2gem : unsupported image type %i", m_header->info.image.encoding );
	  }
	}
  }
}

void pdp2gem :: startRendering()
{
  m_pixBlock.newimage = 1;
}

void pdp2gem :: render(GemState *state)
{
  if ( m_data && state )
  {
    // lock mutex
    pthread_mutex_lock(m_mutex);

    m_pixBlock.newimage = 1;
#if GEM_VERSION_MINOR >= 93
    state->set(GemState::_PIX, &m_pixBlock);
#else
    state->image = &m_pixBlock;
#endif
  }
}

void pdp2gem :: postrender(GemState *state)
{
  m_pixBlock.newimage = 0;

  // unlock mutex
  pthread_mutex_unlock(m_mutex);
}

/////////////////////////////////////////////////////////
// colorspaceMess
//
/////////////////////////////////////////////////////////
void pdp2gem :: csMess(char* format)
{
    if (!strcmp(format, "YUV")){
      m_colorspace = GL_YUV422_GEM;
      post("pdp2gem: colorspace is YUV %d",m_colorspace);
      m_csize = 2;
      m_format = GL_YUV422_GEM;
#ifdef __APPLE__
      m_pixBlock.image.type = GL_UNSIGNED_SHORT_8_8_REV_APPLE;
#else
      m_pixBlock.image.type = GL_UNSIGNED_BYTE;
#endif
      createBuffer();
      return;
    } else
    
    if (!strcmp(format, "RGB")){
      m_colorspace = GL_RGB;
      post("pdp2gem: colorspace is GL_RGB %d",m_colorspace);
      m_csize = 3;
      m_pixBlock.image.type = GL_UNSIGNED_BYTE;
#ifdef __APPLE__
	  m_format = GL_BGR_EXT;
#else
      m_format = GL_RGB;
#endif
      createBuffer();
      return;
    } else
    
    if (!strcmp(format, "RGBA")){
      m_colorspace = GL_RGBA;
      post("pdp2gem: colorspace is GL_RGBA %d",m_colorspace);
      m_csize = 4;
#ifdef __APPLE__
      m_format = GL_BGRA_EXT; m_pixBlock.image.format = GL_UNSIGNED_INT_8_8_8_8_REV;
#else
      m_format = GL_RGBA; m_pixBlock.image.format = GL_UNSIGNED_BYTE;
#endif
      createBuffer();
      return;
    } else {
    
    //post("pdp2gem: colorspace is unknown %d",m_colorspace);
    post("pdp2gem: using default colorspace:  YUV");
    m_csize = 2;
    m_colorspace = GL_YUV422_GEM;
    m_format = GL_YUV422_GEM;
    createBuffer();
    }
}

void pdp2gem :: obj_setupCallback(t_class *classPtr)
{
  ::post( "pdp2gem : a bridge between PDP/PiDiP and GEM v"GEM2PDP_VERSION" (ydegoyon@free.fr & tigital@mac.com)" );
  class_addmethod(classPtr, (t_method)&pdp2gem::pdpCallback, 
				  gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);
  class_addmethod(classPtr, (t_method)&pdp2gem::csMessCallback,
				  gensym("colorspace"), A_DEFSYMBOL, A_NULL);
  class_sethelpsymbol( classPtr, gensym("pdp2gem.pd") );
}

void pdp2gem :: pdpCallback(void *data, t_symbol *action, t_floatarg fpcktno)
{
  //post("pdp2gem : callback : action : %s : no : %f\n", action->s_name, fpcktno );
  GetMyClass(data)->pdpMess(action, (int)fpcktno);
}
void pdp2gem :: csMessCallback (void *data, t_symbol *colorspace)
{
  GetMyClass(data)->csMess((char*)colorspace->s_name);
}
