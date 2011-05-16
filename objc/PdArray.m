//
//  PdArray.m
//  libpd
//
//  Created by Rich E on 16/05/11.
//  Copyright 2011 Richard T. Eakin. All rights reserved.
//

#import "PdArray.h"
#import "PdBase.h"

/* The methods I want to call are:
 
 + (int)arraySizeForArrayNamed:(NSString *)arrayName;
 + (int)readArrayNamed:(NSString *)arrayName distination:(float *)destinationArray offset:(int)offset size:(int)n;
 + (int)writeArrayNamed:(NSString *)arrayName source:(float *)sourceArray offset:(int)offset size:(int)n;
 
 */

@interface PdArray ()

@property (nonatomic, assign) float *array;
@property (nonatomic, copy) NSString *name;

@end

@implementation PdArray

@synthesize array = array_;
@synthesize name = name_;

#pragma mark -
#pragma mark - Init / Dealloc

- (void)dealloc {
  // ???: array?
  self.name = nil;
  [super dealloc];
}

#pragma mark -
#pragma mark Public

- (int)size {
  if (self.name && self.array) {
    return [PdBase arraySizeForArrayNamed:self.name];
  } else {
    return 0;
  }
}

- (void)readArrayNamed:(NSString *)arrayName {
  int arraySize = [PdBase arraySizeForArrayNamed:arrayName];
  [PdBase readArrayNamed:arrayName distination:self.array offset:0 size:arraySize];
  NSLog(@"%s read array named: %@", __PRETTY_FUNCTION__, arrayName);
}

- (float)floatAtIndex:(int)index {
  if (self.array && index < [self size]) {
    return self.array[index];
  }
}

@end
