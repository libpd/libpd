//
//  PdArray.h
//  libpd
//
//  Created by Rich E on 16/05/11.
//  Copyright 2011 Richard T. Eakin. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface PdArray : NSObject {
    int size_;
    NSString *name_;
    float *array_;
}

@property (nonatomic, assign, readonly) int size;
@property (nonatomic, copy, readonly) NSString *name;

- (void)readArrayNamed:(NSString *)arrayName;       // read a pd array given a name, locally storing the array
- (void)update;                                     // re-read the pd array that was allocated with readArrayNamed:arrayName
- (float)floatAtIndex:(int)index;                   // retrieve a single float value from the array
- (void)setFloat:(float)value atIndex:(int)index;   // set a single float value from the array

@end
