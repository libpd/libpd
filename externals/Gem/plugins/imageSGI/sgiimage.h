
/*
 *  porting to Windows NT
 *              Masayuki Mastumoto - 1995
 */
/*
 * Modified so that it doesn't exit on error.
 * Added support for reading grayscale, rgb, and rgba images
 *				Mark Danks - 1998
 */

#ifndef __SGIIMAGE_H__
#define __SGIIMAGE_H__

#define int32 int


extern unsigned int32 *getLongImage(const char *textureFile, int32 *xsize, int32 *ysize, int32 *csize);
extern int longstoimage(unsigned int32 *lptr, int32 xsize, int32 ysize, int32 zsize, const char *name);
extern unsigned int32 *longimagedata(const char *name);
extern int sizeofimage(const char *name, int32 *xsize, int32 *ysize, int32 *csize);

#endif  /* __SGIIMAGE_H__ */
