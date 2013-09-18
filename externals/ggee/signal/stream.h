/* (C) Guenter Geiger 1999 */

/*!
 * \file
 * \author Guenter Geiger
 *
 * \brief Definitions used by both streamin~ and streamout~
 *
 * \todo This code does not honor network byte order.
 */

#ifndef __PD_STREAM_H
#define __PD_STREAM_H

#include <inttypes.h>

/*!
 * \brief Format identifiers for frames
 */
enum tag_format {
    SF_FLOAT  = 1,
    SF_DOUBLE = 2,
    SF_8BIT   = 10,
    SF_16BIT  = 11,
    SF_32BIT  = 12,
    SF_ALAW   = 20,
    SF_MP3    = 30
};

#define SF_SIZEOF(a) (a == SF_FLOAT ? sizeof(t_float) : a == SF_16BIT ? sizeof(short) : 1)

#ifdef __GNUC__
#define PACKED __attribute__ ((packed))
#endif

/*!
 * \brief 16-byte frame header
 *
 * \note Version 1
 */
typedef struct _tag {
     /*! Frame header version.  Currently ignored in streamin, but always set
      *  to 1 for streamout.
      *
      *  \todo Add version checking on incoming frames.  However, this could
      *        break existing uses of the external. */
     char version;
     /*! This field identifies the type of data is in the frame payload */
     char format;
     /*! ??? */
     int32_t count;
     /*! ??? */
     char channels;
     /*! This indicates the full size of the frame.  It is basically
      *  ( sizeof(t_tag) + payload length ). */
     int32_t framesize;
     /*! ??? */
     char extension[5];
} PACKED t_tag;


/*!
 * \brief A complete frame
 */
typedef struct _frame {
     /*! This is the frame header that contains the metadata about the frame */
     t_tag tag;
     /*! This buffer stores the frame data payload.  The amount of data in this
      *  buffer is the tag.framesize - sizeof(t_tag).  Its contents can be
      *  interpreted according to the tag.version. */
     char *data;
} t_frame;

#endif /* __PD_STREAM_H */
