/*
	File:		HID_Transaction_Utilities.h

	Contains:	Implementation of the HID queue functions for the HID utilites.
    
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

#ifndef _HID_Transaction_Utilities_h_
#define _HID_Transaction_Utilities_h_

#include "HID_Utilities.h"

// ==================================

#ifdef __cplusplus
extern "C" {
#endif

// ==================================
// Create and open an transaction interface to device, required prior to extracting values or building Transactions
extern unsigned long HIDTransactionAddElement(pRecDevice pDevice, pRecElement pElement);

// removes an element from a Transaction
extern unsigned long  HIDTransactionRemoveElement(pRecDevice pDevice, pRecElement pElement);

// return true if this transaction contains this element
extern Boolean HIDTransactionHasElement(pRecDevice pDevice, pRecElement pElement);

/* This changes the default value of an element, when the values of the */
/* elements are cleared, on clear or commit, they are reset to the */
/* default value */
/* This call can be made on elements that are not in the transaction, but */
/* has undefined behavior if made on elements not in the transaction */
/* which are later added to the transaction. */
/* In other words, an element should be added before its default is */
/* set, for well defined behavior. */
extern unsigned long  HIDTransactionSetElementDefault(pRecDevice pDevice, pRecElement pElement,IOHIDEventStruct* pValueEvent);

/* Get the current setting of an element's default value */
extern unsigned long  HIDTransactionGetElementDefault(pRecDevice pDevice, pRecElement pElement,IOHIDEventStruct* pValueEvent);

/* Add a change to the transaction, by setting an element value */
/* The change is not actually made until it is commited */
/* The element must be part of the transaction or this call will fail */
extern unsigned long  HIDTransactionSetElementValue(pRecDevice pDevice, pRecElement pElement,IOHIDEventStruct* pValueEvent);

/* Get the current setting of an element value */
extern unsigned long  HIDTransactionGetElementValue(pRecDevice pDevice, pRecElement pElement,IOHIDEventStruct* pValueEvent);

/* Commit the transaction, or clear all the changes and start over */
/* timoutMS is the timeout in milliseconds, a zero timeout will cause */
/*	this call to be non-blocking (returning queue empty) if there */
/*	is a NULL callback, and blocking forever until the queue is */
/*	non-empty if their is a valid callback */
/* callback, if non-NULL is a callback to be called when data is */
/*  inserted to the queue  */
/* callbackTarget and callbackRefcon are passed to the callback */
extern unsigned long  HIDTransactionCommit(pRecDevice pDevice);

/* Clear all the changes and start over */
extern unsigned long  HIDTransactionClear(pRecDevice pDevice);

#ifdef __cplusplus
}
#endif

#endif // _HID_Transaction_Utilities_h_

