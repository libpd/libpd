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
//  Updated 2013, 2018 Dan Wilcox <danomatika@gmail.com>
//

#import <Foundation/Foundation.h>

@interface PdFile : NSObject

/// Underlying t_pd pointer
@property (nonatomic, strong, readonly) NSValue *fileReference;

/// Unique $0 argument assigned by pd
@property (nonatomic, assign, readonly) int dollarZero;

/// Stored file base name
@property (nonatomic, copy, readonly) NSString *baseName;

/// Stored file path name
@property (nonatomic, copy, readonly) NSString *pathName;

/// Open a pd file/patch and return a representative PdFile object
+ (id)openFileNamed:(NSString *)baseName path:(NSString *)pathName;

/// Open a pd file/patch, returns YES on success
- (BOOL)openFile:(NSString *)baseName path:(NSString *)pathName;

/// Open a new instance of an existing PdFile
- (id)openNewInstance;

/// Is the file reference valid? (aka non-nil)
- (bool)isValid;

/// Close an opened pd file (also called in dealloc)
- (void)closeFile;

@end
