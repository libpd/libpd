//
//  PdFile.h
//  libpd
//
//  Created by Richard Eakin on 21/02/11.
//
//  Copyright (c) 2011 Richard Eakin (reakinator@gmail.com)
//
//  For information on usage and redistribution, and for a DISCLAIMER OF ALL
//  WARRANTIES, see the file, "LICENSE.txt," in this distribution.
//
//  Updated 2013, 2018, 2020 Dan Wilcox <danomatika@gmail.com>
//

#import <Foundation/Foundation.h>

/// Pd patch file pointer wrapper
@interface PdFile : NSObject

/// underlying opaque patch handle pointer
@property (nonatomic, strong, readonly) NSValue *fileReference;

/// unique $0 id of the patch assigned by pd
@property (nonatomic, assign, readonly) int dollarZero;

/// stored patch filename
@property (nonatomic, copy, readonly) NSString *baseName;

/// stored parent dir path for the file
@property (nonatomic, copy, readonly) NSString *pathName;

/// open a pd file/patch by filename and parent dir path
/// returns representative PdFile on success or nil on failure
+ (instancetype)openFileNamed:(NSString *)baseName path:(NSString *)pathName;

/// open a pd file/patch by filename and parent dir path,
/// returns YES on success or NO on failure
- (BOOL)openFile:(NSString *)baseName path:(NSString *)pathName;

/// open a new instance of an existing PdFile on success or nil on failure
- (instancetype)openNewInstance;

/// is the file reference valid, aka non-nil?
- (bool)isValid;

/// close an opened pd file, also called in dealloc
- (void)closeFile;

@end
