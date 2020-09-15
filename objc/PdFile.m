//
//  PdFile.m
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

#import "PdFile.h"
#import "PdBase.h"

@interface PdFile ()
@property (nonatomic, strong, readwrite) NSValue *fileReference;
@property (nonatomic, assign, readwrite) int dollarZero;
@property (nonatomic, copy, readwrite) NSString *baseName;
@property (nonatomic, copy, readwrite) NSString *pathName;
@end

@implementation PdFile

#pragma mark Class Methods

+ (id)openFileNamed:(NSString *)baseName path:(NSString *)pathName {
	PdFile *pdFile = [[self alloc] init];
	if (pdFile) {
		[pdFile openFile:baseName path:pathName];
		if (!pdFile.fileReference) {
			return nil;
		}
	}
	return pdFile;
}

#pragma mark Instance Methods

- (void)dealloc {
	[self closeFile];
	self.pathName = nil;
	self.baseName = nil;
	self.fileReference = nil;
}

- (BOOL)openFile:(NSString *)baseName path:(NSString *)pathName {
	self.baseName = baseName;
	self.pathName = pathName;
	void *x = [PdBase openFile:baseName path:pathName];
	if (x) {
		self.fileReference = [NSValue valueWithPointer:x];
		self.dollarZero = [PdBase dollarZeroForFile:x];
		return YES;
	}
	return NO;
}

- (PdFile *)openNewInstance {
	return [PdFile openFileNamed:self.baseName path:self.pathName];
}

- (bool)isValid {
	return (bool) self.fileReference;
}

- (void)closeFile {
	void *x = (self.fileReference).pointerValue;
	if (x) {
		[PdBase closeFile:x];
		self.fileReference = nil;
	}
}

#pragma mark Util

- (NSString *)description {
	return [NSString stringWithFormat: @"Patch: \"%@\" $0: %d valid: %d",
	self.baseName, self.dollarZero, [self isValid]];
}

@end
