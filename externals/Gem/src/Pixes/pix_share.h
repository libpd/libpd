/*
 *  pix_share_write.h
 *  GEM_darwin
 *
 *  Created by cgc on 9/29/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _INCLUDE__GEM_PIXES_PIX_SHARE_H_
#define _INCLUDE__GEM_PIXES_PIX_SHARE_H_

#include "Base/GemBase.h"
#include <sys/types.h>
#ifndef _WIN32
# include <sys/ipc.h>
# include <sys/shm.h>
#endif

// this is the header of the shared-memory segment
typedef struct _pixshare_header {
  size_t    size;      // total size of the shared-memory segment (without header)
  GLint     xsize;     // width of the image in the shm-segment
  GLint     ysize;     // height of the image in the shm-segment
  GLenum    format;    // format of the image (calculate csize,... from that)
  GLboolean upsidedown;// is the stored image swapped?
} t_pixshare_header;

#endif

