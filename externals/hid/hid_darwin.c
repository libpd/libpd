#ifdef __APPLE__
/*
 *  Apple Darwin HID Manager support for Pd [hid] object
 *
 *  some code from SuperCollider3's SC_HID.cpp by Jan Truetzschler Falkenstein
 *
 *  Copyright (c) 2004 Hans-Christoph All rights reserved.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 */

/* struct IOHIDEventStruct */
/* { */
/*     IOHIDElementType	type; */
/*     IOHIDElementCookie	elementCookie; */
/*     SInt32		value; */
/*     AbsoluteTime	timestamp; */
/*     UInt32		longValueSize; */
/*     void *		longValue; */
/* }; */

/* typedef struct { */
/*         natural_t hi; */
/*         natural_t lo; */
/* } AbsoluteTime; */



#include <Carbon/Carbon.h>

#include "HID_Utilities_External.h"
#include "ImmrHIDUtilAddOn.h"

#include <IOKit/hid/IOHIDUsageTables.h>
#include <ForceFeedback/ForceFeedback.h>

#include <mach/mach.h>
#include <mach/mach_error.h>

#include "hid.h"

#define DEBUG(x)
//#define DEBUG(x) x 

/*==============================================================================
 *  GLOBAL VARS
 *======================================================================== */

/* store device pointers so I don't have to query them all the time */
pRecDevice device_pointer[MAX_DEVICES];


// this stuff is moving to the t_hid_element struct

/* store element pointers for elements that are not queued (absolute axes) */
//pRecElement element[MAX_DEVICES][MAX_ELEMENTS];
/* number of active elements per device */
//unsigned short element_count[MAX_DEVICES]; 

/*==============================================================================
 * FUNCTION PROTOTYPES
 *==============================================================================
 */

/* conversion functions */
static char *convertEventsFromDarwinToLinux(pRecElement element);

/*==============================================================================
 * EVENT TYPE/CODE CONVERSION FUNCTIONS
 *==============================================================================
 */

/*
 * This function is needed to translate the USB HID relative flag into the
 * [hid]/linux style events
 */
static void convert_axis_to_symbols(pRecElement pCurrentHIDElement, t_hid_element *new_element, char *axis) 
{
	char buffer[MAXPDSTRING];
	if (pCurrentHIDElement->relative) 
	{ 
		new_element->type = gensym("rel"); 
		snprintf(buffer, sizeof(buffer), "rel_%s", axis); 
		new_element->name = gensym(buffer);
	}
	else 
	{ 
		new_element->type = gensym("abs"); 
		snprintf(buffer, sizeof(buffer), "abs_%s", axis); 
		new_element->name = gensym(buffer);
	}
}


static void get_usage_symbols(pRecElement pCurrentHIDElement, t_hid_element *new_element) 
{
//	debug_print(LOG_DEBUG,"get_usage_symbols");
	char buffer[MAXPDSTRING];

	if(new_element == NULL)
	{
		debug_print(LOG_EMERG,"[hid] new_element == NULL!!  This is a bug, please report it.");
		return;
	}
	if(pCurrentHIDElement == NULL)
	{
		debug_print(LOG_EMERG,"[hid] pCurrentHIDElement == NULL!!  This is a bug, please report it.");
		return;
	}

	switch(pCurrentHIDElement->type)
	{
	case kIOHIDElementTypeInput_Button:
		new_element->type = gensym("key");
		break;
	}
	switch (pCurrentHIDElement->usagePage)
	{
	case kHIDPage_GenericDesktop:
		switch (pCurrentHIDElement->usage)
		{
		case kHIDUsage_GD_X: convert_axis_to_symbols(pCurrentHIDElement, new_element, "x"); break;
		case kHIDUsage_GD_Y: convert_axis_to_symbols(pCurrentHIDElement, new_element, "y"); break;
		case kHIDUsage_GD_Z: convert_axis_to_symbols(pCurrentHIDElement, new_element, "z"); break;
		case kHIDUsage_GD_Rx: convert_axis_to_symbols(pCurrentHIDElement, new_element, "rx"); break;
		case kHIDUsage_GD_Ry: convert_axis_to_symbols(pCurrentHIDElement, new_element, "ry"); break;
		case kHIDUsage_GD_Rz: convert_axis_to_symbols(pCurrentHIDElement, new_element, "rz"); break;
		case kHIDUsage_GD_Slider:
			new_element->type = gensym("abs");
			new_element->name = gensym("abs_throttle");
			break;
		case kHIDUsage_GD_Dial:
			new_element->type = gensym("abs");
			new_element->name = gensym("abs_ry");
			break;
		case kHIDUsage_GD_Wheel: 
			new_element->type = gensym("rel"); 
			new_element->name = gensym("rel_wheel");
			break;
		case kHIDUsage_GD_Hatswitch:
			// this is still a mystery how to handle
			new_element->type = gensym("abs"); 
			new_element->name = gensym("hatswitch");
			break;
		default:
			new_element->type = gensym("DESKTOP");
			snprintf(buffer, sizeof(buffer), "DESKTOP%ld", pCurrentHIDElement->usage); 
			new_element->name = gensym(buffer);
		}
		break;
	case kHIDPage_Simulation:
		switch (pCurrentHIDElement->usage)
		{
		case kHIDUsage_Sim_Rudder: 
			new_element->type = gensym("abs");
			new_element->name = gensym("abs_rz");
			break;
		case kHIDUsage_Sim_Throttle:
			new_element->type = gensym("abs");
			new_element->name = gensym("abs_throttle");
			break;
		default:
			new_element->type = gensym("SIMULATION");
			snprintf(buffer, sizeof(buffer), "SIMULATION%ld", pCurrentHIDElement->usage); 
			new_element->name = gensym(buffer);
		}
		break;
	case kHIDPage_KeyboardOrKeypad:
		new_element->type = gensym("key"); 
		/* temporary kludge until I feel like writing the translation table */
		snprintf(buffer, sizeof(buffer), "key_%ld", pCurrentHIDElement->usage); 
		new_element->name = gensym(buffer);
		break;
	case kHIDPage_Button:
		new_element->type = gensym("key"); 
		/* HID Manager button numbers start at 1, [hid] start at 0 */
		snprintf(buffer, sizeof(buffer), "btn_%ld", pCurrentHIDElement->usage - 1); 
		new_element->name = gensym(buffer);
		break;
	case kHIDPage_LEDs:
		/* temporary kludge until I feel like writing the translation table */
		new_element->type = gensym("led"); 
		snprintf(buffer, sizeof(buffer), "led_%ld", pCurrentHIDElement->usage); 
		new_element->name = gensym(buffer);
		break;
	case kHIDPage_PID:
		/* temporary kludge until I feel like writing the translation table */
		new_element->type = gensym("ff"); 
		snprintf(buffer, sizeof(buffer), "ff_%ld", pCurrentHIDElement->usage); 
		new_element->name = gensym(buffer);
		break;
	default:
		/* the rest are "vendor defined" so no translation table is possible */
		snprintf(buffer, sizeof(buffer), "0x%04x", (unsigned int) pCurrentHIDElement->usagePage); 
		new_element->type = gensym(buffer); 
		snprintf(buffer, sizeof(buffer), "0x%04x", (unsigned int) pCurrentHIDElement->usage); 
		new_element->name = gensym(buffer);
	}
}


static t_float get_type_name_instance(t_symbol *type, t_symbol *name, 
									   int argc, t_hid_element **argv) 
{
	int i;
	int instance_count = 0;
	for(i=0; i<argc; ++i)
	{
		if( (argv[i]->name == name) && (argv[i]->type == type) )
		{
			++instance_count;
//			post("found %d instances of %s %s", instance_count, type->s_name, name->s_name);
		}
	}
	return((t_float) instance_count);
}


/*
 * Linux input events report hatswitches as absolute axes with -1, 0, 1 as
 * possible values.  MacOS X HID Manager reports hatswitches as a specific
 * hatswitch type with each direction represented by a unique number.  This
 * function converts the unique number to the Linux style axes.
 */

/* 
 * hmm, not sure how to implement this cleanly yet, 
 * MacOS X represents this as one event, while [hid] represents it as two
 * distinct axes.  So the conversion requires an added hid_output_event().
 void hid_convert_hatswitch_values(IOHIDEventStruct event, t_symbol *type, t_atom *usage)
 {
			case 0: 
				name = gensym("abs_hat0y");value = 1;
				hid_output_event(x, type, name, value);
				name = gensym("abs_hat0x");value = 0;
				break;
			case 1: 
				name = gensym("abs_hat0y");value = 1;
				hid_output_event(x, type, name, value);
				name = gensym("abs_hat0x");value = 1;
				break;
			case 2: 
				name = gensym("abs_hat0y");value = 0;
				hid_output_event(x, type, name, value);
				name = gensym("abs_hat0x");value = 1;
				break;
			case 3: 
				name = gensym("abs_hat0y");value = -1;
				hid_output_event(x, type, name, value);
				name = gensym("abs_hat0x");value = 1;
				break;
			case 4: 
				name = gensym("abs_hat0y");value = -1;
				hid_output_event(x, type, name, value);
				name = gensym("abs_hat0x");value = 0;
				break;
			case 5: 
				name = gensym("abs_hat0y");value = -1;
				hid_output_event(x, type, name, value);
				name = gensym("abs_hat0x");value = -1;
				break;
			case 6: 
				name = gensym("abs_hat0y");value = 0;
				hid_output_event(x, type, name, value);
				name = gensym("abs_hat0x");value = -1;
				break;
			case 7: 
				name = gensym("abs_hat0y");value = 1;
				hid_output_event(x, type, name, value);
				name = gensym("abs_hat0x");value = -1;
				break;
			case 8: 
				name = gensym("abs_hat0y");value = 0;
				hid_output_event(x, type, name, value);
				name = gensym("abs_hat0x");value = 0;
				break;
			}

 }
*/

/* ============================================================================== */
/* DARWIN-SPECIFIC SUPPORT FUNCTIONS */
/* ============================================================================== */

short get_device_number_by_id(unsigned short vendor_id, unsigned short product_id)
{
	debug_print(LOG_DEBUG,"get_device_number_from_usage");

	pRecDevice    pCurrentHIDDevice;
	t_int i;
	short return_device_number = -1;

	if( !HIDHaveDeviceList() ) hid_build_device_list();

	pCurrentHIDDevice = HIDGetFirstDevice();
	i = HIDCountDevices();
	while(pCurrentHIDDevice != NULL)
	{
		--i;
		debug_print(LOG_INFO,"compare 0x%04x == 0x%04x  0x%04x == 0x%04x",
					pCurrentHIDDevice->vendorID,
					vendor_id,
					pCurrentHIDDevice->productID,
					product_id);
		if( (pCurrentHIDDevice->vendorID == vendor_id) && 
			(pCurrentHIDDevice->productID == product_id) )
		{
			return_device_number = i;
			pCurrentHIDDevice = NULL;
		}
		else
			pCurrentHIDDevice = HIDGetNextDevice(pCurrentHIDDevice);
	}
	return(return_device_number);
}

short get_device_number_from_usage(short device_number, 
										unsigned short usage_page, 
										unsigned short usage)
{
//	debug_print(LOG_DEBUG,"get_device_number_from_usage");

	pRecDevice    pCurrentHIDDevice;
	t_int i;
	short return_device_number = -1;
	t_int total_devices = 0;
	char cstrDeviceName[MAXPDSTRING];

	if( !HIDHaveDeviceList() ) hid_build_device_list();

	pCurrentHIDDevice = HIDGetFirstDevice();
	while(pCurrentHIDDevice != NULL)
	{
		if( (pCurrentHIDDevice->usagePage == usage_page) && 
			(pCurrentHIDDevice->usage == usage) )
		{
			++total_devices;
		}
		pCurrentHIDDevice = HIDGetNextDevice(pCurrentHIDDevice);
	}
	i = total_devices;
	return_device_number = HIDCountDevices();
	pCurrentHIDDevice = HIDGetFirstDevice();
	while( (pCurrentHIDDevice != NULL) && (i > device_number) ) 
	{
		return_device_number--;
		if( (pCurrentHIDDevice->usagePage == usage_page) && 
			(pCurrentHIDDevice->usage == usage) )
		{
			i--;
			HIDGetUsageName(pCurrentHIDDevice->usagePage, 
							pCurrentHIDDevice->usage, 
							cstrDeviceName);
			debug_print(LOG_DEBUG,"[hid]: found a %s at %d/%d: %s %s"
						,cstrDeviceName,
						i,
						total_devices,
						pCurrentHIDDevice->manufacturer,
						pCurrentHIDDevice->product);
		}
		pCurrentHIDDevice = HIDGetNextDevice(pCurrentHIDDevice);
	}
	if(i < total_devices)
		return(return_device_number);
	else
		return(-1);
}


static void hid_build_element_list(t_hid *x) 
{
	char type_name[256];
	char usage_name[256];
	pRecElement pCurrentHIDElement;
	pRecDevice pCurrentHIDDevice = device_pointer[x->x_device_number];
	t_hid_element *new_element;
  
	element_count[x->x_device_number] = 0;
	if( HIDIsValidDevice(pCurrentHIDDevice) ) 
	{
		/* queuing one element at a time only works for the first element, so
		 * try queuing the whole device, then removing specific elements from
		 * the queue */
		HIDQueueDevice(pCurrentHIDDevice);
		pCurrentHIDElement = HIDGetFirstDeviceElement( pCurrentHIDDevice,
													   kHIDElementTypeInput );
		while( pCurrentHIDElement != NULL) 
		{
			/* these two functions just get the pretty names for display */
			HIDGetTypeName((IOHIDElementType) pCurrentHIDElement->type, type_name);
			HIDGetUsageName(pCurrentHIDElement->usagePage, 
							pCurrentHIDElement->usage, usage_name);

			new_element = getbytes(sizeof(t_hid_element));
			new_element->os_pointer = (void *) pCurrentHIDElement;
			get_usage_symbols(pCurrentHIDElement, new_element);
			new_element->relative = pCurrentHIDElement->relative;
			new_element->instance = get_type_name_instance(new_element->type, 
														   new_element->name,
														   element_count[x->x_device_number],
														   element[x->x_device_number]);
			
			if( (pCurrentHIDElement->usagePage == kHIDPage_GenericDesktop) &&
				(!pCurrentHIDElement->relative) )
			{
				switch(pCurrentHIDElement->usage)
				{
				case kHIDUsage_GD_X:
				case kHIDUsage_GD_Y:
				case kHIDUsage_GD_Z:
				case kHIDUsage_GD_Rx:
				case kHIDUsage_GD_Ry:
				case kHIDUsage_GD_Rz:
				case kHIDUsage_GD_Slider:
				case kHIDUsage_GD_Dial:
				case kHIDUsage_GD_Wheel:
					//case kHIDUsage_GD_Hatswitch: // hatswitches are more like buttons, so queue them
					debug_print(LOG_INFO,"[hid] storing absolute axis to poll %s, %s (0x%04x 0x%04x)",
								type_name, usage_name, 
								pCurrentHIDElement->usagePage, pCurrentHIDElement->usage);
					if(HIDDequeueElement(pCurrentHIDDevice,pCurrentHIDElement) != kIOReturnSuccess)
						debug_print(LOG_ERR,"[hid] could not dequeue element");
					new_element->polled = 1;
					break;
				}
			}
			else
			{
				debug_print(LOG_INFO,"[hid] queuing element %s, %s (0x%04x 0x%04x)",
							type_name, usage_name,
							pCurrentHIDElement->usagePage, pCurrentHIDElement->usage);
			}
			new_element->min = pCurrentHIDElement->min;
			new_element->max = pCurrentHIDElement->max;
			debug_print(LOG_DEBUG,"\tlogical min %d max %d",
						pCurrentHIDElement->min,pCurrentHIDElement->max);
			element[x->x_device_number][element_count[x->x_device_number]] = new_element;
			++element_count[x->x_device_number];
			pCurrentHIDElement = HIDGetNextDeviceElement(pCurrentHIDElement, kHIDElementTypeInput);
		}
	}
}

static void hid_print_element_list(t_hid *x)
{
//	debug_print(LOG_DEBUG,"hid_print_element_list");
	int i;
	pRecElement	pCurrentHIDElement;
	pRecDevice pCurrentHIDDevice;
	t_hid_element *current_element;
	char type_name[256];
	char usage_name[256];

	pCurrentHIDDevice = device_pointer[x->x_device_number];
	if ( ! HIDIsValidDevice(pCurrentHIDDevice) )
	{
		error("[hid]: device %d is not a valid device\n",x->x_device_number);
		return;
	}
	post("[hid] found %d elements:",element_count[x->x_device_number]);
	post("\n  TYPE\tCODE\t#\tEVENT NAME");
	post("-----------------------------------------------------------");
	for(i=0; i<element_count[x->x_device_number]; i++)
	{
		current_element = element[x->x_device_number][i];
		pCurrentHIDElement = (pRecElement) current_element->os_pointer;
		HIDGetTypeName((IOHIDElementType) pCurrentHIDElement->type, type_name);
		HIDGetUsageName(pCurrentHIDElement->usagePage, 
						pCurrentHIDElement->usage, usage_name);
		post("  %s\t%s\t%d\t%s, %s", current_element->type->s_name,
			 current_element->name->s_name,(int) current_element->instance,
			 type_name, usage_name);
	}
	post("");
}


void hid_ff_print( t_hid *x )
{
	debug_print(LOG_DEBUG,"hid_ff_print");
	HRESULT result;
	UInt32 value;

	if ( x->x_has_ff ) 
	{
		result = FFDeviceGetForceFeedbackProperty( (FFDeviceObjectReference) x->x_ff_device, 
												   FFPROP_AUTOCENTER, 
												   &value, 
												   (IOByteCount) sizeof( value ) );
		if ( result == FF_OK ) post( "autocenter: %d",value );

		result = FFDeviceGetForceFeedbackProperty( (FFDeviceObjectReference) x->x_ff_device, 
												   FFPROP_FFGAIN, 
												   &value, 
												   (IOByteCount) sizeof( value ) );
		if ( result == FF_OK ) post( "gain: %d", value );
	}

//	FFEffectGetParameters(  ); 
}


static void hid_print_device_list(t_hid *x) 
{
	char device_type_buffer[256];
	t_int i, numdevs;
	unsigned int j;
	pRecDevice pCurrentHIDDevice = NULL;

	if( HIDHaveDeviceList() )
	{
		numdevs = (t_int) HIDCountDevices();
		
		post("");
		/* display device list in console */
		for(i=0; i < numdevs; i++)
		{
			pCurrentHIDDevice = device_pointer[i];
			post("__________________________________________________");
			post("Device %d: '%s' '%s' version %d @ location 0x%08x", i, 
				 pCurrentHIDDevice->manufacturer, pCurrentHIDDevice->product, 
				 pCurrentHIDDevice->version,pCurrentHIDDevice->locID);
			HIDGetUsageName(pCurrentHIDDevice->usagePage, pCurrentHIDDevice->usage, 
							device_type_buffer);
			for(j=0; j< strlen(device_type_buffer); ++j)
				device_type_buffer[j] = tolower(device_type_buffer[j]);
			post("\tdevice type: %s\tusage page: 0x%04x\tusage: 0x%04x",
				 device_type_buffer, pCurrentHIDDevice->usagePage,
				 pCurrentHIDDevice->usage);
			post("\tvendorID: 0x%04x\tproductID: 0x%04x",
				 pCurrentHIDDevice->vendorID, pCurrentHIDDevice->productID);
		}
		post("");
	}
}
/* ============================================================================== 
 * STATUS/INFO OUTPUT
 * ============================================================================== */

void hid_platform_specific_info(t_hid *x)
{
	unsigned int i;
	pRecDevice  pCurrentHIDDevice = NULL;
	char vendor_id_string[7];
	char product_id_string[7];
	char device_type_buffer[256];
	t_symbol *output_symbol;
	t_atom *output_atom = getbytes(sizeof(t_atom));

	if(x->x_device_number > -1)
	{
//		pCurrentHIDDevice = hid_get_device_by_number(x->x_device_number);
		pCurrentHIDDevice = device_pointer[x->x_device_number];
		if(pCurrentHIDDevice != NULL)
		{
            /* product */
			SETSYMBOL(output_atom, gensym(pCurrentHIDDevice->product));
			outlet_anything( x->x_status_outlet, gensym("product"), 
							 1, output_atom);
			/* manufacturer */
			SETSYMBOL(output_atom, gensym(pCurrentHIDDevice->manufacturer));
			outlet_anything( x->x_status_outlet, gensym("manufacturer"), 
							 1, output_atom);
			/* serial number */
			if(pCurrentHIDDevice->serial != NULL)
			{
				output_symbol = gensym(pCurrentHIDDevice->serial);
				if( output_symbol != &s_ )
				{ /* the serial is rarely used on USB devices, so test for it */
					SETSYMBOL(output_atom, output_symbol);
					outlet_anything( x->x_status_outlet, gensym("serial"), 
									 1, output_atom);
				}
			}
			/* transport */
			SETSYMBOL(output_atom, gensym(pCurrentHIDDevice->transport));
			outlet_anything( x->x_status_outlet, gensym("transport"), 
							 1, output_atom);
            /* vendor id */
			sprintf(vendor_id_string,"0x%04x",
					(unsigned int)pCurrentHIDDevice->vendorID);
			SETSYMBOL(output_atom, gensym(vendor_id_string));
			outlet_anything( x->x_status_outlet, gensym("vendorID"), 
							 1, output_atom);
            /* product id */
			sprintf(product_id_string,"0x%04x",
					(unsigned int)pCurrentHIDDevice->productID);
			SETSYMBOL(output_atom, gensym(product_id_string));
			outlet_anything( x->x_status_outlet, gensym("productID"), 
							 1, output_atom);
            /* type */
			HIDGetUsageName(pCurrentHIDDevice->usagePage, 
							pCurrentHIDDevice->usage, 
							device_type_buffer);
			for(i=0; i< strlen(device_type_buffer); ++i)
				device_type_buffer[i] = tolower(device_type_buffer[i]);
			SETSYMBOL(output_atom, gensym(device_type_buffer));
			outlet_anything( x->x_status_outlet, gensym("type"), 
							 1, output_atom);
		}
	}
	freebytes(output_atom,sizeof(t_atom));
}

/* ============================================================================== */
/* Pd [hid] FUNCTIONS */
/* ============================================================================== */

void hid_get_events(t_hid *x)
{
	unsigned int i;
	pRecDevice  pCurrentHIDDevice;
	t_hid_element *current_element;
	IOHIDEventStruct event;

	pCurrentHIDDevice = device_pointer[x->x_device_number];

	/* get the queued events first and store them */
//	while( (HIDGetEvent(pCurrentHIDDevice, (void*) &event)) && (event_counter < MAX_EVENTS_PER_POLL) ) 
	while(HIDGetEvent(pCurrentHIDDevice, (void*) &event))
	{
		i=0;
		do {
			current_element = element[x->x_device_number][i];
			++i;
		} while( (i < element_count[x->x_device_number]) && 
				 (((pRecElement)current_element->os_pointer)->cookie != 
				  (IOHIDElementCookie) event.elementCookie) );
		
		current_element->value = event.value;
		debug_print(LOG_DEBUG,"output this: %s %s %d prev %d",current_element->type->s_name,
			 current_element->name->s_name, current_element->value, 
			 current_element->previous_value);
	}
	/* absolute axes don't need to be queued, they can just be polled */
	for(i=0; i< element_count[x->x_device_number]; ++i)
	{
		current_element = element[x->x_device_number][i];
		if(current_element->polled) 
		{
			current_element->value = 
				HIDGetElementValue(pCurrentHIDDevice, 
								   (pRecElement)current_element->os_pointer);
		}
	}
}


t_int hid_open_device(t_hid *x, short device_number)
{
	debug_print(LOG_DEBUG,"hid_open_device");

	t_int result = 0;
	pRecDevice pCurrentHIDDevice = NULL;

	io_service_t hidDevice = 0;
	FFDeviceObjectReference ffDeviceReference = NULL;

/* rebuild device list to make sure the list is current */
	if( !HIDHaveDeviceList() ) hid_build_device_list();
	
//	pCurrentHIDDevice = hid_get_device_by_number(device_number);
	pCurrentHIDDevice = device_pointer[device_number];
	if( HIDIsValidDevice(pCurrentHIDDevice) )
	{
		x->x_device_number = device_number;
	}
	else
	{
		debug_error(x,LOG_ERR,"[hid]: device %d is not a valid device\n",device_number);
		return(1);
	}
	debug_print(LOG_WARNING,"[hid] opened device %d: %s %s",
				device_number, pCurrentHIDDevice->manufacturer, pCurrentHIDDevice->product);

	hid_build_element_list(x);

	hidDevice = AllocateHIDObjectFromRecDevice( pCurrentHIDDevice );
	if( FFIsForceFeedback(hidDevice) == FF_OK ) 
	{
		debug_print(LOG_WARNING,"\tdevice has Force Feedback support");
		if( FFCreateDevice(hidDevice,&ffDeviceReference) == FF_OK ) 
		{
			x->x_has_ff = 1;
			x->x_ff_device = ffDeviceReference;
		}
		else
		{
			x->x_has_ff = 0;
			post("[hid]: FF device creation failed!");
			return( -1 );
		}
	}
	return(result);
}


t_int hid_close_device(t_hid *x)
{
	debug_print(LOG_DEBUG,"hid_close_device");

	t_int result = 0;
//	pRecDevice pCurrentHIDDevice = hid_get_device_by_number(x->x_device_number);
	pRecDevice pCurrentHIDDevice = device_pointer[x->x_device_number];

	HIDDequeueDevice(pCurrentHIDDevice);
// this doesn't seem to be needed at all, but why not use it?
//   result = HIDCloseReleaseInterface(pCurrentHIDDevice);
	
	return(result);
}


void hid_build_device_list(void)
{
	int device_number = 0;
	pRecDevice pCurrentHIDDevice;
	
	debug_print(LOG_DEBUG,"hid_build_device_list");

	debug_print(LOG_WARNING,"[hid] Building device list...");
	if(HIDBuildDeviceList (0, 0)) 
		post("[hid]: no HID devices found\n");

/*	The most recently discovered HID is the first element of the list here.  I
 *	want the oldest to be number 0 rather than the newest. */
	device_number = (int) HIDCountDevices();
	pCurrentHIDDevice = HIDGetFirstDevice();
	while(pCurrentHIDDevice != NULL)
	{
		--device_number;
		if(device_number < MAX_DEVICES)
			device_pointer[device_number] = pCurrentHIDDevice;
		pCurrentHIDDevice = HIDGetNextDevice(pCurrentHIDDevice);
	}
	device_count = (unsigned int) HIDCountDevices(); // set the global variable
	debug_print(LOG_WARNING,"[hid] completed device list.");
}


void hid_print(t_hid *x)
{
	if( !HIDHaveDeviceList() ) hid_build_device_list();
	hid_print_device_list(x);
	if(x->x_device_open) 
	{
		hid_print_element_list(x);
		hid_ff_print(x);
	}
}


void hid_platform_specific_free(t_hid *x)
{
	debug_print(LOG_DEBUG,"hid_platform_specific_free");
/* only call this if the last instance is being freed */
	if (hid_instance_count < 1) 
	{
		DEBUG(post("RELEASE ALL hid_instance_count: %d", hid_instance_count););
		HIDReleaseAllDeviceQueues();
		HIDReleaseDeviceList();
	}
}


/* ============================================================================== 
 * FORCE FEEDBACK
 * ============================================================================== */

/* --------------------------------------------------------------------------
 * FF "Properties"
 * autocenter ( 0/1 ), ffgain (overall feedback gain 0-10000)
 */

t_int hid_ff_autocenter(t_hid *x, t_float value)
{
	debug_print(LOG_DEBUG,"hid_ff_autocenter");
	HRESULT result;
	UInt32 autocenter_value;

	if( x->x_has_ff ) 
	{
		if ( value > 0 ) autocenter_value = 1;
		else if ( value <= 0 ) autocenter_value = 0;
		/* FFPROP_AUTOCENTER is either 0 or 1 */
		result = FFDeviceSetForceFeedbackProperty( 
			(FFDeviceObjectReference) x->x_ff_device, 
			FFPROP_AUTOCENTER, 
			&autocenter_value );
		if ( result != FF_OK )
		{
			post("[hid]: ff_autocenter failed!");
		}
	}
	
	return(0);
}

t_int hid_ff_gain(t_hid *x, t_float value)
{
	debug_print(LOG_DEBUG,"hid_ff_gain");
	HRESULT result;
	UInt32 ffgain_value;
	
	if( x->x_has_ff ) 
	{
		if ( value > 1 ) value = 1;
		else if ( value < 0 ) value = 0;
		ffgain_value = value * 10000;
		/* FFPROP_FFGAIN has a integer range of 0-10000 */
		result = FFDeviceSetForceFeedbackProperty( 
			(FFDeviceObjectReference)x->x_ff_device, FFPROP_FFGAIN, &ffgain_value );
		if ( result != FF_OK )
		{
			post("[hid]: ff_gain failed!");
		}
	}
	
	return(0);
}

/* --------------------------------------------------------------------------
 * FF "Commands"
 * continue, pause, reset, setactuatorsoff, setactuatorson, stopall
 */

t_int hid_ff_send_ff_command (t_hid *x, UInt32 ff_command)
{
	HRESULT result = 0;

	if( x->x_has_ff ) 
	{
		result = FFDeviceSendForceFeedbackCommand( x->x_ff_device, ff_command );
	}

	return ( (t_int) result );
}

t_int hid_ff_continue( t_hid *x )
{
	debug_print(LOG_DEBUG,"hid_ff_continue");
	return(  hid_ff_send_ff_command( x, FFSFFC_CONTINUE ) );
}

t_int hid_ff_pause( t_hid *x )
{
	debug_print(LOG_DEBUG,"hid_ff_pause");
	return(  hid_ff_send_ff_command( x, FFSFFC_PAUSE ) );
}

t_int hid_ff_reset( t_hid *x )
{
	debug_print(LOG_DEBUG,"hid_ff_reset");
	return(  hid_ff_send_ff_command( x, FFSFFC_RESET ) );
}

t_int hid_ff_setactuatorsoff( t_hid *x )
{
	debug_print(LOG_DEBUG,"hid_ff_setactuatorsoff");
	return(  hid_ff_send_ff_command( x, FFSFFC_SETACTUATORSOFF ) );
}

t_int hid_ff_setactuatorson( t_hid *x )
{
	debug_print(LOG_DEBUG,"hid_ff_setactuatorson");
	return(  hid_ff_send_ff_command( x, FFSFFC_SETACTUATORSON ) );
}

t_int hid_ff_stopall( t_hid *x )
{
	debug_print(LOG_DEBUG,"hid_ff_stopall");
	return(  hid_ff_send_ff_command( x, FFSFFC_STOPALL ) );
}

t_int hid_ff_motors( t_hid *x, t_float value )
{
	if ( value > 0 ) 
	{
		return ( hid_ff_setactuatorson( x )  );
	}
	else
	{
		return ( hid_ff_setactuatorsoff( x )  );
	}
}


/* --------------------------------------------------------------------------
 * FF test functions
 */

t_int hid_ff_fftest ( t_hid *x, t_float value)
{
	debug_print(LOG_DEBUG,"hid_ff_fftest");
	
	return( 0 );
}

/* ---------------------------------------------------------------------------------------------------- 
 * the big monster look up table (not used yet)
 */

//void HIDGetUsageName (const long valueUsagePage, const long valueUsage, char * cstrName)
char *convertEventsFromDarwinToLinux(pRecElement element)
{
	char *cstrName = "";
// this allows these definitions to exist in an XML .plist file
/* 	if (xml_GetUsageName(valueUsagePage, valueUsage, cstrName)) */
/* 		return; */

    switch (element->usagePage)
    {
	case kHIDPage_Undefined:
		switch (element->usage)
		{
		default: sprintf (cstrName, "Undefined Page, Usage 0x%lx", element->usage); break;
		}
		break;
	case kHIDPage_GenericDesktop:
		switch (element->usage)
		{
		case kHIDUsage_GD_Pointer: sprintf (cstrName, "Pointer"); break;
		case kHIDUsage_GD_Mouse: sprintf (cstrName, "Mouse"); break;
		case kHIDUsage_GD_Joystick: sprintf (cstrName, "Joystick"); break;
		case kHIDUsage_GD_GamePad: sprintf (cstrName, "GamePad"); break;
		case kHIDUsage_GD_Keyboard: sprintf (cstrName, "Keyboard"); break;
		case kHIDUsage_GD_Keypad: sprintf (cstrName, "Keypad"); break;
		case kHIDUsage_GD_MultiAxisController: sprintf (cstrName, "Multi-Axis Controller"); break;

		case kHIDUsage_GD_X: sprintf (cstrName, "X-Axis"); break;
		case kHIDUsage_GD_Y: sprintf (cstrName, "Y-Axis"); break;
		case kHIDUsage_GD_Z: sprintf (cstrName, "Z-Axis"); break;
		case kHIDUsage_GD_Rx: sprintf (cstrName, "X-Rotation"); break;
		case kHIDUsage_GD_Ry: sprintf (cstrName, "Y-Rotation"); break;
		case kHIDUsage_GD_Rz: sprintf (cstrName, "Z-Rotation"); break;
		case kHIDUsage_GD_Slider: sprintf (cstrName, "Slider"); break;
		case kHIDUsage_GD_Dial: sprintf (cstrName, "Dial"); break;
		case kHIDUsage_GD_Wheel: sprintf (cstrName, "Wheel"); break;
		case kHIDUsage_GD_Hatswitch: sprintf (cstrName, "Hatswitch"); break;
		case kHIDUsage_GD_CountedBuffer: sprintf (cstrName, "Counted Buffer"); break;
		case kHIDUsage_GD_ByteCount: sprintf (cstrName, "Byte Count"); break;
		case kHIDUsage_GD_MotionWakeup: sprintf (cstrName, "Motion Wakeup"); break;
		case kHIDUsage_GD_Start: sprintf (cstrName, "Start"); break;
		case kHIDUsage_GD_Select: sprintf (cstrName, "Select"); break;

		case kHIDUsage_GD_Vx: sprintf (cstrName, "X-Velocity"); break;
		case kHIDUsage_GD_Vy: sprintf (cstrName, "Y-Velocity"); break;
		case kHIDUsage_GD_Vz: sprintf (cstrName, "Z-Velocity"); break;
		case kHIDUsage_GD_Vbrx: sprintf (cstrName, "X-Rotation Velocity"); break;
		case kHIDUsage_GD_Vbry: sprintf (cstrName, "Y-Rotation Velocity"); break;
		case kHIDUsage_GD_Vbrz: sprintf (cstrName, "Z-Rotation Velocity"); break;
		case kHIDUsage_GD_Vno: sprintf (cstrName, "Vno"); break;

		case kHIDUsage_GD_SystemControl: sprintf (cstrName, "System Control"); break;
		case kHIDUsage_GD_SystemPowerDown: sprintf (cstrName, "System Power Down"); break;
		case kHIDUsage_GD_SystemSleep: sprintf (cstrName, "System Sleep"); break;
		case kHIDUsage_GD_SystemWakeUp: sprintf (cstrName, "System Wake Up"); break;
		case kHIDUsage_GD_SystemContextMenu: sprintf (cstrName, "System Context Menu"); break;
		case kHIDUsage_GD_SystemMainMenu: sprintf (cstrName, "System Main Menu"); break;
		case kHIDUsage_GD_SystemAppMenu: sprintf (cstrName, "System App Menu"); break;
		case kHIDUsage_GD_SystemMenuHelp: sprintf (cstrName, "System Menu Help"); break;
		case kHIDUsage_GD_SystemMenuExit: sprintf (cstrName, "System Menu Exit"); break;
		case kHIDUsage_GD_SystemMenu: sprintf (cstrName, "System Menu"); break;
		case kHIDUsage_GD_SystemMenuRight: sprintf (cstrName, "System Menu Right"); break;
		case kHIDUsage_GD_SystemMenuLeft: sprintf (cstrName, "System Menu Left"); break;
		case kHIDUsage_GD_SystemMenuUp: sprintf (cstrName, "System Menu Up"); break;
		case kHIDUsage_GD_SystemMenuDown: sprintf (cstrName, "System Menu Down"); break;

		case kHIDUsage_GD_DPadUp: sprintf (cstrName, "DPad Up"); break;
		case kHIDUsage_GD_DPadDown: sprintf (cstrName, "DPad Down"); break;
		case kHIDUsage_GD_DPadRight: sprintf (cstrName, "DPad Right"); break;
		case kHIDUsage_GD_DPadLeft: sprintf (cstrName, "DPad Left"); break;

		case kHIDUsage_GD_Reserved: sprintf (cstrName, "Reserved"); break;

		default: sprintf (cstrName, "Generic Desktop Usage 0x%lx", element->usage); break;
		}
		break;
	case kHIDPage_Simulation:
		switch (element->usage)
		{
		default: sprintf (cstrName, "Simulation Usage 0x%lx", element->usage); break;
		}
		break;
	case kHIDPage_VR:
		switch (element->usage)
		{
		default: sprintf (cstrName, "VR Usage 0x%lx", element->usage); break;
		}
		break;
	case kHIDPage_Sport:
		switch (element->usage)
		{
		default: sprintf (cstrName, "Sport Usage 0x%lx", element->usage); break;
		}
		break;
	case kHIDPage_Game:
		switch (element->usage)
		{
		default: sprintf (cstrName, "Game Usage 0x%lx", element->usage); break;
		}
		break;
	case kHIDPage_KeyboardOrKeypad:
		switch (element->usage)
		{
		default: sprintf (cstrName, "Keyboard Usage 0x%lx", element->usage); break;
		}
		break;
	case kHIDPage_LEDs:
		switch (element->usage)
		{
			// some LED usages
		case kHIDUsage_LED_IndicatorRed: sprintf (cstrName, "Red LED"); break;
		case kHIDUsage_LED_IndicatorGreen: sprintf (cstrName, "Green LED"); break;
		case kHIDUsage_LED_IndicatorAmber: sprintf (cstrName, "Amber LED"); break;
		case kHIDUsage_LED_GenericIndicator: sprintf (cstrName, "Generic LED"); break;
		case kHIDUsage_LED_SystemSuspend: sprintf (cstrName, "System Suspend LED"); break;
		case kHIDUsage_LED_ExternalPowerConnected: sprintf (cstrName, "External Power LED"); break;
		default: sprintf (cstrName, "LED Usage 0x%lx", element->usage); break;
		}
		break;
	case kHIDPage_Button:
		switch (element->usage)
		{
		default: sprintf (cstrName, "Button #%ld", element->usage); break;
		}
		break;
	case kHIDPage_Ordinal:
		switch (element->usage)
		{
		default: sprintf (cstrName, "Ordinal Instance %lx", element->usage); break;
		}
		break;
	case kHIDPage_Telephony:
		switch (element->usage)
		{
		default: sprintf (cstrName, "Telephony Usage 0x%lx", element->usage); break;
		}
		break;
	case kHIDPage_Consumer:
		switch (element->usage)
		{
		default: sprintf (cstrName, "Consumer Usage 0x%lx", element->usage); break;
		}
		break;
	case kHIDPage_Digitizer:
		switch (element->usage)
		{
		default: sprintf (cstrName, "Digitizer Usage 0x%lx", element->usage); break;
		}
		break;
	case kHIDPage_PID:
		if (((element->usage >= 0x02) && (element->usage <= 0x1F)) || ((element->usage >= 0x29) && (element->usage <= 0x2F)) ||
			((element->usage >= 0x35) && (element->usage <= 0x3F)) || ((element->usage >= 0x44) && (element->usage <= 0x4F)) ||
			(element->usage == 0x8A) || (element->usage == 0x93)  || ((element->usage >= 0x9D) && (element->usage <= 0x9E)) ||
			((element->usage >= 0xA1) && (element->usage <= 0xA3)) || ((element->usage >= 0xAD) && (element->usage <= 0xFFFF)))
			sprintf (cstrName, "PID Reserved");
		else
			switch (element->usage)
			{
			case 0x00: sprintf (cstrName, "PID Undefined Usage"); break;
			case kHIDUsage_PID_PhysicalInterfaceDevice: sprintf (cstrName, "Physical Interface Device"); break;
			case kHIDUsage_PID_Normal: sprintf (cstrName, "Normal Force"); break;

			case kHIDUsage_PID_SetEffectReport: sprintf (cstrName, "Set Effect Report"); break;
			case kHIDUsage_PID_EffectBlockIndex: sprintf (cstrName, "Effect Block Index"); break;
			case kHIDUsage_PID_ParamBlockOffset: sprintf (cstrName, "Parameter Block Offset"); break;
			case kHIDUsage_PID_ROM_Flag: sprintf (cstrName, "ROM Flag"); break;

			case kHIDUsage_PID_EffectType: sprintf (cstrName, "Effect Type"); break;
			case kHIDUsage_PID_ET_ConstantForce: sprintf (cstrName, "Effect Type Constant Force"); break;
			case kHIDUsage_PID_ET_Ramp: sprintf (cstrName, "Effect Type Ramp"); break;
			case kHIDUsage_PID_ET_CustomForceData: sprintf (cstrName, "Effect Type Custom Force Data"); break;
			case kHIDUsage_PID_ET_Square: sprintf (cstrName, "Effect Type Square"); break;
			case kHIDUsage_PID_ET_Sine: sprintf (cstrName, "Effect Type Sine"); break;
			case kHIDUsage_PID_ET_Triangle: sprintf (cstrName, "Effect Type Triangle"); break;
			case kHIDUsage_PID_ET_SawtoothUp: sprintf (cstrName, "Effect Type Sawtooth Up"); break;
			case kHIDUsage_PID_ET_SawtoothDown: sprintf (cstrName, "Effect Type Sawtooth Down"); break;
			case kHIDUsage_PID_ET_Spring: sprintf (cstrName, "Effect Type Spring"); break;
			case kHIDUsage_PID_ET_Damper: sprintf (cstrName, "Effect Type Damper"); break;
			case kHIDUsage_PID_ET_Inertia: sprintf (cstrName, "Effect Type Inertia"); break;
			case kHIDUsage_PID_ET_Friction: sprintf (cstrName, "Effect Type Friction"); break;
			case kHIDUsage_PID_Duration: sprintf (cstrName, "Effect Duration"); break;
			case kHIDUsage_PID_SamplePeriod: sprintf (cstrName, "Effect Sample Period"); break;
			case kHIDUsage_PID_Gain: sprintf (cstrName, "Effect Gain"); break;
			case kHIDUsage_PID_TriggerButton: sprintf (cstrName, "Effect Trigger Button"); break;
			case kHIDUsage_PID_TriggerRepeatInterval: sprintf (cstrName, "Effect Trigger Repeat Interval"); break;

			case kHIDUsage_PID_AxesEnable: sprintf (cstrName, "Axis Enable"); break;
			case kHIDUsage_PID_DirectionEnable: sprintf (cstrName, "Direction Enable"); break;

			case kHIDUsage_PID_Direction: sprintf (cstrName, "Direction"); break;

			case kHIDUsage_PID_TypeSpecificBlockOffset: sprintf (cstrName, "Type Specific Block Offset"); break;

			case kHIDUsage_PID_BlockType: sprintf (cstrName, "Block Type"); break;

			case kHIDUsage_PID_SetEnvelopeReport: sprintf (cstrName, "Set Envelope Report"); break;
			case kHIDUsage_PID_AttackLevel: sprintf (cstrName, "Envelope Attack Level"); break;
			case kHIDUsage_PID_AttackTime: sprintf (cstrName, "Envelope Attack Time"); break;
			case kHIDUsage_PID_FadeLevel: sprintf (cstrName, "Envelope Fade Level"); break;
			case kHIDUsage_PID_FadeTime: sprintf (cstrName, "Envelope Fade Time"); break;

			case kHIDUsage_PID_SetConditionReport: sprintf (cstrName, "Set Condition Report"); break;
			case kHIDUsage_PID_CP_Offset: sprintf (cstrName, "Condition CP Offset"); break;
			case kHIDUsage_PID_PositiveCoefficient: sprintf (cstrName, "Condition Positive Coefficient"); break;
			case kHIDUsage_PID_NegativeCoefficient: sprintf (cstrName, "Condition Negative Coefficient"); break;
			case kHIDUsage_PID_PositiveSaturation: sprintf (cstrName, "Condition Positive Saturation"); break;
			case kHIDUsage_PID_NegativeSaturation: sprintf (cstrName, "Condition Negative Saturation"); break;
			case kHIDUsage_PID_DeadBand: sprintf (cstrName, "Condition Dead Band"); break;

			case kHIDUsage_PID_DownloadForceSample: sprintf (cstrName, "Download Force Sample"); break;
			case kHIDUsage_PID_IsochCustomForceEnable: sprintf (cstrName, "Isoch Custom Force Enable"); break;

			case kHIDUsage_PID_CustomForceDataReport: sprintf (cstrName, "Custom Force Data Report"); break;
			case kHIDUsage_PID_CustomForceData: sprintf (cstrName, "Custom Force Data"); break;

			case kHIDUsage_PID_CustomForceVendorDefinedData: sprintf (cstrName, "Custom Force Vendor Defined Data"); break;
			case kHIDUsage_PID_SetCustomForceReport: sprintf (cstrName, "Set Custom Force Report"); break;
			case kHIDUsage_PID_CustomForceDataOffset: sprintf (cstrName, "Custom Force Data Offset"); break;
			case kHIDUsage_PID_SampleCount: sprintf (cstrName, "Custom Force Sample Count"); break;

			case kHIDUsage_PID_SetPeriodicReport: sprintf (cstrName, "Set Periodic Report"); break;
			case kHIDUsage_PID_Offset: sprintf (cstrName, "Periodic Offset"); break;
			case kHIDUsage_PID_Magnitude: sprintf (cstrName, "Periodic Magnitude"); break;
			case kHIDUsage_PID_Phase: sprintf (cstrName, "Periodic Phase"); break;
			case kHIDUsage_PID_Period: sprintf (cstrName, "Periodic Period"); break;

			case kHIDUsage_PID_SetConstantForceReport: sprintf (cstrName, "Set Constant Force Report"); break;

			case kHIDUsage_PID_SetRampForceReport: sprintf (cstrName, "Set Ramp Force Report"); break;
			case kHIDUsage_PID_RampStart: sprintf (cstrName, "Ramp Start"); break;
			case kHIDUsage_PID_RampEnd: sprintf (cstrName, "Ramp End"); break;

			case kHIDUsage_PID_EffectOperationReport: sprintf (cstrName, "Effect Operation Report"); break;

			case kHIDUsage_PID_EffectOperation: sprintf (cstrName, "Effect Operation"); break;
			case kHIDUsage_PID_OpEffectStart: sprintf (cstrName, "Op Effect Start"); break;
			case kHIDUsage_PID_OpEffectStartSolo: sprintf (cstrName, "Op Effect Start Solo"); break;
			case kHIDUsage_PID_OpEffectStop: sprintf (cstrName, "Op Effect Stop"); break;
			case kHIDUsage_PID_LoopCount: sprintf (cstrName, "Op Effect Loop Count"); break;

			case kHIDUsage_PID_DeviceGainReport: sprintf (cstrName, "Device Gain Report"); break;
			case kHIDUsage_PID_DeviceGain: sprintf (cstrName, "Device Gain"); break;

			case kHIDUsage_PID_PoolReport: sprintf (cstrName, "PID Pool Report"); break;
			case kHIDUsage_PID_RAM_PoolSize: sprintf (cstrName, "RAM Pool Size"); break;
			case kHIDUsage_PID_ROM_PoolSize: sprintf (cstrName, "ROM Pool Size"); break;
			case kHIDUsage_PID_ROM_EffectBlockCount: sprintf (cstrName, "ROM Effect Block Count"); break;
			case kHIDUsage_PID_SimultaneousEffectsMax: sprintf (cstrName, "Simultaneous Effects Max"); break;
			case kHIDUsage_PID_PoolAlignment: sprintf (cstrName, "Pool Alignment"); break;

			case kHIDUsage_PID_PoolMoveReport: sprintf (cstrName, "PID Pool Move Report"); break;
			case kHIDUsage_PID_MoveSource: sprintf (cstrName, "Move Source"); break;
			case kHIDUsage_PID_MoveDestination: sprintf (cstrName, "Move Destination"); break;
			case kHIDUsage_PID_MoveLength: sprintf (cstrName, "Move Length"); break;

			case kHIDUsage_PID_BlockLoadReport: sprintf (cstrName, "PID Block Load Report"); break;

			case kHIDUsage_PID_BlockLoadStatus: sprintf (cstrName, "Block Load Status"); break;
			case kHIDUsage_PID_BlockLoadSuccess: sprintf (cstrName, "Block Load Success"); break;
			case kHIDUsage_PID_BlockLoadFull: sprintf (cstrName, "Block Load Full"); break;
			case kHIDUsage_PID_BlockLoadError: sprintf (cstrName, "Block Load Error"); break;
			case kHIDUsage_PID_BlockHandle: sprintf (cstrName, "Block Handle"); break;

			case kHIDUsage_PID_BlockFreeReport: sprintf (cstrName, "PID Block Free Report"); break;

			case kHIDUsage_PID_TypeSpecificBlockHandle: sprintf (cstrName, "Type Specific Block Handle"); break;

			case kHIDUsage_PID_StateReport: sprintf (cstrName, "PID State Report"); break;
			case kHIDUsage_PID_EffectPlaying: sprintf (cstrName, "Effect Playing"); break;

			case kHIDUsage_PID_DeviceControlReport: sprintf (cstrName, "PID Device Control Report"); break;

			case kHIDUsage_PID_DeviceControl: sprintf (cstrName, "PID Device Control"); break;
			case kHIDUsage_PID_DC_EnableActuators: sprintf (cstrName, "Device Control Enable Actuators"); break;
			case kHIDUsage_PID_DC_DisableActuators: sprintf (cstrName, "Device Control Disable Actuators"); break;
			case kHIDUsage_PID_DC_StopAllEffects: sprintf (cstrName, "Device Control Stop All Effects"); break;
			case kHIDUsage_PID_DC_DeviceReset: sprintf (cstrName, "Device Control Reset"); break;
			case kHIDUsage_PID_DC_DevicePause: sprintf (cstrName, "Device Control Pause"); break;
			case kHIDUsage_PID_DC_DeviceContinue: sprintf (cstrName, "Device Control Continue"); break;
			case kHIDUsage_PID_DevicePaused: sprintf (cstrName, "Device Paused"); break;
			case kHIDUsage_PID_ActuatorsEnabled: sprintf (cstrName, "Actuators Enabled"); break;
			case kHIDUsage_PID_SafetySwitch: sprintf (cstrName, "Safety Switch"); break;
			case kHIDUsage_PID_ActuatorOverrideSwitch: sprintf (cstrName, "Actuator Override Switch"); break;
			case kHIDUsage_PID_ActuatorPower: sprintf (cstrName, "Actuator Power"); break;
			case kHIDUsage_PID_StartDelay: sprintf (cstrName, "Start Delay"); break;

			case kHIDUsage_PID_ParameterBlockSize: sprintf (cstrName, "Parameter Block Size"); break;
			case kHIDUsage_PID_DeviceManagedPool: sprintf (cstrName, "Device Managed Pool"); break;
			case kHIDUsage_PID_SharedParameterBlocks: sprintf (cstrName, "Shared Parameter Blocks"); break;

			case kHIDUsage_PID_CreateNewEffectReport: sprintf (cstrName, "Create New Effect Report"); break;
			case kHIDUsage_PID_RAM_PoolAvailable: sprintf (cstrName, "RAM Pool Available"); break;
			default: sprintf (cstrName, "PID Usage 0x%lx", element->usage); break;
			}
		break;
	case kHIDPage_Unicode:
		switch (element->usage)
		{
		default: sprintf (cstrName, "Unicode Usage 0x%lx", element->usage); break;
		}
		break;
	case kHIDPage_PowerDevice:
		if (((element->usage >= 0x06) && (element->usage <= 0x0F)) || ((element->usage >= 0x26) && (element->usage <= 0x2F)) ||
			((element->usage >= 0x39) && (element->usage <= 0x3F)) || ((element->usage >= 0x48) && (element->usage <= 0x4F)) ||
			((element->usage >= 0x58) && (element->usage <= 0x5F)) || (element->usage == 0x6A) ||
			((element->usage >= 0x74) && (element->usage <= 0xFC)))
			sprintf (cstrName, "Power Device Reserved");
		else
			switch (element->usage)
			{
			case kHIDUsage_PD_Undefined: sprintf (cstrName, "Power Device Undefined Usage"); break;
			case kHIDUsage_PD_iName: sprintf (cstrName, "Power Device Name Index"); break;
			case kHIDUsage_PD_PresentStatus: sprintf (cstrName, "Power Device Present Status"); break;
			case kHIDUsage_PD_ChangedStatus: sprintf (cstrName, "Power Device Changed Status"); break;
			case kHIDUsage_PD_UPS: sprintf (cstrName, "Uninterruptible Power Supply"); break;
			case kHIDUsage_PD_PowerSupply: sprintf (cstrName, "Power Supply"); break;

			case kHIDUsage_PD_BatterySystem: sprintf (cstrName, "Battery System Power Module"); break;
			case kHIDUsage_PD_BatterySystemID: sprintf (cstrName, "Battery System ID"); break;
			case kHIDUsage_PD_Battery: sprintf (cstrName, "Battery"); break;
			case kHIDUsage_PD_BatteryID: sprintf (cstrName, "Battery ID"); break;
			case kHIDUsage_PD_Charger: sprintf (cstrName, "Charger"); break;
			case kHIDUsage_PD_ChargerID: sprintf (cstrName, "Charger ID"); break;
			case kHIDUsage_PD_PowerConverter: sprintf (cstrName, "Power Converter Power Module"); break;
			case kHIDUsage_PD_PowerConverterID: sprintf (cstrName, "Power Converter ID"); break;
			case kHIDUsage_PD_OutletSystem: sprintf (cstrName, "Outlet System power module"); break;
			case kHIDUsage_PD_OutletSystemID: sprintf (cstrName, "Outlet System ID"); break;
			case kHIDUsage_PD_Input: sprintf (cstrName, "Power Device Input"); break;
			case kHIDUsage_PD_InputID: sprintf (cstrName, "Power Device Input ID"); break;
			case kHIDUsage_PD_Output: sprintf (cstrName, "Power Device Output"); break;
			case kHIDUsage_PD_OutputID: sprintf (cstrName, "Power Device Output ID"); break;
			case kHIDUsage_PD_Flow: sprintf (cstrName, "Power Device Flow"); break;
			case kHIDUsage_PD_FlowID: sprintf (cstrName, "Power Device Flow ID"); break;
			case kHIDUsage_PD_Outlet: sprintf (cstrName, "Power Device Outlet"); break;
			case kHIDUsage_PD_OutletID: sprintf (cstrName, "Power Device Outlet ID"); break;
			case kHIDUsage_PD_Gang: sprintf (cstrName, "Power Device Gang"); break;
			case kHIDUsage_PD_GangID: sprintf (cstrName, "Power Device Gang ID"); break;
			case kHIDUsage_PD_PowerSummary: sprintf (cstrName, "Power Device Power Summary"); break;
			case kHIDUsage_PD_PowerSummaryID: sprintf (cstrName, "Power Device Power Summary ID"); break;

			case kHIDUsage_PD_Voltage: sprintf (cstrName, "Power Device Voltage"); break;
			case kHIDUsage_PD_Current: sprintf (cstrName, "Power Device Current"); break;
			case kHIDUsage_PD_Frequency: sprintf (cstrName, "Power Device Frequency"); break;
			case kHIDUsage_PD_ApparentPower: sprintf (cstrName, "Power Device Apparent Power"); break;
			case kHIDUsage_PD_ActivePower: sprintf (cstrName, "Power Device RMS Power"); break;
			case kHIDUsage_PD_PercentLoad: sprintf (cstrName, "Power Device Percent Load"); break;
			case kHIDUsage_PD_Temperature: sprintf (cstrName, "Power Device Temperature"); break;
			case kHIDUsage_PD_Humidity: sprintf (cstrName, "Power Device Humidity"); break;
			case kHIDUsage_PD_BadCount: sprintf (cstrName, "Power Device Bad Condition Count"); break;

			case kHIDUsage_PD_ConfigVoltage: sprintf (cstrName, "Power Device Nominal Voltage"); break;
			case kHIDUsage_PD_ConfigCurrent: sprintf (cstrName, "Power Device Nominal Current"); break;
			case kHIDUsage_PD_ConfigFrequency: sprintf (cstrName, "Power Device Nominal Frequency"); break;
			case kHIDUsage_PD_ConfigApparentPower: sprintf (cstrName, "Power Device Nominal Apparent Power"); break;
			case kHIDUsage_PD_ConfigActivePower: sprintf (cstrName, "Power Device Nominal RMS Power"); break;
			case kHIDUsage_PD_ConfigPercentLoad: sprintf (cstrName, "Power Device Nominal Percent Load"); break;
			case kHIDUsage_PD_ConfigTemperature: sprintf (cstrName, "Power Device Nominal Temperature"); break;

			case kHIDUsage_PD_ConfigHumidity: sprintf (cstrName, "Power Device Nominal Humidity"); break;
			case kHIDUsage_PD_SwitchOnControl: sprintf (cstrName, "Power Device Switch On Control"); break;
			case kHIDUsage_PD_SwitchOffControl: sprintf (cstrName, "Power Device Switch Off Control"); break;
			case kHIDUsage_PD_ToggleControl: sprintf (cstrName, "Power Device Toogle Sequence Control"); break;
			case kHIDUsage_PD_LowVoltageTransfer: sprintf (cstrName, "Power Device Min Transfer Voltage"); break;
			case kHIDUsage_PD_HighVoltageTransfer: sprintf (cstrName, "Power Device Max Transfer Voltage"); break;
			case kHIDUsage_PD_DelayBeforeReboot: sprintf (cstrName, "Power Device Delay Before Reboot"); break;
			case kHIDUsage_PD_DelayBeforeStartup: sprintf (cstrName, "Power Device Delay Before Startup"); break;
			case kHIDUsage_PD_DelayBeforeShutdown: sprintf (cstrName, "Power Device Delay Before Shutdown"); break;
			case kHIDUsage_PD_Test: sprintf (cstrName, "Power Device Test Request/Result"); break;
			case kHIDUsage_PD_ModuleReset: sprintf (cstrName, "Power Device Reset Request/Result"); break;
			case kHIDUsage_PD_AudibleAlarmControl: sprintf (cstrName, "Power Device Audible Alarm Control"); break;

			case kHIDUsage_PD_Present: sprintf (cstrName, "Power Device Present"); break;
			case kHIDUsage_PD_Good: sprintf (cstrName, "Power Device Good"); break;
			case kHIDUsage_PD_InternalFailure: sprintf (cstrName, "Power Device Internal Failure"); break;
			case kHIDUsage_PD_VoltageOutOfRange: sprintf (cstrName, "Power Device Voltage Out Of Range"); break;
			case kHIDUsage_PD_FrequencyOutOfRange: sprintf (cstrName, "Power Device Frequency Out Of Range"); break;
			case kHIDUsage_PD_Overload: sprintf (cstrName, "Power Device Overload"); break;
			case kHIDUsage_PD_OverCharged: sprintf (cstrName, "Power Device Over Charged"); break;
			case kHIDUsage_PD_OverTemperature: sprintf (cstrName, "Power Device Over Temperature"); break;
			case kHIDUsage_PD_ShutdownRequested: sprintf (cstrName, "Power Device Shutdown Requested"); break;

			case kHIDUsage_PD_ShutdownImminent: sprintf (cstrName, "Power Device Shutdown Imminent"); break;
			case kHIDUsage_PD_SwitchOnOff: sprintf (cstrName, "Power Device On/Off Switch Status"); break;
			case kHIDUsage_PD_Switchable: sprintf (cstrName, "Power Device Switchable"); break;
			case kHIDUsage_PD_Used: sprintf (cstrName, "Power Device Used"); break;
			case kHIDUsage_PD_Boost: sprintf (cstrName, "Power Device Boosted"); break;
			case kHIDUsage_PD_Buck: sprintf (cstrName, "Power Device Bucked"); break;
			case kHIDUsage_PD_Initialized: sprintf (cstrName, "Power Device Initialized"); break;
			case kHIDUsage_PD_Tested: sprintf (cstrName, "Power Device Tested"); break;
			case kHIDUsage_PD_AwaitingPower: sprintf (cstrName, "Power Device Awaiting Power"); break;
			case kHIDUsage_PD_CommunicationLost: sprintf (cstrName, "Power Device Communication Lost"); break;

			case kHIDUsage_PD_iManufacturer: sprintf (cstrName, "Power Device Manufacturer String Index"); break;
			case kHIDUsage_PD_iProduct: sprintf (cstrName, "Power Device Product String Index"); break;
			case kHIDUsage_PD_iserialNumber: sprintf (cstrName, "Power Device Serial Number String Index"); break;
			default: sprintf (cstrName, "Power Device Usage 0x%lx", element->usage); break;
			}
		break;
	case kHIDPage_BatterySystem:
		if (((element->usage >= 0x0A) && (element->usage <= 0x0F)) || ((element->usage >= 0x1E) && (element->usage <= 0x27)) ||
			((element->usage >= 0x30) && (element->usage <= 0x3F)) || ((element->usage >= 0x4C) && (element->usage <= 0x5F)) ||
			((element->usage >= 0x6C) && (element->usage <= 0x7F)) || ((element->usage >= 0x90) && (element->usage <= 0xBF)) ||
			((element->usage >= 0xC3) && (element->usage <= 0xCF)) || ((element->usage >= 0xDD) && (element->usage <= 0xEF)) ||
			((element->usage >= 0xF2) && (element->usage <= 0xFF)))
			sprintf (cstrName, "Power Device Reserved");
		else
			switch (element->usage)
			{
			case kHIDUsage_BS_Undefined: sprintf (cstrName, "Battery System Undefined"); break;
			case kHIDUsage_BS_SMBBatteryMode: sprintf (cstrName, "SMB Mode"); break;
			case kHIDUsage_BS_SMBBatteryStatus: sprintf (cstrName, "SMB Status"); break;
			case kHIDUsage_BS_SMBAlarmWarning: sprintf (cstrName, "SMB Alarm Warning"); break;
			case kHIDUsage_BS_SMBChargerMode: sprintf (cstrName, "SMB Charger Mode"); break;
			case kHIDUsage_BS_SMBChargerStatus: sprintf (cstrName, "SMB Charger Status"); break;
			case kHIDUsage_BS_SMBChargerSpecInfo: sprintf (cstrName, "SMB Charger Extended Status"); break;
			case kHIDUsage_BS_SMBSelectorState: sprintf (cstrName, "SMB Selector State"); break;
			case kHIDUsage_BS_SMBSelectorPresets: sprintf (cstrName, "SMB Selector Presets"); break;
			case kHIDUsage_BS_SMBSelectorInfo: sprintf (cstrName, "SMB Selector Info"); break;
			case kHIDUsage_BS_OptionalMfgFunction1: sprintf (cstrName, "Battery System Optional SMB Mfg Function 1"); break;
			case kHIDUsage_BS_OptionalMfgFunction2: sprintf (cstrName, "Battery System Optional SMB Mfg Function 2"); break;
			case kHIDUsage_BS_OptionalMfgFunction3: sprintf (cstrName, "Battery System Optional SMB Mfg Function 3"); break;
			case kHIDUsage_BS_OptionalMfgFunction4: sprintf (cstrName, "Battery System Optional SMB Mfg Function 4"); break;
			case kHIDUsage_BS_OptionalMfgFunction5: sprintf (cstrName, "Battery System Optional SMB Mfg Function 5"); break;
			case kHIDUsage_BS_ConnectionToSMBus: sprintf (cstrName, "Battery System Connection To System Management Bus"); break;
			case kHIDUsage_BS_OutputConnection: sprintf (cstrName, "Battery System Output Connection Status"); break;
			case kHIDUsage_BS_ChargerConnection: sprintf (cstrName, "Battery System Charger Connection"); break;
			case kHIDUsage_BS_BatteryInsertion: sprintf (cstrName, "Battery System Battery Insertion"); break;
			case kHIDUsage_BS_Usenext: sprintf (cstrName, "Battery System Use Next"); break;
			case kHIDUsage_BS_OKToUse: sprintf (cstrName, "Battery System OK To Use"); break;
			case kHIDUsage_BS_BatterySupported: sprintf (cstrName, "Battery System Battery Supported"); break;
			case kHIDUsage_BS_SelectorRevision: sprintf (cstrName, "Battery System Selector Revision"); break;
			case kHIDUsage_BS_ChargingIndicator: sprintf (cstrName, "Battery System Charging Indicator"); break;
			case kHIDUsage_BS_ManufacturerAccess: sprintf (cstrName, "Battery System Manufacturer Access"); break;
			case kHIDUsage_BS_RemainingCapacityLimit: sprintf (cstrName, "Battery System Remaining Capacity Limit"); break;
			case kHIDUsage_BS_RemainingTimeLimit: sprintf (cstrName, "Battery System Remaining Time Limit"); break;
			case kHIDUsage_BS_AtRate: sprintf (cstrName, "Battery System At Rate..."); break;
			case kHIDUsage_BS_CapacityMode: sprintf (cstrName, "Battery System Capacity Mode"); break;
			case kHIDUsage_BS_BroadcastToCharger: sprintf (cstrName, "Battery System Broadcast To Charger"); break;
			case kHIDUsage_BS_PrimaryBattery: sprintf (cstrName, "Battery System Primary Battery"); break;
			case kHIDUsage_BS_ChargeController: sprintf (cstrName, "Battery System Charge Controller"); break;
			case kHIDUsage_BS_TerminateCharge: sprintf (cstrName, "Battery System Terminate Charge"); break;
			case kHIDUsage_BS_TerminateDischarge: sprintf (cstrName, "Battery System Terminate Discharge"); break;
			case kHIDUsage_BS_BelowRemainingCapacityLimit: sprintf (cstrName, "Battery System Below Remaining Capacity Limit"); break;
			case kHIDUsage_BS_RemainingTimeLimitExpired: sprintf (cstrName, "Battery System Remaining Time Limit Expired"); break;
			case kHIDUsage_BS_Charging: sprintf (cstrName, "Battery System Charging"); break;
			case kHIDUsage_BS_Discharging: sprintf (cstrName, "Battery System Discharging"); break;
			case kHIDUsage_BS_FullyCharged: sprintf (cstrName, "Battery System Fully Charged"); break;
			case kHIDUsage_BS_FullyDischarged: sprintf (cstrName, "Battery System Fully Discharged"); break;
			case kHIDUsage_BS_ConditioningFlag: sprintf (cstrName, "Battery System Conditioning Flag"); break;
			case kHIDUsage_BS_AtRateOK: sprintf (cstrName, "Battery System At Rate OK"); break;
			case kHIDUsage_BS_SMBErrorCode: sprintf (cstrName, "Battery System SMB Error Code"); break;
			case kHIDUsage_BS_NeedReplacement: sprintf (cstrName, "Battery System Need Replacement"); break;
			case kHIDUsage_BS_AtRateTimeToFull: sprintf (cstrName, "Battery System At Rate Time To Full"); break;
			case kHIDUsage_BS_AtRateTimeToEmpty: sprintf (cstrName, "Battery System At Rate Time To Empty"); break;
			case kHIDUsage_BS_AverageCurrent: sprintf (cstrName, "Battery System Average Current"); break;
			case kHIDUsage_BS_Maxerror: sprintf (cstrName, "Battery System Max Error"); break;
			case kHIDUsage_BS_RelativeStateOfCharge: sprintf (cstrName, "Battery System Relative State Of Charge"); break;
			case kHIDUsage_BS_AbsoluteStateOfCharge: sprintf (cstrName, "Battery System Absolute State Of Charge"); break;
			case kHIDUsage_BS_RemainingCapacity: sprintf (cstrName, "Battery System Remaining Capacity"); break;
			case kHIDUsage_BS_FullChargeCapacity: sprintf (cstrName, "Battery System Full Charge Capacity"); break;
			case kHIDUsage_BS_RunTimeToEmpty: sprintf (cstrName, "Battery System Run Time To Empty"); break;
			case kHIDUsage_BS_AverageTimeToEmpty: sprintf (cstrName, "Battery System Average Time To Empty"); break;
			case kHIDUsage_BS_AverageTimeToFull: sprintf (cstrName, "Battery System Average Time To Full"); break;
			case kHIDUsage_BS_CycleCount: sprintf (cstrName, "Battery System Cycle Count"); break;
			case kHIDUsage_BS_BattPackModelLevel: sprintf (cstrName, "Battery System Batt Pack Model Level"); break;
			case kHIDUsage_BS_InternalChargeController: sprintf (cstrName, "Battery System Internal Charge Controller"); break;
			case kHIDUsage_BS_PrimaryBatterySupport: sprintf (cstrName, "Battery System Primary Battery Support"); break;
			case kHIDUsage_BS_DesignCapacity: sprintf (cstrName, "Battery System Design Capacity"); break;
			case kHIDUsage_BS_SpecificationInfo: sprintf (cstrName, "Battery System Specification Info"); break;
			case kHIDUsage_BS_ManufacturerDate: sprintf (cstrName, "Battery System Manufacturer Date"); break;
			case kHIDUsage_BS_SerialNumber: sprintf (cstrName, "Battery System Serial Number"); break;
			case kHIDUsage_BS_iManufacturerName: sprintf (cstrName, "Battery System Manufacturer Name Index"); break;
			case kHIDUsage_BS_iDevicename: sprintf (cstrName, "Battery System Device Name Index"); break;
			case kHIDUsage_BS_iDeviceChemistry: sprintf (cstrName, "Battery System Device Chemistry Index"); break;
			case kHIDUsage_BS_ManufacturerData: sprintf (cstrName, "Battery System Manufacturer Data"); break;
			case kHIDUsage_BS_Rechargable: sprintf (cstrName, "Battery System Rechargable"); break;
			case kHIDUsage_BS_WarningCapacityLimit: sprintf (cstrName, "Battery System Warning Capacity Limit"); break;
			case kHIDUsage_BS_CapacityGranularity1: sprintf (cstrName, "Battery System Capacity Granularity 1"); break;
			case kHIDUsage_BS_CapacityGranularity2: sprintf (cstrName, "Battery System Capacity Granularity 2"); break;
			case kHIDUsage_BS_iOEMInformation: sprintf (cstrName, "Battery System OEM Information Index"); break;
			case kHIDUsage_BS_InhibitCharge: sprintf (cstrName, "Battery System Inhibit Charge"); break;
			case kHIDUsage_BS_EnablePolling: sprintf (cstrName, "Battery System Enable Polling"); break;
			case kHIDUsage_BS_ResetToZero: sprintf (cstrName, "Battery System Reset To Zero"); break;
			case kHIDUsage_BS_ACPresent: sprintf (cstrName, "Battery System AC Present"); break;
			case kHIDUsage_BS_BatteryPresent: sprintf (cstrName, "Battery System Battery Present"); break;
			case kHIDUsage_BS_PowerFail: sprintf (cstrName, "Battery System Power Fail"); break;
			case kHIDUsage_BS_AlarmInhibited: sprintf (cstrName, "Battery System Alarm Inhibited"); break;
			case kHIDUsage_BS_ThermistorUnderRange: sprintf (cstrName, "Battery System Thermistor Under Range"); break;
			case kHIDUsage_BS_ThermistorHot: sprintf (cstrName, "Battery System Thermistor Hot"); break;
			case kHIDUsage_BS_ThermistorCold: sprintf (cstrName, "Battery System Thermistor Cold"); break;
			case kHIDUsage_BS_ThermistorOverRange: sprintf (cstrName, "Battery System Thermistor Over Range"); break;
			case kHIDUsage_BS_VoltageOutOfRange: sprintf (cstrName, "Battery System Voltage Out Of Range"); break;
			case kHIDUsage_BS_CurrentOutOfRange: sprintf (cstrName, "Battery System Current Out Of Range"); break;
			case kHIDUsage_BS_CurrentNotRegulated: sprintf (cstrName, "Battery System Current Not Regulated"); break;
			case kHIDUsage_BS_VoltageNotRegulated: sprintf (cstrName, "Battery System Voltage Not Regulated"); break;
			case kHIDUsage_BS_MasterMode: sprintf (cstrName, "Battery System Master Mode"); break;
			case kHIDUsage_BS_ChargerSelectorSupport: sprintf (cstrName, "Battery System Charger Support Selector"); break;
			case kHIDUsage_BS_ChargerSpec: sprintf (cstrName, "attery System Charger Specification"); break;
			case kHIDUsage_BS_Level2: sprintf (cstrName, "Battery System Charger Level 2"); break;
			case kHIDUsage_BS_Level3: sprintf (cstrName, "Battery System Charger Level 3"); break;
			default: sprintf (cstrName, "Battery System Usage 0x%lx", element->usage); break;
			}
		break;
	case kHIDPage_AlphanumericDisplay:
		switch (element->usage)
		{
		default: sprintf (cstrName, "Alphanumeric Display Usage 0x%lx", element->usage); break;
		}
		break;
	case kHIDPage_BarCodeScanner:
		switch (element->usage)
		{
		default: sprintf (cstrName, "Bar Code Scanner Usage 0x%lx", element->usage); break;
		}
		break;
	case kHIDPage_Scale:
		switch (element->usage)
		{
		default: sprintf (cstrName, "Scale Usage 0x%lx", element->usage); break;
		}
		break;
	case kHIDPage_CameraControl:
		switch (element->usage)
		{
		default: sprintf (cstrName, "Camera Control Usage 0x%lx", element->usage); break;
		}
		break;
	case kHIDPage_Arcade:
		switch (element->usage)
		{
		default: sprintf (cstrName, "Arcade Usage 0x%lx", element->usage); break;
		}
		break;
	default:
		if (element->usagePage > kHIDPage_VendorDefinedStart)
			sprintf (cstrName, "Vendor Defined Usage 0x%lx", element->usage);
		else
			sprintf (cstrName, "Page: 0x%lx, Usage: 0x%lx", element->usagePage, element->usage);
		break;
    }
	 
	return(cstrName);
}




#endif  /* #ifdef __APPLE__ */

