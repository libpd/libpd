/*
 *  ImmrHIDUtilAddOn.h
 *  UseFFAPIFromHIDUtilities
 *
 *  Created by rlacroix on Wed Oct 16 2002.
 *  Copyright (c) 2002 Immersion Corporation. All rights reserved.
 *
 */

//extern io_object_t AllocateHIDObjectFromRecDevice( pRecDevice pDevice );
//extern Boolean FreeHIDObject( io_object_t hidDevice );
io_service_t AllocateHIDObjectFromRecDevice( pRecDevice pDevice );
bool FreeHIDObject( io_service_t hidDevice );
