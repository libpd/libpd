/* --------------------------------------------------------------------------*/
/*                                                                           */
/* Pd interface to the USB HID API using libhid, which is built on libusb    */
/* Written by Hans-Christoph Steiner <hans@at.or.at>                         */
/*                                                                           */
/* Copyright (c) 2004 Hans-Christoph Steiner                                 */
/*                                                                           */
/* This program is free software; you can redistribute it and/or             */
/* modify it under the terms of the GNU General Public License               */
/* as published by the Free Software Foundation; either version 3            */
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
/* Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.           */
/*                                                                           */
/* --------------------------------------------------------------------------*/

/* libhid hasn't been ported to Win32 yet */
#ifndef _WIN32

#include <usb.h>
#include <hid.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <float.h>
#include "m_pd.h"

/* 
 * TODO: rename read, get, set messages so they make sense
 * TODO: make size output work well
 */

/* 
 * BUG: for some reason it skips one of the button bits
 */


/*------------------------------------------------------------------------------
 *  INCLUDE KLUDGE
 */

/* NOTE: included from libusb/usbi.h. UGLY, i know, but so is libusb! 
 * this struct doesn't seem to be defined in usb.h, only prototyped
 */
struct usb_dev_handle {
	int fd;
	struct usb_bus *bus;
	struct usb_device *device;
	int config;
	int interface;
	int altsetting;
	void *impl_info;
};


/*------------------------------------------------------------------------------
 *  GLOBAL VARIABLES
 */

/* count the number of instances of this object so that certain free()
 * functions can be called only after the final instance is detroyed. */
t_int usbhid_instance_count;

#define HID_ID_MAX 32
char *hid_id[HID_ID_MAX]; /* FIXME: 32 devices MAX */
t_int hid_id_count;

/* this array is for keeping track of whether each device has been read in
 * each cycle */
char usbhid_device_read_status[HID_ID_MAX];

/*------------------------------------------------------------------------------
 *  CLASS DEF
 */

typedef struct _usbhid 
{
	t_object            x_obj;
/* usbhid types */
	HIDInterface        *x_hidinterface;
	hid_return          x_hid_return;
/* internal state */
	unsigned short      vendor_id;    // idVendor for current device
	unsigned short      product_id;   // idProduct for current device
	unsigned short      debug_level;  // control debug messages to the console
	t_int               x_device_number;
	t_int               x_read_element_count;
	t_int               *x_read_elements;
	t_int               x_write_path_count;
	t_int               *x_write_paths;
	t_int               report_size;  // size in bytes of the HID report
/* output */
	t_atom              *output; // holder for a list of atoms to be outputted
	t_int               output_count;  // number of atoms in in x->output 
	t_outlet            *x_data_outlet;
	t_outlet            *x_status_outlet;
} t_usbhid;


/*------------------------------------------------------------------------------
 * LOCAL DEFINES
 */

#define USBHID_MAJOR_VERSION 0
#define USBHID_MINOR_VERSION 0

static t_class *usbhid_class;

/* for USB strings */
#define STRING_BUFFER_LENGTH 256

#define SEND_PACKET_LENGTH 1
#define RECEIVE_PACKET_LENGTH 6
#define PATH_LENGTH 3


/*------------------------------------------------------------------------------
 * SUPPORT FUNCTIONS
 */

/* add one new atom to the list to be outputted */
static void add_atom_to_output(t_usbhid *x, t_atom *new_atom)
{
    t_atom *new_atom_list;

	new_atom_list = (t_atom *)getbytes((x->output_count + 1) * sizeof(t_atom));
	memcpy(new_atom_list, x->output, x->output_count * sizeof(t_atom));
	freebytes(x->output, x->output_count * sizeof(t_atom));
	x->output = new_atom_list;
	memcpy(x->output + x->output_count, new_atom, sizeof(t_atom));
	++(x->output_count);
}

static void add_symbol_to_output(t_usbhid *x, t_symbol *s)
{
	t_atom *temp_atom = getbytes(sizeof(t_atom));
	SETSYMBOL(temp_atom, s); 
	add_atom_to_output(x,temp_atom);
	freebytes(temp_atom,sizeof(t_atom));
}
		
static void add_float_to_output(t_usbhid *x, t_float f)
{
	t_atom *temp_atom = getbytes(sizeof(t_atom));
	SETFLOAT(temp_atom, f);
	add_atom_to_output(x,temp_atom);
	freebytes(temp_atom,sizeof(t_atom));
}

static void reset_output(t_usbhid *x)
{
	if(x->output)
	{
		freebytes(x->output, x->output_count * sizeof(t_atom));
		x->output = NULL;
		x->output_count = 0;
	}
}


static t_int init_libhid(t_usbhid *x)
{
	if (! hid_is_initialised() )
	{
		x->x_hid_return = hid_init();
		if(x->x_hid_return != HID_RET_SUCCESS)
		{
			error("[usbhid] hid_init failed with return code %d\n", 
				  x->x_hid_return);
		}
	}
	return(x->x_hid_return);
}


/* 
 * This function is used in a HIDInterfaceMatcher to iterate thru all of the
 * HID devices on the USB bus
 */
static bool device_iterator (struct usb_dev_handle const* usbdev, void* custom, 
							 unsigned int length)
{
	bool ret = false;
	t_int i;
	char current_dev_path[10];
  
	/* only here to prevent the unused warning */
	/* TODO remove */
	length = *((unsigned long*)custom);
 
	/* Obtain the device's full path */
	sprintf(current_dev_path, "%s/%s", usbdev->bus->dirname, usbdev->device->filename);

	/* Check if we already saw this dev */
	for ( i = 0 ; ( hid_id[i] != NULL ) ; i++ )
	{
		if (!strcmp(hid_id[i], current_dev_path ) )
			break;
	}
  
	/* Append device to the list if needed */
	if (hid_id[i] == NULL)
	{
		hid_id[i] = (char *) malloc(strlen(usbdev->device->filename) + strlen(usbdev->bus->dirname) );
		sprintf(hid_id[i], "%s/%s", usbdev->bus->dirname, usbdev->device->filename);
	}
	else /* device already seen */
	{
		return false;
	}
  
	/* Filter non HID device */
	if ( (usbdev->device->descriptor.bDeviceClass == 0) /* Class defined at interface level */
		 && usbdev->device->config
		 && usbdev->device->config->interface->altsetting->bInterfaceClass == USB_CLASS_HID)
	{
		
		post("bus %s device %s: %d %d",
			 usbdev->bus->dirname, 
			 usbdev->device->filename,
			 usbdev->device->descriptor.idVendor,
			 usbdev->device->descriptor.idProduct);
		ret = true;
	}
	
	else
	{
		ret = false;
	}
	
  
	return ret;
}

/* -------------------------------------------------------------------------- */
/* This function is used in a HIDInterfaceMatcher in order to match devices by
 * serial number.
 */
/* static bool match_serial_number(struct usb_dev_handle* usbdev, void* custom, unsigned int length) */
/* { */
/*   bool ret; */
/*   char* buffer = (char*)malloc(length); */
/*   usb_get_string_simple(usbdev, usb_device(usbdev)->descriptor.iSerialNumber, */
/*       buffer, length); */
/*   ret = strncmp(buffer, (char*)custom, length) == 0; */
/*   free(buffer); */
/*   return ret; */
/* } */


/* -------------------------------------------------------------------------- */
/* static bool get_device_by_number(struct usb_dev_handle* usbdev,  */
/* 								 void* custom,  */
/* 								 unsigned int length)  */
/* { */
/* 	bool ret; */
	
	
/* 	return ret; */
/* } */


/* -------------------------------------------------------------------------- */
static long* make_hid_path(t_int argc, t_atom *argv)
{
	t_int i;
    t_symbol *tmp_symbol;
	t_int *return_array = NULL;
	
    // TODO: free memory first
	return_array = (t_int *) getbytes(sizeof(t_int) * argc);
/* A usbhid path component is 32 bits, the high 16 bits identify the usage page,
 * and the low 16 bits the item number.
 */
/* TODO: print error if a symbol is found in the data list */
    for(i=0; i<argc; i++) {
        tmp_symbol = atom_getsymbolarg(i, argc, argv);
        if(tmp_symbol == &s_) // non-symbol is hopefully a float
            return_array[i] = atom_getintarg(i, argc, argv);
        else
            return_array[i] = strtoul(tmp_symbol->s_name, 0, 16);
        post("make_hid_path[%d]: 0x%08x %s", i, 
             return_array[i], tmp_symbol->s_name);
    }
	return return_array;
}

static char* make_hid_packet(t_int argc, t_atom *argv)
{
	t_int i;
    t_symbol *tmp_symbol;
	char *return_array = NULL;
	
    // TODO: free memory first
	return_array = (char *) getbytes(sizeof(char) * argc);
/* A usbhid path component is 32 bits, the high 16 bits identify the usage page,
 * and the low 16 bits the item number.
 */
/* TODO: print error if a symbol is found in the data list */
    for(i=0; i<argc; i++) {
        tmp_symbol = atom_getsymbolarg(i, argc, argv);
        if(tmp_symbol == &s_) // non-symbol is hopefully a float
            return_array[i] = (char) atom_getintarg(i, argc, argv);
        else
            return_array[i] = (char) strtoul(tmp_symbol->s_name, 0, 16);
        post("make_hid_packet[%d]: 0x%02x %s", i, 
             return_array[i], tmp_symbol->s_name);
    }
	return return_array;
}

static t_int get_device_string(HIDInterface *hidif, char *device_string)
{
	int length;
	t_int ret = 0;
	char buffer[STRING_BUFFER_LENGTH] = "";
	char return_buffer[STRING_BUFFER_LENGTH] = "";

	if ( !hid_is_opened(hidif) ) 
		return(0);

	if (hidif->device->descriptor.iManufacturer) {
		length = usb_get_string_simple(hidif->dev_handle,
									   hidif->device->descriptor.iManufacturer, 
									   buffer, 
									   STRING_BUFFER_LENGTH);
		if (length > 0)
		{
			strncat(return_buffer, buffer, STRING_BUFFER_LENGTH - strlen(device_string));
			strncat(return_buffer, " ",1);
			ret = 1;
		}
		else
		{
			post("(unable to fetch manufacturer string)");
		}
	}
	
	if (hidif->device->descriptor.iProduct) {
		length = usb_get_string_simple(hidif->dev_handle,
									   hidif->device->descriptor.iProduct, 
									   buffer, 
									   STRING_BUFFER_LENGTH);
		if (length > 0)
		{
			strncat(return_buffer, buffer, STRING_BUFFER_LENGTH - strlen(device_string));
			strncat(return_buffer, " ",1);
			ret = 1;
		}
		else
		{
			post("(unable to fetch product string)");
		}
	}

	if (hidif->device->descriptor.iSerialNumber) {
		length = usb_get_string_simple(hidif->dev_handle,
									   hidif->device->descriptor.iSerialNumber, 
									   buffer, 
									   STRING_BUFFER_LENGTH);
		if (length > 0)
			strncat(return_buffer, buffer, STRING_BUFFER_LENGTH - strlen(device_string));
		else
			post("(unable to fetch product string)");
	}

	if (return_buffer)
		strncpy(device_string, return_buffer, STRING_BUFFER_LENGTH);

	return ret;
}


/*------------------------------------------------------------------------------
 * IMPLEMENTATION                    
 */


/* -------------------------------------------------------------------------- */
static void usbhid_close(t_usbhid *x) 
{
	if(x->debug_level) post("usbhid_close");
	t_int ret;
	char string_buffer[STRING_BUFFER_LENGTH];

	if ( hid_is_opened(x->x_hidinterface) ) 
	{
		ret = get_device_string(x->x_hidinterface,string_buffer);
		x->x_hid_return = hid_close(x->x_hidinterface);
		if (x->x_hid_return == HID_RET_SUCCESS) 
		{
			if (ret) 
				post("[usbhid]: closed %s",string_buffer);
			else
				post("[usbhid]: closed device");
			reset_output(x);
			add_float_to_output(x,0);
			outlet_anything(x->x_status_outlet, gensym("open"), 
							x->output_count, x->output);
		}
		else
		{
			error("[usbhid] could not close %d, error #%d",x->x_device_number,x->x_hid_return);
		}
	}
}


/* -------------------------------------------------------------------------- */
static void usbhid_open(t_usbhid *x, t_symbol *vendor_id_hex, t_symbol *product_id_hex)
{
	if(x->debug_level) post("usbhid_open");
	char string_buffer[STRING_BUFFER_LENGTH];

	if( init_libhid(x) != HID_RET_SUCCESS ) return;

/* convert hex symbols to ints */
	x->vendor_id = (unsigned short) strtol(vendor_id_hex->s_name, NULL, 16);
	x->product_id = (unsigned short) strtol(product_id_hex->s_name, NULL, 16);
 	if( hid_is_opened(x->x_hidinterface) ) 
	{
		if( (x->vendor_id == x->x_hidinterface->device->descriptor.idVendor) &&
			(x->product_id == x->x_hidinterface->device->descriptor.idProduct))
		{
			post("[usbhid] device already opened");
			return;
		}
		else
		{
			usbhid_close(x);
		}
	}

 	if( !hid_is_opened(x->x_hidinterface) ) 
	{
		HIDInterfaceMatcher matcher = { x->vendor_id, 
										x->product_id, 
										NULL, 
										NULL, 
										0 };
		x->x_hid_return = hid_force_open(x->x_hidinterface, 0, &matcher, 3);
		if (x->x_hid_return == HID_RET_SUCCESS) 
		{
			if (get_device_string(x->x_hidinterface,string_buffer))
				post("[usbhid]: opened %s",string_buffer);
			reset_output(x);
			add_float_to_output(x,1);
			outlet_anything(x->x_status_outlet, gensym("open"), 
							x->output_count, x->output);
		}
		else
		{
			error("[usbhid] hid_force_open failed with return code %d\n", 
				  x->x_hid_return);
		}
	}
}


/* -------------------------------------------------------------------------- */
static void usbhid_get(t_usbhid *x, t_float length_arg)
{
	if(x->debug_level) post("usbhid_get");
	int i;
	int packet_bytes = (int)length_arg;
	char packet[packet_bytes];

 	if ( !hid_is_opened(x->x_hidinterface) )
	{
		error("[usbhid] device not open, can't get data");
		return;
	}
	x->x_hid_return = hid_get_input_report(x->x_hidinterface, 
										   NULL, 
										   x->x_read_element_count, 
										   packet, 
										   length_arg);
	if (x->x_hid_return != HID_RET_SUCCESS) 
	{
		error("[usbhid] hid_get_input_report failed with return code %d\n", 
			  x->x_hid_return);
		reset_output(x);
		add_float_to_output(x, x->x_hid_return);
		outlet_anything(x->x_status_outlet, gensym("getError"), 
						x->output_count, x->output);
	}

	reset_output(x);
	for(i=0; i<packet_bytes; ++i)
		add_float_to_output(x,packet[i]);
	outlet_list(x->x_data_outlet, &s_list, x->output_count, x->output);
	post("x->x_read_elements %d",x->x_read_elements);
}


/* -------------------------------------------------------------------------- */
static void usbhid_path(t_usbhid *x,  t_symbol *s, int argc, t_atom *argv) 
{
	if(x->debug_level) post("usbhid_path");
    int i;
    t_symbol *tmp_symbol;

	if(argc > x->x_write_path_count) {
        if(x->debug_level) post("usbhid_path: freeing/allocating memory");
        freebytes(x->x_write_paths, sizeof(t_int) * x->x_write_path_count);
        x->x_write_paths = (t_int *) getbytes(sizeof(t_int) * argc);
    }
    
    for(i=0; i<argc; i++) {
        tmp_symbol = atom_getsymbolarg(i, argc, argv);
        x->x_write_paths[i] = strtoul(tmp_symbol->s_name, 0, 16);
        post("x->x_write_paths[%d]: 0x%08x %s", i, x->x_write_paths[i], tmp_symbol->s_name);
    }
}


/* -------------------------------------------------------------------------- */
static void usbhid_set(t_usbhid *x,  t_symbol *s, int argc, t_atom *argv) 
{
	if(x->debug_level) post("usbhid_set");
	char *packet;

 	if ( !hid_is_opened(x->x_hidinterface) )
	{
		error("[usbhid] device not open, can't set data");
		return;
	}
    packet = make_hid_packet(argc, argv);
	x->x_hid_return = hid_set_output_report(x->x_hidinterface, 
                                            x->x_write_paths, 
                                            x->x_write_path_count, 
                                            packet, 
                                            argc);

	if (x->x_hid_return != HID_RET_SUCCESS) 
	{
		error("[usbhid] hid_get_input_report failed with return code %d\n", 
			  x->x_hid_return);
		reset_output(x);
		add_float_to_output(x, x->x_hid_return);
		outlet_anything(x->x_status_outlet, gensym("set_error"), 
						x->output_count, x->output);
	}
}


/* -------------------------------------------------------------------------- */
static void usbhid_read(t_usbhid *x)
{
	if(x->debug_level) post("usbhid_read");
}


/* -------------------------------------------------------------------------- */
static void usbhid_write(t_usbhid *x,  t_symbol *s, int argc, t_atom *argv) 
{
	if(x->debug_level) post("usbhid_write");
    int i;
//	const int path[] = {0x000c0001, 0x000c0001};
//	int path[] = {0x00010005, 0x00010036};
	int path[] = {0xffa00001, 0xffa00005};
//    int *path;
	unsigned int const depth = 2;  // number of 32bit chunks in the path
	unsigned char const SEND_PACKET_LEN = 1; // number of bytes in packet
	char const PACKET[] = { 0xff, 0xff }; // the data to write
//    char const PACKET[] = { 0x00, 0x00 }; // the data to write
//	char PACKET[] = { 0x00, 0x00 }; // the data to write
    

 	if ( !hid_is_opened(x->x_hidinterface) )
	{
		error("[usbhid] device not open, can't set data");
		return;
	}
/*
    path = getbytes(sizeof(int) * (argc - 1));
    depth = (argc - 1) / 2;
    for(i = 0; i < argc - 1; ++i)
    {
        path[(i+1)/2] = (strtol(atom_getsymbol(argv + i)->s_name, NULL, 16) << 16) + 
            (strtol(atom_getsymbol(argv + i + 1)->s_name, NULL, 16) & 0x0000ffff);
        ++i;
    }
    SEND_PACKET_LEN = 2;
    PACKET[1] = (unsigned short) atom_getfloat(argv + argc - 1);
*/
    post("depth: %d  SEND_PACKET_LEN: %d   PACKET[0]: %d  PACKET[1]: %d", 
         depth, SEND_PACKET_LEN, PACKET[0], PACKET[1]);
    for(i = 0; i < (argc - 1) / 2; ++i)
    {
        post("path %d: 0x%08x", i, path[i]);
    }
    
	x->x_hid_return = hid_set_output_report(x->x_hidinterface, 
											path, depth, PACKET,
											SEND_PACKET_LEN);
	if (x->x_hid_return != HID_RET_SUCCESS) 
	{
		error("[usbhid] hid_set_output_report failed with return code %d", 
			  x->x_hid_return);
		reset_output(x);
		add_float_to_output(x, x->x_hid_return);
		outlet_anything(x->x_status_outlet, gensym("get_error"), 
						x->output_count, x->output);
	}
	post("wrote");
}


/* -------------------------------------------------------------------------- */
/* reinit libhid to get update the list of attached devices */
static void usbhid_refresh(t_usbhid *x)
{
	x->x_hid_return = hid_cleanup();
	if (x->x_hid_return != HID_RET_SUCCESS) 
		error("[usbhid] hid_cleanup failed with return code %d\n", 
			  x->x_hid_return);
	if( init_libhid(x) != HID_RET_SUCCESS ) return;
}

/* -------------------------------------------------------------------------- */
/* convert a list to a HID packet and set it */
/* set the HID packet for which elements to write */
static void usbhid_set_descriptor(t_usbhid *x, int argc, t_atom *argv)
{
	if(x->debug_level) post("usbhid_descriptor");
/* int const PATH_IN[PATH_LENGTH] = { 0xffa00001, 0xffa00002, 0xffa10003 }; */
//	int const PATH_OUT[3] = { 0x00010002, 0x00010001, 0x00010030 };
	t_int i;

}



/* -------------------------------------------------------------------------- */
static void usbhid_get_descriptor(t_usbhid *x)
{
	if(x->debug_level) post("usbhid_get");
	unsigned int i = 0;
	t_int input_size = 0;
	t_int output_size = 0;
	t_int feature_size = 0;
    char buf[MAXPDSTRING];

	if (!hid_is_opened(x->x_hidinterface)) {
		error("[usbget] cannot dump tree of unopened HIDinterface.");
	}
	else 
	{
		post("[usbhid] parse tree of HIDInterface %s:\n", x->x_hidinterface->id);
//		reset_output(x);
		while (HIDParse(x->x_hidinterface->hid_parser, x->x_hidinterface->hid_data)) {
            reset_output(x);
//			add_symbol_to_output(x, gensym("path"));
			switch(x->x_hidinterface->hid_data->Type)
			{
			case 0x80: 
				add_symbol_to_output(x, gensym("input")); 
				input_size = input_size + x->x_hidinterface->hid_data->Size;
				break;
			case 0x90: 
				add_symbol_to_output(x, gensym("output")); 
				output_size = output_size + x->x_hidinterface->hid_data->Size;
				break;
			case 0xb0: 
				add_symbol_to_output(x, gensym("feature")); 
				feature_size = feature_size + x->x_hidinterface->hid_data->Size;
				break;
			default: add_symbol_to_output(x, gensym("UNKNOWN_TYPE"));
			}
			add_float_to_output(x, x->x_hidinterface->hid_data->Size);
			add_float_to_output(x, x->x_hidinterface->hid_data->Offset);
            add_symbol_to_output(x, gensym("path"));
			for (i = 0; i < x->x_hidinterface->hid_data->Path.Size; ++i) {
//                sprintf(buf, "0x%04x", x->x_hidinterface->hid_data->Path.Node[i].UPage);
//				add_symbol_to_output(x, gensym(buf));
//                sprintf(buf, "0x%04x", x->x_hidinterface->hid_data->Path.Node[i].Usage);
//				add_symbol_to_output(x, gensym(buf));
//                post("0x%04x%04x",x->x_hidinterface->hid_data->Path.Node[i].UPage,
//                     x->x_hidinterface->hid_data->Path.Node[i].Usage);
			}
			add_symbol_to_output(x, gensym("logical"));
			add_float_to_output(x, x->x_hidinterface->hid_data->LogMin);
			add_float_to_output(x, x->x_hidinterface->hid_data->LogMax);
            outlet_anything(x->x_status_outlet, gensym("element"), 
                            x->output_count, x->output);
		}
		reset_output(x);
//		add_symbol_to_output(x, gensym("totalSize"));
		add_float_to_output(x, input_size);
		add_float_to_output(x, output_size);
		add_float_to_output(x, feature_size);
		outlet_anything(x->x_status_outlet, gensym("totalSize"), 
						x->output_count, x->output);
//		outlet_anything(x->x_status_outlet, gensym("device"), 
//						x->output_count, x->output);
	}
}

/* -------------------------------------------------------------------------- */
static void usbhid_descriptor(t_usbhid *x, t_symbol *s, int argc, t_atom *argv)
{
	if(s == gensym("descriptor")) // get rid of unused s warning
	{
		if(argc)
			usbhid_set_descriptor(x,argc,argv);
		else
			usbhid_get_descriptor(x);
	}
}

/* -------------------------------------------------------------------------- */
static void usbhid_print(t_usbhid *x)
{
	if(x->debug_level) post("usbhid_print");
	t_int i;
	char string_buffer[STRING_BUFFER_LENGTH];
/* 	t_atom event_data[3]; */

	for ( i = 0 ; ( hid_id[i] != NULL ) ; i++ )
	{
		if( hid_id[i] != NULL )
			post("hid_id[%d]: %s",i,hid_id[i]);
	}
	if(get_device_string(x->x_hidinterface,string_buffer))
		post("%s is currently open",string_buffer);

	x->x_hid_return = hid_dump_tree(stdout, x->x_hidinterface);
	if (x->x_hid_return != HID_RET_SUCCESS) {
		fprintf(stderr, "hid_dump_tree failed with return code %d\n", x->x_hid_return);
		return;
	}
	
/* 	SETSYMBOL(event_data, gensym(type));	   /\* type *\/ */
/* 	SETSYMBOL(event_data + 1, gensym(code));	/\* code *\/ */
/* 	SETSYMBOL(event_data + 2, value);	         /\* value *\/ */
//	outlet_list(x->x_status_outlet, &s_list,
}


/* -------------------------------------------------------------------------- */
static void usbhid_reset(t_usbhid *x)
{
	if(x->debug_level) post("usbhid_reset");
	
	hid_reset_HIDInterface(x->x_hidinterface);
}


/* -------------------------------------------------------------------------- */
static void usbhid_debug(t_usbhid *x, t_float f)
{
	x->debug_level = f;
	
	switch(x->debug_level)
	{
	case 0: hid_set_usb_debug(0); hid_set_debug(HID_DEBUG_NONE); break;
	case 1: hid_set_usb_debug(0); hid_set_debug(HID_DEBUG_ERRORS); break;
	case 2: hid_set_usb_debug(0); 
		hid_set_debug(HID_DEBUG_ERRORS | HID_DEBUG_WARNINGS); break;
	case 3: hid_set_usb_debug(0); 
		hid_set_debug(HID_DEBUG_ERRORS | HID_DEBUG_WARNINGS | HID_DEBUG_NOTICES); break;
	case 4: hid_set_usb_debug(0); hid_set_debug(HID_DEBUG_NOTRACES); break;
	case 5: hid_set_usb_debug(0); hid_set_debug(HID_DEBUG_ALL); break;
	default:
		hid_set_usb_debug(x->debug_level - 5); hid_set_debug(HID_DEBUG_ALL); break;
	}
}


/* -------------------------------------------------------------------------- */
static void usbhid_free(t_usbhid* x) 
{
	if(x->debug_level) post("usbhid_free");
		
	usbhid_close(x);

	freebytes(x->x_read_elements,sizeof(t_int) * x->x_read_element_count);
	freebytes(x->x_write_paths,sizeof(t_int) * x->x_write_path_count);

	if(x->debug_level) 
		post("[usbhid] freeing instance %d",usbhid_instance_count);
	hid_delete_HIDInterface(&(x->x_hidinterface));
	if(usbhid_instance_count <= 1)
	{
		post("[usbhid] freeing last instance");
		x->x_hid_return = hid_cleanup();
		if (x->x_hid_return != HID_RET_SUCCESS) 
			error("[usbhid] hid_cleanup failed with return code %d\n", 
				  x->x_hid_return);
	}

	usbhid_instance_count--;
}



/* -------------------------------------------------------------------------- */
static void *usbhid_new(t_float f) 
{
	t_int i;
	HIDInterfaceMatcher matcher;
	t_usbhid *x = (t_usbhid *)pd_new(usbhid_class);
	
	if(x->debug_level) post("usbhid_new");
	
/* only display the version when the first instance is loaded */
	if(!usbhid_instance_count)
		post("[usbhid] %d.%d, written by Hans-Christoph Steiner <hans@eds.org>",
			 USBHID_MAJOR_VERSION, USBHID_MINOR_VERSION);  
	
	/* create anything outlet used for HID data */ 
	x->x_data_outlet = outlet_new(&x->x_obj, 0);
	x->x_status_outlet = outlet_new(&x->x_obj, 0);

	/* turn off the flood of libhid debug messages by default */
	hid_set_debug(HID_DEBUG_NONE);
	hid_set_debug_stream(stdout);
	hid_set_usb_debug(0);
	
	/* data init */
	x->output = NULL;
	x->output_count = 0;
	for (i = 0 ; i < HID_ID_MAX ; i++)
		hid_id[i] = NULL;
	
	x->x_hidinterface = hid_new_HIDInterface();
	matcher.vendor_id = HID_ID_MATCH_ANY;
	matcher.product_id = HID_ID_MATCH_ANY;
	matcher.matcher_fn = device_iterator;
    x->x_write_path_count = 0;
	
	x->x_device_number = f;
/* Open the device and save settings.  If there is an error, return the object
 * anyway, so that the inlets and outlets are created, thus not breaking the
 * patch.   */
/* 	if (usbhid_open(x,f)) */
/* 		error("[usbhid] device %d did not open",(t_int)f); */

	usbhid_instance_count++;

	return (x);
}

void usbhid_setup(void) 
{
	usbhid_class = class_new(gensym("usbhid"), 
							 (t_newmethod)usbhid_new, 
							 (t_method)usbhid_free,
							 sizeof(t_usbhid),
							 CLASS_DEFAULT,
							 A_DEFFLOAT,
							 NULL);
	
/* add inlet datatype methods */
//	class_addbang(usbhid_class,(t_method) usbhid_bang);

/* add inlet message methods */
	class_addmethod(usbhid_class,(t_method)usbhid_print, gensym("print"), 0);
	class_addmethod(usbhid_class,(t_method)usbhid_reset, gensym("reset"), 0);
	class_addmethod(usbhid_class,(t_method)usbhid_refresh, gensym("refresh"), 0);
	class_addmethod(usbhid_class,(t_method)usbhid_debug, gensym("debug"),
					A_DEFFLOAT,0);
	class_addmethod(usbhid_class,(t_method)usbhid_descriptor, gensym("descriptor"),
					A_GIMME, 0);
	class_addmethod(usbhid_class,(t_method)usbhid_path, gensym("path"),
					A_GIMME, 0);
	class_addmethod(usbhid_class,(t_method)usbhid_get,gensym("get"),
					A_DEFFLOAT, 0);
	class_addmethod(usbhid_class,(t_method)usbhid_set,gensym("set"),
					A_GIMME,0);
	class_addmethod(usbhid_class,(t_method)usbhid_read,gensym("read"), 
                    0);
	class_addmethod(usbhid_class,(t_method)usbhid_write,gensym("write"),
					A_GIMME, 0);
	class_addmethod(usbhid_class,(t_method)usbhid_open,gensym("open"),
					A_DEFSYM, A_DEFSYM, 0);
	class_addmethod(usbhid_class,(t_method)usbhid_close,gensym("close"), 0);
}

#endif /* NOT _WIN32 */
