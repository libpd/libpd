/* --------------------------------------------------------------------------*/
/*                                                                           */
/* read the sudden motion sensor on Apple Mac OS X                           */
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

static t_class *sudden_motion_sensor_class;

typedef struct _sudden_motion_sensor {
    t_object            x_obj;
    t_symbol*           sensor_name;
    
    io_connect_t        io_connect;
    int                 kernel_function;
    int                 data_size;

    t_outlet*           data_outlet;
    t_outlet*           status_outlet;
} t_sudden_motion_sensor;

struct data {
	char x;
	char y;
	char z;
	char pad[57];
};

/*------------------------------------------------------------------------------
 * IMPLEMENTATION                    
 */

static void sudden_motion_sensor_output(t_sudden_motion_sensor* x)
{
	DEBUG(post("sudden_motion_sensor_output"););
    t_atom output_atoms[3];

    kern_return_t kern_return;
	mach_port_t mach_port;
	io_iterator_t io_iterator;
	io_object_t io_object;
	io_connect_t  io_connect;
	int kernel_function, sizeof_data;
	
	IOItemCount structureInputSize;
	IOByteCount structureOutputSize;

	struct data inputStructure;
	struct data outputStructure;

	kern_return = IOMasterPort(MACH_PORT_NULL, &mach_port);
	if (kern_return != KERN_SUCCESS) 
    {
		pd_error(x, "[sudden_motion_sensor]: cannot get mach_port");
		return;
	}

	//PowerBookG4, iBookG4
	kern_return = IOServiceGetMatchingServices(mach_port, IOServiceMatching("IOI2CMotionSensor"), &io_iterator);
	if (kern_return == KERN_SUCCESS && io_iterator != 0) 
    {
        x->sensor_name = gensym("IOI2CMotionSensor");
		sizeof_data = 60;
		kernel_function = 21;
		goto FOUND_SENSOR;
	}
	//
	kern_return = IOServiceGetMatchingServices(mach_port, IOServiceMatching("PMUMotionSensor"), &io_iterator);
	if(kern_return == KERN_SUCCESS && io_iterator != 0) 
    {
        x->sensor_name = gensym("PMUMotionSensor");
		sizeof_data = 60;
		kernel_function = 21;
		goto FOUND_SENSOR;
	}
	// MacBook Pro
	kern_return = IOServiceGetMatchingServices(mach_port, IOServiceMatching("SMCMotionSensor"), &io_iterator);
	if(kern_return == KERN_SUCCESS && io_iterator != 0) 
    {
        x->sensor_name = gensym("SMCMotionSensor");
		sizeof_data = 40;
		kernel_function = 5;
		goto FOUND_SENSOR;
	}
	
	pd_error(x,"[sudden_motion_sensor]: cannot find motionsensor\n");
	return;


FOUND_SENSOR:
	io_object = IOIteratorNext(io_iterator);
	IOObjectRelease(io_iterator);
	
	if(!io_object)
    {
		pd_error(x,"[sudden_motion_sensor]: No motion sensor available.");
        return;
    }

	kern_return = IOServiceOpen(io_object, mach_task_self(), 0, &io_connect);
	IOObjectRelease(io_object);

	if(kern_return != KERN_SUCCESS) 
    {
		pd_error(x,"[sudden_motion_sensor]: Could not open motion sensor device.");
        return;
    }

	structureInputSize = sizeof_data;	//sizeof(struct data);
	structureOutputSize = sizeof_data;	//sizeof(struct data);

	memset(&inputStructure, 0, sizeof(inputStructure));
	memset(&outputStructure, 0, sizeof(outputStructure));

#if !defined(__LP64__)
	// Check if Mac OS X 10.5/10.6 API is available...
    SInt32 MacVersion;
    if ((Gestalt(gestaltSystemVersion, &MacVersion) == noErr) && (MacVersion >= 0x1060)) {
		// ...and use it if it is.
#endif
#ifdef AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER
		kern_return = IOConnectCallStructMethod(
            io_connect,         // an io_connect_t returned from IOServiceOpen().
            kernel_function,	// selector of the function to be called via the user client.
            &inputStructure,	    // pointer to the input struct parameter.
            structureInputSize, // the size of the input structure parameter.
            &outputStructure,    // pointer to the output struct parameter.
            &structureOutputSize// pointer to the size of the output structure parameter.
            );
#endif
#if !defined(__LP64__)
	}
	else {
		// Otherwise fall back to older API.
        kern_return = IOConnectMethodStructureIStructureO(
            io_connect,          // an io_connect_t returned from IOServiceOpen().
            kernel_function,	 // an index to the function to be called via the user client.
            structureInputSize,  // the size of the input struct paramter.
            &structureOutputSize,// a pointer to the size of the output struct paramter.
            &inputStructure,     // a pointer to the input struct parameter.
            &outputStructure);   // a pointer to the output struct parameter.
	}
#endif

    if( kern_return == KERN_SUCCESS)
    {
        SETFLOAT(output_atoms, outputStructure.x);
        SETFLOAT(output_atoms + 1, outputStructure.y);
        SETFLOAT(output_atoms + 2, outputStructure.z);
        outlet_list(x->data_outlet, &s_list, 3, output_atoms);
    }
    else if(kern_return == kIOReturnBusy)
        pd_error(x,"[sudden_motion_sensor]: device busy");
    else
        pd_error(x,"[sudden_motion_sensor]: could not read device");
	IOServiceClose(io_connect);
}


static void sudden_motion_sensor_info(t_sudden_motion_sensor* x)
{
    t_atom output_atom;
    SETSYMBOL(&output_atom, x->sensor_name);
    outlet_anything(x->status_outlet, gensym("sensor"), 1, &output_atom);
}


static void *sudden_motion_sensor_new(void) 
{
	DEBUG(post("sudden_motion_sensor_new"););
	t_sudden_motion_sensor *x = (t_sudden_motion_sensor *)pd_new(sudden_motion_sensor_class);

    x->data_outlet = outlet_new(&x->x_obj, &s_list);
	x->status_outlet = outlet_new(&x->x_obj, &s_anything);

	return (x);
}

void sudden_motion_sensor_setup(void) 
{
	sudden_motion_sensor_class = class_new(gensym("sudden_motion_sensor"), 
                                           (t_newmethod)sudden_motion_sensor_new,
                                           NULL,
                                           sizeof(t_sudden_motion_sensor), 
                                           CLASS_DEFAULT, 
                                           0);
	/* add inlet datatype methods */
	class_addbang(sudden_motion_sensor_class,(t_method) sudden_motion_sensor_output);
	class_addmethod(sudden_motion_sensor_class,(t_method) sudden_motion_sensor_info, 
                    gensym("info"), 0);
}
