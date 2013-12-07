/*
	File:		HID_Transaction_Utilities.h

	Contains:	Definitions of the HID queue functions for the HID utilites.
    
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

#include <CoreServices/CoreServices.h>

#include "HID_Utilities_Internal.h"
#include "HID_Utilities_External.h"

// ==================================
// private functions

// creates a transaction for a device, allocates and creates the transaction interface if required

static IOReturn hid_CreateTransaction(pRecDevice pDevice)
{
    IOReturn result = kIOReturnSuccess;

	if (HIDIsValidDevice(pDevice))
	{
		if (NULL == pDevice->transaction) // if we don't already have a transaction…
		{
			if (NULL != pDevice->interface) // and we do have an interface…
			{
				pDevice->transaction = (void *) (*(IOHIDDeviceInterface**) pDevice->interface)->allocOutputTransaction (pDevice->interface); // alloc Transaction
				if (pDevice->transaction)	// if it was successful…
				{
					result = (*(IOHIDOutputTransactionInterface**) pDevice->transaction)->create (pDevice->transaction); // create actual transaction
					if (kIOReturnSuccess != result)
						HIDReportErrorNum ("\nFailed to create transaction via create: error = %ld.", result);
				}
				else
				{
					HIDReportError ("\nhid_CreateTransaction failed to allocOutputTransaction");
					if (!result)
						result = kIOReturnError; // synthesis error
				}
			}
			else
				HIDReportErrorNum ("\nhid_CreateTransaction failed: Device inteface does not exist.", result);
		}
	}
    return result;
}

// ==================================
// public functions
// ---------------------------------
// add an element to a Transaction
unsigned long HIDTransactionAddElement(pRecDevice pDevice, pRecElement pElement)
{
    IOReturn result = hid_CreateTransaction(pDevice);

	if (HIDIsValidElement(pDevice,pElement))
	{
		if (pDevice->transaction)
		{
			result = (*(IOHIDOutputTransactionInterface**) pDevice->transaction)->addElement (pDevice->transaction,pElement->cookie); // add element
			if (kIOReturnSuccess != result)
				HIDReportErrorNum ("\nHIDTransactionAddElement failed to add Element: error = %ld.", result);
		}
		else
		{
			HIDReportError ("\nHIDTransactionAddElement failed: no transaction interface");
			if (!result)
				result = kIOReturnError; // synthesis error
		}
	}
	return result;
}

// removes an element from a Transaction
unsigned long HIDTransactionRemoveElement(pRecDevice pDevice, pRecElement pElement)
{
    IOReturn result = hid_CreateTransaction(pDevice);

	if (HIDIsValidElement(pDevice,pElement))
	{
		if (pDevice->transaction)
		{
			result = (*(IOHIDOutputTransactionInterface**) pDevice->transaction)->removeElement (pDevice->transaction,pElement->cookie); // remove element
			if (kIOReturnSuccess != result)
				HIDReportErrorNum ("\nHIDTransactionRemoveElement failed to remove Element: error = %ld.", result);
		}
		else
		{
			HIDReportError ("\nHIDTransactionRemoveElement failed: no transaction interface");
			if (!result)
				result = kIOReturnError; // synthesis error
		}
	}

	return result;
}

// return true if this transaction contains this element
Boolean HIDTransactionHasElement(pRecDevice pDevice, pRecElement pElement)
{
    Boolean result = false;

	if (HIDIsValidElement(pDevice,pElement))
	{
		(void) hid_CreateTransaction(pDevice);

		if (pDevice->transaction)
		{
			result = (*(IOHIDOutputTransactionInterface**) pDevice->transaction)->hasElement (pDevice->transaction,pElement->cookie); // remove element
		}
		else
		{
			HIDReportError ("\nHIDTransactionHasElement failed: no transaction interface");
		}
	}
	return result;
}

/* This changes the default value of an element, when the values of the */
/* elements are cleared, on clear or commit, they are reset to the */
/* default value */
/* This call can be made on elements that are not in the transaction, but */
/* has undefined behavior if made on elements not in the transaction */
/* which are later added to the transaction. */
/* In other words, an element should be added before its default is */
/* set, for well defined behavior. */
unsigned long HIDTransactionSetElementDefault(pRecDevice pDevice, pRecElement pElement,IOHIDEventStruct* pValueEvent)
{
    IOReturn result = hid_CreateTransaction(pDevice);

	if (HIDIsValidElement(pDevice,pElement))
	{
		if (pDevice->transaction)
		{
			result = (*(IOHIDOutputTransactionInterface**) pDevice->transaction)->setElementDefault (pDevice->transaction,pElement->cookie, pValueEvent);
			if (kIOReturnSuccess != result)
				HIDReportErrorNum ("\nHIDTransactionSetElementDefault failed to set Element Default: error = %ld.", result);
		}
		else
		{
			HIDReportError ("\nHIDTransactionSetElementDefault failed: no transaction interface");
			if (!result)
				result = kIOReturnError; // synthesis error
		}
	}

	return result;
}

/* Get the current setting of an element's default value */
unsigned long HIDTransactionGetElementDefault(pRecDevice pDevice, pRecElement pElement,IOHIDEventStruct* pValueEvent)
{
    IOReturn result = hid_CreateTransaction(pDevice);

	if (HIDIsValidElement(pDevice,pElement))
	{
		if (pDevice->transaction)
		{
			result = (*(IOHIDOutputTransactionInterface**) pDevice->transaction)->getElementDefault (pDevice->transaction,pElement->cookie, pValueEvent);
			if (kIOReturnSuccess != result)
				HIDReportErrorNum ("\nHIDTransactionGetElementDefault failed to get Element Default: error = %ld.", result);
		}
		else
		{
			HIDReportError ("\nHIDTransactionGetElementDefault failed: no transaction interface");
			if (!result)
				result = kIOReturnError; // synthesis error
		}
	}
	return result;
}

/* Add a change to the transaction, by setting an element value */
/* The change is not actually made until it is commited */
/* The element must be part of the transaction or this call will fail */
unsigned long HIDTransactionSetElementValue(pRecDevice pDevice, pRecElement pElement,IOHIDEventStruct* pValueEvent)
{
    IOReturn result = hid_CreateTransaction(pDevice);

	if (HIDIsValidElement(pDevice,pElement))
	{
		if (pDevice->transaction)
		{
			result = (*(IOHIDOutputTransactionInterface**) pDevice->transaction)->setElementValue (pDevice->transaction,pElement->cookie, pValueEvent);
			if (kIOReturnSuccess != result)
				HIDReportErrorNum ("\nHIDTransactionSetElementValue failed to set Element Default: error = %ld.", result);
		}
		else
		{
			HIDReportError ("\nHIDTransactionSetElementValue failed: no transaction interface");
			if (!result)
				result = kIOReturnError; // synthesis error
		}
	}
	return result;
}

/* Get the current setting of an element value */
unsigned long HIDTransactionGetElementValue(pRecDevice pDevice, pRecElement pElement,IOHIDEventStruct* pValueEvent)
{
    IOReturn result = hid_CreateTransaction(pDevice);

	if (HIDIsValidElement(pDevice,pElement))
	{
		if (pDevice->transaction)
		{
			result = (*(IOHIDOutputTransactionInterface**) pDevice->transaction)->getElementValue (pDevice->transaction,pElement->cookie, pValueEvent);
			if (kIOReturnSuccess != result)
				HIDReportErrorNum ("\nHIDTransactionGetElementValue failed to get Element Default: error = %ld.", result);
		}
		else
		{
			HIDReportError ("\nHIDTransactionGetElementValue failed: no transaction interface");
			if (!result)
				result = kIOReturnError; // synthesis error
		}
	}
	return result;
}

/* Commit the transaction, or clear all the changes and start over */
unsigned long HIDTransactionCommit(pRecDevice pDevice)
{
    IOReturn result = hid_CreateTransaction(pDevice);

	if (HIDIsValidDevice(pDevice))
	{
		if (pDevice->transaction)
		{
#if 0000
			// NOTE: this code is to workaround a bug where if you commit transactions
			// too fast then some of the reports get dropped.
			// (fixed in 10.2.1)
			static AbsoluteTime nextTime = {0,0};	// first time this should be no delay

			if (nextTime.hi || nextTime.lo)
				MPDelayUntil(&nextTime);
#endif
			result = (*(IOHIDOutputTransactionInterface**) pDevice->transaction)->commit (pDevice->transaction,-1,NULL,NULL,NULL);
			if (kIOReturnSuccess != result)
				HIDReportErrorNum ("\nHIDTransactionCommit failed to commit: error = %ld.", result);
#if 0000
			nextTime = AddDurationToAbsolute(20 * kDurationMillisecond,UpTime());
#endif
		}
		else
		{
			HIDReportError ("\nHIDTransactionCommit failed: no transaction interface");
			if (!result)
				result = kIOReturnError; // synthesis error
		}
	}
	return result;
}

/* Clear all the changes and start over */
unsigned long HIDTransactionClear(pRecDevice pDevice)
{
    IOReturn result = hid_CreateTransaction(pDevice);

	if (HIDIsValidDevice(pDevice))
	{
		if (pDevice->transaction)
		{
			result = (*(IOHIDOutputTransactionInterface**) pDevice->transaction)->clear (pDevice->transaction);
			if (kIOReturnSuccess != result)
				HIDReportErrorNum ("\nHIDTransactionClear failed to get Element Default: error = %ld.", result);
		}
		else
		{
			HIDReportError ("\nHIDTransactionClear failed: no transaction interface");
			if (!result)
				result = kIOReturnError; // synthesis error
		}
	}
	return result;
}
