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

#import <Foundation/Foundation.h>

@interface PdFile : NSObject {
  NSValue *fileReference_;
  int dollarZero_;
  NSString *baseName_;
  NSString *pathName_;
}

@property (nonatomic, assign, readonly) int dollarZero;             // unique $0 argument assigned by pd
@property (nonatomic, copy, readonly) NSString *baseName;           // stored file base name
@property (nonatomic, copy, readonly) NSString *pathName;           // stored file path name

+ (id)openFileNamed:(NSString *)baseName path:(NSString *)pathName; // open a pd file/patch and return a representative PdFile object
- (id)openNewInstance;                      // open a new instance of an existing PdFile
- (bool)isValid;                            // is the file reference valid? (aka non-nil)
- (void)closeFile;                          // close an opened pd file (also called in dealloc)

@end
