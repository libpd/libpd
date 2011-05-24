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

@property (nonatomic, assign) int length;
@property (nonatomic, assign) float *array;
@property (nonatomic, copy) NSString *name;

@end

@implementation PdArray

@synthesize length = size_;
@synthesize array = array_;
@synthesize name = name_;

#pragma mark -
#pragma mark - Init / Dealloc

- (void)dealloc {
  free(self.array);
  self.array = nil;
  self.name = nil;
  [super dealloc];
}

#pragma mark -
#pragma mark Public

- (void)readArrayNamed:(NSString *)arrayName {
  self.length = [PdBase arraySizeForArrayNamed:arrayName];
  if (self.length <= 0) {
    return;
  }
  if (self.array) {
    free(self.array);
  }
  self.array = calloc(self.length, sizeof(float));
  [PdBase readArrayNamed:arrayName distination:self.array offset:0 size:self.length];
  self.name = arrayName;
}

- (float)floatAtIndex:(int)index {
  if (self.array && index < [self length]) {
    return self.array[index];
  } else {
    return 0; // in the spirit of pd's tabread
  }
}

- (void)setFloat:(float)value atIndex:(int)index {
  float *array = self.array;
  if (array && index < [self length]) {
    array[index] = value;
    [PdBase writeArrayNamed:self.name source:(array+index) offset:index size:1];
  }
}

@end
