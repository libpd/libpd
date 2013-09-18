
/*
	File:		HID_APIs.h

	Contains:   Definition of the HID Utilities exported API's
    
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
// ----------------------------------------------------------------
// The DO_API micro takes four parameters:
//    #define DO_API(r,n,p,a)
//        r - what the API Returns
//        n - the Name of the API
//        p - the Parameters to the API
//        a - the Arguments passed by the API
//
//    DO_APIr is used when an API returns a value.
//    DO_API is used when an API doesn't return a value.
//
//  For example:
//
//  if you have a routine that doesn't return a value:
//
//  void MyFunction(const UInt32 pSelector, void* pPtr);
//
//  It's macro would look like this:
//
//  DO_API(
//      void,									// this is what the function returns
//      MyFunction,								// this is its Name
//      (const UInt32 pSelector, void* pPtr),	// these are its parameters
//      (pSelector, pPtr)) 						// and these are just the arguments (no type info)
//
//  If it returns a value like this
//
//  OSStatus MyFunction2(const UInt32 pSelector, void* pPtr);
//
//  It's macro would look like this:
//
//  DO_APIr(
//      OSStatus,								// this is what the function returns
//      MyFunction2,							// this is its Name
//      (const UInt32 pSelector, void* pPtr), 	// these are its parameters
//      (pSelector, pPtr))                    	// and these are just the arguments (no type info)
//

#ifndef DO_API		// if this isn't defined…
	#define DO_API	// … then don't do anything.
#endif

#ifndef DO_APIr				// if this isn't defined…
	#define DO_APIr	DO_API	// … then do the same as DO_API
#endif

DO_APIr(Boolean,			HIDBuildDeviceList,							(UInt32 usagePage, UInt32 usage),																							(usagePage, usage))
DO_APIr(Boolean,			HIDFindActionDeviceAndElement,				(const pRecDevice pSearchDevice, const pRecElement pSearchElement,pRecDevice *ppFoundDevice, pRecElement *ppFoundElement),	(pSearchDevice, pSearchElement,ppFoundDevice, ppFoundElement))
DO_APIr(Boolean,			HIDFindDevice,								(const pRecDevice pSearchDevice, pRecDevice *ppFoundDevice),																(pSearchDevice, ppFoundDevice))
DO_APIr(Boolean,			HIDFindSubElement,							(const pRecElement pStartElement, const pRecElement pSearchElement, pRecElement *ppFoundElement),							(pStartElement, pSearchElement, ppFoundElement))
DO_APIr(Boolean,			HIDGetElementNameFromVendorProductCookie,	(const long vendorID, const long productID, const long cookie, char * pName),												(vendorID, productID, cookie, pName))
DO_APIr(Boolean,			HIDGetElementNameFromVendorProductUsage,	(const long vendorID, const long productID, const long pUsagePage, const long pUsage, char * pName),						(vendorID, productID, pUsagePage, pUsage, pName))
DO_APIr(Boolean,			HIDHaveDeviceList,							(void),																														())
DO_APIr(Boolean,			HIDIsValidDevice,							(const pRecDevice pSearchDevice),																							(pSearchDevice))
DO_APIr(Boolean,			HIDIsValidElement,							(const pRecDevice pSearchDevice, const pRecElement pSearchElement),															(pSearchDevice, pSearchElement))
DO_APIr(Boolean,			HIDRestoreElementPref,						(CFStringRef keyCFStringRef, CFStringRef appCFStringRef, pRecDevice * ppDevice, pRecElement * ppElement),					(keyCFStringRef, appCFStringRef, ppDevice, ppElement))
DO_APIr(Boolean,			HIDSaveElementPref,							(CFStringRef keyCFStringRef, CFStringRef appCFStringRef, pRecDevice pDevice, pRecElement pElement),							(keyCFStringRef, appCFStringRef, pDevice, pElement))
DO_APIr(Boolean,			HIDTransactionHasElement,					(pRecDevice pDevice, pRecElement pElement),																					(pDevice, pElement))
DO_APIr(HIDElementTypeMask,	HIDConvertElementTypeToMask,				(const long type),											(type))
DO_APIr(int,				HIDPrintElement,							(const pRecElement pElement),								(pElement))
DO_APIr(long,				HIDGetElementValue,							(pRecDevice pDevice, pRecElement pElement),					(pDevice, pElement))
//DO_APIr(long,				HIDGetReport,								(pRecDevice pDevice,const IOHIDReportType reportType, const unsigned long reportID, void* reportBuffer, unsigned long* reportBufferSize),			(pDevice,reportType, reportID, reportBuffer, reportBufferSize))
DO_APIr(long,				HIDRestoreElementConfig,					(FILE * fileRef, pRecDevice * ppDevice, pRecElement * ppElement),			(fileRef, ppDevice, ppElement))
DO_APIr(long,				HIDSetElementValue,							(pRecDevice pDevice, pRecElement pElement,void* pIOHIDEvent),			(pDevice, pElement,pIOHIDEvent))
DO_APIr(long,				HIDSetQueueCallback,						(pRecDevice pDevice, IOHIDCallbackFunction callback,void* callbackTarget, void* callbackRefcon),			(pDevice, callback,callbackTarget, callbackRefcon))
//DO_APIr(long,				HIDSetReport,								(pRecDevice pDevice,const IOHIDReportType reportType, const unsigned long reportID, void* reportBuffer, const unsigned long reportBufferSize),(pDevice,reportType, reportID, reportBuffer, reportBufferSize))
DO_APIr(pRecDevice,			HIDGetFirstDevice,							(void),													())
DO_APIr(pRecDevice,			HIDGetNextDevice,							(pRecDevice pDevice),									(pDevice))
DO_APIr(pRecElement,		HIDGetFirstDeviceElement,					(pRecDevice pDevice, HIDElementTypeMask typeMask),		(pDevice, typeMask))
DO_APIr(pRecElement,		HIDGetNextDeviceElement,					(pRecElement pElement, HIDElementTypeMask typeMask),	(pElement, typeMask))
DO_APIr(pRecElement,		HIDGetPreviousDeviceElement,				(pRecElement pElement, HIDElementTypeMask typeMask),	(pElement, typeMask))
DO_APIr(SInt32,				HIDCalibrateValue,							(SInt32 value, pRecElement pElement),					(value, pElement))
DO_APIr(SInt32,				HIDScaleValue,								(SInt32 value, pRecElement pElement),					(value, pElement))
DO_APIr(UInt32,				HIDCountDeviceElements,						(pRecDevice pDevice, HIDElementTypeMask typeMask),		(pDevice, typeMask))
DO_APIr(UInt32,				HIDCountDevices,							(void),													())
DO_APIr(unsigned char,		HIDConfigureAction,							(pRecDevice * ppDevice, pRecElement * ppElement, float timeout),	(ppDevice, ppElement, timeout))
DO_APIr(unsigned char,		HIDGetEvent,								(pRecDevice pDevice, void * pHIDEvent),					(pDevice, pHIDEvent))
DO_APIr(unsigned long,		HIDQueueElement,							(pRecDevice pDevice, pRecElement pElement),				(pDevice, pElement))
DO_APIr(unsigned long,		HIDCloseReleaseInterface,					(pRecDevice pDevice),									(pDevice))
DO_APIr(unsigned long,		HIDCreateOpenDeviceInterface,				(UInt32 hidDevice, pRecDevice pDevice),					(hidDevice, pDevice))
DO_APIr(unsigned long,		HIDDequeueDevice,							(pRecDevice pDevice),									(pDevice))
DO_APIr(unsigned long,		HIDDequeueElement,							(pRecDevice pDevice, pRecElement pElement),				(pDevice, pElement))
DO_APIr(unsigned long,		HIDQueueDevice,								(pRecDevice pDevice),									(pDevice))
DO_APIr(unsigned long,		HIDReleaseAllDeviceQueues,					(void),													())

DO_APIr(unsigned long,		HIDTransactionAddElement,					(pRecDevice pDevice, pRecElement pElement),				(pDevice, pElement))
DO_APIr(unsigned long,		HIDTransactionClear,						(pRecDevice pDevice),									(pDevice))
DO_APIr(unsigned long,		HIDTransactionCommit,						(pRecDevice pDevice),									(pDevice))
DO_APIr(unsigned long,		HIDTransactionGetElementDefault,			(pRecDevice pDevice, pRecElement pElement,IOHIDEventStruct* pValueEvent),	(pDevice, pElement,pValueEvent))
DO_APIr(unsigned long,		HIDTransactionGetElementValue,				(pRecDevice pDevice, pRecElement pElement,IOHIDEventStruct* pValueEvent),	(pDevice, pElement,pValueEvent))
DO_APIr(unsigned long,		HIDTransactionRemoveElement,				(pRecDevice pDevice, pRecElement pElement),									(pDevice, pElement))
DO_APIr(unsigned long,		HIDTransactionSetElementDefault,			(pRecDevice pDevice, pRecElement pElement,IOHIDEventStruct* pValueEvent),	(pDevice, pElement,pValueEvent))
DO_APIr(unsigned long,		HIDTransactionSetElementValue,				(pRecDevice pDevice, pRecElement pElement,IOHIDEventStruct* pValueEvent),	(pDevice, pElement,pValueEvent))

DO_API(void,				HIDGetTypeName,								(IOHIDElementType theType, char * cstrName),									(theType, cstrName))
DO_API(void,				HIDGetUsageName,							(long valueUsagePage, long valueUsage, char * cstrName),						(valueUsagePage, valueUsage, cstrName))
DO_API(void,				HIDReleaseDeviceList,						(void),																			())
DO_API(void,				HIDSaveElementConfig,						(FILE * fileRef, pRecDevice pDevice, pRecElement pElement, long actionCookie),	(fileRef, pDevice, pElement, actionCookie))

DO_APIr(io_object_t, AllocateHIDObjectFromRecDevice,(pRecDevice pDevice),(pDevice))
DO_APIr(Boolean, FreeHIDObject,(io_object_t hidDevice), (hidDevice))

#undef DO_API
#undef DO_APIr

