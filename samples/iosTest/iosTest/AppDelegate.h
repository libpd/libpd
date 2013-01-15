//
//  AppDelegate.h
//  iosTest
//
//  Created by Dan Wilcox on 1/14/13.
//  Copyright (c) 2013 libpd. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "PdBase.h"

@interface AppDelegate : UIResponder <UIApplicationDelegate, PdReceiverDelegate>

@property (strong, nonatomic) UIWindow *window;

@end
