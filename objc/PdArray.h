//
//  PdArray.h
//  libpd
//
//  Created by Rich E on 16/05/11.
//  Copyright 2011 Richard T. Eakin. All rights reserved.
//
//  For information on usage and redistribution, and for a DISCLAIMER OF ALL
//  WARRANTIES, see the file, "LICENSE.txt," in this distribution.
//

#import <Foundation/Foundation.h>

@interface PdArray : NSObject {
    float *array_;
    NSString *name_;
    int size_;
}

@property (nonatomic, copy, readonly) NSString *name; // the name of the array in pd
@property (nonatomic, assign, readonly) int size;     // size of the pd array

// read the entire contents of a pd array given a name, locally storing the array. Sets size = maximum length and offset = 0
- (void)readArrayNamed:(NSString *)arrayName;

// (re)read the pd array with parameters provided by ivars name, size and offset
- (void)read;

// write to the pd array with parameters provided by ivars name, size and offset
- (void)write;

// retrieve a float from the local array at the given index
- (float)floatAtIndex:(int)index;

// set a single float value in both the local array and pd's array
- (void)setFloat:(float)value atIndex:(int)index;

// set a single float value only in the local array. returns NO if it could not set because of bad parameters
- (BOOL)setLocalFloat:(float)value atIndex:(int)index;

@end
