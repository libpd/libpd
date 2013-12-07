/*
 *  pix_share_write.cpp
 *  GEM_darwin
 *
 *  Created by lincoln on 9/29/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#include "pix_share_write.h"
#include "Gem/Image.h"
#include "Gem/State.h"
#include "Gem/Exception.h"

#include <errno.h>
#include <stdio.h>

#ifdef _MSC_VER
# define snprintf _snprintf
#endif


CPPEXTERN_NEW_WITH_GIMME(pix_share_write);
#if 0
  ;
#endif


int hash_str2us(std::string s) {
  /*
  def self.rs( str, len=str.length )
    a,b = 63689,378551
    hash = 0
    len.times{ |i|
      hash = hash*a + str[i]
      a *= b
    }
    hash & SIGNEDSHORT
  end
  */

  int result=0;
  int a=63689;
  int b=378551;


  if(s.length()<1)return -1;

  unsigned int i=0;
  for(i=0; i<s.length(); i++) {
    result=result*a+s[i];
    a *= b;
  }

  return ((unsigned short)(result) & 0x7FFFFFFF);
}

/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_share_write :: pix_share_write(int argc, t_atom*argv)
#ifdef _WIN32
#else
  : shm_id(0), shm_addr(NULL)
#endif
{
  if(argc<1){
    throw(GemException("no ID given"));
  }
  int err  = getShm(argc, argv);

  switch(err){
  case 0:
    break;
  case 1:
    throw(GemException("no valid size given"));
    break;
  case 2:
    throw(GemException("given size < 0"));
    break;
  case 3:
    throw(GemException("no valid dimensions given"));
    break;
  case 4:
    throw(GemException("<color> must be one of: 4,2,1,RGBA,YUV,Grey"));
    break;
  case 5:
    throw(GemException("arguments: <id> <width> <height> <color>"));
    break;
  case 6:
    throw(GemException("couldn't get shared memory"));
    break;
  case 7:
    throw(GemException("no ID given"));
    break;
  case 8:
    throw(GemException("invalid ID..."));
    break;
   default:
    throw(GemException("unknown error"));
    break;
  }
}

pix_share_write :: ~pix_share_write()
{
  freeShm();
}


void pix_share_write :: freeShm()
{
#ifdef _WIN32
#else
  if(shm_addr){
    if (shmdt(shm_addr) == -1) error("shmdt failed at %x", shm_addr);
  }
  shm_addr=NULL;

  if(shm_id>0){
    if (shmctl(shm_id,IPC_STAT, &shm_desc) != -1){
      if(shm_desc.shm_nattch<=0){
        if (shmctl(shm_id,IPC_RMID, &shm_desc) == -1) error("shmctl remove failed for %d", shm_id);
      }
    }
  }
  shm_id=0;
#endif
}

int pix_share_write :: getShm(int argc,t_atom*argv)
{
  int fake = 0;

  size_t size=0;
  int    xsize=1;
  int    ysize=1;
  GLenum color=GL_RGBA;

  if(argc<1)return 7;
#ifdef _WIN32
#else
  if(shm_id>0)freeShm();
#endif
  if(A_FLOAT==argv->a_type){
    char buf[MAXPDSTRING];
    snprintf(buf, MAXPDSTRING-1, "%g", atom_getfloat(argv));
    buf[MAXPDSTRING-1]=0;
    fake = hash_str2us(buf);
  } else if(A_SYMBOL==argv->a_type){
    fake = hash_str2us(atom_getsymbol(argv)->s_name);
  }
  if(fake<=0)return 8;

  argc--; argv++;

  switch(argc)
    {
    case 1: /* just the size */
      {
        if(A_FLOAT!=argv->a_type)return 1;
        size=atom_getint(argv);
        if(size<0)return 2;
      }
      break;
    case 2: /* x*y; assume GL_RGBA */
      {
        if((A_FLOAT!=(argv+0)->a_type)||(A_FLOAT!=(argv+1)->a_type))return 3;
        xsize=atom_getint(argv);
        ysize=atom_getint(argv+1);
      }
      break;
    case 3:
      {
        if((A_FLOAT!=(argv+0)->a_type)||(A_FLOAT!=(argv+1)->a_type))return 3;
        xsize=atom_getint(argv);
        ysize=atom_getint(argv+1);
        if(A_FLOAT==(argv+2)->a_type)
          {
            int csize=atom_getint(argv+2);
            switch(csize)
              {
              case 1:
                color = GL_LUMINANCE;
                break;
              case 2:
                color = GL_YUV422_GEM;
                break;
              case 4:
                color = GL_RGBA;
                break;
              default:
                return 4;
                break;
              }
          } else { // the 4th argument is a symbol: either "RGBA", "YUV" or "Grey"
          char c=atom_getsymbol(argv+2)->s_name[0];
          switch(c)
            {
            case 'G': case 'g':
              color = GL_LUMINANCE;
              break;
            case 'Y': case 'y':
              color = GL_YUV422_GEM;
              break;
            case 'R': case 'r':
              color = GL_RGBA;
              break;
            default:
              return 4;
              break;
            }
        }
      }
      break;
    default:
      return 5;
    }
  
  if (xsize <= 0 || ysize <= 0){
    return 3;
  }

  imageStruct dummy;
  dummy.setCsizeByFormat(color);

  m_size = (size)?(size):(xsize * ysize * dummy.csize);
	
  logpost(NULL, 5, "%dx%dx%d: %d",
       xsize,ysize,dummy.csize, m_size);

#ifdef _WIN32
  error("no shared memory on w32!");
#else

  /* get a new segment with the size specified by the user
   * OR an old segment with the size specified in its header
   * why: if somebody has already created the segment with our key
   * we want to reuse it, even if its size is smaller than we requested
   */
  errno=0;
  shm_id = shmget(fake,m_size+sizeof(t_pixshare_header), IPC_CREAT | 0666);

  if((shm_id<0) && (EINVAL==errno)){
    errno=0;
    // the segment already exists, but is smaller than we thought!
    int id = shmget(fake,sizeof(t_pixshare_header),0666);
    if(id>0){ /* yea, we got it! */
      t_pixshare_header*h=(t_pixshare_header*)shmat(id,NULL,0666);
      /* read the size of the blob from the shared segment */
      if(h&&h->size){
        error("someone was faster: only got %d bytes instead of %d",
              h->size, m_size);
        m_size=h->size;

        /* so free this shm-segment before we re-try with a smaller size */
        shmdt(h);

        /* now get the shm-segment with the correct size */
        shm_id = shmget(fake,m_size+sizeof(t_pixshare_header), IPC_CREAT | 0666);
      }
    }
  }

  if(shm_id>0){
    /* now that we have a shm-segment, get the pointer to the data */
    shm_addr = (unsigned char*)shmat(shm_id,NULL,0666);

    if (!shm_addr) return 6;
    shmctl(shm_id,IPC_STAT,&shm_desc);
    /* write the size into the shm-segment */
    t_pixshare_header *h=(t_pixshare_header *)shm_addr;
    h->size = (shm_desc.shm_segsz-sizeof(t_pixshare_header));
    
    logpost(NULL, 5, "shm:: id(%d) segsz(%d) cpid (%d) mem(0x%X)",
         shm_id,shm_desc.shm_segsz,shm_desc.shm_cpid, shm_addr);
  } else {
    error("couldn't get shm_id: error %d", errno);
  }
#endif /* _WIN32 */
  return 0;
}


void pix_share_write :: render(GemState *state)
{
  if (!state)return;
  pixBlock*img=NULL;
  state->get(GemState::_PIX, img);
  if(!img) return;

#ifndef _WIN32
  if(shm_id>0){
    imageStruct *pix = &img->image;
    size_t size=pix->xsize*pix->ysize*pix->csize;
	
    if (!shm_addr){
      error("no shmaddr");
      return;
    }

    if (size<=m_size) {
      t_pixshare_header *h=(t_pixshare_header *)shm_addr;
      h->size =m_size;
      h->xsize=pix->xsize;
      h->ysize=pix->ysize;
      h->format=pix->format;
      h->upsidedown=pix->upsidedown;
      memcpy(shm_addr+sizeof(t_pixshare_header),pix->data,size);
    }
    else{
      error("input image too large: %dx%dx%d=%d>%d", 
           pix->xsize, pix->ysize, pix->csize, 
           pix->xsize*pix->ysize*pix->csize, 
           m_size);
    }
  }
#endif /* WIN32 */			
}

void pix_share_write :: obj_setupCallback(t_class *classPtr)
{
   class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_share_write::setMessCallback),
		  gensym("set"), A_GIMME, A_NULL);
}

void pix_share_write :: setMessCallback(void *data, t_symbol *s, int argc, t_atom *argv)
{
  if(argc){
    int err  = 0;
    //GetMyClass(data)->freeShm();
    err = GetMyClass(data)->getShm(argc, argv);
    if(err)GetMyClass(data)->error("couldn't get new shared memory block! %d", err);
  } else
    GetMyClass(data)->error("no args given!");
}
