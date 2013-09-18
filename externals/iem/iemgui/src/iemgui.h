/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemgui written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#ifndef __IEMGUI_H__
#define __IEMGUI_H__

//t_symbol *iemgui_key_sym2=0;

typedef struct _my_iemgui_4hex
{
  unsigned int dummy  : 8;
  unsigned int hex4   : 6;
  unsigned int hex3   : 6;
  unsigned int hex2   : 6;
  unsigned int hex1   : 6;
} t_my_iemgui_4hex;

typedef struct _my_iemgui_3byte
{
  unsigned int dummy    : 8;
  unsigned int byte3    : 8;
  unsigned int byte2    : 8;
  unsigned int byte1    : 8;
} t_my_iemgui_3byte;

typedef union _my_iemgui_3u4
{
  t_my_iemgui_4hex    h4;
  t_my_iemgui_3byte   b3;
} t_my_iemgui_3u4;

extern char my_iemgui_black_vscale_gif[];
extern char my_iemgui_black_test___gif[];
extern char my_iemgui_black_hlscale_gif[];
extern char my_iemgui_black_hrscale_gif[];
extern char my_iemgui_base64[];
extern int my_iemgui_color_hex[];
extern int simularca_color_hex[];

extern void my_iemgui_change_scale_col(char *gif, int color);

#endif
