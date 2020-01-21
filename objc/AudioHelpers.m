//
//  AudioHelpers.m
//  libpd
//
//  Created on 18/10/11.
//
//  For information on usage and redistribution, and for a DISCLAIMER OF ALL
//  WARRANTIES, see the file, "LICENSE.txt," in this distribution.
//
//  Updated 2018, 2020 Dan Wilcox <danomatika@gmail.com>
//

#import "AudioHelpers.h"
#import <AudioToolbox/AudioToolbox.h>

#pragma mark Audio Unit / Audio Session Debugging

NSString *AVStatusCodeAsString(OSStatus status) {
	switch (status) {
		case AVAudioSessionErrorCodeNone:
			return @"AVAudioSessionErrorCodeNone";
		case AVAudioSessionErrorCodeMediaServicesFailed:
			return @"AVAudioSessionErrorCodeMediaServicesFailed";
		case AVAudioSessionErrorCodeIsBusy:
			return @"AVAudioSessionErrorCodeIsBusy";
		case AVAudioSessionErrorCodeIncompatibleCategory:
			return @"AVAudioSessionErrorCodeIncompatibleCategory";
		case AVAudioSessionErrorCodeCannotInterruptOthers:
			return @"AVAudioSessionErrorCodeCannotInterruptOthers";
		case AVAudioSessionErrorCodeMissingEntitlement:
			return @"AVAudioSessionErrorCodeMissingEntitlement";
		case AVAudioSessionErrorCodeSiriIsRecording:
			return @"AVAudioSessionErrorCodeSiriIsRecording";
		case AVAudioSessionErrorCodeCannotStartPlaying:
			return @"AVAudioSessionErrorCodeCannotStartPlaying";
		case AVAudioSessionErrorCodeCannotStartRecording:
			return @"AVAudioSessionErrorCodeCannotStartRecording";
		case AVAudioSessionErrorCodeBadParam:
			return @"AVAudioSessionErrorCodeBadParam";
		case AVAudioSessionErrorCodeInsufficientPriority:
			return @"AVAudioSessionErrorCodeInsufficientPriority";
		case AVAudioSessionErrorCodeResourceNotAvailable:
			return @"AVAudioSessionErrorCodeResourceNotAvailable";
		case AVAudioSessionErrorCodeUnspecified:
			return @"AVAudioSessionErrorCodeUnspecified";
		case AVAudioSessionErrorCodeExpiredSession:
			return @"AVAudioSessionErrorCodeExpiredSession";
		case AVAudioSessionErrorCodeSessionNotActive:
			return @"AVAudioSessionErrorCodeSessionNotActive";
		default:
			return [NSString stringWithFormat:@"unknown error code %d", (int)status];
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
			return [NSString stringWithFormat:@"unknown error code %d", (int)status];
	}
}

#pragma mark Math Helpers

BOOL floatsAreEqual(Float64 f1, Float64 f2) {
	return ((fabs(f1 - f2) < 0.0001) ? YES : NO);
}

// http://graphics.stanford.edu/~seander/bithacks.html#IntegerLogObvious
int log2int(int x) {
	int y = 0;
	while (x >>= 1) {
		++y;
	}
	return y;
}
