/* (C) Guenter Geiger 1999 */

#define SF_FLOAT  1
#define SF_16BIT  2
#define SF_8BIT   3
#define SF_MP3    4

#define SF_SIZEOF(a) (a == SF_FLOAT ? sizeof(t_float) : \
                     a == SF_16BIT ? sizeof(short) : 1)

