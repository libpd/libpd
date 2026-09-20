//
//  AppDelegate.m
//  pdtest
//
//  Created by Dan Wilcox on 1/16/13.
//  Copyright (c) 2013, 2020 libpd team. All rights reserved.
//

#import "AppDelegate.h"

#import "ViewController.h"

@implementation AppDelegate

#pragma mark - Application lifecycle

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
	
	// simple check to see which mode the app is running in, useful when trying to debug 64 bit issues
	if (sizeof(void*) == 4) {
		NSLog(@"32-bit App");
	} else if (sizeof(void*) == 8) {
		NSLog(@"64-bit App");
	}
	
	return YES;
}

- (void)applicationWillTerminate:(UIApplication *)application {
	// Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}

@end
