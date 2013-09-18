/*
	File:		HID_Config_Utilities.c

	Contains:   Implementation of the HID configuration utilities for the HID utilities.
    
	DRI: George Warner

	Copyright:	Copyright © 2002 Apple Computer, Inc., All Rights Reserved

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <stdlib.h> // malloc
#include <time.h> // clock

#include <IOKit/hid/IOHIDUsageTables.h>

#include "HID_Utilities_Internal.h"
#include "HID_Utilities_External.h"

// ---------------------------------
// polls all devices and elements for a change greater than kPercentMove.  Times out after given time
// returns 1 and pointer to device and element if found
// returns 0 and NULL for both parameters if not found

unsigned char HIDConfigureAction (pRecDevice * ppDevice, pRecElement * ppElement, float timeout)
{
    long numDevices, maxElements = 0;
    long * saveValueArray;
    pRecDevice pDevice = NULL;
    pRecElement pElement = NULL;
    short deviceNum = 0;
    unsigned char found = 0, done = 0;
	clock_t start = clock (), end;
    
     if (!HIDHaveDeviceList ())   // if we do not have a device list
		if (0 == HIDBuildDeviceList (kHIDPage_GenericDesktop, 0)) // if we could not build another list (use generic page)
			return 0; // return 0

    // build list of device and elements to save current values
    numDevices = HIDCountDevices ();
    pDevice = HIDGetFirstDevice ();
    while (pDevice)
    {
		long numElements = HIDCountDeviceElements (pDevice, kHIDElementTypeInput);
		if (numElements > maxElements)
			maxElements = numElements;
		pDevice = HIDGetNextDevice (pDevice);
	}
	saveValueArray = (long *) malloc (sizeof (long) * numDevices * maxElements); // 2D array to save values
	bzero(saveValueArray,sizeof (long) * numDevices * maxElements); // clear array

	// store current values
	deviceNum = 0;
	pDevice = HIDGetFirstDevice ();
	while (pDevice)
	{
		short elementNum = 0;
		pElement = HIDGetFirstDeviceElement (pDevice, kHIDElementTypeInput);
		while (pElement)
		{
			*(saveValueArray + (deviceNum * maxElements) + elementNum) = HIDGetElementValue (pDevice, pElement);
			pElement = HIDGetNextDeviceElement (pElement, kHIDElementTypeInput); 
			elementNum++;
		}
		pDevice = HIDGetNextDevice (pDevice);
		deviceNum++;
    }
    
    // poll all devices and elements, compare current value to save +/- kPercentMove
    while ((!found) && (!done))
    {
		double secs;
		// are we done?
		end = clock();
		secs = (double)(end - start) / CLOCKS_PER_SEC;
		if (secs > timeout)
			done = 1;
		deviceNum = 0;
		pDevice = HIDGetFirstDevice ();
		while (pDevice)
		{
			short elementNum = 0;
			pElement = HIDGetFirstDeviceElement (pDevice, kHIDElementTypeInput);
			while (pElement)
			{
				// ignore force feedback devices AND arrays
				if ((kHIDPage_PID != pElement->usagePage) && (-1 != pElement->usage))
				{
					long initialValue = *(saveValueArray + (deviceNum * maxElements) + elementNum);
					long value = HIDGetElementValue (pDevice, pElement);
					long delta = (float)(pElement->max - pElement->min) * kPercentMove * 0.01;

					if (((initialValue + delta) < value) || ((initialValue - delta) > value))
					{
						found = 1;
						break;
					}
				}
				pElement = HIDGetNextDeviceElement (pElement, kHIDElementTypeInput); 
				elementNum++;
			}
			if (found)
				break;
			pDevice = HIDGetNextDevice (pDevice);
			deviceNum++;
		}
    }
    
    // return device and element moved
    if (found)
    {
		*ppDevice = pDevice;
		*ppElement = pElement;
		return 1;
    }
	else
	{
		*ppDevice = NULL;
		*ppElement = NULL;
		return 0;
	}
}

// ---------------------------------
// takes input records, save required info
// assume file is open and at correct position.
// will always write to file (if file exists) size of recSaveHID, even if device and or element is bad

void HIDSaveElementConfig (FILE * fileRef, pRecDevice pDevice, pRecElement pElement, long actionCookie)
{
	recSaveHID saveRec;

	if (HIDIsValidElement(pDevice,pElement))
	{
		// clear rec
		bzero(&saveRec,sizeof(recSaveHID));

		saveRec.actionCookie = actionCookie;

		// must save
		// actionCookie
		// Device: serial,vendorID, productID, location, usagePage, usage
		// Element: cookie, usagePage, usage,
		// need to add serial number when I have a test case
		
		saveRec.vendorID = pDevice->vendorID;
		saveRec.productID = pDevice->productID;
		saveRec.locID = pDevice->locID;
		saveRec.usage = pDevice->usage;
		saveRec.usagePage = pDevice->usagePage;

		saveRec.usagePageE = pElement->usagePage;
		saveRec.usageE = pElement->usage;
		saveRec.cookie = pElement->cookie;

		// write to file
		if (fileRef)
			fwrite ((void *)&saveRec, sizeof (recSaveHID), 1, fileRef);
	}
}

// ---------------------------------
// take file, read one record (assume file position is correct and file is open)
// search for matching device
// return pDevice, pElement and cookie for action
 
long HIDRestoreElementConfig (FILE * fileRef, pRecDevice * ppDevice, pRecElement * ppElement)
{
    // Device: serial,vendorID, productID, location, usagePage, usage
    // Element: cookie, usagePage, usage,
    
    pRecDevice pDevice, pFoundDevice = NULL;
    pRecElement pElement, pFoundElement = NULL;
 
    recSaveHID restoreRec;
    
    fread ((void *) &restoreRec, 1, sizeof (recSaveHID), fileRef);

    // compare to current device list for matches
    // look for device
    if (restoreRec.locID && restoreRec.vendorID && restoreRec.productID)
	{ // look for specific device type plug in to same port
		pDevice = HIDGetFirstDevice ();
		while (pDevice)
		{
			if ((restoreRec.locID == pDevice->locID) &&
			(restoreRec.vendorID == pDevice->vendorID) &&
			(restoreRec.productID == pDevice->productID))
			pFoundDevice = pDevice;
			if (pFoundDevice)
				break;
			pDevice = HIDGetNextDevice (pDevice);
		}
		if (pFoundDevice)
		{
			pElement = HIDGetFirstDeviceElement (pFoundDevice, kHIDElementTypeIO);
			while (pElement)
			{
				if (restoreRec.cookie == pElement->cookie)
					pFoundElement = pElement;
				if (pFoundElement)
					break;
				pElement = HIDGetNextDeviceElement (pElement, kHIDElementTypeIO); 
			}
			// if no cookie match (should NOT occur) match on usage
			pElement = HIDGetFirstDeviceElement (pFoundDevice, kHIDElementTypeIO);
			while (pElement)
			{
				if ((restoreRec.usageE == pElement->usage) &&
					(restoreRec.usagePageE == pElement->usagePage))
					pFoundElement = pElement;
				if (pFoundElement)
					break;
				pElement = HIDGetNextDeviceElement (pElement, kHIDElementTypeIO); 
			}
		}
	}
    // if we have not found a match, look at just vendor and product
    if ((NULL == pFoundDevice) &&
	(restoreRec.vendorID && restoreRec.productID))
    {
		pDevice = HIDGetFirstDevice ();
		while (pDevice)
		{
			if ((restoreRec.vendorID == pDevice->vendorID) &&
			(restoreRec.productID == pDevice->productID))
			pFoundDevice = pDevice;
			if (pFoundDevice)
			break;
			pDevice = HIDGetNextDevice (pDevice);
		}
		// match elements by cookie since same device type
		if (pFoundDevice)
		{
			pElement = HIDGetFirstDeviceElement (pFoundDevice, kHIDElementTypeIO);
			while (pElement)
			{
				if (restoreRec.cookie == pElement->cookie)
					pFoundElement = pElement;
				if (pFoundElement)
					break;
				pElement = HIDGetNextDeviceElement (pElement, kHIDElementTypeIO); 
			}
			// if no cookie match (should NOT occur) match on usage
			pElement = HIDGetFirstDeviceElement (pFoundDevice, kHIDElementTypeIO);
			while (pElement)
			{
				if ((restoreRec.usageE == pElement->usage) &&
					(restoreRec.usagePageE == pElement->usagePage))
					pFoundElement = pElement;
				if (pFoundElement)
					break;
				pElement = HIDGetNextDeviceElement (pElement, kHIDElementTypeIO); 
			}
		}
    }
    // if we have not found a match look for just same type of device
    if ((NULL == pFoundDevice) && (restoreRec.usage && restoreRec.usagePage))
    {
		pDevice = HIDGetFirstDevice ();
		while (pDevice)
		{
			if ((restoreRec.usage == pDevice->usage) && (restoreRec.usagePage == pDevice->usagePage))
				pFoundDevice = pDevice;
			if (pFoundDevice)
				break;
			pDevice = HIDGetNextDevice (pDevice);
		}
		// match elements by type
		if (pFoundDevice)
		{
			pElement = HIDGetFirstDeviceElement (pFoundDevice, kHIDElementTypeIO);
			while (pElement)
			{
				if ((restoreRec.usageE == pElement->usage) &&
					(restoreRec.usagePageE == pElement->usagePage))
					pFoundElement = pElement;
				if (pFoundElement)
					break;
				pElement = HIDGetNextDeviceElement (pElement, kHIDElementTypeIO); 
			}
		}
    }
    // if still not found just get first device
    if (NULL == pFoundDevice)
    {
		pFoundDevice = HIDGetFirstDevice ();
		// match elements by type
		if (pFoundDevice)
		{
			pElement = HIDGetFirstDeviceElement (pFoundDevice, kHIDElementTypeIO);
			while (pElement)
			{
				if ((restoreRec.usageE == pElement->usage) &&
					(restoreRec.usagePageE == pElement->usagePage))
					pFoundElement = pElement;
				if (pFoundElement)
					break;
				pElement = HIDGetNextDeviceElement (pElement, kHIDElementTypeIO); 
			}
		}
    }
    if ((NULL == pFoundDevice) || (NULL == pFoundElement))
    {
		// no HID device
		*ppDevice = NULL;
		*ppElement = NULL;
		return restoreRec.actionCookie;
    }
    else
    {
		// no HID device
		*ppDevice = pFoundDevice;
		*ppElement = pFoundElement;
		return restoreRec.actionCookie;
    }
  
}

// ---------------------------------
// Find the specified preference in the specified application
// search for matching device and element
// return pDevice, pElement that matches

Boolean HIDRestoreElementPref (CFStringRef keyCFStringRef, CFStringRef appCFStringRef, pRecDevice * ppDevice, pRecElement * ppElement)
{
	Boolean found = false;

	if ((NULL != keyCFStringRef) && (NULL != appCFStringRef) && (NULL != ppDevice) && (NULL != ppElement))
	{
		CFPropertyListRef prefCFPropertyListRef = CFPreferencesCopyAppValue(keyCFStringRef, appCFStringRef);

		if (NULL != prefCFPropertyListRef)
		{
			if (CFStringGetTypeID() == CFGetTypeID(prefCFPropertyListRef))
			{
				char buffer[256];

				if (CFStringGetCString((CFStringRef) prefCFPropertyListRef, buffer, sizeof(buffer), kCFStringEncodingASCII))
				{
					recDevice	searchDevice;
					recElement	searchElement;
					int count = sscanf(buffer, "d:{v:%ld, p:%ld, l:%ld, p:%ld, u:%ld}, e:{t:%ld, p:%ld, u:%ld, c:%ld}", 
						&searchDevice.vendorID, &searchDevice.productID, &searchDevice.locID, &searchDevice.usagePage, &searchDevice.usage, 
						&searchElement.type, &searchElement.usagePage, &searchElement.usage, (long*) &searchElement.cookie);

					if (9 == count)	// if we found all nine parameters…
					{	// and can find a device & element that matches these…
						if (HIDFindActionDeviceAndElement(&searchDevice, &searchElement,ppDevice, ppElement))
						{
							found = true;
						}
					}
				}
			}
			else
			{
				// We found the entry with this key but it's the wrong type; delete it.
				CFPreferencesSetAppValue(keyCFStringRef, NULL, appCFStringRef);
				(void) CFPreferencesAppSynchronize(appCFStringRef);
			}
			CFRelease(prefCFPropertyListRef);
		}
	}
	return found;
}

// ---------------------------------
// Save the device & element values into the specified key in the specified applications preferences

Boolean HIDSaveElementPref (const CFStringRef keyCFStringRef, CFStringRef appCFStringRef, pRecDevice pDevice, pRecElement pElement)
{
	Boolean success = false;

	if ((NULL != keyCFStringRef) && (NULL != appCFStringRef) && HIDIsValidElement(pDevice,pElement))
	{
		CFStringRef prefCFStringRef =
			CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("d:{v:%ld, p:%ld, l:%ld, p:%ld, u:%ld}, e:{t:%ld, p:%ld, u:%ld, c:%ld}"), 
							pDevice->vendorID, pDevice->productID, pDevice->locID, pDevice->usagePage, pDevice->usage, 
							pElement->type, pElement->usagePage, pElement->usage, pElement->cookie);

		if (NULL != prefCFStringRef)
		{
			CFPreferencesSetAppValue(keyCFStringRef, prefCFStringRef, kCFPreferencesCurrentApplication);
			CFRelease(prefCFStringRef);
			success = true;
		}
	}
	return success;
}
