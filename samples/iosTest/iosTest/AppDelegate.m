//
//  AppDelegate.m
//  iosTest
//
//  Created by Dan Wilcox on 1/14/13.
//  Copyright (c) 2013 lib[PdBase  All rights reserved.
//

#import "AppDelegate.h"

#import "PdAudioController.h"
#import "PdFile.h"

@interface AppDelegate () {}

@property (nonatomic, retain) PdAudioController *audioController;

- (void)setupPd;
- (void)testPd;

@end

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    // Override point for customization after application launch.
    self.window.backgroundColor = [UIColor whiteColor];
    [self.window makeKeyAndVisible];
	
	[self setupPd];
	
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
	// be nice and clean up on exit
	[PdBase computeAudio:NO];}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
	[self testPd];
}

- (void)setupPd {
	// Configure a typical audio session with 2 output channels
	self.audioController = [[PdAudioController alloc] init];
	PdAudioStatus status = [self.audioController configurePlaybackWithSampleRate:44100
																  numberChannels:2
																	inputEnabled:NO
																   mixingEnabled:YES];
	if (status == PdAudioError) {
		NSLog(@"Error: Could not configure PdAudioController");
	} else if (status == PdAudioPropertyChanged) {
		NSLog(@"Warning: Some of the audio parameters were not accceptable");
	} else {
		NSLog(@"Audio Configuration successful");
	}
	
	// log settings
	[self.audioController print];

	// set AppDelegate as PdRecieverDelegate to recieve messages from pd
    [PdBase setDelegate:self];

	// recieve messages to fromPD: [r fromPD]
	[PdBase subscribe:@"fromPD"];
	
	// add search path
	[PdBase addToSearchPath:@"pd"];//[NSString stringWithFormat:@"%@/pd", [[NSBundle mainBundle]]]];
	
	// turn on dsp
	[PdBase computeAudio:true];
}

- (void)testPd {

	int midiChan = 1; // midi channels are 0-15

	NSLog(@"BEGIN Patch Test");
	
	// open patch
	PdFile *patch = [PdFile openFileNamed:@"test.pd" path:[[NSBundle mainBundle] bundlePath]];
	NSLog(@"%@", patch);
	
	// close patch
	[patch closeFile];
	NSLog(@"%@", patch);
	
	// open patch again
	patch = [PdFile openFileNamed:patch];
	NSLog(@"%@", patch);
	
	NSLog(@"FINISH Patch Test");
	
	
	NSLog(@"BEGIN Message Test");
	
	// test basic atoms
	[PdBase sendBangToReceiver:@"toPd"];
	[PdBase sendFloat:100 toReceiver:@"toPD"];
	[PdBase sendSymbol:@"test string" toReceiver:@"toPD" ];
	
	// send a list
	NSArray *list = [[NSArray alloc] initWithObjects:
		[NSNumber numberWithFloat:1.23], @"a symbol", nil];
	[PdBase sendList:list toReceiver:@"toPd"];
	
	// send a list to the $0 receiver ie $0-toOF
	[PdBase sendList:list toReceiver:[NSString stringWithFormat:@"%d-toPd", patch.dollarZero]];
	
    // send a message
	[PdBase sendMessage:@"msg" withArguments:list toReceiver:@"toPD"];
    

	NSLog(@"FINISH Message Test");
	
	
	NSLog(@"BEGIN MIDI Test");
	
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
    
	NSLog(@"FINISH MIDI Test");
	
	
	NSLog(@"BEGIN Array Test");
	
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

	NSLog(@"FINISH Array Test");

	
	//NSLog(@"BEGIN PD Test");
	//[PdBase sendSymbol:@"test" toReceiver:@"toPD"];
	//NSLog(@"FINISH PD Test");
	
	
	[patch closeFile];
}

#pragma mark - PdRecieverDelegate

// uncomment this to get print statements from pd
- (void)receivePrint:(NSString *)message {
	NSLog(@"Pd Console: %@", message);
}

- (void)receiveBangFromSource:(NSString *)source {
	NSLog(@"Pd Bang from %@", source);
}

- (void)receiveFloat:(float)received fromSource:(NSString *)source {
	NSLog(@"Pd Float from %@: %f", source, received);
}

- (void)receiveSymbol:(NSString *)symbol fromSource:(NSString *)source {
	NSLog(@"Pd Symbol from %@: %@", source, symbol);
}

- (void)receiveList:(NSArray *)list fromSource:(NSString *)source {
	NSLog(@"Pd List from %@", source);
}

- (void)receiveMessage:(NSString *)message withArguments:(NSArray *)arguments fromSource:(NSString *)source {
	NSLog(@"Pd Message to %@ from %@", message, source);
}

@end
