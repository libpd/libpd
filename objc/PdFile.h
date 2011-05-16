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
	NSValue *fileReference_; // pointer to the file
	int dollarZero_;		 // unique $0 argument assigned by pd
	NSString *baseName_;	 // stored file base name
	NSString *pathName_;	 // stored file path name
}

@property (nonatomic, assign, readonly) int dollarZero;
@property (nonatomic, copy, readonly) NSString *baseName;
@property (nonatomic, copy, readonly) NSString *pathName;

+ (id)openFileNamed:(NSString *)baseName path:(NSString *)pathName;	// convenience class method for opening a file and returning a reference to self
- (void)openFile:(NSString *)baseName path:(NSString *)pathName;	// open a file with base and path names
- (void)closeFile;													// close an opened file, also called in dealloc

@end
