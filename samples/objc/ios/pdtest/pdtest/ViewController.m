//
//  ViewController.m
//  pdtest
//
//  Created by Dan Wilcox on 1/16/13.
//  Copyright (c) 2013, 2020 libpd team. All rights reserved.
//

#import "ViewController.h"

#import <AVKit/AVKit.h>
#import "AppDelegate.h"
#import "PdAudioController.h"
#import "PdFile.h"

// uncomment to update number of inputs & outputs on device changes
//#define AUTO_IO

@interface ViewController() {}

@property (strong, nonatomic) PdAudioController *audioController;
@property (strong, nonatomic) PdFile *patch;

@end

@implementation ViewController

- (void)viewDidLoad {
	[super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.

	// add audio route picker button to view
	if (@available(iOS 11.0, *)) {
		AVRoutePickerView *pickerView = [[AVRoutePickerView alloc] initWithFrame:self.routePickerContainer.bounds];
		[self.routePickerContainer addSubview:pickerView];
		self.routePickerContainer.backgroundColor = UIColor.clearColor;
	}
	else { // AVRoutePickerView not available before iOS 11
		self.routePickerContainer.hidden = YES;
	}

	// setup and run tests
	[self setupPd];
	[self testPd];

	// setup audio and application state change notifications
	[self setupNotifications];
}

- (void)dealloc {
	[self clearNotifications];
}

- (void)didReceiveMemoryWarning {
	[super didReceiveMemoryWarning];
	// Dispose of any resources that can be recreated.
}

// play tone on touch: y to pitch and x to pan pos
- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {

	UITouch *touch = [touches anyObject];
	CGPoint pos = [touch locationInView:self.view];
	int pitch = (-1 * (pos.y/CGRectGetHeight(self.view.frame)) + 1) * 127;

	[PdBase sendList:@[@"pitch", @(pitch)] toReceiver:@"tone"];
	[PdBase sendBangToReceiver:@"tone"];

	NSLog(@"touch at %.f %.f with pitch: %d", pos.x, pos.y, pitch);
}

#pragma mark Pd

- (void)setupPd {

	// note: define or uncomment AU_DEBUG_VERBOSE in AudioHelpers.h for verbose debug prints

	// set required inputs & outputs...
	int inputChannels = 2;
	int outputChannels = 2;

	// ... or use current hardware channels, also see routeChanged: for handling device changes
	// note: set PlayAndRecord category so number of inputs will not be 0 when calling
	//       AVAUdioSession.sharedInstance.inputNumberOfChannels the first time,
	//       then setActive: so audio session updates its max channels
	#ifdef AUTO_IO
	[AVAudioSession.sharedInstance setCategory:AVAudioSessionCategoryPlayAndRecord error:nil];
	[AVAudioSession.sharedInstance setActive:YES error:nil];
	inputChannels = (int)AVAudioSession.sharedInstance.maximumInputNumberOfChannels;
	outputChannels = (int)AVAudioSession.sharedInstance.maximumOutputNumberOfChannels;
	#endif

	// configure a typical audio session with input and output
	self.audioController = [[PdAudioController alloc] init];
	// add/override some common session settings
	//self.audioController.mixWithOthers = NO; // this app's audio only
	//self.audioController.defaultToSpeaker = NO; // use receiver (earpiece) instead
	self.audioController.allowBluetooth = YES; // allow hands free Bluetooth input/output
	self.audioController.allowBluetoothA2DP = YES; // allow stereo Bluetooth output
	self.audioController.allowAirPlay = YES; // allow AirPlay (output only categories before iOS 10)
	//self.audioController.preferStereo = NO; // allow mono
	//self.audioController.mode = AVAudioSessionModeVideoRecording; // set custom mode, depends on category
	PdAudioStatus status = [self.audioController configurePlaybackWithSampleRate:44100
	                                                               inputChannels:inputChannels
	                                                              outputChannels:outputChannels
	                                                                inputEnabled:YES];
	if (status == PdAudioError) {
		NSLog(@"could not configure audio");
	} else if (status == PdAudioPropertyChanged) {
		NSLog(@"some of the audio properties were changed during configuration");
	} else {
		NSLog(@"audio configuration successful");
	}

	// other AVAudioSession configurations can be done manually (see the Apple docs)
	// for example, force voice chat mode: enable echo cancelation, Bluetooth, and default to speaker output
	// this works for the PlayAndRecord category only, ie. playback with input and output channels > 0
	//[AVAudioSession.sharedInstance setMode:AVAudioSessionModeVoiceChat error:nil];

	// log actual settings
	[self.audioController print];
	[self updateInfoLabels];

	// set AppDelegate as PdReceiverDelegate to receive messages from pd
	[PdBase setDelegate:self];
	[PdBase setMidiDelegate:self]; // for midi too

	// receive messages to fromPD: [r fromPD]
	[PdBase subscribe:@"fromPD"];

	// add search path
	[PdBase addToSearchPath:[NSString stringWithFormat:@"%@/pd/abs", NSBundle.mainBundle.bundlePath]];

	// turn on dsp
	self.audioController.active = YES;
	[PdBase computeAudio:YES];
}

- (void)testPd {

	int midiChan = 1; // midi channels are 1-16

	NSLog(@"-- BEGIN Patch Test");

	// open patch
	self.patch = [PdFile openFileNamed:@"test.pd"
								  path:[NSString stringWithFormat:@"%@/pd", NSBundle.mainBundle.bundlePath]];
	NSLog(@"%@", self.patch);

	// close patch
	[self.patch closeFile];
	NSLog(@"%@", self.patch);

	// open patch again
	self.patch = [self.patch openNewInstance];
	NSLog(@"%@", self.patch);

	NSLog(@"-- FINISH Patch Test");


	NSLog(@"-- BEGIN Message Test");

	// test basic atoms
	[PdBase sendBangToReceiver:@"toPd"];
	[PdBase sendFloat:100 toReceiver:@"toPD"];
	[PdBase sendSymbol:@"test string" toReceiver:@"toPD" ];

	// send a list
	NSArray *list = @[@1.23f, @"a symbol"];
	[PdBase sendList:list toReceiver:@"toPd"];

	// send a list to the $0 receiver ie $0-toPd
	[PdBase sendList:list toReceiver:[NSString stringWithFormat:@"%d-toPd", self.patch.dollarZero]];

	// send a message
	[PdBase sendMessage:@"msg" withArguments:list toReceiver:@"toPD"];


	NSLog(@"-- FINISH Message Test");


	NSLog(@"-- BEGIN MIDI Test");

	// send functions
	[PdBase sendNoteOn:midiChan pitch:60 velocity:64];
	[PdBase sendControlChange:midiChan controller:0 value:64];
	[PdBase sendProgramChange:midiChan value:100];
	[PdBase sendPitchBend:midiChan value:2000];

	[PdBase sendAftertouch:midiChan value:100];
	[PdBase sendPolyAftertouch:midiChan pitch:64 value:100];
	[PdBase sendMidiByte:0 byte:239];
	[PdBase sendSysex:0 byte:239];
	[PdBase sendSysRealTime:0 byte:239];

	NSLog(@"-- FINISH MIDI Test");


	NSLog(@"-- BEGIN Array Test");

	// array check length
	int array1Len = [PdBase arraySizeForArrayNamed:@"array1"];
	NSLog(@"array1 len: %d", array1Len);

	// read array
	float array1[array1Len];
	[PdBase copyArrayNamed:@"array1" withOffset:0 toArray:array1 count:array1Len];
	NSMutableString *array1String = [[NSMutableString alloc] init];
	for(int i = 0; i < array1Len; ++i)
		[array1String appendString:[NSString stringWithFormat:@"%f ", array1[i]]];
	NSLog(@"%@", array1String);

	// clear array
	for(int i = 0; i < array1Len; ++i)
		array1[i] = 0;
	[PdBase copyArray:array1 toArrayNamed:@"array1" withOffset:0 count:array1Len];

	// read array
	[array1String setString:@""];
	[PdBase copyArrayNamed:@"array1" withOffset:0 toArray:array1 count:array1Len];
	for(int i = 0; i < array1Len; ++i)
		[array1String appendString:[NSString stringWithFormat:@"%f ", array1[i]]];
	NSLog(@"%@", array1String);

	// write array
	for(int i = 0; i < array1Len; ++i)
		array1[i] = i;
	[PdBase copyArray:array1 toArrayNamed:@"array1" withOffset:0 count:array1Len];

	// read array
	[array1String setString:@""];
	[PdBase copyArrayNamed:@"array1" withOffset:0 toArray:array1 count:array1Len];
	for(int i = 0; i < array1Len; ++i)
		[array1String appendString:[NSString stringWithFormat:@"%f ", array1[i]]];
	NSLog(@"%@", array1String);

	NSLog(@"-- FINISH Array Test");


	NSLog(@"-- BEGIN PD Test");

	[PdBase sendSymbol:@"test" toReceiver:@"toPD"];

	NSLog(@"-- FINISH PD Test");


	NSLog(@"-- BEGIN Polling Test");

	// set delegates again, but disable polling
	[PdBase setDelegate:nil]; // clear delegate & stop polling timer
	[PdBase setMidiDelegate:nil];
	[PdBase setDelegate:self pollingEnabled:NO];
	[PdBase setMidiDelegate:self pollingEnabled:NO];

	[PdBase sendSymbol:@"test" toReceiver:@"toPD"];

	// process messages manually
	[PdBase receiveMessages];
	[PdBase receiveMidi];

	NSLog(@"-- FINISH Polling Test");

	// reenable delegates
	[PdBase setDelegate:self];
	[PdBase setMidiDelegate:self];
}

#pragma mark PdReceiverDelegate

// uncomment this to get print statements from pd
- (void)receivePrint:(NSString *)message {
	NSLog(@"%@", message);
}

- (void)receiveBangFromSource:(NSString *)source {
	NSLog(@"Bang from %@", source);
}

- (void)receiveFloat:(float)received fromSource:(NSString *)source {
	NSLog(@"Float from %@: %f", source, received);
}

- (void)receiveSymbol:(NSString *)symbol fromSource:(NSString *)source {
	NSLog(@"Symbol from %@: %@", source, symbol);
}

- (void)receiveList:(NSArray *)list fromSource:(NSString *)source {
	NSLog(@"List from %@", source);
}

- (void)receiveMessage:(NSString *)message withArguments:(NSArray *)arguments fromSource:(NSString *)source {
	NSLog(@"Message to %@ from %@", message, source);
}

- (void)receiveNoteOn:(int)pitch withVelocity:(int)velocity forChannel:(int)channel{
	NSLog(@"NoteOn: %d %d %d", channel, pitch, velocity);
}

- (void)receiveControlChange:(int)value forController:(int)controller forChannel:(int)channel{
	NSLog(@"Control Change: %d %d %d", channel, controller, value);
}

- (void)receiveProgramChange:(int)value forChannel:(int)channel{
	NSLog(@"Program Change: %d %d", channel, value);
}

- (void)receivePitchBend:(int)value forChannel:(int)channel{
	NSLog(@"Pitch Bend: %d %d", channel, value);
}

- (void)receiveAftertouch:(int)value forChannel:(int)channel{
	NSLog(@"Aftertouch: %d %d", channel, value);
}

- (void)receivePolyAftertouch:(int)value forPitch:(int)pitch forChannel:(int)channel{
	NSLog(@"Poly Aftertouch: %d %d %d", channel, pitch, value);
}

- (void)receiveMidiByte:(int)byte forPort:(int)port{
	NSLog(@"Midi Byte: %d 0x%X", port, byte);
}

#pragma mark UI

- (void)updateInfoLabels {
	AVAudioSession *session = AVAudioSession.sharedInstance;
	self.pdLabel.text = [NSString stringWithFormat:
		@"Pd:  in %d out %d sr %gk",
		self.audioController.audioUnit.inputChannels,
		self.audioController.audioUnit.outputChannels,
		(self.audioController.audioUnit.sampleRate / 1000)];
	if (session.inputNumberOfChannels > 0) {
		self.inputLabel.text = [NSString stringWithFormat:
			@"In:  %@ %d %gk",
			[session.currentRoute.inputs.firstObject portName],
			(int)session.inputNumberOfChannels,
			(session.sampleRate / 1000)];
	}
	else {
		self.inputLabel.text = @"In: none";
	}
	if (session.outputNumberOfChannels > 0) {
		self.outputLabel.text = [NSString stringWithFormat:
			@"Out: %@ %d %gk",
			[session.currentRoute.outputs.firstObject portName],
			(int)session.outputNumberOfChannels,
			(session.sampleRate / 1000)];
	}
	else {
		self.outputLabel.text = @"Out: none";
	}
}

#pragma mark Notifications

- (void)setupNotifications {
	[NSNotificationCenter.defaultCenter addObserver:self
	                                       selector:@selector(routeChanged:)
	                                           name:AVAudioSessionRouteChangeNotification
	                                         object:nil];
	[NSNotificationCenter.defaultCenter addObserver:self
	                                       selector:@selector(didBecomeActive:)
	                                           name:UIApplicationDidBecomeActiveNotification
	                                         object:nil];
}

- (void)clearNotifications {
	[NSNotificationCenter.defaultCenter removeObserver:self
	                                              name:AVAudioSessionRouteChangeNotification
	                                            object:nil];
	[NSNotificationCenter.defaultCenter removeObserver:self
	                                              name:UIApplicationDidBecomeActiveNotification
	                                            object:nil];
}

// update the info labels if the audio session route has changed, ie. device plugged-in
- (void)routeChanged:(NSNotification *)notification {
	NSDictionary *dict = notification.userInfo;
	NSUInteger reason = [dict[AVAudioSessionRouteChangeReasonKey] unsignedIntegerValue];
	switch (reason) {
		default: return;
		case AVAudioSessionRouteChangeReasonNewDeviceAvailable:
			NSLog(@"route changed: new device");
			break;
		case AVAudioSessionRouteChangeReasonOldDeviceUnavailable:
			NSLog(@"route changed: old device unavailable");
			break;
		case AVAudioSessionRouteChangeReasonOverride:
			NSLog(@"route changed: override");
			break;
	}

	// update inputs and outputs on a route change?
	#ifdef AUTO_IO
	[self.audioController configurePlaybackWithSampleRate:self.audioController.sampleRate
	                                        inputChannels:(int)AVAudioSession.sharedInstance.inputNumberOfChannels
	                                       outputChannels:(int)AVAudioSession.sharedInstance.outputNumberOfChannels
	                                         inputEnabled:YES];
	#endif

	// update the UI on the main thread, wait a little so audio changes have time to finalize,
	// don't update UI if backgrounded as this might cause the app to be terminated!
	dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1.0 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
		if (UIApplication.sharedApplication.applicationState == UIApplicationStateActive) {
			[self updateInfoLabels];
		}
	});
}

/// update the info labels when the application becomes active again, this will handle the case
/// where the audio route might have changed when the app was running in the background
- (void)didBecomeActive:(NSNotification *)notification {
	[self updateInfoLabels];
}

@end
