/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemgui written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"
#include "iemgui.h"

char my_iemgui_black_vscale_gif[]="R0lGODlhDgCCAJEAANnZ2QAAAP///////yH5BAEAAAAALAAAAAAOAIIAAAL/hI+pa0EBICIIdkRYBCEBgIigEHbEBQSFsCPOIghGxF0EwQkzg6AAEQTfIoxiE3xMvQgK4WMERBDsuIg7owAQEQEEgGKDgRHsuAgICmHHRUBQCB8jguBj6nL7AxITPkZABMGOu4gzCuFHHMUGACP4cRBxByQARBB8NIpN8DF1uf1JTPgYAREE/yIgguBfBEQQjIiwCEJC8C8CIgj+RUAEwceIIPiYutz+gMSEjxEQQfDRIjKNEgCABRCACIKPEJEJlBA+GsUm+Ji63P4kJnyMgAiCfxGISHEIdkRYBCEh+BcBEQT/IiCC4GNEEHxMXW5fkgDAgmBHhEUQEgCICAph5UdcQFAIO+IsgmBE3EUQnDAzCAoQQfAtwig2wcfU5fYkMQAsAAhIQAQEBQmCHwERQAnhR0AEUELYERYBlAAADICQQAQAUAIAMDCCGRvgCD6mLrcriQFgAUBAAiIgKEgQ/AiIAEoIO8IigBLCj4AIoITwIyACKAGAgQElMAAAC4ITQfAxdbldSRyAI9gRFwGUAMAAgJBABABQHAADAwICEAFBQYLgR0AEUAIAAwNKYAAAFgQnguBj6nL7OxIHLgCAkABEBBASAIgAQhIAEUBAAiAiCEgYRBjFAAMAoDhwQfAxdbkdSQEAOw==";
char my_iemgui_black_test___gif[]="R0lGODlhDgCEAJEAANnZ2QAAAP///////yH5BAEAAAAALAAAAAAOAIQAAAL/hI+pa0EBICIIdkRYBCEBgIigEHbEBQSFsCPOIghGxF0EwQkzg6AAEQTfIoxiE3xMXW5/EhM+RkAEwY6LuDMKABERQAAoNhgYwY6LgKAQdlwEBIXwMSIIPqYutz8gMeFjBEQQ7LiLOKMQfsRRbAAwgh8HEXdAAkAEwUej2AQfU5fbn8SEjxEQQfAvAiII/kVABMGICIsgJAT/IiCC4F8ERBB8jAiCj6nL7Q9ITPgYAREEHy0i0ygBAFgAAYgg+AgRmUAJ4aNRbIKPqcvtT2LCxwiIIPgXgYgUh2BHhEUQEoJ/ERBB8C8CIgg+RgTBx9Tl9iUJACwIdkRYBCEBgIig5RB2xAUEhbAjziIIRsRdBMEJM4OgABEE3yKMYhN8TF1uTxIDwAKAgAREQFCQIPgREAGUEH4ERAAlhB1hEUAJAMAACAlEAAAlAAADI5ixAY7gY+pyu5IYABYABCQgAoKCBMGPgAighLAjLAIoIfwIiABKCD8CIoASABgYUAIDALAgOBEEH1OX25XEATiCHXERQAkADAAICUQAAMUBMDAgIAAREBQkCH4ERAAlADAwoAQGAGBBcCIIPqYut78jceACAAgJQEQAIQGACCAkARABBCQAIoKAhEGEUQwwAACKAxcEH1OXmwoAOw";
char my_iemgui_black_hlscale_gif[]="R0lGODlhgwAaAJEAANnZ2QAAAP///////yH5BAEAAAAALAAAAACDABoAAAL/hI+py+0Po5y02ouz3rz7D4biKBIJ4WPqcvsDkhD+RQQiUEz4FxGBiAgZEhM+pi63P4kJ3yIgguBDBEQQfItARMRD4hB8CMQLgo8QmZmZDolD8BEiMikhfIhAREREggTBh0C8IPgIkZmZ6ZAgQfARIhIpIXyLiEBEhBBJCJtiE3yg2AQfKDbBt4hABIoJHyMQjRLCR4hIowARBB8iAhGBJIQPgXhB8BEiMzNTIgnhXwQiUkL4GIGIiIckhA+BeEHwESIzM1MiCeFbBKJRAiP4EAERBB9TLYLgY+py+wOSEL5RbIJvEYEIFBM+pi63P4xy0gpITPgRcXcUE/5FBCJQ/0z4FxGIQDHhX0QgAsWEfxGBiIgMSWAAAEZICD5EQATBhwiIIPgQAREEHyIgguBDBEQQfIiACIIPERBB8CECIgh2RAQEJSH4GIERBB8vIo0SGMHHCLygED5eRBolMIIPERBB8CECIghGhEUEAQkzM6AEAGBG8BEi0iiBEXyMwAtKQvARItIogRF8iIAIgg8REEEwIiwiCEiYmQWFsCMg4u5IQvgWAREE/yIQg4IEwb8IxKAgQfAhAiIIPkRABMGIsIggIGFmEBTCj4CIOyMJ4V8ERBB8i0AESmAE3yIQgRIYwYcIiCD4EAERBCPCIoKAhJlFEHyIgAiCDxGIRgmM4FVDBKJRAiP4EIFolMAIPkRABMGHCIgg2BEREJSE4APFJtgRcWcUm+BbRCBSbIJvEYFIsQm+RQQiUEz4FxEEH1OX2x9GOWm1F2e9efcfDMWRLM0TfYgCADs=";
char my_iemgui_black_hrscale_gif[]="R0lGODlhhgAbAJEAANnZ2QAAAP///////yH5BAEAAAAALAAAAACGABsAAAL/hI+py+0Po5y02ouz3rz7D4biSG5EHIJvEYEIFBM+RhB8TF1uf0ASwsfUjCD4FwERBB8iIILgW0QQfExdbn8SEz6mYgTBx4vIpITwISICERFCCBkSwkeITKAQfkQEwcdUjKCE8DEC8SghfAiIQESEEEKGhPARIhMohB8QcXd3SEz4EBaBaCQh/ItANEoI/yICESgmfKPYBB8oNsEHik3wIwg+pm4EwccIxKAQPkJEZmZKiITwESITKIRvEYiIiABJYAQfIiCC4EMEIlJC+Ji6GgQfITKBQvgWgYiIyJCY8C8iECk2wb8Igo+py+0PSEL4mLrc/jDKSesiMeFfRCAC/8WEfxGBCBQT/kUEIlBM+BcRiEAx4UfE3R3FgQsAICQQQfAhAiIIPkRABMGHCIgg+BABEQQfIiCC4EMERBB8iIAIgg8REEEwIiDi7gBJABYARnACIswMSAIj+BABEQQfI/CCQvh4EWmUwAg+RuAFhfAxAtEoAVgAGMEJiDAzIAmM4EMERBB8jMALSkLwESLSKIERfIzAC0pC8C0C0SgBWAAYwQmIMDMgCYzgQwREEPyLQAwKEgT/IhCDggTBvwjEoCBB8CPuzigBWAAYwQmIMDMgCYzgQwREEHyLQARKYATfIhCBEhjBtwhEoARGsCPu7oDiwAUAEBKIIPgQAREEH1YiIILgQwSiUQIj+BCBaJTACD5EIBolMAAAI/iYuhEUwr+IQASKCd8oNsG3iECk2ATfIgKRYhN8iwgEik3wMXW5/WGUk1Z7cdabd//BUBzJ0jzRVF1NogA7";

char my_iemgui_base64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int my_iemgui_color_hex[]=
{
  16579836, 10526880, 4210752, 16572640, 16572608,
    16579784, 14220504, 14220540, 14476540, 16308476,
    14737632, 8158332, 2105376, 16525352, 16559172,
    15263784, 1370132, 2684148, 3952892, 16003312,
    12369084, 6316128, 0, 9177096, 5779456,
    7874580, 2641940, 17488, 5256, 5767248
};

int simularca_color_hex[]=
{
  16003312, 3952892, 2684148, 15263784, 16559172, 16525352, 8158332
};

void my_iemgui_change_scale_col(char *gif, int color)
{
  int i;
  unsigned int red = (color & 0xff0000) >> 16;
  unsigned int green = (color & 0xff00) >> 8;
  unsigned int blue = color & 0xff;
  t_my_iemgui_3u4 hexbyte;
  
  hexbyte.b3.dummy = 0;
  
  hexbyte.b3.byte1 = 0xD6;
  hexbyte.b3.byte2 = red;
  hexbyte.b3.byte3 = green;
  i = hexbyte.h4.hex1;
  gif[20] = my_iemgui_base64[i];
  i = hexbyte.h4.hex2;
  gif[21] = my_iemgui_base64[i];
  i = hexbyte.h4.hex3;
  gif[22] = my_iemgui_base64[i];
  i = hexbyte.h4.hex4;
  gif[23] = my_iemgui_base64[i];
  
  hexbyte.b3.byte1 = blue;
  hexbyte.b3.byte2 = 0xFF;
  hexbyte.b3.byte3 = 0xFF;
  i = hexbyte.h4.hex1;
  gif[24] = my_iemgui_base64[i];
  i = hexbyte.h4.hex2;
  gif[25] = my_iemgui_base64[i];
  i = hexbyte.h4.hex3;
  gif[26] = my_iemgui_base64[i];
  i = hexbyte.h4.hex4;
  gif[27] = my_iemgui_base64[i];
}


// Georg Holzmann:
#ifdef IEMGUI_SINGLE_OBJ
// for single externals disable the iemgui object
#else
// build as library

static t_class *iemgui_class;

static void *iemgui_new(void)
{
  t_object *x = (t_object *)pd_new(iemgui_class);
  
  return (x);
}

//  void simularca_2d_setup(void);
//  void simularca_3d_setup(void);
void room_sim_2d_setup(void);
void room_sim_3d_setup(void);
//  void simularca_3d_no_z_clip_setup(void);
void cube_sphere_setup(void);
void sym_dial_setup(void);
void iem_image_setup(void);
void iem_vu_setup(void);
void hfadl_scale_setup(void);
void hfadr_scale_setup(void);
void vfad_scale_setup(void);
//  void vfad_test_setup(void);
void numberbox_matrix_setup(void);
void iem_event_setup(void);
//  void toggle_matrix_setup(void);

/* ------------------------ setup routine ------------------------- */

void iemgui_setup(void)
{
  //  simularca_2d_setup();
  //  simularca_3d_setup();
  room_sim_2d_setup();
  room_sim_3d_setup();
  //  simularca_3d_no_z_clip_setup();
  cube_sphere_setup();
  sym_dial_setup();
  iem_image_setup();
  iem_vu_setup();
  hfadl_scale_setup();
  hfadr_scale_setup();
  vfad_scale_setup();
  //  vfad_test_setup();
  numberbox_matrix_setup();
  iem_event_setup();
  //  toggle_matrix_setup();
  
  post("iemgui (R-1.17) library loaded!   (c) Thomas Musil 11.2006");
  post("   musil%ciem.at iem KUG Graz Austria", '@');
}

#endif // library
