//
//  PdAudioUnit.m
//  libpd
//
//  Created on 29/09/11.
//
//  For information on usage and redistribution, and for a DISCLAIMER OF ALL
//  WARRANTIES, see the file, "LICENSE.txt," in this distribution.
//

#import "PdAudioUnit.h"
#import "PdBase.h"
#import "AudioHelpers.h"
#import <AudioToolbox/AudioToolbox.h>

static const AudioUnitElement kInputElement = 1;
static const AudioUnitElement kOutputElement = 0;

OSStatus createAggregateDevice(AudioDeviceID inputDevice, AudioDeviceID outputDevice, AudioDeviceID *aggregateDevice);
OSStatus destroyAggregateDevice(AudioDeviceID inDeviceToDestroy);

@interface PdAudioUnit () {
@private
	BOOL inputEnabled_;
	BOOL initialized_;
	int blockSizeAsLog_;
	AudioDeviceID aggregateDevice; /// needed on OSX for separate input & output devices
}
@property (nonatomic) AUGraph auGraph;
//@property (nonatomic) AUdioDeviceID auGraph;
- (BOOL)initAudioUnitWithSampleRate:(Float64)sampleRate numberChannels:(int)numChannels inputEnabled:(BOOL)inputEnabled;
- (void)destroyAudioUnits;
@end

@implementation PdAudioUnit

@synthesize audioUnit = audioUnit_;
//@synthesize inputAudioUnit = inputAudioUnit_;
@synthesize active = active_;
@synthesize auGraph = auGraph_;

#pragma mark - Init / Dealloc

- (id)init {
	self = [super init];
	if (self) {
		initialized_ = NO;
		active_ = NO;
		blockSizeAsLog_ = log2int([PdBase getBlockSize]);
	}
	return self;
}

- (void)dealloc {
	[self destroyAudioUnits];
	[super dealloc];
}

#pragma mark - Public Methods

- (void)setActive:(BOOL)active {
	if (!initialized_) {
		return;
	}
	if (active == active_) {
		return;
	}
	if (active) {
		AU_RETURN_IF_ERROR(AudioOutputUnitStart(audioUnit_));
		//AU_RETURN_IF_ERROR(AudioOutputUnitStart(inputAudioUnit_));
	} else {
		AU_RETURN_IF_ERROR(AudioOutputUnitStop(audioUnit_));
		//AU_RETURN_IF_ERROR(AudioOutputUnitStop(inputAudioUnit_));
	}
	active_ = active;
}

- (int)configureWithSampleRate:(Float64)sampleRate numberChannels:(int)numChannels inputEnabled:(BOOL)inputEnabled {
	Boolean wasActive = self.isActive;
	inputEnabled_ = inputEnabled;
	if (![self initAudioUnitWithSampleRate:sampleRate numberChannels:numChannels inputEnabled:inputEnabled_]) {
		return -1;
	}
	[PdBase openAudioWithSampleRate:sampleRate inputChannels:(inputEnabled_ ? numChannels : 0) outputChannels:numChannels];
	[PdBase computeAudio:YES];
	self.active = wasActive;
	return 0;
}

- (void)print {
	if (!initialized_) {
		AU_LOG(@"Audio Unit not initialized");
		return;
	}
	
	AudioDeviceID device;
	CFStringRef deviceName;
	UInt32 size = 0;
	AudioObjectPropertyAddress property;
	property.mScope = kAudioObjectPropertyScopeGlobal;
	property.mElement = kAudioObjectPropertyElementMaster;
	
	if (inputEnabled_) {

		#if TARGET_OS_MAC
			// try to print the device name
			size = sizeof(AudioDeviceID);
			AU_RETURN_IF_ERROR(AudioUnitGetProperty(audioUnit_,
													kAudioOutputUnitProperty_CurrentDevice,
													kAudioUnitScope_Global,
													0,
													&device,
													&size));
			property.mSelector = kAudioObjectPropertyName;
			if (AudioObjectHasProperty(device, &property)) {
				size = sizeof(CFStringRef);
				OSStatus status = AudioObjectGetPropertyData(device, &property, 0, NULL, &size, &deviceName);
				if(status == kAudioHardwareNoError) {
					AU_LOG(@"input device: %@", deviceName);
					CFRelease(deviceName);
				}
			}
		#endif
		
		// description
		AudioStreamBasicDescription inputStreamDescription;
		size = sizeof(inputStreamDescription);
		memset (&inputStreamDescription, 0, size);
		AU_RETURN_IF_ERROR(AudioUnitGetProperty(audioUnit_,
                           kAudioUnitProperty_StreamFormat,
                           kAudioUnitScope_Input,
                           kInputElement,
                           &inputStreamDescription,
                           &size));
		AU_LOG(@"input ASBD:");
		AU_LOG(@"  mSampleRate: %.0fHz", inputStreamDescription.mSampleRate);
		AU_LOG(@"  mChannelsPerFrame: %lu", inputStreamDescription.mChannelsPerFrame);
		AU_LOGV(@"  mFormatID: %lu", inputStreamDescription.mFormatID);
		AU_LOGV(@"  mFormatFlags: %lu", inputStreamDescription.mFormatFlags);
		AU_LOGV(@"  mBytesPerPacket: %lu", inputStreamDescription.mBytesPerPacket);
		AU_LOGV(@"  mFramesPerPacket: %lu", inputStreamDescription.mFramesPerPacket);
		AU_LOGV(@"  mBytesPerFrame: %lu", inputStreamDescription.mBytesPerFrame);
		AU_LOGV(@"  mBitsPerChannel: %lu", inputStreamDescription.mBitsPerChannel);
	} else {
		AU_LOG(@"input not enabled");
	}
	
	#if TARGET_OS_MAC
		// try to print the device name
		size = sizeof(AudioDeviceID);
		AU_RETURN_IF_ERROR(AudioUnitGetProperty(audioUnit_,
												kAudioOutputUnitProperty_CurrentDevice,
												kAudioUnitScope_Global,
												0,
												&device,
												&size));
		property.mSelector = kAudioObjectPropertyName;
		if (AudioObjectHasProperty(device, &property)) {
			size = sizeof(CFStringRef);
			OSStatus status = AudioObjectGetPropertyData(device, &property, 0, NULL, &size, &deviceName);
			if(status == kAudioHardwareNoError) {
				AU_LOG(@"output device: %@", deviceName);
				CFRelease(deviceName);
			}
		}
	#endif
	
	// description
	AudioStreamBasicDescription outputStreamDescription;
	size = sizeof(outputStreamDescription);
	memset(&outputStreamDescription, 0, size);
	AU_RETURN_IF_ERROR(AudioUnitGetProperty(audioUnit_,
                       kAudioUnitProperty_StreamFormat,
                       kAudioUnitScope_Output,
                       kOutputElement,
                       &outputStreamDescription,
                       &size));
	AU_LOG(@"output ASBD:");
	AU_LOG(@"  mSampleRate: %.0fHz", outputStreamDescription.mSampleRate);
	AU_LOG(@"  mChannelsPerFrame: %lu", outputStreamDescription.mChannelsPerFrame);
	AU_LOGV(@"  mFormatID: %lu", outputStreamDescription.mFormatID);
	AU_LOGV(@"  mFormatFlags: %lu", outputStreamDescription.mFormatFlags);
	AU_LOGV(@"  mBytesPerPacket: %lu", outputStreamDescription.mBytesPerPacket);
	AU_LOGV(@"  mFramesPerPacket: %lu", outputStreamDescription.mFramesPerPacket);
	AU_LOGV(@"  mBytesPerFrame: %lu", outputStreamDescription.mBytesPerFrame);
	AU_LOGV(@"  mBitsPerChannel: %lu", outputStreamDescription.mBitsPerChannel);
}

// sets the format to 32 bit, floating point, linear PCM, interleaved
- (AudioStreamBasicDescription)ASBDForSampleRate:(Float64)sampleRate numberChannels:(UInt32)numberChannels {
	const int kFloatSize = 4;
	const int kBitSize = 8;
	
	AudioStreamBasicDescription description;
	memset(&description, 0, sizeof(description));
	
	description.mSampleRate = sampleRate;
	description.mFormatID = kAudioFormatLinearPCM;
	description.mFormatFlags = kAudioFormatFlagsNativeFloatPacked;
	description.mBytesPerPacket = kFloatSize * numberChannels;
	description.mFramesPerPacket = 1;
	description.mBytesPerFrame = kFloatSize * numberChannels;
	description.mChannelsPerFrame = numberChannels;
	description.mBitsPerChannel = kFloatSize * kBitSize;
	
	return description;
}

#pragma mark - AURenderCallback

static OSStatus AudioRenderCallback(void *inRefCon,
                                    AudioUnitRenderActionFlags *ioActionFlags,
                                    const AudioTimeStamp *inTimeStamp,
                                    UInt32 inBusNumber,
                                    UInt32 inNumberFrames,
                                    AudioBufferList *ioData) {
	
	PdAudioUnit *pdAudioUnit = (PdAudioUnit *)inRefCon;
	Float32 *auBuffer = (Float32 *)ioData->mBuffers[0].mData;
	
	if (pdAudioUnit->inputEnabled_) {
		AudioUnitRender(pdAudioUnit->audioUnit_, ioActionFlags, inTimeStamp, kInputElement, inNumberFrames, ioData);
	}
	
	int ticks = inNumberFrames >> pdAudioUnit->blockSizeAsLog_; // this is a faster way of computing (inNumberFrames / blockSize)
	[PdBase processFloatWithInputBuffer:auBuffer outputBuffer:auBuffer ticks:ticks];
	return noErr;
}


- (AURenderCallback)renderCallback {
	return AudioRenderCallback;
}

- (AudioDeviceID)aggregateDevice {
	return aggregateDevice;
}

#pragma mark - Private

- (void)destroyAudioUnits {
	if (!initialized_) {
		return;
	}
	self.active = NO;
	initialized_ = NO;
	AU_RETURN_IF_ERROR(AudioUnitUninitialize(audioUnit_));
	AU_RETURN_IF_ERROR(AudioComponentInstanceDispose(audioUnit_));
	if(aggregateDevice != kAudioDeviceUnknown) { // OSX
//		AU_RETURN_IF_ERROR(AUGraphStop(auGraph_));
//		AUGraphClearConnections(auGraph_);
//		DisposeAUGraph(auGraph_);
		destroyAggregateDevice(aggregateDevice);
//		AU_RETURN_IF_ERROR(AudioUnitUninitialize(inputAudioUnit_));
//		AU_RETURN_IF_ERROR(AudioComponentInstanceDispose(inputAudioUnit_));
//		AUGraphClose(auGraph_);
//		AU_LOGV(@"destroyed audio units");
	}
	else {
		AU_LOGV(@"destroyed audio unit");
	}
}

#if TARGET_OS_IPHONE

- (BOOL)initAudioUnitWithSampleRate:(Float64)sampleRate numberChannels:(int)numChannels inputEnabled:(BOOL)inputEnabled {
	
	[self destroyAudioUnits];
	
	AudioComponentDescription description;
	description.componentType = kAudioUnitType_Output;
	description.componentSubType = kAudioUnitSubType_RemoteIO;
	description.componentManufacturer = kAudioUnitManufacturer_Apple;
	description.componentFlags = 0;
	description.componentFlagsMask = 0;
	
	AudioComponent audioComponent = AudioComponentFindNext(NULL, &description);
	AU_RETURN_FALSE_IF_ERROR(AudioComponentInstanceNew(audioComponent, &audioUnit_));
	
	AudioStreamBasicDescription streamDescription = [self ASBDForSampleRate:sampleRate numberChannels:numChannels];
	if (inputEnabled) {
		UInt32 enableInput = 1;
		AU_RETURN_FALSE_IF_ERROR(AudioUnitSetProperty(audioUnit_,
                                                      kAudioOutputUnitProperty_EnableIO,
                                                      kAudioUnitScope_Input,
                                                      kInputElement,
                                                      &enableInput,
                                                      sizeof(enableInput)));
		
		AU_RETURN_FALSE_IF_ERROR(AudioUnitSetProperty(audioUnit_,
                                                      kAudioUnitProperty_StreamFormat,
                                                      kAudioUnitScope_Output,  // Output scope because we're defining the output of the input element _to_ our render callback
                                                      kInputElement,
                                                      &streamDescription,
                                                      sizeof(streamDescription)));
	}
	
	AU_RETURN_FALSE_IF_ERROR(AudioUnitSetProperty(audioUnit_,
                                                  kAudioUnitProperty_StreamFormat,
                                                  kAudioUnitScope_Input,  // Input scope because we're defining the input of the output element _from_ our render callback.
                                                  kOutputElement,
                                                  &streamDescription,
                                                  sizeof(streamDescription)));
	
	AURenderCallbackStruct callbackStruct;
	callbackStruct.inputProc = self.renderCallback;
	callbackStruct.inputProcRefCon = self;
	AU_RETURN_FALSE_IF_ERROR(AudioUnitSetProperty(audioUnit_,
                                                  kAudioUnitProperty_SetRenderCallback,
                                                  kAudioUnitScope_Input,
                                                  kOutputElement,
                                                  &callbackStruct,
                                                  sizeof(callbackStruct)));
	
	AU_RETURN_FALSE_IF_ERROR(AudioUnitInitialize(audioUnit_));
	initialized_ = YES;
	AU_LOGV(@"initialized audio unit");
	return true;
}

#else

// https://developer.apple.com/library/mac/technotes/tn2091/_index.htmlq
- (BOOL)initAudioUnitWithSampleRate:(Float64)sampleRate numberChannels:(int)numChannels inputEnabled:(BOOL)inputEnabled {
	
	[self destroyAudioUnits];
	
	// setup output
	AudioComponentDescription description;
	description.componentType = kAudioUnitType_Output;
	description.componentSubType = kAudioUnitSubType_HALOutput;
	description.componentManufacturer = kAudioUnitManufacturer_Apple;
	description.componentFlags = 0;
	description.componentFlagsMask = 0;
	AudioComponent audioComponent = AudioComponentFindNext(NULL, &description);
	AU_RETURN_FALSE_IF_ERROR(AudioComponentInstanceNew(audioComponent, &audioUnit_));
	
	AudioStreamBasicDescription streamDescription = [self ASBDForSampleRate:sampleRate numberChannels:numChannels];
	if (inputEnabled) {
	
		UInt32 enableInput = 1;
		AU_RETURN_FALSE_IF_ERROR(AudioUnitSetProperty(audioUnit_,
                                                      kAudioOutputUnitProperty_EnableIO,
                                                      kAudioUnitScope_Input,
                                                      kInputElement,
                                                      &enableInput,
                                                      sizeof(enableInput)));
	
//		// setup input, disable output
//		description.componentSubType = kAudioUnitSubType_HALOutput;
//		audioComponent = AudioComponentFindNext(NULL, &description);
//		AU_RETURN_FALSE_IF_ERROR(AudioComponentInstanceNew(audioComponent, &inputAudioUnit_));
		
//		UInt32 enableOutput = 0;
//		AU_RETURN_FALSE_IF_ERROR(AudioUnitSetProperty(inputAudioUnit_,
//                                                      kAudioOutputUnitProperty_EnableIO,
//                                                      kAudioUnitScope_Output,
//                                                      kOutputElement,
//                                                      &enableOutput,
//                                                      sizeof(enableOutput)));
//		UInt32 enableInput = 1;
//		AU_RETURN_FALSE_IF_ERROR(AudioUnitSetProperty(inputAudioUnit_,
//                                                      kAudioOutputUnitProperty_EnableIO,
//                                                      kAudioUnitScope_Input,
//                                                      kInputElement,
//                                                      &enableInput,
//                                                      sizeof(enableInput)));
		
//		AU_RETURN_FALSE_IF_ERROR(AudioUnitSetProperty(inputAudioUnit_,
//                                                      kAudioUnitProperty_StreamFormat,
//                                                      kAudioUnitScope_Output,
//                                                      kInputElement,
//                                                      &streamDescription,
//                                                      sizeof(streamDescription)));

#if TARGET_OS_MAC
		// set input device (osx only... iphone has only one device)
		AudioDeviceID inputDevice, outputDevice;// = kAudioObjectUnknown;
		UInt32 size = sizeof(inputDevice);

		AudioObjectPropertyAddress property;
		property.mSelector = kAudioHardwarePropertyDefaultInputDevice;
		property.mScope = kAudioObjectPropertyScopeGlobal;
		property.mElement = kAudioObjectPropertyElementMaster;

//		AU_RETURN_FALSE_IF_ERROR(AudioHardwareServiceGetPropertyData(kAudioObjectSystemObject, &property, 0, NULL, &size, &inputDevice));
//
		AU_RETURN_FALSE_IF_ERROR(AudioObjectGetPropertyData(kAudioObjectSystemObject,
															&property,
															0,
															NULL,
															&size,
															&inputDevice));
		
		property.mSelector = kAudioHardwarePropertyDefaultOutputDevice;
		AU_RETURN_FALSE_IF_ERROR(AudioObjectGetPropertyData(kAudioObjectSystemObject,
															&property,
															0,
															NULL,
															&size,
															&outputDevice));
		
	UInt32 bufferSize = 4096;
	size = sizeof(UInt32);
	property.mSelector = kAudioDevicePropertyBufferFrameSize;
	OSStatus status = AudioHardwareServiceSetPropertyData(outputDevice, &property, 0, NULL, size, &bufferSize);
	AU_LOG_IF_ERROR(status, @"error setting output device buffer size (status = %ld)", status);
	status = AudioHardwareServiceSetPropertyData(outputDevice, &property, 0, NULL, size, &bufferSize);
	AU_LOG_IF_ERROR(status, @"error setting output device buffer size (status = %ld)", status);
	
	Float64 sampleRate = 44100;
	size = sizeof(Float64);
	property.mSelector = kAudioDevicePropertyNominalSampleRate;
	status = AudioHardwareServiceSetPropertyData(inputDevice, &property, 0, NULL, size, &sampleRate);
	AU_LOG_IF_ERROR(status, @"error setting input device sample rate (status = %ld)", status);
	status = AudioHardwareServiceSetPropertyData(outputDevice, &property, 0, NULL, size, &sampleRate);
	AU_LOG_IF_ERROR(status, @"error setting output device sample rate (status = %ld)", status);
		
		status = createAggregateDevice(inputDevice, outputDevice, &aggregateDevice);
		AU_LOG_IF_ERROR(status, @"error creating aggregate device: %@", AUStatusCodeAsString(status));
		
		status = AudioUnitSetProperty(audioUnit_, kAudioOutputUnitProperty_CurrentDevice,
							                   kAudioUnitScope_Global,
							                   0,
											   &aggregateDevice,
											   sizeof(aggregateDevice));
		AU_LOG_IF_ERROR(status, @"error setting input device: %@", AUStatusCodeAsString(status));
		
//		AU_RETURN_FALSE_IF_ERROR(AudioUnitSetProperty(audioUnit_,
//											  kAudioUnitProperty_StreamFormat,
//											  kAudioUnitScope_Input,  // Input scope because we're defining the input of the output element _from_ our render callback.
//											  kOutputElement,
//											  &streamDescription,
//											  sizeof(streamDescription)));
#endif
	}
	else {
	AU_RETURN_FALSE_IF_ERROR(AudioUnitSetProperty(audioUnit_,
                                                  kAudioUnitProperty_StreamFormat,
                                                  kAudioUnitScope_Input,  // Input scope because we're defining the input of the output element _from_ our render callback.
                                                  kOutputElement,
                                                  &streamDescription,
                                                  sizeof(streamDescription)));
	}
//		AU_RETURN_FALSE_IF_ERROR(AudioUnitSetProperty(audioUnit_,
//                                                  kAudioUnitProperty_StreamFormat,
//                                                  kAudioUnitScope_Output,  // Input scope because we're defining the input of the output element _from_ our render callback.
//                                                  kOutputElement,
//                                                  &streamDescription,
//                                                  sizeof(streamDescription)));
	
	AURenderCallbackStruct callbackStruct;
	callbackStruct.inputProc = self.renderCallback;
	callbackStruct.inputProcRefCon = self;
	AU_RETURN_FALSE_IF_ERROR(AudioUnitSetProperty(audioUnit_,
                                                  kAudioUnitProperty_SetRenderCallback,
                                                  kAudioUnitScope_Input,
                                                  kOutputElement,
                                                  &callbackStruct,
                                                  sizeof(callbackStruct)));
	
	AU_RETURN_FALSE_IF_ERROR(AudioUnitInitialize(audioUnit_));
//	if(inputAudioUnit_) {
//		AU_RETURN_FALSE_IF_ERROR(AudioUnitInitialize(inputAudioUnit_));
//
//		// setup audio graph to connect input -> output
//		AUNode inputNode, outputNode;
//		AudioComponentDescription inputDescription = { kAudioUnitType_Output, kAudioUnitSubType_HALOutput, kAudioUnitManufacturer_Apple, 0, 0 };
//		AudioComponentDescription outputDescription = { kAudioUnitType_Output, kAudioUnitSubType_HALOutput, kAudioUnitManufacturer_Apple, 0, 0 };
//		AU_RETURN_FALSE_IF_ERROR(NewAUGraph(&auGraph_));
//		//description.componentType = kAudioUnitType_Output;
//		//description.componentSubType = kAudioUnitSubType_HALOutput;
//		AU_RETURN_FALSE_IF_ERROR(AUGraphOpen(auGraph_));
//		
//		AU_RETURN_FALSE_IF_ERROR(AUGraphAddNode(auGraph_, &inputDescription, &inputNode));
//		//description.componentSubType = kAudioUnitSubType_DefaultOutput;
//		AU_RETURN_FALSE_IF_ERROR(AUGraphAddNode(auGraph_, &outputDescription, &outputNode));
//		AU_RETURN_FALSE_IF_ERROR(AUGraphConnectNodeInput(auGraph_, inputNode, 1, outputNode, 0));
//		
//		AU_RETURN_FALSE_IF_ERROR(AUGraphNodeInfo(auGraph_, inputNode, NULL, &inputAudioUnit_));
//		AU_RETURN_FALSE_IF_ERROR(AUGraphNodeInfo(auGraph_, outputNode, NULL, &audioUnit_));
//		AU_RETURN_FALSE_IF_ERROR(AUGraphStart(auGraph_));
//	}
	initialized_ = YES;
	AU_LOGV(@"initialized audio unit");
	return true;
}

// modified from http://daveaddey.com/?p=51
OSStatus createAggregateDevice(AudioDeviceID inputDevice, AudioDeviceID outputDevice, AudioDeviceID *aggregateDevice) {

	if((*aggregateDevice) != kAudioDeviceUnknown) {
		return destroyAggregateDevice(*aggregateDevice);
		//return noErr;
	}
	OSStatus status = noErr;
	UInt32 outSize;
	Boolean outWritable;
	CFStringRef inputDeviceUID, outputDeviceUID;
	
	// get
	UInt32 size = 0;
	AudioObjectPropertyAddress property;
	property.mScope = kAudioObjectPropertyScopeGlobal;
	property.mElement = kAudioObjectPropertyElementMaster;
	
	size = sizeof(AudioDeviceID);
	property.mSelector = kAudioDevicePropertyDeviceUID;;
	status = AudioObjectHasProperty(inputDevice, &property);
	if (status != noErr) {
		size = sizeof(CFStringRef);
		status = AudioObjectGetPropertyData(inputDevice, &property, 0, NULL, &size, &inputDeviceUID);
		if (status != noErr) return status;
	}
	else {
		return status;
	}
	AU_LOGV(@"input device UID: %@", inputDeviceUID);
	
	status = AudioObjectHasProperty(outputDevice, &property);
	if (status != noErr) {
		size = sizeof(CFStringRef);
		status = AudioObjectGetPropertyData(outputDevice, &property, 0, NULL, &size, &outputDeviceUID);
		if (status != noErr) return status;
	}
	else {
		return status;
	}
	AU_LOGV(@"output device UID: %@", outputDeviceUID);

	//-----------------------
	// Start to create a new aggregate by getting the base audio hardware plugin
	//-----------------------

	status = AudioHardwareGetPropertyInfo(kAudioHardwarePropertyPlugInForBundleID, &outSize, &outWritable);
	if (status != noErr) return status;

	AudioValueTranslation pluginAVT;

	CFStringRef inBundleRef = CFSTR("com.apple.audio.CoreAudio");
	AudioObjectID pluginID;

	pluginAVT.mInputData = &inBundleRef;
	pluginAVT.mInputDataSize = sizeof(inBundleRef);
	pluginAVT.mOutputData = &pluginID;
	pluginAVT.mOutputDataSize = sizeof(pluginID);

	status = AudioHardwareGetProperty(kAudioHardwarePropertyPlugInForBundleID, &outSize, &pluginAVT);
	if (status != noErr) return status;

	//-----------------------
	// Create a CFDictionary for our aggregate device
	//-----------------------

	CFMutableDictionaryRef aggDeviceDict = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

	CFStringRef AggregateDeviceNameRef = CFSTR("libpd aggregate device");
	CFStringRef AggregateDeviceUIDRef = CFSTR("org.libpd.libpd");

	// add the name of the device to the dictionary
	CFDictionaryAddValue(aggDeviceDict, CFSTR(kAudioAggregateDeviceNameKey), AggregateDeviceNameRef);

	// add our choice of UID for the aggregate device to the dictionary
	CFDictionaryAddValue(aggDeviceDict, CFSTR(kAudioAggregateDeviceUIDKey), AggregateDeviceUIDRef);
	
	// make the device private to this process only
	CFDictionaryAddValue(aggDeviceDict, CFSTR(kAudioAggregateDeviceIsPrivateKey), kCFBooleanTrue);

	//-----------------------
	// Create a CFMutableArray for our sub-device list
	//-----------------------

	// this example assumes that you already know the UID of the device to be added
	// you can find this for a given AudioDeviceID via AudioDeviceGetProperty for the kAudioDevicePropertyDeviceUID property
	// obviously the example deviceUID below won't actually work!
	//CFStringRef deviceUID = CFSTR("UIDOfDeviceToBeAdded");

	// we need to append the UID for each device to a CFMutableArray, so create one here
	CFMutableArrayRef subDevicesArray = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	// just the one sub-device in this example, so append the sub-device's UID to the CFArray
	CFArrayAppendValue(subDevicesArray, inputDeviceUID);
	CFArrayAppendValue(subDevicesArray, outputDeviceUID);

	// if you need to add more than one sub-device, then keep calling CFArrayAppendValue here for the other sub-device UIDs

	//-----------------------
	// Feed the dictionary to the plugin, to create a blank aggregate device
	//-----------------------

	AudioObjectPropertyAddress pluginAOPA;
	pluginAOPA.mSelector = kAudioPlugInCreateAggregateDevice;
	pluginAOPA.mScope = kAudioObjectPropertyScopeGlobal;
	pluginAOPA.mElement = kAudioObjectPropertyElementMaster;
	UInt32 outDataSize;// = sizeof(AudioDeviceID);

	status = AudioObjectGetPropertyDataSize(pluginID, &pluginAOPA, 0, NULL, &outDataSize);
	if (status != noErr) return status;

	//AudioDeviceID outAggregateDevice = kAudioDeviceUnknown;;

	status = AudioObjectGetPropertyData(pluginID, &pluginAOPA, sizeof(aggDeviceDict), &aggDeviceDict, &outDataSize, aggregateDevice);
	if (status != noErr) return status;

	// pause for a bit to make sure that everything completed correctly
	// this is to work around a bug in the HAL where a new aggregate device seems to disappear briefly after it is created
	CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.1, false);

	//-----------------------
	// Set the sub-device list
	//-----------------------

	pluginAOPA.mSelector = kAudioAggregateDevicePropertyFullSubDeviceList;
	pluginAOPA.mScope = kAudioObjectPropertyScopeGlobal;
	pluginAOPA.mElement = kAudioObjectPropertyElementMaster;
	outDataSize = sizeof(CFMutableArrayRef);
	status = AudioObjectSetPropertyData((*aggregateDevice), &pluginAOPA, 0, NULL, outDataSize, &subDevicesArray);
	if (status != noErr) return status;

	// pause again to give the changes time to take effect
	CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.1, false);

	//-----------------------
	// Set the master device
	//-----------------------

	// set the master device manually (this is the device which will act as the master clock for the aggregate device)
	// pass in the UID of the device you want to use
	pluginAOPA.mSelector = kAudioAggregateDevicePropertyMasterSubDevice;
	pluginAOPA.mScope = kAudioObjectPropertyScopeGlobal;
	pluginAOPA.mElement = kAudioObjectPropertyElementMaster;
	outDataSize = sizeof(outputDeviceUID);
	status = AudioObjectSetPropertyData((*aggregateDevice), &pluginAOPA, 0, NULL, outDataSize, &outputDeviceUID);
	if (status != noErr) return status;

	// pause again to give the changes time to take effect
	CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.1, false);

	//-----------------------
	// Clean up
	//-----------------------

	// release the CF objects we have created - we don't need them any more
	CFRelease(aggDeviceDict);
	CFRelease(subDevicesArray);

	// release the device UID
	//CFRelease(deviceUID);

	return noErr;

}

OSStatus destroyAggregateDevice(AudioDeviceID inDeviceToDestroy) {

	OSStatus status = noErr;

	//-----------------------
	// Start by getting the base audio hardware plugin
	//-----------------------

	UInt32 outSize;
	Boolean outWritable;
	status = AudioHardwareGetPropertyInfo(kAudioHardwarePropertyPlugInForBundleID, &outSize, &outWritable);
	if (status != noErr) return status;

	AudioValueTranslation pluginAVT;

	CFStringRef inBundleRef = CFSTR("com.apple.audio.CoreAudio");
	AudioObjectID pluginID;

	pluginAVT.mInputData = &inBundleRef;
	pluginAVT.mInputDataSize = sizeof(inBundleRef);
	pluginAVT.mOutputData = &pluginID;
	pluginAVT.mOutputDataSize = sizeof(pluginID);

	status = AudioHardwareGetProperty(kAudioHardwarePropertyPlugInForBundleID, &outSize, &pluginAVT);
	if (status != noErr) return status;

	//-----------------------
	// Feed the AudioDeviceID to the plugin, to destroy the aggregate device
	//-----------------------

	AudioObjectPropertyAddress pluginAOPA;
	pluginAOPA.mSelector = kAudioPlugInDestroyAggregateDevice;
	pluginAOPA.mScope = kAudioObjectPropertyScopeGlobal;
	pluginAOPA.mElement = kAudioObjectPropertyElementMaster;
	UInt32 outDataSize;

	status = AudioObjectGetPropertyDataSize(pluginID, &pluginAOPA, 0, NULL, &outDataSize);
	if (status != noErr) return status;

	status = AudioObjectGetPropertyData(pluginID, &pluginAOPA, 0, NULL, &outDataSize, &inDeviceToDestroy);
	if (status != noErr) return status;

	return noErr;

}

#endif

@end
