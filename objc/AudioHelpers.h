//
//  AudioHelpers.h
//  libpd
//
//  Created on 18/10/11.
//
//  For information on usage and redistribution, and for a DISCLAIMER OF ALL
//  WARRANTIES, see the file, "LICENSE.txt," in this distribution.
//
//  Updated 2018, 2020 Dan Wilcox <danomatika@gmail.com>
//

#import <Foundation/Foundation.h>

#pragma mark Audio Unit / Audio Session Debugging

/// uncomment this to log more information from the audio classes
/// or define it in "Other C Flags" build settings
//#define AU_DEBUG_VERBOSE

/// returns AVAudioSession OSStatus error code as a string
extern NSString *AVStatusCodeAsString(OSStatus status);

/// returns the AudioUnit OSStatus error code as a string
extern NSString *AUStatusCodeAsString(OSStatus status);

/// log debug info along with the class, function, and line number
#define AU_LOG(nslog_string, ...) do {\
NSLog((@"%s[%d] " nslog_string), __func__, __LINE__, ##__VA_ARGS__);\
} while (0)

/// same as AU_LOG, but only logs if AU_DEBUG_VERBOSE is defined
#if defined(AU_DEBUG_VERBOSE)
	#define AU_LOGV(nslog_string, ...) AU_LOG(nslog_string, ##__VA_ARGS__)
#else
	#define AU_LOGV(nslog_string, ...)
#endif

/// a debug check, which will only log if the value is non-zero
#define AU_LOG_IF_ERROR(value, nslog_string, ...) do {\
if (value) {\
NSLog((@"*** ERROR *** %s[%d] " nslog_string), __func__, __LINE__, ##__VA_ARGS__);\
}\
} while (0)

/// check if the audio unit had an error, and if so print it and return
#define AU_RETURN_IF_ERROR(status) do {\
if (status) {\
NSLog(@"*** ERROR *** %s[%d] status code =  %@", __func__, __LINE__, AUStatusCodeAsString(status));\
return;\
}\
} while (0)

/// check if the audio unit had an error, and if so print it and return false
#define AU_RETURN_FALSE_IF_ERROR(status) do {\
if (status) {\
NSLog(@"*** ERROR *** %s[%d] status code =  %@", __func__, __LINE__, AUStatusCodeAsString(status));\
return false;\
}\
} while (0)

/// check if the audio unit had an error, and if so print it, dispose the audio unit, and return
#define AU_DISPOSE_IF_ERROR(status, audioUnit) do {\
if (status) {\
AudioComponentInstanceDispose(audioUnit);\
NSLog(@"*** ERROR *** %s[%d] status code =  %@", __func__, __LINE__, AUStatusCodeAsString(status));\
return;\
}\
} while (0)

/// check if the audio unit had an error, and if so print it, dispose the audio unit, and return false
#define AU_DISPOSE_FALSE_IF_ERROR(status, audioUnit) do {\
if (status) {\
AudioComponentInstanceDispose(audioUnit);\
NSLog(@"*** ERROR *** %s[%d] status code =  %@", __func__, __LINE__, AUStatusCodeAsString(status));\
return false;\
}\
} while (0)

#pragma mark Math Helpers

/// returns YES if floats are equal within 0.0001
extern BOOL floatsAreEqual(Float64 f1, Float64 f2);

/// log shift calculation
extern int log2int(int x);
