//
//  AudioDebug.m
//  libpd
//
//  Created on 18/10/11.
//
//  For information on usage and redistribution, and for a DISCLAIMER OF ALL
//  WARRANTIES, see the file, "LICENSE.txt," in this distribution.
//

#import "AudioHelpers.h"
#import <AudioToolbox/AudioToolbox.h>

#pragma mark - Audio Unit / Audio Session Debugging

#define UNDEFINED_BAD_CATEGORY_ERROR -12986

NSString *AVStatusCodeAsString(OSStatus status) {
	switch (status) {
		case kAudioSessionNoError:
			return @"kAudioSessionNoError";
		case kAudioSessionNotInitialized:
			return @"kAudioSessionNotInitialized";
		case kAudioSessionAlreadyInitialized:
			return @"kAudioSessionAlreadyInitialized";
		case kAudioSessionInitializationError:
			return @"kAudioSessionInitializationError";
		case kAudioSessionBadPropertySizeError:
			return @"kAudioSessionBadPropertySizeError";
		case kAudioSessionNotActiveError:
			return @"kAudioSessionNotActiveError";
		case kAudioServicesNoHardwareError:
			return @"kAudioServicesNoHardwareError";
		case kAudioSessionNoCategorySet:
			return @"kAudioSessionNoCategorySet";
		case kAudioSessionIncompatibleCategory:
			return @"kAudioSessionIncompatibleCategory";
		case kAudioSessionUnspecifiedError:
			return @"kAudioSessionUnspecifiedError";
		case UNDEFINED_BAD_CATEGORY_ERROR:
			return [NSString stringWithFormat:@"unknown error code %ld, but known to be related to a bad audio session category setting", status];
		default:
			return [NSString stringWithFormat:@"unknown error code %ld", status];
	}
}

NSString *AUStatusCodeAsString(OSStatus status) {
	switch (status) {
		case kAudioUnitErr_InvalidProperty:
			return @"kAudioUnitErr_InvalidProperty";
		case kAudioUnitErr_InvalidParameter:
			return @"kAudioUnitErr_InvalidParameter";
		case kAudioUnitErr_InvalidElement:
			return @"kAudioUnitErr_InvalidElement";
		case kAudioUnitErr_NoConnection:
			return @"kAudioUnitErr_NoConnection";
		case kAudioUnitErr_FailedInitialization:
			return @"kAudioUnitErr_FailedInitialization";
		case kAudioUnitErr_TooManyFramesToProcess:
			return @"kAudioUnitErr_TooManyFramesToProcess";
		case kAudioUnitErr_InvalidFile:
			return @"kAudioUnitErr_InvalidFile";
		case kAudioUnitErr_FormatNotSupported:
			return @"kAudioUnitErr_FormatNotSupported";
		case kAudioUnitErr_Uninitialized:
			return @"kAudioUnitErr_Uninitialized";
		case kAudioUnitErr_InvalidScope:
			return @"kAudioUnitErr_InvalidScope";
		case kAudioUnitErr_PropertyNotWritable:
			return @"kAudioUnitErr_PropertyNotWritable";
		case kAudioUnitErr_CannotDoInCurrentContext:
			return @"kAudioUnitErr_CannotDoInCurrentContext";
		case kAudioUnitErr_InvalidPropertyValue:
			return @"kAudioUnitErr_InvalidPropertyValue";
		case kAudioUnitErr_PropertyNotInUse:
			return @"kAudioUnitErr_PropertyNotInUse";
		case kAudioUnitErr_Initialized:
			return @"kAudioUnitErr_Initialized";
		case kAudioUnitErr_InvalidOfflineRender:
			return @"kAudioUnitErr_InvalidOfflineRender";
		case kAudioUnitErr_Unauthorized:
			return @"kAudioUnitErr_Unauthorized";
		default:
			return [NSString stringWithFormat:@"unknown error code %ld", status];
	}
}

#pragma mark - Math Helpers

BOOL floatsAreEqual(Float64 f1, Float64 f2) {
	return ((fabs(f1 - f2) < 0.0001) ? YES : NO);
}

//http://graphics.stanford.edu/~seander/bithacks.html#IntegerLogObvious
int log2int(int x) {
	int y = 0;
	while (x >>= 1) {
		++y;
	}
	return y;
}

