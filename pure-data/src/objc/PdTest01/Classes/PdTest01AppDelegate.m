//
//  PdTest01AppDelegate.m
//  PdTest01
//
//  Created by Richard Lawler on 10/3/10.
/**
 * This software is copyrighted by Richard Lawler. 
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

#import "PdTest01AppDelegate.h"
#import "PdTest01ViewController.h"
@interface PdTest01AppDelegate()

- (void) openAndRunTestPatch;
- (void) copyDemoPatchesToUserDomain;


@end


@implementation PdTest01AppDelegate

@synthesize window;
@synthesize viewController;

NSString *patchFileTypeExtension = @"pd";  


#pragma mark -
#pragma mark Application lifecycle

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {    
    
	pdAudio = [[PdAudio alloc] initWithSampleRate:44100.0 andTicksPerBuffer:64 andNumberOfInputChannels:2 andNumberOfOutputChannels:2];
	
	[self copyDemoPatchesToUserDomain];  // move the bundled patches to the documents dir
	[self openAndRunTestPatch]; 
	
    [window addSubview:viewController.view];
    [window makeKeyAndVisible];

	return YES;
}

- (void) openAndRunTestPatch
{
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *documentsDirectory = [paths objectAtIndex:0];
	
	NSString *documentsPatchFilePath = [documentsDirectory stringByAppendingPathComponent:@"test.pd"];
	
	[PdBase openPatch: documentsPatchFilePath];
	[PdBase computeAudio:YES];
	[pdAudio play];	
}

- (void) copyDemoPatchesToUserDomain
{
	NSFileManager *fm = [NSFileManager defaultManager];
	NSError *fileError;
	
	NSBundle *mainBundle = [NSBundle mainBundle];
	NSString *bundlePath = [mainBundle bundlePath];
	
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *documentsDirectory = [paths objectAtIndex:0];
	
	NSArray *bundleFiles = [fm contentsOfDirectoryAtPath:bundlePath error:&fileError];
	
	for( NSString *patchFile in bundleFiles )
	{
		if ([[patchFile pathExtension] isEqualToString:patchFileTypeExtension ])
		{
			NSString *bundlePatchFilePath = [bundlePath stringByAppendingPathComponent:patchFile]; 
			NSString *documentsPatchFilePath = [documentsDirectory stringByAppendingPathComponent:patchFile];
			
			if ([fm fileExistsAtPath:bundlePatchFilePath]) 
			{
				if( ![fm fileExistsAtPath:documentsPatchFilePath] )
					if( ![fm copyItemAtPath:bundlePatchFilePath toPath: documentsPatchFilePath error:&fileError] )
						NSLog(@"Error copying demo patch:%@", [fileError localizedDescription]);
			} 
		}
	}
}


- (void)applicationWillResignActive:(UIApplication *)application {
    /*
     Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
     Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
     */
}


- (void)applicationDidBecomeActive:(UIApplication *)application {
    /*
     Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
     */
}


- (void)applicationWillTerminate:(UIApplication *)application {
    /*
     Called when the application is about to terminate.
     See also applicationDidEnterBackground:.
     */
}


#pragma mark -
#pragma mark Memory management

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application {
    /*
     Free up as much memory as possible by purging cached data objects that can be recreated (or reloaded from disk) later.
     */
}


- (void)dealloc {
    [viewController release];
    [window release];
    [super dealloc];
}


@end
