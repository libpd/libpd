//
//  PdPatch.h
//  GuitarTuner
//
//  Created by Daniel Cardona on 9/06/14.
//  Copyright (c) 2014 Coco. All rights reserved.
//
/*
 This class is entended to use with patches that use the $0 patch identifier in all of its receivers
 this way PdPatchArray class can decide to witch patch it will send a given message. You can have multiple differently named patches within the same array but you must be careful, the method send:toReceiver:inFilesNamed is designed for this purpose.
 
 
 */

#import <Foundation/Foundation.h>
#import "PdFile.h"
#import "PdDispatcher.h"



@interface PdPatchArray : PdFile 

@property (strong, nonatomic) PdFile *patchFile;

@property (strong,nonatomic) NSMutableArray *patches;
@property (strong,nonatomic) NSMutableArray *dollarZeros;
//@property (strong,nonatomic) NSMutableArray *patchNames;

/*--------- Makes instances or add patches to array ------ */
-(void)makeInstancesOfPatch:(int)n ;
-(void)addInstance;
-(void)addPatchNamed:(NSString*)patchName;

/*------------------ Messages to Pd patches --------------*/

//Sends float to a especific patch in array
-(void)sendFloat:(float)n toReceiver: (NSString*)receiverName atIndex: (NSUInteger)arrayIndex;
//Sends float to all patchfiles in array named the same
-(void)sendFloat:(float)n toReceiver:(NSString *)receiverName inFilesNamed:(NSString*)patchName;
//Sends float to all patches in array
-(void)sendFloat:(float)n toReceiver:(NSString*)receiverName;

-(void)sendBangToReceiver:(NSString*)receiverName;//send bang to all patch instances
-(void)sendBangToReceiver:(NSString*)receiverName atIndex:(NSUInteger)arrayIndex;
-(void)sendBangToReceiver:(NSString *)receiverName inFilesNamed: (NSString*)patchName;

-(void)sendMessage:(NSString*) message toReceiver: (NSString*)receiverName atIndex: (NSUInteger)arrayIndex;
-(void)sendMessage:(NSString*) message toReceiver:(NSString *)receiverName inFilesNamed:(NSString*)patchName;
-(void)sendMessage:(NSString*)message toReceiver:(NSString*)receiverName;

-(void)sendList:(NSArray *)list toReceiver:(NSString *)receiverName atIndex:(NSUInteger)arrayIndex;
-(void)sendList:(NSArray *)list toReceiver:(NSString *)receiverName inFilesNamed:(NSString*)patchName;
-(void)sendList:(NSArray *)list toReceiver:(NSString *)receiverName;


/* ------------------------ Utilities -------------------------*/
-(NSString *)patchNameAtIndex:(NSUInteger)arrayIndex; //returns $0-patchName


/* TODO: Make it possible for an Pd Array (aka tables) to be copied from certain patch index to another */


/* ------------------ Open close Files -------------------*/
-(id) initWithFileName:(NSString*)patchName;
-(void)closeFiles;




@end
