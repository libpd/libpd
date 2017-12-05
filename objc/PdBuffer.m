//
//  PdBuffer.m
//  libpd
//
//  Created by Satoshi Muraki on 2017/12/05.
//
//  Copyright (c) 2017 Satoshi Muraki
//
//  For information on usage and redistribution, and for a DISCLAIMER OF ALL
//  WARRANTIES, see the file, "LICENSE.txt," in this distribution.
//

#import "PdBuffer.h"

@interface PdBuffer ()

@property (nonatomic, readonly) Float32 *pointer;
@property (nonatomic) NSInteger start;
@property (nonatomic) NSInteger end;

@end

@implementation PdBuffer

- (instancetype)initWithCapacity:(NSInteger)capacity {
    self = [super init];
    if (self != nil) {
        _capacity = capacity;
        _pointer = (Float32 *)calloc(sizeof(Float32), capacity);
        _start = 0;
        _end = 0;
    }
    return self;
}

- (void)dealloc {
    if (_pointer != NULL) {
        free(_pointer);
    }
}

- (NSInteger)readable {
    if (0 < _start && _end <= _start) {
        return _start - _end;
    } else {
        return (_capacity - _end) + _start;
    }
}

- (void)readFrom:(const Float32 *)bytes count:(NSInteger)count {
    assert(count <= self.readable);
    NSInteger remain = count;
    while (0 < remain) {
        NSInteger delta = MIN(remain, (0 < _start && _end <= _start) ? (_start - _end) : (_capacity - _end));
        size_t length = sizeof(Float32) * delta;
        memcpy((_pointer + _end), bytes, length);
        remain -= delta;
        _end += delta;
        if (_end == _capacity && 0 < _start) {
            _end = 0;
        }
    }
}

- (NSInteger)writable {
    if (0 < _start && _end <= _start) {
        return _end + (_capacity - _start);
    } else {
        return _end - _start;
    }
}

- (void)writeTo:(Float32 *)bytes count:(NSInteger)count {
    assert(count <= self.writable);
    NSInteger remain = count;
    while (0 < remain) {
        NSInteger delta = MIN(remain, (0 < _start && _end <= _start) ? (_capacity - _start) : (_end - _start));
        size_t length = sizeof(Float32) * delta;
        memcpy(bytes, (_pointer + _start), length);
        remain -= delta;
        _start += delta;
        if (_start == _end) {
            assert(remain == 0);
            _start = 0;
            _end = 0;
        } else if (_start == _capacity) {
            _start = 0;
        }
    }
}

- (void)setBytes:(NSInteger (^)(Float32 *ptr))handler {
    NSInteger count = handler(_pointer);
    assert(count <= _capacity);
    _start = 0;
    _end = count;
}

@end
