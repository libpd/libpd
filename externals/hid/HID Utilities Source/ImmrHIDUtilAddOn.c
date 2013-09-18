/*
 *  ImmrHIDUtilAddOn.c
 *  UseFFAPIFromHIDUtilities
 *
 *  Created by rlacroix on Wed Oct 16 2002.
 *  Copyright (c) 2002 Immersion Corporation. All rights reserved.
 *
 */

#include <mach/mach.h>
#include <mach/mach_error.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hid/IOHIDUsageTables.h>

#include "HID_Utilities_External.h"
#include "ImmrHIDUtilAddOn.h"

//---------------------------------------------------------------------------------
//
// AllocateHIDObjectFromRecDevice()
//
//	returns:
//		NULL, or acceptable io_object_t
//
//---------------------------------------------------------------------------------
io_service_t AllocateHIDObjectFromRecDevice( pRecDevice pDevice )
{
    CFMutableDictionaryRef	matchingDict;
    UInt32					locationID = pDevice->locID;
    CFNumberRef				refUsage = NULL;
	mach_port_t masterPort = NULL;
    IOReturn result = kIOReturnSuccess;
	io_service_t	hidDevice = NULL;
	
    do // while( 0 )
	{
		result = IOMasterPort (bootstrap_port, &masterPort);
		if( result != kIOReturnSuccess )
		{
			break;
		}
		
		// Set up the matching criteria for the devices we're interested in.
		// We are interested in instances of class IOHIDDevice.
		// matchingDict is consumed below (in IOServiceGetMatchingService)
		// so we have no leak here.
		//
		matchingDict = IOServiceMatching(kIOHIDDeviceKey);
		if (!matchingDict)
		{
			break;
		}
	
		// Add a key for locationID to our matching dictionary.  This works for matching to 
		// IOHIDDevices, so we will only look for a device attached to that particular port
		// on the machine. 
		//  
		refUsage = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &locationID);
		CFDictionaryAddValue(   matchingDict,
								CFSTR(kIOHIDLocationIDKey),
								refUsage);
		CFRelease(refUsage);
		
		// IOServiceGetMatchingService assumes that we already know that there is only one device
		// that matches.  This way we don't have to do the whole iteration dance to look at each
		// device that matches.  This is a new API in 10.2
		//
		hidDevice = IOServiceGetMatchingService( masterPort, matchingDict);
	}
	while( 0 );
	
	// Free master port if we created one.
	//
	if (masterPort)
		mach_port_deallocate(mach_task_self(), masterPort);
   
   return hidDevice;
}

//---------------------------------------------------------------------------------
//
// FreeHIDObject()
//
//---------------------------------------------------------------------------------
bool FreeHIDObject( io_service_t hidDevice )
{
    kern_return_t			kr;

    kr = IOObjectRelease(hidDevice);

	return( kIOReturnSuccess == kr );
}