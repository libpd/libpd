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

@property (nonatomic, assign, readonly) int length;

- (void)readArrayNamed:(NSString *)arrayName;
- (float)floatAtIndex:(int)index;
- (void)setFloat:(float)value atIndex:(int)index;

@end
