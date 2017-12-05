//
//  PdBuffer.h
//  libpd
//
//  Created by Satoshi Muraki on 2017/12/05.
//
//  Copyright (c) 2017 Satoshi Muraki
//
//  For information on usage and redistribution, and for a DISCLAIMER OF ALL
//  WARRANTIES, see the file, "LICENSE.txt," in this distribution.
//

#import <Foundation/Foundation.h>

@interface PdBuffer : NSObject

@property (nonatomic, readonly) NSInteger capacity;

- (instancetype)initWithCapacity:(NSInteger)capacity;

- (NSInteger)readable;
- (void)readFrom:(const Float32 *)bytes count:(NSInteger)count;

- (NSInteger)writable;
- (void)writeTo:(Float32 *)bytes count:(NSInteger)count;

- (void)setBytes:(NSInteger (^)(Float32 *ptr))handler;

@end
