//
//  AppDelegate.m
//  pdtest
//
//  Created by Dan Wilcox on 1/16/13.
//  Copyright (c) 2013 libpd. All rights reserved.
//

#import "AppDelegate.h"

#import "ViewController.h"

#import "PdAudioController.h"
#import "PdFile.h"

@interface AppDelegate () {}

@property (nonatomic, retain) PdAudioController *audioController;
@property (nonatomic, retain) PdFile *patch;

- (void)setupPd;
- (void)testPd;

@end

@implementation AppDelegate

@synthesize window = window_;
@synthesize viewController = viewController_;
@synthesize audioController = audioController_;

#pragma mark - Application lifecycle

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
	self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
	// Override point for customization after application launch.
	self.viewController = [[ViewController alloc] initWithNibName:@"ViewController" bundle:nil];
	self.window.rootViewController = self.viewController;
	
	// simple check to see which mode the app is running in, useful when trying to debug 64 bit issues
	if (sizeof(void*) == 4) {
		NSLog(@"32-bit App");
	} else if (sizeof(void*) == 8) {
		NSLog(@"64-bit App");
	}
	
	[self setupPd];
	[self testPd];
	
	[self.window makeKeyAndVisible];
	
	return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application {
	// Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
	// Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
	// Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later. 
	// If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
	// Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
	// Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}

- (void)applicationWillTerminate:(UIApplication *)application {
	// Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
	
	UITouch *touch = [touches anyObject];
	CGPoint pos = [touch locationInView:self.viewController.view];
	int pitch = (-1 * (pos.y/CGRectGetHeight(self.viewController.view.frame)) + 1) * 127;
	
	[PdBase sendList:[NSArray arrayWithObjects:@"pitch", [NSNumber numberWithInt:pitch], nil] toReceiver:@"tone"];
	[PdBase sendBangToReceiver:@"tone"];
	
	NSLog(@"touch at %.f %.f with pitch: %d", pos.x, pos.y, pitch);
}

#pragma mark - Pd

- (void)setupPd {
	// Configure a typical audio session with 2 output channels
	self.audioController = [[PdAudioController alloc] init];
	PdAudioStatus status = [self.audioController configurePlaybackWithSampleRate:44100
																  numberChannels:2
																	inputEnabled:YES
																   mixingEnabled:YES];
	if (status == PdAudioError) {
		NSLog(@"Error! Could not configure PdAudioController");
	} else if (status == PdAudioPropertyChanged) {
		NSLog(@"Warning: some of the audio parameters were not accceptable.");
	} else {
		NSLog(@"Audio Configuration successful.");
	}

	// log actual settings
	[self.audioController print];

	// set AppDelegate as PdRecieverDelegate to receive messages from pd
	[PdBase setDelegate:self];
	[PdBase setMidiDelegate:self]; // for midi too
	
	// receive messages to fromPD: [r fromPD]
	[PdBase subscribe:@"fromPD"];
	
	// add search path
	[PdBase addToSearchPath:[NSString stringWithFormat:@"%@/pd/abs", [[NSBundle mainBundle] bundlePath]]];
	
	// turn on dsp
	self.audioController.active = YES;
	[PdBase computeAudio:YES];
}

- (void)testPd {

	int midiChan = 1; // midi channels are 1-16

	NSLog(@"-- BEGIN Patch Test");
	
	// open patch
	self.patch = [PdFile openFileNamed:@"test.pd" path:[NSString stringWithFormat:@"%@/pd", [[NSBundle mainBundle] bundlePath]]];
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
	NSArray *list = [[NSArray alloc] initWithObjects:
		[NSNumber numberWithFloat:1.23], @"a symbol", nil];
	[PdBase sendList:list toReceiver:@"toPd"];
	
	// send a list to the $0 receiver ie $0-toOF
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

#pragma mark - PdRecieverDelegate

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

@end
