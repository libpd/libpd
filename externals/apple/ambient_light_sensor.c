/* --------------------------------------------------------------------------*/
/*                                                                           */
/* read the ambient light sensor on Apple Mac OS X                           */
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

/*------------------------------------------------------------------------------
 *  CLASS DEF
 */

static t_class *ambient_light_sensor_class;

typedef struct _ambient_light_sensor {
    t_object            x_obj;
    t_symbol*           sensor_name;
    
    io_service_t        io_service;
    io_connect_t        io_connect;

    t_outlet*           data_outlet;
    t_outlet*           status_outlet;
} t_ambient_light_sensor;


enum {
	kGetSensorReadingID = 0, // getSensorReading(int *, int *)  
	kGetLEDBrightnessID = 1, // getLEDBrightness(int, int *)  
	kSetLEDBrightnessID = 2, // setLEDBrightness(int, int, int *)  
	kSetLEDFadeID = 3,       // setLEDFade(int, int, int, int *)  
	// other firmware-related functions  
	verifyFirmwareID = 4,    // verifyFirmware(int *)  
	getFirmwareVersionID = 5,// getFirmwareVersion(int *)  
	// other flashing-related functions  
	// ... 
}; 

/*------------------------------------------------------------------------------
 * IMPLEMENTATION                    
 */

static void ambient_light_sensor_output(t_ambient_light_sensor* x)
{
	DEBUG(post("ambient_light_sensor_output"););
    kern_return_t kernResult;  
	t_float left = 0, right = 0;
    t_atom output_atoms[2];

    if(! x->io_connect) return;

#if !defined(__LP64__)
	// Check if Mac OS X 10.5/10.6 API is available...
    SInt32 MacVersion;
    if ((Gestalt(gestaltSystemVersion, &MacVersion) == noErr) && (MacVersion >= 0x1060)) {
		// ...and use it if it is.
#endif
#ifdef AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER
        uint64_t inputValues[0];
        uint32_t inputCount = 0;
        uint64_t outputValues[2];
        uint32_t outputCount = 2;
        kernResult = IOConnectCallScalarMethod(x->io_connect,
                                               kGetSensorReadingID,
                                               inputValues,
                                               inputCount,
                                               outputValues,
                                               &outputCount);
        left = (t_float) (outputValues[0] / 2000.0);
        right = (t_float) (outputValues[1] / 2000.0);
#endif
#if !defined(__LP64__)
	}
	else {
		// Otherwise fall back to older API.
        IOItemCount scalarInputCount = 0;  
        IOItemCount scalarOutputCount = 2;  
        SInt32 out0, out1;
        kernResult = IOConnectMethodScalarIScalarO(x->io_connect, 
                                                   kGetSensorReadingID,  
                                                   scalarInputCount, 
                                                   scalarOutputCount, 
                                                   &out0, 
                                                   &out1);
        left = (t_float) (out0 / 2000.0);
        right = (t_float) (out1 / 2000.0);
	}    
#endif
    if( kernResult == KERN_SUCCESS)
    {
        SETFLOAT(output_atoms, left);
        SETFLOAT(output_atoms + 1, right);
        outlet_list(x->data_outlet, &s_list, 2, output_atoms);
    }
    else if(kernResult == kIOReturnBusy)
            pd_error(x,"[ambient_light_sensor]: device busy");
    else
        pd_error(x,"[ambient_light_sensor]: could not read device");
}


static void ambient_light_sensor_info(t_ambient_light_sensor* x)
{
    t_atom output_atom;
    SETSYMBOL(&output_atom, x->sensor_name);
    outlet_anything(x->status_outlet, gensym("sensor"), 1, &output_atom);
}


static void ambient_light_sensor_free(t_ambient_light_sensor* x)
{
    IOServiceClose(x->io_connect);
}


static void *ambient_light_sensor_new(void) 
{
	DEBUG(post("ambient_light_sensor_new"););
	t_ambient_light_sensor *x = (t_ambient_light_sensor *)pd_new(ambient_light_sensor_class);
    kern_return_t kernResult;

    x->io_service = IOServiceGetMatchingService(kIOMasterPortDefault, 
                                                IOServiceMatching("AppleLMUController"));
    if(x->io_service)
    {
        logpost(x, 4, "[ambient_light_sensor]: found AppleLMUController");
        x->sensor_name = gensym("AppleLMUController");
    }
    else
    {
        error("[ambient_light_sensor]: AppleLMUController not found, trying IOI2CDeviceLMU");
        x->io_service = IOServiceGetMatchingService(kIOMasterPortDefault, 
                                                    IOServiceMatching("IOI2CDeviceLMU"));
        if(x->io_service)
        {
            x->sensor_name = gensym("IOI2CDeviceLMU");
            logpost(x, 4, "[ambient_light_sensor]: found IOI2CDeviceLMU");
        }
        else
            pd_error(x,"[ambient_light_sensor]: no sensor found");
    }
	kernResult = IOServiceOpen(x->io_service, mach_task_self(), 0, &x->io_connect);  
    IOObjectRelease(x->io_service);  
	if (kernResult != KERN_SUCCESS) 
    {
		error("[ambient_light_sensor]: IOServiceOpen(): %d", kernResult);  
	}

	x->data_outlet = outlet_new(&x->x_obj, &s_list);
	x->status_outlet = outlet_new(&x->x_obj, &s_anything);

	return (x);
}

void ambient_light_sensor_setup(void) 
{
	ambient_light_sensor_class = class_new(gensym("ambient_light_sensor"), 
                                           (t_newmethod)ambient_light_sensor_new,
                                           (t_method)ambient_light_sensor_free,
                                           sizeof(t_ambient_light_sensor), 
                                           CLASS_DEFAULT, 
                                           0);
	/* add inlet datatype methods */
	class_addbang(ambient_light_sensor_class,(t_method) ambient_light_sensor_output);
	class_addmethod(ambient_light_sensor_class,(t_method) ambient_light_sensor_info, 
                    gensym("info"), 0);
}
