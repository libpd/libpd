/* --------------------------------------------------------------------------*/
/*                                                                           */
/* get/set the keyboard light level on Apple Mac OS X                        */
/* Written by Hans-Christoph Steiner <hans@eds.org>                         */
/*                                                                           */
/* Copyright (c) 2008 Free Software Foundation                               */
/*                                                                           */
/* This program is free software; you can redistribute it and/or             */
/* modify it under the terms of the GNU General Public License               */
/* as published by the Free Software Foundation; either version 2            */
/* of the License, or (at your option) any later version.                    */
/*                                                                           */
/* See file LICENSE for further informations on licensing terms.             */
/*                                                                           */
/* This program is distributed in the hope that it will be useful,           */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU General Public License for more details.                              */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with this program; if not, write to the Free Software Foundation,   */
/* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA */
/*                                                                           */
/* --------------------------------------------------------------------------*/

#include <mach/mach.h> 
#include <IOKit/IOKitLib.h> 
#include <CoreFoundation/CoreFoundation.h> 
#include <CoreServices/CoreServices.h>
#include <m_pd.h>

#define DEBUG(x)
//#define DEBUG(x) x 

#define DEFAULT_DELAYTIME  250
#define BRIGHTNESS_MAX     0xfff

/*------------------------------------------------------------------------------
 *  CLASS DEF
 */

static t_class *keyboard_light_class;

typedef struct _keyboard_light {
    t_object            x_obj;
    t_float             fade_time;
    io_service_t        io_service;
    io_connect_t        io_connect;
} t_keyboard_light;



enum {
	kGetSensorReadingID = 0, // getSensorReading(int *, int *)  
	kGetLEDBrightnessID = 1, // getLEDBrightness(int, int *)  
	kSetLEDBrightnessID = 2, // setLEDBrightness(int, int, int *)  
	kSetLEDFadeID = 3, // setLEDFade(int, int, int, int *)  
	// other firmware-related functions  
	verifyFirmwareID = 4,    // verifyFirmware(int *)  
	getFirmwareVersionID = 5,// getFirmwareVersion(int *)  
	// other flashing-related functions  
	// ... 
}; 

/*------------------------------------------------------------------------------
 * IMPLEMENTATION                    
 */

static void keyboard_light_output(t_keyboard_light* x)
{
	DEBUG(post("keyboard_light_output"););
    kern_return_t kernResult;  
    t_float brightness;

    if(! x->io_connect) return;

#if !defined(__LP64__)
	// Check if Mac OS X 10.5 API is available...
//	if (IOConnectCallScalarMethod != NULL) {
    if(0) {
		// ...and use it if it is.
#endif
        uint64_t inputValues[1] = {0};
        uint64_t inputCount = 1;
        uint64_t outputValues[1] = {0};
        uint32_t outputCount = 1;
        kernResult = IOConnectCallScalarMethod(x->io_connect,
                                               kGetLEDBrightnessID,  
                                               inputValues,
                                               inputCount,
                                               outputValues,
                                               &outputCount);
        brightness = (t_float)outputValues[0] / BRIGHTNESS_MAX;
#if !defined(__LP64__)
	}
	else {
		// Otherwise fall back to older API.
        IOItemCount scalarInputCount = 1;  
        IOItemCount scalarOutputCount = 1;  
        uint32_t in_brightness = 0;
        uint32_t out_brightness;
        kernResult = IOConnectMethodScalarIScalarO(x->io_connect, 
                                                   kGetLEDBrightnessID,  
                                                   scalarInputCount, 
                                                   scalarOutputCount, 
                                                   in_brightness, 
                                                   &out_brightness);
        brightness = (t_float)out_brightness / BRIGHTNESS_MAX;
	}    
#endif
    if( kernResult == KERN_SUCCESS)
        outlet_float(x->x_obj.ob_outlet, brightness);
    else if(kernResult == kIOReturnBusy)
        pd_error(x,"[keyboard_light]: device busy");
    else
        pd_error(x,"[keyboard_light]: could not read device");
}


static void keyboard_light_float(t_keyboard_light* x, t_float f)
{
	DEBUG(post("keyboard_light_float"););

    kern_return_t kernResult;  
    t_float brightness;

    if(!x->io_connect) return;
    
    brightness = f * BRIGHTNESS_MAX;
    if(brightness < 0)
        brightness = 0;
    else if(brightness > BRIGHTNESS_MAX)
        brightness = BRIGHTNESS_MAX;
    
#if !defined(__LP64__)
	// Check if Mac OS X 10.5/10.6 API is available...
    SInt32 MacVersion;
    if ((Gestalt(gestaltSystemVersion, &MacVersion) == noErr) && (MacVersion >= 0x1060)) {
		// ...and use it if it is.
#endif
#ifdef AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER
        uint64_t inputValues[3];
        uint32_t inputCount = 3;
        uint64_t outputValues[1];
        uint32_t outputCount = 1;
        inputValues[0] = 0;
        inputValues[1] = brightness;
        inputValues[2] = x->fade_time;
        kernResult = IOConnectCallScalarMethod(x->io_connect,
                                               kSetLEDFadeID,
                                               inputValues,
                                               inputCount,
                                               outputValues,
                                               &outputCount);
#endif
#if !defined(__LP64__)
	}
	else {
		// Otherwise fall back to older API.
        IOItemCount scalarInputCount = 3;
        IOItemCount scalarOutputCount = 1;
        SInt32 in_unknown = 0, in_brightness, out_brightness;
        in_brightness = brightness;
        kernResult = IOConnectMethodScalarIScalarO(x->io_connect, 
                                                   kSetLEDFadeID,  
                                                   scalarInputCount, 
                                                   scalarOutputCount, 
                                                   in_unknown,
                                                   in_brightness,
                                                   (SInt32) x->fade_time,
                                                   &out_brightness);
	}    
#endif

    if( kernResult != KERN_SUCCESS)
    {
        if(kernResult == kIOReturnBusy)
            pd_error(x,"[keyboard_light]: device busy");
        else
            pd_error(x,"[keyboard_light]: could not write to device");
    }
}


static void keyboard_light_free(t_keyboard_light* x)
{
	DEBUG(post("keyboard_light_free"););
}


static void *keyboard_light_new(t_float level, t_float fade_time) 
{
	DEBUG(post("keyboard_light_new"););
	t_keyboard_light *x = (t_keyboard_light *)pd_new(keyboard_light_class);
    kern_return_t kernResult;

    x->io_service = IOServiceGetMatchingService(kIOMasterPortDefault, 
                                                IOServiceMatching("AppleLMUController"));
    if(x->io_service)
    {
        logpost(x, 4, "[keyboard_light]: found AppleLMUController");
    }
    else
    {
        error("[keyboard_light]: AppleLMUController not found, trying IOI2CDeviceLMU");
        x->io_service = IOServiceGetMatchingService(kIOMasterPortDefault, 
                                                    IOServiceMatching("IOI2CDeviceLMU"));
        if(x->io_service)
        {
            logpost(x, 4, "[keyboard_light]: found IOI2CDeviceLMU");
        }
        else
            pd_error(x,"[keyboard_light]: no sensor found");
    }
	kernResult = IOServiceOpen(x->io_service, mach_task_self(), 0, &x->io_connect);  

    IOObjectRelease(x->io_service);  
	if (kernResult != KERN_SUCCESS) 
    {
		error("[keyboard_light]: IOServiceOpen(): %d", kernResult);  
	}

    x->fade_time = fade_time;
	floatinlet_new(&x->x_obj, &x->fade_time);
	outlet_new(&x->x_obj, &s_float);

    keyboard_light_float(x, level);

	return (x);
}

void keyboard_light_setup(void) 
{
	keyboard_light_class = class_new(gensym("keyboard_light"), 
                                     (t_newmethod)keyboard_light_new,
                                     (t_method)keyboard_light_free,
                                     sizeof(t_keyboard_light), 
                                     CLASS_DEFAULT, 
                                     A_DEFFLOAT, A_DEFFLOAT, 0);
	/* add inlet datatype methods */
	class_addbang(keyboard_light_class,(t_method) keyboard_light_output);
	class_addfloat(keyboard_light_class,(t_method) keyboard_light_float);
}
