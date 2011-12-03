//
//  AudioDebug.h
//  libpd
//
//  Created on 18/10/11.
//
//  For information on usage and redistribution, and for a DISCLAIMER OF ALL
//  WARRANTIES, see the file, "LICENSE.txt," in this distribution.
//

#import <Foundation/Foundation.h>

#pragma mark - Audio Unit / Audio Session Debugging

// uncomment this to log more information from the audio classes, or define it in "Other C Flags" build settings
//#define AU_DEBUG_VERBOSE

// returns the name of the const value assosated with the OSStatus as a string
extern NSString *AVStatusCodeAsString(OSStatus status);
extern NSString *AUStatusCodeAsString(OSStatus status);

// log debug info along with the class, function and line number
#define AU_LOG(nslog_string, ...) do {\
NSLog((@"%s[%d] " nslog_string), __func__, __LINE__, ##__VA_ARGS__);\
} while (0)

// same as AU_Log, but only logs if AU_DEBUG_VERBOSE is defined
#if defined(AU_DEBUG_VERBOSE)
#define AU_LOGV(nslog_string, ...) AU_LOG(nslog_string, ##__VA_ARGS__)
#else
#define AU_LOGV(nslog_string, ...)
#endif

// check if the audio unit had an error, and if so print it and return
#define AU_CHECK_STATUS(status) do {\
if(status) {\
NSLog(@"*** ERROR *** %s[%d] status code =  %@", __func__, __LINE__, AUStatusCodeAsString(status));\
return;\
}\
} while (0)

// check if the audio unit had an error, and if so print it and return false
#define AU_CHECK_STATUS_FALSE(status) do {\
if(status) {\
NSLog(@"*** ERROR *** %s[%d] status code =  %@", __func__, __LINE__, AUStatusCodeAsString(status));\
return false;\
}\
} while (0)

// check if the AVAudioSession had an error, and if so print it and return
#define AV_CHECK_ERROR(error) do {\
if(error) {\
NSLog(@"*** ERROR *** %s[%d] %@, status code = %@ ", __func__, __LINE__, [error localizedDescription], AVStatusCodeAsString([error code]));\
error = nil;\
return;\
}\
} while (0)

// a debug check, which will only log if the value is non-zero
#define AU_CHECK(value, nslog_string, ...) do {\
if(!value) {\
NSLog((@"*** ERROR *** %s[%d] " nslog_string), __func__, __LINE__, ##__VA_ARGS__);\
}\
} while (0)

// same as AU_CHECK, but returns if the value is non-zero
#define AU_CHECK_RETURN(value, nslog_string, ...) do {\
AU_CHECK(value, nslog_string, ##__VA_ARGS__);\
if(!value) return;\
} while (0)

// same as AU_CHECK, but returns false if the value is non-zero
#define AU_CHECK_RETURN_FALSE(value, nslog_string, ...) do {\
AU_CHECK(value, nslog_string, ##__VA_ARGS__);\
if(!value) return false;\
} while (0)

#pragma mark - Math Helpers

extern BOOL floatsAreEqual(Float64 f1, Float64 f2);
extern int log2int(int x);
