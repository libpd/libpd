/**
 * This software is copyrighted by Reality Jockey Ltd. and Peter Brinkmann. 
 * The following terms (the "Standard Improved BSD License") apply to 
 * all files associated with the software unless explicitly disclaimed 
 * in individual files:
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above  
 * copyright notice, this list of conditions and the following 
 * disclaimer in the documentation and/or other materials provided
 * with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 * products derived from this software without specific prior 
 * written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,   
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "PdAudio.h"

@interface PdAudio()

// private
- (OSStatus)initializeAudioSession:(int)ticks withAudioCategory:(UInt32)audioCategory;
- (OSStatus)initializeAudioUnit;

@end


@implementation PdAudio

@synthesize audioUnit;
@synthesize sampleRate;
@synthesize ticksPerBuffer;
@synthesize numInputChannels;
@synthesize numOutputChannels;
@synthesize microphoneVolume;
@synthesize floatBuffer;
@synthesize floatBufferLength;

/** The render callback used by the audio unit. This is where all of the action is regarding the AU. */
// This function must be listed first (and thus defined) because a pointer to this function is used
// to define the AU later in the code during setup.
// http://developer.apple.com/iphone/library/documentation/AudioUnit/Reference/AUComponentServicesReference/Reference/reference.html#//apple_ref/doc/c_ref/AURenderCallback
OSStatus renderCallback(void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags, 
                        const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber,
                        UInt32 inNumberFrames, AudioBufferList *ioData) {
  
  PdAudio *controller = (PdAudio *) inRefCon;
  
  // This NSAutoreleasePool is actually only needed if the callbacks from Pd use
  // Foundation objects. It is also possible to force the programmer to create their own pool
  // in their callback functions. For ease of use a pool is created in each invocation of the audio
  // callback. There is a certain amount of overhead in this, but through experience it has been
  // shown not to negatively impact performance.
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  
  // Get the remote io audio unit to render its input into the buffers
  // 1 == inBusNumber for mic input
  AudioUnitRender(controller.audioUnit, ioActionFlags, inTimeStamp, 1, inNumberFrames, ioData);
  
  // the buffer contains the input, and when libpd_process_float returns, it contains the output
  short *shortBuffer = (short *) ioData->mBuffers[0].mData;
  
  /*
   #if __IPHONE_OS_VERSION_MAX_ALLOWED >= 4000 || TARGET_OS_MAC
   float *floatBuffer = controller.floatBuffer;
   int floatBufferLen = controller.floatBufferLen;
   for (int i = 0; i < floatBufferLen; i++) {
   floatBuffer[i] = (float) shortBuffer[i];
   }
   float a = 0.000030517578125f * controller.micVolume; // == 1/32768 * microphone volume
   vDSP_vsmul(floatBuffer, 1, &a, floatBuffer, 1, floatBufferLen);
   [PdBase processFloatWithInputBuffer:floatBuffer andOutputBuffer:floatBuffer];
   float min = -1.0f;
   float max = 1.0f;
   vDSP_vclip(floatBuffer, 1, &min, &max, floatBuffer, 1, floatBufferLen);
   a = 32767.0f;
   vDSP_vsmul(floatBuffer, 1, &a, floatBuffer, 1, floatBufferLen);
   for (int i = 0; i < floatBufferLen; i++) {
   shortBuffer[i] = (short) floatBuffer[i];
   }
   #else
   */
  float *floatBuffer = controller.floatBuffer;
  int floatBufferLength = controller.floatBufferLength;
  float a = 0.000030517578125f * controller.microphoneVolume; // == 1/32768 * microphone volume
  
  // If the numbers of input and output channels differ, we may not need to initialize the entire 
  // buffer, but we do it anyway for simplicity. Same issue when filling the output buffer.
  for (int i = 0; i < floatBufferLength; i++) {
    floatBuffer[i] = ((float) shortBuffer[i]) * a;
  }
  [PdBase processFloatWithInputBuffer:floatBuffer outputBuffer:floatBuffer ticks:controller.ticksPerBuffer];
  for (int i = 0; i < floatBufferLength; i++) {
    float f = floatBuffer[i];
    if (f < -1.0f) shortBuffer[i] = -32767;
    else if (f > 1.0f) shortBuffer[i] = 32767;
    else shortBuffer[i] = (short) (f * 32767.0f);
  }
  //#endif
  
  [pool drain]; // drain the pool and release any retained objects

  return 0; // no errors
}

// the interrupt listener for the audio session
void audioSessionInterruptListener(void *inClientData, UInt32 inInterruption) {
  PdAudio *controller = (PdAudio *) inClientData;
  switch (inInterruption) {
    case kAudioSessionBeginInterruption: {
      // when the interruption begins, suspend audio playback
      NSLog(@"AudioSession === kAudioSessionBeginInterruption");
      [controller pause];
      break;
    }
    case kAudioSessionEndInterruption: {
      // when the interruption ends, resume audio playback
      NSLog(@"AudioSession === kAudioSessionEndInterruption");
      [controller play];
      break;
    }
    default: {
      break;
    }
  }
}

- (id)initWithSampleRate:(float)newSampleRate andTicksPerBuffer:(int)ticks 
  andNumberOfInputChannels:(int)inputChannels andNumberOfOutputChannels:(int)outputChannels {

  // this version of init uses PlayAndRecord audio session
  // set the audio category to PlayAndRecord so that we can have low-latency IO
  UInt32 audioCategory = kAudioSessionCategory_PlayAndRecord;
  
  return [self initWithSampleRate:newSampleRate andTicksPerBuffer:ticks 
    andNumberOfInputChannels:inputChannels andNumberOfOutputChannels:outputChannels 
    andAudioSessionCategory:audioCategory];
}

#define FAIL_ON_ERROR(s, x) \
    if ((status = (x))) { \
        NSLog(s, status); \
        [self release]; \
        self = nil; \
        return nil; \
    }

- (id)initWithSampleRate:(float)newSampleRate andTicksPerBuffer:(int)ticks 
  andNumberOfInputChannels:(int)inputChannels andNumberOfOutputChannels:(int)outputChannels
    andAudioSessionCategory:(UInt32)audioSessionCategory {
    
  self = [super init];
  if (self != nil) {
    audioUnit = NULL;
    ticksPerBuffer = ticks;
    numInputChannels = inputChannels;
    numOutputChannels = outputChannels;
    sampleRate = (Float64) newSampleRate;
    microphoneVolume = 1.0f;
    
    int numberOfChannels = (numInputChannels < numOutputChannels) ? numOutputChannels : numInputChannels;
    floatBufferLength = [PdBase getBlockSize] * ticks * numberOfChannels;
    floatBuffer = (float *) malloc(floatBufferLength * sizeof(float));;
    
    OSStatus status;
    FAIL_ON_ERROR(@"init audio session failed with status %ld",
                  [self initializeAudioSession:ticks withAudioCategory:audioSessionCategory]);
    FAIL_ON_ERROR(@"init audio unit failed with status %ld", [self initializeAudioUnit]);
    FAIL_ON_ERROR(@"PdBase openAudio failed wit status %ld",
                  [PdBase openAudioWithSampleRate:sampleRate inputChannels:numInputChannels 
                                outputChannels:numOutputChannels]);

    [PdBase computeAudio:YES];
  }
  return self;
}

- (void)dealloc {
  [self pause];
  // TODO: delete audioUnit?
  free(floatBuffer);
  floatBuffer = nil;
  [super dealloc];
}

/** Begin audio/scene playback.*/
- (int)play {
  NSLog(@"AudioSession === starting audio unit.");
  return AudioOutputUnitStart(audioUnit);
}

/** Pause audio/scene playback. */
- (int)pause {
  NSLog(@"AudioSession === stopping audio unit.");
  return AudioOutputUnitStop(audioUnit);
}

// private

#define CHECK_STATUS(x) if ((status = (x))) return status

- (OSStatus)initializeAudioSession:(int)ticks withAudioCategory:(UInt32) audioCategory {
  /*** Create AudioSession interface to Core Audio === ***/
  
  OSStatus status;

  // initialise the audio session
  CHECK_STATUS(AudioSessionInitialize(NULL, NULL, audioSessionInterruptListener, self));
  
  CHECK_STATUS(AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(audioCategory), &audioCategory));
  
  // set the sample rate of the session
  CHECK_STATUS(AudioSessionSetProperty(kAudioSessionProperty_PreferredHardwareSampleRate, sizeof(sampleRate), &sampleRate));
  NSLog(@"AudioSession === setting PreferredHardwareSampleRate to %.0fHz.", sampleRate);
  
  // set buffer size
  Float32 bufferSize = (Float32) [PdBase getBlockSize] * ticks; // requested buffer size
  Float32 bufferDuration = (bufferSize + 0.5) / sampleRate; // buffer duration in seconds - add 0.5 due to differences in armv6 and armv7 fp
  CHECK_STATUS(AudioSessionSetProperty(kAudioSessionProperty_PreferredHardwareIOBufferDuration, 
      sizeof(bufferDuration), &bufferDuration));
  NSLog(@"AudioSession === setting PreferredHardwareIOBufferDuration to %3.2fms.", bufferDuration*1000.0);
  
  // NOTE: note that round-off errors make it hard to determine whether the requested buffersize
  // was granted. we just assume that it was and carry on.
  
  CHECK_STATUS(AudioSessionSetActive(true));
  NSLog(@"AudioSession === starting Audio Session.");
  
  // print value of properties to check that everything was set properly
  Float64 audioSessionProperty64 = 0;
  Float32 audioSessionProperty32 = 0;
  UInt32 audioSessionPropertySize64 = sizeof(audioSessionProperty64);
  UInt32 audioSessionPropertySize32 = sizeof(audioSessionProperty32);
  CHECK_STATUS(AudioSessionGetProperty(kAudioSessionProperty_CurrentHardwareSampleRate, 
      &audioSessionPropertySize64, &audioSessionProperty64));
  NSLog(@"AudioSession === CurrentHardwareSampleRate: %.0fHz", audioSessionProperty64);
  sampleRate = audioSessionProperty64;
  
  CHECK_STATUS(AudioSessionGetProperty(kAudioSessionProperty_CurrentHardwareIOBufferDuration, 
      &audioSessionPropertySize32, &audioSessionProperty32));
  int blockSize = lrint(audioSessionProperty32 * audioSessionProperty64);
  NSLog(@"AudioSession === CurrentHardwareIOBufferDuration: %3.2fms", audioSessionProperty32*1000.0f);
  NSLog(@"AudioSession === block size: %i", blockSize);
  return status;
}

- (OSStatus)initializeAudioUnit {
  // http://developer.apple.com/iphone/library/documentation/Audio/Conceptual/AudioUnitLoadingGuide_iPhoneOS/AccessingAudioUnits/LoadingIndividualAudioUnits.html#//apple_ref/doc/uid/TP40008781-CH103-SW11
  
  OSStatus status;

  // create an AudioComponentDescription describing a RemoteIO audio unit
  // such a component provides an interface from microphone to speaker
  AudioComponentDescription auDescription;
  auDescription.componentType = kAudioUnitType_Output;
  auDescription.componentSubType = kAudioUnitSubType_RemoteIO;
  auDescription.componentManufacturer = kAudioUnitManufacturer_Apple;
  auDescription.componentFlags = 0;
  auDescription.componentFlagsMask = 0;
  
  // find an audio component fitting the given description
  AudioComponent foundComponent = AudioComponentFindNext(NULL, &auDescription);
  
  // create a new audio unit instance
  CHECK_STATUS(AudioComponentInstanceNew(foundComponent, &audioUnit));
  
  // connect the AU to hardware input and output
  UInt32 doSetProperty = 1;
  AudioUnitElement inputBus = 1;
  AudioUnitElement outputBus = 0;
  // connect the AU to the microphone 
  CHECK_STATUS(AudioUnitSetProperty(audioUnit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Input, 
      inputBus, &doSetProperty, sizeof(doSetProperty)));
  // connect the AU to the soundout
  CHECK_STATUS(AudioUnitSetProperty(audioUnit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Output, 
      outputBus, &doSetProperty, sizeof(doSetProperty)));
  
  // set the sample rate on the input and output busses
  CHECK_STATUS(AudioUnitSetProperty(audioUnit, kAudioUnitProperty_SampleRate, kAudioUnitScope_Input, outputBus, 
      &sampleRate, sizeof(sampleRate)));
  CHECK_STATUS(AudioUnitSetProperty(audioUnit, kAudioUnitProperty_SampleRate, kAudioUnitScope_Output, inputBus, 
      &sampleRate, sizeof(sampleRate)));
  
  // request the audio data stream format for input and output
  // NOTE: this really is a request. The system will set the stream to whatever it damn well pleases.
  // The settings here are what are known to work: 16-bit mono (interleaved) @ 22050
  // and thus no rigorous checking is done in order to ensure that the request stream format
  // is actually being used. It would be nice to be able to use format kAudioFormatFlagsNativeFloatPacked
  // which would allow us to avoid converting between float and int sample type manually.
  AudioStreamBasicDescription toneStreamFormatInput;
  memset (&toneStreamFormatInput, 0, sizeof (toneStreamFormatInput)); // clear all fields
  toneStreamFormatInput.mSampleRate       = sampleRate;
  toneStreamFormatInput.mFormatID         = kAudioFormatLinearPCM;
  toneStreamFormatInput.mFormatFlags      = kAudioFormatFlagsCanonical;
  toneStreamFormatInput.mBytesPerPacket   = 2 * numInputChannels;
  toneStreamFormatInput.mFramesPerPacket  = 1;
  toneStreamFormatInput.mBytesPerFrame    = 2 * numInputChannels;
  toneStreamFormatInput.mChannelsPerFrame = numInputChannels;
  toneStreamFormatInput.mBitsPerChannel   = 16;
  // apply the audio data stream format to bus 0 of the input scope of the Remote I/O AU. This is
  // actually the OUTPUT to the system.
  CHECK_STATUS(AudioUnitSetProperty(audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, outputBus, 
                             &toneStreamFormatInput, sizeof(toneStreamFormatInput)));
  
  // set audio output format to 16-bit stereo
  AudioStreamBasicDescription toneStreamFormatOutput;
  memset (&toneStreamFormatOutput, 0, sizeof(toneStreamFormatOutput));
  toneStreamFormatOutput.mSampleRate       = sampleRate;
  toneStreamFormatOutput.mFormatID         = kAudioFormatLinearPCM;
  toneStreamFormatOutput.mFormatFlags      = kAudioFormatFlagsCanonical;
  toneStreamFormatOutput.mBytesPerPacket   = 2 * numOutputChannels;
  toneStreamFormatOutput.mFramesPerPacket  = 1;
  toneStreamFormatOutput.mBytesPerFrame    = 2 * numOutputChannels;
  toneStreamFormatOutput.mChannelsPerFrame = numOutputChannels;
  toneStreamFormatOutput.mBitsPerChannel   = 16;
  
  // apply the audio data stream format to bus 1 of the output scope of the Remote I/O AU. This is
  // actually the INPUT to the system.
  CHECK_STATUS(AudioUnitSetProperty(audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, inputBus, 
                       &toneStreamFormatOutput, sizeof(toneStreamFormatOutput)));
  
  // register the render callback. This is the function that the audio unit calls when it needs audio
  // the callback function (renderCallback()) is defined at the top of the page.
  AURenderCallbackStruct renderCallbackStruct;
  renderCallbackStruct.inputProc = renderCallback;
  renderCallbackStruct.inputProcRefCon = self; // this is an optional data pointer
                                               // pass the AudioController object
  
  CHECK_STATUS(AudioUnitSetProperty(audioUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 
                       outputBus, &renderCallbackStruct, sizeof(renderCallbackStruct)));
  
  // disable buffer allocation on output (necessary?)
  // http://developer.apple.com/iphone/library/documentation/Audio/Conceptual/AudioUnitLoadingGuide_iPhoneOS/AccessingAudioUnits/LoadingIndividualAudioUnits.html#//apple_ref/doc/uid/TP40008781-CH103-SW19
  doSetProperty = 0;
  CHECK_STATUS(AudioUnitSetProperty(audioUnit, kAudioUnitProperty_ShouldAllocateBuffer, kAudioUnitScope_Output, 
                       outputBus, &doSetProperty, sizeof(doSetProperty)));
  
  // finally, initialise the audio unit. It is ready to go.
  CHECK_STATUS(AudioUnitInitialize(audioUnit));
  
  // ensure that all parameters and settings have been successfully applied
  AudioStreamBasicDescription toneStreamFormatInputTest;
  memset(&toneStreamFormatInputTest, 0, sizeof(toneStreamFormatInputTest));
  UInt32 toneStreamFormatSize = sizeof(AudioStreamBasicDescription);
  CHECK_STATUS(AudioUnitGetProperty(audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input,
                       outputBus, &toneStreamFormatInputTest, &toneStreamFormatSize));
  NSLog(@"=== input stream format:");
  NSLog(@"  mSampleRate: %.0fHz", toneStreamFormatInputTest.mSampleRate);
  NSLog(@"  mFormatID: %lu", toneStreamFormatInputTest.mFormatID);
  NSLog(@"  mFormatFlags: %lu", toneStreamFormatInputTest.mFormatFlags);
  NSLog(@"  mBytesPerPacket: %lu", toneStreamFormatInputTest.mBytesPerPacket);
  NSLog(@"  mFramesPerPacket: %lu", toneStreamFormatInputTest.mFramesPerPacket);
  NSLog(@"  mBytesPerFrame: %lu", toneStreamFormatInputTest.mBytesPerFrame);
  NSLog(@"  mChannelsPerFrame: %lu", toneStreamFormatInputTest.mChannelsPerFrame);
  NSLog(@"  mBitsPerChannel: %lu", toneStreamFormatInputTest.mBitsPerChannel);
  
  
  AudioStreamBasicDescription toneStreamFormatOutputTest;
  memset (&toneStreamFormatOutputTest, 0, sizeof(toneStreamFormatOutputTest));
  CHECK_STATUS(AudioUnitGetProperty(audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output,
                       inputBus, &toneStreamFormatOutputTest, &toneStreamFormatSize));
  NSLog(@"=== output stream format:");
  NSLog(@"  mSampleRate: %.0fHz", toneStreamFormatOutputTest.mSampleRate);
  NSLog(@"  mFormatID: %lu", toneStreamFormatOutputTest.mFormatID);
  NSLog(@"  mFormatFlags: %lu", toneStreamFormatOutputTest.mFormatFlags);
  NSLog(@"  mBytesPerPacket: %lu", toneStreamFormatOutputTest.mBytesPerPacket);
  NSLog(@"  mFramesPerPacket: %lu", toneStreamFormatOutputTest.mFramesPerPacket);
  NSLog(@"  mBytesPerFrame: %lu", toneStreamFormatOutputTest.mBytesPerFrame);
  NSLog(@"  mChannelsPerFrame: %lu", toneStreamFormatOutputTest.mChannelsPerFrame);
  NSLog(@"  mBitsPerChannel: %lu", toneStreamFormatOutputTest.mBitsPerChannel);
  
  Float64 auSampleRate = 0.0;
  UInt32 sampleRateSize = sizeof(auSampleRate);
  CHECK_STATUS(AudioUnitGetProperty(audioUnit, kAudioUnitProperty_SampleRate, kAudioUnitScope_Input, outputBus, &auSampleRate, &sampleRateSize));
  NSLog(@"=== input sample rate: %.0fHz", auSampleRate);
  
  CHECK_STATUS(AudioUnitGetProperty(audioUnit, kAudioUnitProperty_SampleRate, kAudioUnitScope_Output, inputBus, &auSampleRate, &sampleRateSize));
  NSLog(@"=== output sample rate: %.0fHz", auSampleRate);
    return status;
}

@end
