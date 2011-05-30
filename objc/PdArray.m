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

@property (nonatomic, copy) NSString *name;
@property (nonatomic, assign) float *array;
@property (nonatomic, assign) int size;

@end

@implementation PdArray

@synthesize array = array_;
@synthesize name = name_;
@synthesize size = size_;

#pragma mark -
#pragma mark - Init / Dealloc

+ (id)arrayNamed:(NSString *)arrayName {
  PdArray *pdArray = [[[self alloc] init] autorelease];
  if (pdArray) {
    pdArray.size = [PdBase arraySizeForArrayNamed:arrayName];
    if (pdArray.size <= 0) {
      return nil;
    }
    pdArray.array = calloc(pdArray.size, sizeof(float));
    [PdBase readArrayNamed:arrayName distination:pdArray.array offset:0 size:pdArray.size];
    pdArray.name = arrayName;
  }
  return pdArray;
}

- (void)dealloc {
  free(self.array);
  self.array = nil;
  self.name = nil;
  [super dealloc];
}

#pragma mark -
#pragma mark Public

- (void)read {
  if (self.array) {
    [PdBase readArrayNamed:self.name distination:self.array offset:0 size:self.size];
  }
}

- (void)write {
  if (self.array) {
    [PdBase writeArrayNamed:self.name source:self.array offset:0 size:self.size];
  }
}

- (float)floatAtIndex:(int)index {
  if (self.array && index >= 0 && index < self.size) {
    return self.array[index];
  } else {
    return 0; // in the spirit of pd's tabread
  }
}

- (void)setFloat:(float)value atIndex:(int)index {
  if ([self setLocalFloat:value atIndex:index]) {
    [PdBase writeArrayNamed:self.name source:(self.array+index) offset:index size:1];
  }
}

- (BOOL)setLocalFloat:(float)value atIndex:(int)index {
  if (self.array && index >= 0 && index < self.size) {
    self.array[index] = value;
    return YES;
  }
  return NO;
}

@end
