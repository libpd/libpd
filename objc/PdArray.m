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
    pdArray.name = arrayName;
    [pdArray read];
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
		[PdBase copyArrayNamed:self.name withOffset:0 count:self.size toArray:self.array];
  }
}

- (void)write {
  if (self.array) {
		[PdBase copyArray:self.array toArrayNamed:self.name withOffset:0 count:self.size];
  }
}

- (float)floatAtIndex:(int)index {
  [self read]; // TODO: only grab the specific float and put it in the local array
  return [self localFloatAtIndex:index];
}

- (float)localFloatAtIndex:(int)index {
  if (self.array && index >= 0 && index < self.size) {
    return self.array[index];
  } else {
    return 0; // in the spirit of pd's tabread
  }
}

- (void)setFloat:(float)value atIndex:(int)index {
  if ([self setLocalFloat:value atIndex:index]) {
		[PdBase copyArray:(self.array+index) toArrayNamed:self.name withOffset:index count:1];
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
