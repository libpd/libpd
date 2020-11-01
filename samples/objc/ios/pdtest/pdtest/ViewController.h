//
//  ViewController.h
//  pdtest
//
//  Created by Dan Wilcox on 1/16/13.
//  Copyright (c) 2013, 2020 libpd team. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "PdBase.h"

@interface ViewController : UIViewController <PdReceiverDelegate, PdMidiReceiverDelegate>

/// pd audio unit info label
@property (weak, nonatomic) IBOutlet UILabel *pdLabel;

/// input info label
@property (weak, nonatomic) IBOutlet UILabel *inputLabel;

/// output info label
@property (weak, nonatomic) IBOutlet UILabel *outputLabel;

/// route picker view container
@property (weak, nonatomic) IBOutlet UIView *routePickerContainer;

/// setup the Pd audio controller, etc
- (void)setupPd;

/// run some Pd tests
- (void)testPd;

/// update audio session info labels
- (void)updateInfoLabels;

@end
