#ifndef PWC_IOCTL_H
#define PWC_IOCTL_H

/* (C) 2001-2002 Nemosoft Unv.    webcam@smcc.demon.nl
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*         This is pwc-ioctl.h belonging to PWC 8.6                        */

/* 
   Changes
   2001/08/03  Alvarado   Added ioctl constants to access methods for 
                          changing white balance and red/blue gains
 */

/* These are private ioctl() commands, specific for the Philips webcams.
   They contain functions not found in other webcams, and settings not
   specified in the Video4Linux API. 
   
   The #define names are built up like follows:
   VIDIOC		VIDeo IOCtl prefix
         PWC		Philps WebCam
            G           optional: Get
            S           optional: Set
             ... 	the function
 */




/* The frame rate is encoded in the video_window.flags parameter using
   the upper 16 bits, since some flags are defined nowadays. The following
   defines provide a mask and shift to filter out this value.
   
   In 'Snapshot' mode the camera freezes its automatic exposure and colour 
   balance controls.
 */
#define PWC_FPS_SHIFT		16
#define PWC_FPS_MASK		0x00FF0000
#define PWC_FPS_FRMASK		0x003F0000
#define PWC_FPS_SNAPSHOT	0x00400000



struct pwc_probe
{
	char name[32];
	int type;
};


/* pwc_whitebalance.mode values */
#define PWC_WB_INDOOR		0
#define PWC_WB_OUTDOOR		1
#define PWC_WB_FL		2
#define PWC_WB_MANUAL		3
#define PWC_WB_AUTO		4

/* Used with VIDIOCPWC[SG]AWB (Auto White Balance). 
   Set mode to one of the PWC_WB_* values above.
   *red and *blue are the respective gains of these colour components inside 
   the camera; range 0..65535
   When 'mode' == PWC_WB_MANUAL, 'manual_red' and 'manual_blue' are set or read; 
   otherwise undefined.
   'read_red' and 'read_blue' are read-only.
*/   
   
struct pwc_whitebalance
{
	int mode;
	int manual_red, manual_blue;	/* R/W */
	int read_red, read_blue;	/* R/O */
};

/* 
   'control_speed' and 'control_delay' are used in automatic whitebalance mode,
   and tell the camera how fast it should react to changes in lighting, and 
   with how much delay. Valid values are 0..65535.
*/
struct pwc_wb_speed
{
	int control_speed;
	int control_delay;

};

/* Used with VIDIOCPWC[SG]LED */
struct pwc_leds
{
	int led_on;			/* Led on-time; range = 0..25000 */
	int led_off;			/* Led off-time; range = 0..25000  */
};



 /* Restore user settings */
#define VIDIOCPWCRUSER		_IO('v', 192)
 /* Save user settings */
#define VIDIOCPWCSUSER		_IO('v', 193)
 /* Restore factory settings */
#define VIDIOCPWCFACTORY	_IO('v', 194)

 /* You can manipulate the compression factor. A compression preference of 0
    means use uncompressed modes when available; 1 is low compression, 2 is
    medium and 3 is high compression preferred. Of course, the higher the
    compression, the lower the bandwidth used but more chance of artefacts
    in the image. The driver automatically chooses a higher compression when
    the preferred mode is not available.
  */
 /* Set preferred compression quality (0 = uncompressed, 3 = highest compression) */
#define VIDIOCPWCSCQUAL		_IOW('v', 195, int)
 /* Get preferred compression quality */
#define VIDIOCPWCGCQUAL		_IOR('v', 195, int)


 /* This is a probe function; since so many devices are supported, it
    becomes difficult to include all the names in programs that want to
    check for the enhanced Philips stuff. So in stead, try this PROBE;
    it returns a structure with the original name, and the corresponding 
    Philips type.
    To use, fill the structure with zeroes, call PROBE and if that succeeds,
    compare the name with that returned from VIDIOCGCAP; they should be the
    same. If so, you can be assured it is a Philips (OEM) cam and the type
    is valid.
 */    
#define VIDIOCPWCPROBE		_IOR('v', 199, struct pwc_probe)

 /* Set AGC (Automatic Gain Control); int < 0 = auto, 0..65535 = fixed */
#define VIDIOCPWCSAGC		_IOW('v', 200, int)
 /* Get AGC; int < 0 = auto; >= 0 = fixed, range 0..65535 */
#define VIDIOCPWCGAGC		_IOR('v', 200, int)
 /* Set shutter speed; int < 0 = auto; >= 0 = fixed, range 0..65535 */
#define VIDIOCPWCSSHUTTER	_IOW('v', 201, int)

 /* Color compensation (Auto White Balance) */
#define VIDIOCPWCSAWB           _IOW('v', 202, struct pwc_whitebalance)
#define VIDIOCPWCGAWB           _IOR('v', 202, struct pwc_whitebalance)

 /* Auto WB speed */
#define VIDIOCPWCSAWBSPEED	_IOW('v', 203, struct pwc_wb_speed)
#define VIDIOCPWCGAWBSPEED	_IOR('v', 203, struct pwc_wb_speed)

 /* LEDs on/off/blink; int range 0..65535 */
#define VIDIOCPWCSLED           _IOW('v', 205, struct pwc_leds)
#define VIDIOCPWCGLED           _IOR('v', 205, struct pwc_leds)

  /* Contour (sharpness); int < 0 = auto, 0..65536 = fixed */
#define VIDIOCPWCSCONTOUR	_IOW('v', 206, int)
#define VIDIOCPWCGCONTOUR	_IOR('v', 206, int)

  /* Backlight compensation; 0 = off, otherwise on */
#define VIDIOCPWCSBACKLIGHT	_IOW('v', 207, int)
#define VIDIOCPWCGBACKLIGHT	_IOR('v', 207, int)

  /* Flickerless mode; = 0 off, otherwise on */
#define VIDIOCPWCSFLICKER	_IOW('v', 208, int)
#define VIDIOCPWCGFLICKER	_IOR('v', 208, int)  

  /* Dynamic noise reduction; 0 off, 3 = high noise reduction */
#define VIDIOCPWCSDYNNOISE	_IOW('v', 209, int)
#define VIDIOCPWCGDYNNOISE	_IOR('v', 209, int)

#endif
