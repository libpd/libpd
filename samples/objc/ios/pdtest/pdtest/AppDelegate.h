//
//  AppDelegate.h
//  pdtest
//
//  Created by Dan Wilcox on 1/16/13.
//  Copyright (c) 2013 libpd. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "PdBase.h"

@class ViewController;

@interface AppDelegate : UIResponder <UIApplicationDelegate, PdReceiverDelegate, PdMidiReceiverDelegate>

@property (strong, nonatomic) UIWindow *window;

@property (strong, nonatomic) ViewController *viewController;

@end
