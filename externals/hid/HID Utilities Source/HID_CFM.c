/*
 	File:		HID_CFM.c

	Contains:   Implementation of the CFM interfaces to the HID Utilities bundle
    
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
 *
 */

// Note: All the HID API's are defined externally in the file "HID_APIs.h".
// How they are used in this file is defined via one or two macros:
//
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
// ==================================
// includes
// ==================================
#include "HID_Utilities_CFM.h"

typedef OSStatus 		HRESULT;
typedef UInt32 			IOByteCount;
typedef unsigned int	io_service_t;
typedef unsigned int	io_object_t;
#define S_OK		    ((HRESULT)0x00000000L)

// ==================================
// define a ProcPtr type for each API
#define DO_API(r,n,p,a)	typedef r (*fp##n##Type)##p;
#include "HID_APIs.h"

// ==================================
// declare storage for each API's function pointers
#define DO_API(r,n,p,a)	static fp##n##Type fp##n = NULL;
#include "HID_APIs.h"

// ==================================
// globals
// ==================================
CFURLRef gBundleURL = NULL;
CFBundleRef gBundle = NULL;

// ==================================
// setup the CFM to MachO (HID) connection
// ==================================
OSStatus SetupHIDCFM (void)
{
	Boolean didLoad = false; //	Flag that indicates the status returned when attempting to load a bundle's executable code.
	CFBundleRef refMainBundle = NULL;
	CFURLRef refMainBundleURL = NULL, refPathBundleURL = NULL; 

	//	See the Core Foundation URL Services chapter for details.
	// get app bundle (even for a CFM app!)
	refMainBundle = CFBundleGetMainBundle(); 
	if (!refMainBundle)
	{
        DebugStr ("\pCould open main bundle");
		return paramErr;
	}

	// create a URL to the app bundle
	refMainBundleURL = CFBundleCopyBundleURL (refMainBundle); 
	if (!refMainBundleURL)
	{
        DebugStr ("\pCould not copy main bundle URL");
		return paramErr;
	}
#if 0	// This should only be true if the app is bundled
	// create a URL that points to the app's directory
	refPathBundleURL = CFURLCreateCopyDeletingLastPathComponent (kCFAllocatorDefault, refMainBundleURL); 
	if (!refPathBundleURL)
	{
        DebugStr ("\pCould not create new parent URL deleting last path component");
		if (refMainBundleURL != NULL) 
			CFRelease (refMainBundleURL);
		return paramErr;
	}
#else
	refPathBundleURL = refMainBundleURL;
	refMainBundleURL = NULL;
#endif

	// create a URL to the HID library bundle
	gBundleURL = CFURLCreateCopyAppendingPathComponent (kCFAllocatorDefault, refPathBundleURL, CFSTR("HID.bundle"), true); 
	// release created URLs
	if (refMainBundleURL != NULL) 
		CFRelease (refMainBundleURL);
	if (refPathBundleURL != NULL) 
		CFRelease (refPathBundleURL);
	// did we actaully get a bundle URL
	if (!gBundleURL)
	{
        DebugStr ("\pCould not create HID bundle URL");
		return paramErr;
    }
	// get the actual bundle for the HID library
	gBundle = CFBundleCreate (kCFAllocatorDefault, gBundleURL);
	if (!gBundle)
	{
        DebugStr ("\pCould not create HID MachO library bundle");
		CFShow(gBundleURL);
		return paramErr;
	}

    if (!CFBundleLoadExecutable (gBundle)) // If the code was successfully loaded, look for our function.
	{
    	DebugStr ("\pCould not load MachO executable");
    	return paramErr;
	}

	// Now that the code is loaded, search for the functions we want by name.
	// for each API, look up it's function pointer and store it the local ProcPtr.

#define DO_API(r,n,p,a)	fp##n = (fp##n##Type) CFBundleGetFunctionPointerForName (gBundle, CFSTR(#n));;
#include "HID_APIs.h"

	return noErr;
}
// ==================================
// tear down the CFM to MachO (HID) connection
// ==================================
void TearDownHIDCFM (void)
{
	// disassociate function pointers (assign null to each one)
#define DO_API(r,n,p,a)	fp##n = NULL;
#include "HID_APIs.h"

	if (gBundle != NULL)
	{
        CFBundleUnloadExecutable (gBundle);			//	Unload the bundle's executable code. 
        if (gBundleURL != NULL) 
            CFRelease (gBundleURL);
        CFRelease (gBundle);
	}
}

// Now for each API declare a (CFM) routine that calls thru the local ProcPtr to the MachO glue.
// Note: we use two different macros here: with & without a return value.

#define DO_API(r,n,p,a)		void n##p { (*fp##n)##a;}
#define DO_APIr(r,n,p,a)	r n##p { return (r) (*fp##n)##a;}
#include "HID_APIs.h"
// ==================================
