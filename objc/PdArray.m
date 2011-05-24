//
//  PdArray.m
//  libpd
//
//  Created by Rich E on 16/05/11.
//  Copyright 2011 Richard T. Eakin. All rights reserved.
//
//  For information on usage and redistribution, and for a DISCLAIMER OF ALL
//  WARRANTIES, see the file, "LICENSE.txt," in this distribution.
//

#import "PdArray.h"
#import "PdBase.h"

@interface PdArray ()

@property (nonatomic, assign) int size;
@property (nonatomic, assign) float *array;
@property (nonatomic, copy) NSString *name;

@end

@implementation PdArray

@synthesize size = size_;
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
  self.size = [PdBase arraySizeForArrayNamed:arrayName];
  if (self.size <= 0) {
    return;
  }
  if (self.array) {
    free(self.array);
  }
  self.array = calloc(self.size, sizeof(float));
  [PdBase readArrayNamed:arrayName distination:self.array offset:0 size:self.size];
  self.name = arrayName;
}

- (void)update {
  if (self.array) {
    [PdBase readArrayNamed:self.name distination:self.array offset:0 size:self.size];
  }
}

- (float)floatAtIndex:(int)index {
  if (self.array && index > 0 && index < self.size) {
    return self.array[index];
  } else {
    return 0; // in the spirit of pd's tabread
  }
}

- (void)setFloat:(float)value atIndex:(int)index {
  if (self.array && index > 0 && index < self.size) {
    self.array[index] = value;
    [PdBase writeArrayNamed:self.name source:(self.array+index) offset:index size:1];
  }
}

@end
