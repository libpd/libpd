/* --------------------------------------------------------------------------*/
/*                                                                           */
/* read the System Management Controller on Apple Mac OS X                   */
/* Written by Hans-Christoph Steiner <hans@eds.org>                          */
/*                                                                           */
/* Copyright (C) 2008-2009 Hans-Christoph Steiner                            */
/* Copyright (C) 2006 devnull                                                */
/*                                                                           */
/* This program is free software; you can redistribute it and/or             */
/* modify it under the terms of the GNU General Public License               */
/* as published by the Free Software Foundation; either version 3            */
/* of the License, or (at your option) any later version.                    */
/*                                                                           */
/* See file LICENSE for further informations on licensing terms.             */
/*                                                                           */
/* This program is distributed in the hope that it will be useful,           */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU General Public License for more details.                              */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with this program; if not, write to the Free Software Foundation,   */
/* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA */
/*                                                                           */
/* --------------------------------------------------------------------------*/

#include <mach/mach.h> 
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <IOKit/IOKitLib.h> 
#include <CoreServices/CoreServices.h>

#include "m_pd.h"
#include "smc.h"

#define DEBUG(x)
//#define DEBUG(x) x 

/*------------------------------------------------------------------------------
 *  CLASS DEF
 */

static t_class *smc_class;

typedef struct _smc {
    t_object            x_obj;
    t_symbol*           key;
    t_outlet*           data_outlet;
    t_outlet*           status_outlet;
} t_smc;

/* TODO make this in the t_smc struct */
static io_connect_t conn;


/*------------------------------------------------------------------------------
 * WEIRD SHIT TO GET RID OF (why not just use the standard functions?)
 */

UInt32 _strtoul(char *str, int size, int base)
{
    UInt32 total = 0;
    int i;

    for (i = 0; i < size; i++)
    {
        if (base == 16)
            total += str[i] << (size - 1 - i) * 8;
        else
            total += (unsigned char) (str[i] << (size - 1 - i) * 8);
    }
    return total;
}

void _ultostr(char *str, UInt32 val)
{
    str[0] = '\0';
    sprintf(str, "%c%c%c%c", 
            (unsigned int) val >> 24,
            (unsigned int) val >> 16,
            (unsigned int) val >> 8,
            (unsigned int) val);
}

float _strtof(char *str, int size, int e)
{
    float total = 0;
    int i;

    for (i = 0; i < size; i++)
    {
        if (i == (size - 1))
            total += (str[i] & 0xff) >> e;
        else
            total += str[i] << (size - 1 - i) * (8 - e);
    }

    return total;
}

/*------------------------------------------------------------------------------
 * SUPPORT FUNCTIONS
 */

/* this function expects two 5 byte arrays of chars, 4 chars and a null. if
 * the string is shorter than 4 chars, then pad it with spaces */
void padkeycpy(char* dst, char* src)
{
    int i;
    dst = "    ";
    strncpy(dst, src, 4);
    for(i = 0; i < 4; i++) 
        if( ! isprint(src[i])) dst[i] = ' ';
    dst[4] = '\0';
}

kern_return_t SMCOpen(void)
{
    kern_return_t result;
    mach_port_t   masterPort;
    io_iterator_t iterator;
    io_object_t   device;

    result = IOMasterPort(MACH_PORT_NULL, &masterPort);

    CFMutableDictionaryRef matchingDictionary = IOServiceMatching("AppleSMC");
    result = IOServiceGetMatchingServices(masterPort, matchingDictionary, &iterator);
    if (result != kIOReturnSuccess)
    {
        post("Error: IOServiceGetMatchingServices() = %08x\n", result);
        return 1;
    }

    device = IOIteratorNext(iterator);
    IOObjectRelease(iterator);
    if (device == 0)
    {
        post("Error: no SMC found\n");
        return 1;
    }

    result = IOServiceOpen(device, mach_task_self(), 0, &conn);
    IOObjectRelease(device);
    if (result != kIOReturnSuccess)
    {
        post("Error: IOServiceOpen() = %08x\n", result);
        return 1;
    }

    return kIOReturnSuccess;
}

kern_return_t SMCClose()
{
    return IOServiceClose(conn);
}


kern_return_t SMCCall(int index, SMCKeyData_t *inputStructure, SMCKeyData_t *outputStructure)
{
    IOItemCount   structureInputSize;
    IOByteCount   structureOutputSize;

    structureInputSize = sizeof(SMCKeyData_t);
    structureOutputSize = sizeof(SMCKeyData_t);
#if !defined(__LP64__)
	// Check if Mac OS X 10.5/10.6 API is available...
    SInt32 MacVersion;
    if ((Gestalt(gestaltSystemVersion, &MacVersion) == noErr) && (MacVersion >= 0x1060)) {
		// ...and use it if it is.
#endif
#ifdef AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER
		return IOConnectCallStructMethod(
            conn,               // an io_connect_t returned from IOServiceOpen().
            index,	            // selector of the function to be called via the user client.
            inputStructure,	    // pointer to the input struct parameter.
            structureInputSize, // the size of the input structure parameter.
            outputStructure,    // pointer to the output struct parameter.
            &structureOutputSize// pointer to the size of the output structure parameter.
            );
#endif
#if !defined(__LP64__)
	}
	else {
		// Otherwise fall back to older API.
        return IOConnectMethodStructureIStructureO(
            conn,                 // an io_connect_t returned from IOServiceOpen().
            index,                // an index to the function to be called via the user client.
            structureInputSize,   // the size of the input struct paramter.
            &structureOutputSize, // a pointer to the size of the output struct paramter.
            inputStructure,       // a pointer to the input struct parameter.
            outputStructure       // a pointer to the output struct parameter.
            );
	}
#endif
}

kern_return_t SMCReadKey(UInt32Char_t key, SMCVal_t *val)
{
    kern_return_t result;
    SMCKeyData_t  inputStructure;
    SMCKeyData_t  outputStructure;

    memset(&inputStructure, 0, sizeof(SMCKeyData_t));
    memset(&outputStructure, 0, sizeof(SMCKeyData_t));
    memset(val, 0, sizeof(SMCVal_t));

    inputStructure.key = _strtoul(key, 4, 16);
    sprintf(val->key, key);
    inputStructure.data8 = SMC_CMD_READ_KEYINFO;    

    result = SMCCall(KERNEL_INDEX_SMC, &inputStructure, &outputStructure);
    if (result != kIOReturnSuccess)
        return result;

    val->dataSize = outputStructure.keyInfo.dataSize;
    _ultostr(val->dataType, outputStructure.keyInfo.dataType);
    inputStructure.keyInfo.dataSize = val->dataSize;
    inputStructure.data8 = SMC_CMD_READ_BYTES;

    result = SMCCall(KERNEL_INDEX_SMC, &inputStructure, &outputStructure);
    if (result != kIOReturnSuccess)
        return result;

    memcpy(val->bytes, outputStructure.bytes, sizeof(outputStructure.bytes));

    return kIOReturnSuccess;
}

kern_return_t SMCWriteKey(SMCVal_t writeVal)
{
    kern_return_t result;
    SMCKeyData_t  inputStructure;
    SMCKeyData_t  outputStructure;

    SMCVal_t      readVal;

    result = SMCReadKey(writeVal.key, &readVal);
    if (result != kIOReturnSuccess) 
        return result;

    if (readVal.dataSize != writeVal.dataSize)
        return kIOReturnError;

    memset(&inputStructure, 0, sizeof(SMCKeyData_t));
    memset(&outputStructure, 0, sizeof(SMCKeyData_t));

    inputStructure.key = _strtoul(writeVal.key, 4, 16);
    inputStructure.data8 = SMC_CMD_WRITE_BYTES;    
    inputStructure.keyInfo.dataSize = writeVal.dataSize;
    memcpy(inputStructure.bytes, writeVal.bytes, sizeof(writeVal.bytes));

    result = SMCCall(KERNEL_INDEX_SMC, &inputStructure, &outputStructure);
    if (result != kIOReturnSuccess)
        return result;
 
    return kIOReturnSuccess;
}

UInt32 SMCReadIndexCount(void)
{
    SMCVal_t val;

    SMCReadKey("#KEY", &val);
    return _strtoul(val.bytes, val.dataSize, 10);
}

/*------------------------------------------------------------------------------
 * IMPLEMENTATION                    
 */

static void smc_symbol(t_smc* x, t_symbol* key)
{
	DEBUG(post("smc_symbol"););
    kern_return_t result;
    SMCVal_t      val;
    t_atom output_atom;

    SMCOpen();
    result = SMCReadKey(key->s_name, &val);
    if (result == kIOReturnSuccess) {
        x->key = key;
        if (val.dataSize > 0) {
            if (strcmp(val.dataType, DATATYPE_SP78) == 0) {
                int intValue = (val.bytes[0] * 256 + val.bytes[1]) >> 2;
                SETFLOAT(&output_atom, intValue / 64.0);
                outlet_anything(x->data_outlet, key, 1, &output_atom);
            } else if ((strcmp(val.dataType, DATATYPE_UINT8) == 0) ||
                       (strcmp(val.dataType, DATATYPE_UINT16) == 0) ||
                       (strcmp(val.dataType, DATATYPE_UINT32) == 0)) {
                SETFLOAT(&output_atom, _strtoul(val.bytes, val.dataSize, 10));
                outlet_anything(x->data_outlet, gensym(val.key), 1, &output_atom);
            } else if (strcmp(val.dataType, DATATYPE_FPE2) == 0) {
                SETFLOAT(&output_atom, _strtof(val.bytes, val.dataSize, 2));
                outlet_anything(x->data_outlet, gensym(val.key), 1, &output_atom);
            } else {
                UInt32 i;
                t_atom output_list[val.dataSize];
                for (i = 0; i < val.dataSize; i++)
                    SETFLOAT(output_list + i, (unsigned char) val.bytes[i]);
                outlet_anything(x->data_outlet, gensym(val.key), 
                                val.dataSize, output_list);
            }
        }
        
    }
    SMCClose();
}

static void smc_bang(t_smc* x)
{
    smc_symbol(x, x->key);
}

static void smc_keys(t_smc* x)
{
	DEBUG(post("smc_keys"););
    kern_return_t result;
    SMCKeyData_t  inputStructure;
    SMCKeyData_t  outputStructure;

    int           totalKeys, i;
    UInt32Char_t  key;
    SMCVal_t      val;

    SMCOpen();
    totalKeys = SMCReadIndexCount();
    t_atom output_list[totalKeys];
    for (i = 0; i < totalKeys; i++)
    {
        memset(&inputStructure, 0, sizeof(SMCKeyData_t));
        memset(&outputStructure, 0, sizeof(SMCKeyData_t));
        memset(&val, 0, sizeof(SMCVal_t));

        inputStructure.data8 = SMC_CMD_READ_INDEX;
        inputStructure.data32 = i;

        result = SMCCall(KERNEL_INDEX_SMC, &inputStructure, &outputStructure);
        if (result != kIOReturnSuccess)
            continue;

        _ultostr(key, outputStructure.key);
        SETSYMBOL(output_list + i, gensym(key));
    }
    SMCClose();
    outlet_anything(x->status_outlet, gensym("keys"), totalKeys, output_list);
}

static void smc_info(t_smc* x)
{
    t_atom output_atom;
    SETSYMBOL(&output_atom, x->key);
    outlet_anything(x->status_outlet, gensym("key"), 1, &output_atom);
    smc_keys(x);
}

static void smc_anything(t_smc* x, t_symbol *s, int argc, t_atom *argv)
{
	DEBUG(post("smc_anything %d", argc););
    if(argc == 0) {
        x->key = s;
        if(x->key != &s_) smc_bang(x);
    } else if(argc == 1) {
        /* if there is one symbol and one float, then look up the proper data
         * type then convert the float to the proper sequence of bytes */
        SMCVal_t val;
        char value = (char) atom_getfloatarg(0, argc, argv);
        
        x->key = s;
        padkeycpy(val.key, x->key->s_name);
        val.bytes[0] = (value << 2) / 256;
        val.bytes[1] = (value << 2) % 256;
        val.dataSize = 2;
        SMCOpen();
        SMCWriteKey(val);
        SMCClose();
        smc_symbol(x, x->key);
    } else {
        /* if there is a symbol and a list of bytes, then send that list of
         * bytes directly to the key given by the symbol */
        int i;
        SMCVal_t val;
        x->key = s;
        padkeycpy(val.key, x->key->s_name);
        for(i = 0; i < argc && i < 32; i++) {
            val.bytes[i] = (char) atom_getfloatarg(i, argc, argv);
            post(" byte %d: %i", i, val.bytes[i]);
        }
        val.dataSize = argc;
        SMCOpen();
        SMCWriteKey(val);
        SMCClose();
        smc_symbol(x, x->key);
    }
}

static void *smc_new(t_symbol* s) 
{
	DEBUG(post("smc_new"););
	t_smc *x = (t_smc *)pd_new(smc_class);

    x->key = s;
    x->data_outlet = outlet_new(&x->x_obj, &s_anything);
	x->status_outlet = outlet_new(&x->x_obj, &s_anything);

	return (x);
}

void smc_setup(void) 
{
	smc_class = class_new(gensym("smc"), 
                          (t_newmethod)smc_new,
                          NULL,
                          sizeof(t_smc), 
                          CLASS_DEFAULT,
                          A_DEFSYMBOL,
                          0);
	/* add inlet datatype methods */
	class_addbang(smc_class,(t_method) smc_bang);
	class_addsymbol(smc_class,(t_method) smc_symbol);
	class_addmethod(smc_class,(t_method) smc_info, gensym("info"), 0);
	class_addmethod(smc_class,(t_method) smc_keys, gensym("keys"), 0);
	class_addanything(smc_class,(t_method) smc_anything);
}
