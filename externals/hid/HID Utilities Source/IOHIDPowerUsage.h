/*
	File:		IOHIDPowerUsage.h

	Contains:   Definition of the HID power usage constants (NOTE: Moved into <IOKit/hid/IOHIDUsageTables.h>)
    
	DRI: George Warner

	Copyright:	Copyright © 2002 Apple Computer, Inc., All Rights Reserved

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __IOHIDPowerUsage__
#define __IOHIDPowerUsage__

#ifndef __MACTYPES__
//#include <MacTypes.h>
#endif

#ifndef __MACERRORS__
//#include <MacErrors.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

enum
{
	kHIDPage_PowerDevice = 0x84, 				/* Power Device Page */
	kHIDPage_BatterySystem = 0x85, 				/* Battery System Page */
	
	/* 	Usage Types:
		CA- Application Collection
		CL- logical Collection
		CP- Pyiscal Collection
		DF- Dynamic Flag
		DV- Dynamic Value
		SF- Static Flag
		SV- Static Value
		
		Usgae Implementations:
		I- Input
		O- Output
		F- Feature
	*/
	
	/* Power Device Page (0x84) */
	/* This section provides detailed descriptions of the usages employed by Power Devices. */	
	kHIDUsage_PD_Undefined = 0x00,				/* Power Device Undefined Usage */
	kHIDUsage_PD_iName = 0x01,					/* CL- Power Device Name Index */
	kHIDUsage_PD_PresentStatus = 0x02,			/* CL- Power Device Present Status */
	kHIDUsage_PD_ChangedStatus = 0x03,			/* CA- Power Device Changed Status */
	kHIDUsage_PD_UPS = 0x04,					/* CA- Uninterruptible Power Supply */
	kHIDUsage_PD_PowerSupply = 0x05,			/* CA- Power Supply */
	/* Reserved 0x06 - 0x0F */
	kHIDUsage_PD_BatterySystem = 0x10,			/* CP- Battery System power module */
	kHIDUsage_PD_BatterySystemID = 0x11,		/* SV IF- Battery System ID */
	kHIDUsage_PD_Battery = 0x12,				/* CP- Battery */
	kHIDUsage_PD_BatteryID = 0x13,				/* SV IF- Battery ID */
	kHIDUsage_PD_Charger = 0x14,				/* CP- Charger */
	kHIDUsage_PD_ChargerID = 0x15,				/* SV IF- Charger ID */
	kHIDUsage_PD_PowerConverter = 0x16,			/* CP- Power Converter power module */
	kHIDUsage_PD_PowerConverterID = 0x17,		/* SV IF- Power Converter ID */
	kHIDUsage_PD_OutletSystem = 0x18,			/* CP- Outlet System power module */
	kHIDUsage_PD_OutletSystemID = 0x19,			/* SV IF-Outlet System ID */
	kHIDUsage_PD_Input = 0x1A,					/* CP- Power Device Input */
	kHIDUsage_PD_InputID = 0x1B,				/* SV IF- Power Device Input ID */
	kHIDUsage_PD_Output = 0x1C,					/* CP- Power Device Output */
	kHIDUsage_PD_OutputID = 0x1D,				/* SV IF- Power Device Output ID */
	kHIDUsage_PD_Flow = 0x1E,					/* CP- Power Device Flow */
	kHIDUsage_PD_FlowID = 0x1F,					/* Item IF- Power Device Flow ID */
	kHIDUsage_PD_Outlet = 0x20,					/* CP- Power Device Outlet */
	kHIDUsage_PD_OutletID = 0x21,				/* SV IF- Power Device Outlet ID */
	kHIDUsage_PD_Gang = 0x22,					/* CL/CP- Power Device Gang */
	kHIDUsage_PD_GangID = 0x23,					/* SV IF- Power Device Gang ID */
	kHIDUsage_PD_PowerSummary = 0x24,			/* CL/CP- Power Device Power Summary */
	kHIDUsage_PD_PowerSummaryID = 0x25,			/* SV IF- Power Device Power Summary ID */
	/* Reserved 0x26 - 0x2F */
	kHIDUsage_PD_Voltage = 0x30,				/* DV IF- Power Device Voltage */
	kHIDUsage_PD_Current = 0x31,				/* DV IF- Power Device Current */
	kHIDUsage_PD_Frequency = 0x32,				/* DV IF- Power Device Frequency */
	kHIDUsage_PD_ApparentPower = 0x33,			/* DV IF- Power Device Apparent Power */
	kHIDUsage_PD_ActivePower = 0x34,			/* DV IF- Power Device RMS Power */
	kHIDUsage_PD_PercentLoad = 0x35,			/* DV IF- Power Device Percent Load */
	kHIDUsage_PD_Temperature = 0x36,			/* DV IF- Power Device Temperature */
	kHIDUsage_PD_Humidity = 0x37,				/* DV IF- Power Device Humidity */
	kHIDUsage_PD_BadCount = 0x38,				/* DV IF- Power Device Bad Condition Count */
	/* Reserved 0x39 - 0x3F */
	kHIDUsage_PD_ConfigVoltage = 0x40,			/* SV/DV F- Power Device Nominal Voltage */
	kHIDUsage_PD_ConfigCurrent = 0x41,			/* SV/DV F- Power Device Nominal Current */
	kHIDUsage_PD_ConfigFrequency = 0x42,		/* SV/DV F- Power Device Nominal Frequency */
	kHIDUsage_PD_ConfigApparentPower = 0x43,	/* SV/DV F- Power Device Nominal Apparent Power */
	kHIDUsage_PD_ConfigActivePower = 0x44,		/* SV/DV F- Power Device Nominal RMS Power */
	kHIDUsage_PD_ConfigPercentLoad = 0x45,		/* SV/DV F- Power Device Nominal Percent Load */
	kHIDUsage_PD_ConfigTemperature = 0x46,		/* SV/DV F- Power Device Nominal Temperature */
	kHIDUsage_PD_ConfigHumidity = 0x47,			/* SV/DV F- Power Device Nominal Humidity */
	/* Reserved 0x48 - 0x4F */
	kHIDUsage_PD_SwitchOnControl = 0x50,		/* DV F- Power Device Switch On Control */
	kHIDUsage_PD_SwitchOffControl = 0x51,		/* DV F- Power Device Switch Off Control */
	kHIDUsage_PD_ToggleControl = 0x52,			/* DV F- Power Device Toogle Sequence Control */
	kHIDUsage_PD_LowVoltageTransfer = 0x53,		/* DV F- Power Device Min Transfer Voltage */
	kHIDUsage_PD_HighVoltageTransfer = 0x54,	/* DV F- Power Device Max Transfer Voltage */
	kHIDUsage_PD_DelayBeforeReboot = 0x55,		/* DV F- Power Device Delay Before Reboot */
	kHIDUsage_PD_DelayBeforeStartup = 0x56,		/* DV F- Power Device Delay Before Startup */
	kHIDUsage_PD_DelayBeforeShutdown = 0x57,	/* DV F- Power Device Delay Before Shutdown */
	kHIDUsage_PD_Test = 0x58,					/* DV F- Power Device Test Request/Result */
	kHIDUsage_PD_ModuleReset = 0x59,			/* DV F- Power Device Reset Request/Result */
	kHIDUsage_PD_AudibleAlarmControl = 0x5A,	/* DV F- Power Device Audible Alarm Control */
	/* Reserved 0x5B - 0x5F */
	kHIDUsage_PD_Present = 0x60,				/* DV IOF- Power Device Present */
	kHIDUsage_PD_Good = 0x61,					/* DV IOF- Power Device Good */
	kHIDUsage_PD_InternalFailure = 0x62,		/* DV IOF- Power Device Internal Failure */
	kHIDUsage_PD_VoltageOutOfRange = 0x63,		/* DV IOF- Power Device Voltage Out Of Range */
	kHIDUsage_PD_FrequencyOutOfRange = 0x64,	/* DV IOF- Power Device Frequency Out Of Range */
	kHIDUsage_PD_Overload = 0x65,				/* DV IOF- Power Device Overload */
	kHIDUsage_PD_OverCharged = 0x66,			/* DV IOF- Power Device Over Charged */
	kHIDUsage_PD_OverTemperature = 0x67,		/* DV IOF- Power Device Over Temperature */
	kHIDUsage_PD_ShutdownRequested = 0x68,		/* DV IOF- Power Device Shutdown Requested */
	kHIDUsage_PD_ShutdownImminent = 0x69,		/* DV IOF- Power Device Shutdown Imminent */
	/* Reserved 0x6A */
	kHIDUsage_PD_SwitchOnOff = 0x6B,			/* DV IOF- Power Device On/Off Switch Status */
	kHIDUsage_PD_Switchable = 0x6C,				/* DV IOF- Power Device Switchable */
	kHIDUsage_PD_Used = 0x6D,					/* DV IOF- Power Device Used */
	kHIDUsage_PD_Boost = 0x6E,					/* DV IOF- Power Device Boosted */
	kHIDUsage_PD_Buck = 0x6F,					/* DV IOF- Power Device Bucked */
	kHIDUsage_PD_Initialized = 0x70,			/* DV IOF- Power Device Initialized */
	kHIDUsage_PD_Tested = 0x71,					/* DV IOF- Power Device Tested */
	kHIDUsage_PD_AwaitingPower = 0x72,			/* DV IOF- Power Device Awaiting Power */
	kHIDUsage_PD_CommunicationLost = 0x73,		/* DV IOF- Power Device Communication Lost */
	/* Reserved 0x74 - 0xFC */
	kHIDUsage_PD_iManufacturer = 0xFD,			/* SV F- Power Device Manufacturer String Index */
	kHIDUsage_PD_iProduct = 0xFE,				/* SV F- Power Device Product String Index */
	kHIDUsage_PD_iserialNumber = 0xFF,			/* SV F- Power Device Serial Number String Index */

	/* Battery System Page (x85) */
	/* This section provides detailed descriptions of the usages employed by Battery Systems. */	
	kHIDUsage_BS_Undefined = 0x00,				/* Battery System Undefined */
	kHIDUsage_BS_SMBBatteryMode = 0x01,			/* CL - SMB Mode */
	kHIDUsage_BS_SMBBatteryStatus = 0x02,		/* CL - SMB Status */
	kHIDUsage_BS_SMBAlarmWarning = 0x03,		/* CL - SMB Alarm Warning */
	kHIDUsage_BS_SMBChargerMode = 0x04,			/* CL - SMB Charger Mode */
	kHIDUsage_BS_SMBChargerStatus = 0x05,		/* CL - SMB Charger Status */
	kHIDUsage_BS_SMBChargerSpecInfo = 0x06,		/* CL - SMB Charger Extended Status */
	kHIDUsage_BS_SMBSelectorState = 0x07,		/* CL - SMB Selector State */
	kHIDUsage_BS_SMBSelectorPresets = 0x08,		/* CL - SMB Selector Presets */
	kHIDUsage_BS_SMBSelectorInfo = 0x09,		/* CL - SMB Selector Info */
	/* Reserved 0x0A - 0x0F */
	kHIDUsage_BS_OptionalMfgFunction1 = 0x10,	/* DV F - Battery System Optional SMB Mfg Function 1 */
	kHIDUsage_BS_OptionalMfgFunction2 = 0x11,	/* DV F - Battery System Optional SMB Mfg Function 2 */
	kHIDUsage_BS_OptionalMfgFunction3 = 0x12,	/* DV F - Battery System Optional SMB Mfg Function 3 */
	kHIDUsage_BS_OptionalMfgFunction4 = 0x13,	/* DV F - Battery System Optional SMB Mfg Function 4 */
	kHIDUsage_BS_OptionalMfgFunction5 = 0x14,	/* DV F - Battery System Optional SMB Mfg Function 5 */
	kHIDUsage_BS_ConnectionToSMBus = 0x15,		/* DF F - Battery System Connection To System Management Bus */
	kHIDUsage_BS_OutputConnection = 0x16,		/* DF F - Battery System Output Connection Status */
	kHIDUsage_BS_ChargerConnection = 0x17,		/* DF F - Battery System Charger Connection */
	kHIDUsage_BS_BatteryInsertion = 0x18,		/* DF F - Battery System Battery Insertion */
	kHIDUsage_BS_Usenext = 0x19,				/* DF F - Battery System Use Next */
	kHIDUsage_BS_OKToUse = 0x1A,				/* DF F - Battery System OK To Use */
	kHIDUsage_BS_BatterySupported = 0x1B,		/* DF F - Battery System Battery Supported */
	kHIDUsage_BS_SelectorRevision = 0x1C,		/* DF F - Battery System Selector Revision */
	kHIDUsage_BS_ChargingIndicator = 0x1D,		/* DF F - Battery System Charging Indicator */
	/* Reserved 0x1E - 0x27 */
	kHIDUsage_BS_ManufacturerAccess = 0x28,		/* DV F - Battery System Manufacturer Access */
	kHIDUsage_BS_RemainingCapacityLimit = 0x29,	/* DV F - Battery System Remaining Capacity Limit */
	kHIDUsage_BS_RemainingTimeLimit = 0x2A,		/* DV F - Battery System Remaining Time Limit */
	kHIDUsage_BS_AtRate = 0x2B,					/* DV F - Battery System At Rate... */
	kHIDUsage_BS_CapacityMode = 0x2C,			/* DV F - Battery System Capacity Mode */
	kHIDUsage_BS_BroadcastToCharger = 0x2D,		/* DV F - Battery System Broadcast To Charger */
	kHIDUsage_BS_PrimaryBattery = 0x2E,			/* DV F - Battery System Primary Battery */
	kHIDUsage_BS_ChargeController = 0x2F,		/* DV F - Battery System Charge Controller */
	/* Reserved 0x30 - 0x3F */
	kHIDUsage_BS_TerminateCharge = 0x40,		/* DF IOF - Battery System Terminate Charge */
	kHIDUsage_BS_TerminateDischarge = 0x41,		/* DF IOF - Battery System Terminate Discharge */
	kHIDUsage_BS_BelowRemainingCapacityLimit = 0x42, /* DF IOF - Battery System Below Remaining Capacity Limit */
	kHIDUsage_BS_RemainingTimeLimitExpired = 0x43, /* DF IOF - Battery System Remaining Time Limit Expired */
	kHIDUsage_BS_Charging = 0x44,				/* DF IOF - Battery System Charging */
	kHIDUsage_BS_Discharging = 0x45,			/* DV IOF - Battery System Discharging */
	kHIDUsage_BS_FullyCharged = 0x46,			/* DF IOF - Battery System Fully Charged */
	kHIDUsage_BS_FullyDischarged = 0x47,		/* DV IOF - Battery System Fully Discharged */
	kHIDUsage_BS_ConditioningFlag = 0x48,		/* DV IOF - Battery System Conditioning Flag */
	kHIDUsage_BS_AtRateOK = 0x49,				/* DV IOF - Battery System At Rate OK */
	kHIDUsage_BS_SMBErrorCode = 0x4A,			/* DF IOF - Battery System SMB Error Code */
	kHIDUsage_BS_NeedReplacement = 0x4B,		/* DF IOF - Battery System Need Replacement */
	/* Reserved 0x4C - 0x5F */
	kHIDUsage_BS_AtRateTimeToFull = 0x60,		/* DV IF - Battery System At Rate Time To Full */
	kHIDUsage_BS_AtRateTimeToEmpty = 0x61,		/* DV IF - Battery System At Rate Time To Empty */
	kHIDUsage_BS_AverageCurrent = 0x62,			/* DV IF - Battery System Average Current */
	kHIDUsage_BS_Maxerror = 0x63,				/* DV IF - Battery System Max Error */
	kHIDUsage_BS_RelativeStateOfCharge = 0x64,	/* DV IF - Battery System Relative State Of Charge */
	kHIDUsage_BS_AbsoluteStateOfCharge = 0x65,	/* DV IF - Battery System Absolute State Of Charge */
	kHIDUsage_BS_RemainingCapacity = 0x66,		/* DV IF - Battery System Remaining Capacity */
	kHIDUsage_BS_FullChargeCapacity = 0x67,		/* DV IF - Battery System Full Charge Capacity */
	kHIDUsage_BS_RunTimeToEmpty = 0x68,			/* DV IF - Battery System Run Time To Empty */
	kHIDUsage_BS_AverageTimeToEmpty = 0x69,		/* DV IF - Battery System Average Time To Empty */
	kHIDUsage_BS_AverageTimeToFull = 0x6A,		/* DV IF - Battery System Average Time To Full */
	kHIDUsage_BS_CycleCount = 0x6B,				/* DV IF - Battery System Cycle Count */
	/* Reserved 0x6C - 0x7F */
	kHIDUsage_BS_BattPackModelLevel = 0x80,		/* SV F - Battery System Batt Pack Model Level */
	kHIDUsage_BS_InternalChargeController = 0x81, /* SF F - Battery System Internal Charge Controller */
	kHIDUsage_BS_PrimaryBatterySupport = 0x82,	/* SF F - Battery System Primary Battery Support */
	kHIDUsage_BS_DesignCapacity = 0x83,			/* SV F - Battery System Design Capacity */
	kHIDUsage_BS_SpecificationInfo = 0x84,		/* SV F - Battery System Specification Info */
	kHIDUsage_BS_ManufacturerDate = 0x85,		/* SV F - Battery System Manufacturer Date */
	kHIDUsage_BS_SerialNumber = 0x86,			/* SV F - Battery System Serial Number */
	kHIDUsage_BS_iManufacturerName = 0x87,		/* SV F - Battery System Manufacturer Name Index */
	kHIDUsage_BS_iDevicename = 0x88,			/* SV F - Battery System Device Name Index */
	kHIDUsage_BS_iDeviceChemistry = 0x89,		/* SV F - Battery System Device Chemistry Index */
	kHIDUsage_BS_ManufacturerData = 0x8A,		/* SV F - Battery System Manufacturer Data */
	kHIDUsage_BS_Rechargable = 0x8B,			/* SV F - Battery System Rechargable */
	kHIDUsage_BS_WarningCapacityLimit = 0x8C,	/* SV F - Battery System Warning Capacity Limit */
	kHIDUsage_BS_CapacityGranularity1 = 0x8D,	/* SV F - Battery System Capacity Granularity 1 */
	kHIDUsage_BS_CapacityGranularity2 = 0x8E,	/* SV F - Battery System Capacity Granularity 2 */
	kHIDUsage_BS_iOEMInformation = 0x8F,		/* SV F - Battery System OEM Information Index */
	/* Reserved 0x90 - 0xBF */
	kHIDUsage_BS_InhibitCharge = 0xC0,			/* DF IOF - Battery System Inhibit Charge */
	kHIDUsage_BS_EnablePolling = 0xC1,			/* DF IOF - Battery System Enable Polling */
	kHIDUsage_BS_ResetToZero = 0xC2,			/* DF IOF - Battery System Reset To Zero */
	/* Reserved 0xC3 - 0xCF */
	kHIDUsage_BS_ACPresent = 0xD0,				/* DF IOF - Battery System AC Present */
	kHIDUsage_BS_BatteryPresent = 0xD1,			/* DF IOF - Battery System Battery Present */
	kHIDUsage_BS_PowerFail = 0xD2,				/* DF IOF - Battery System Power Fail */
	kHIDUsage_BS_AlarmInhibited = 0xD3,			/* DF IOF - Battery System Alarm Inhibited */
	kHIDUsage_BS_ThermistorUnderRange = 0xD4,	/* DF IOF - Battery System Thermistor Under Range */
	kHIDUsage_BS_ThermistorHot = 0xD5,			/* DF IOF - Battery System Thermistor Hot */
	kHIDUsage_BS_ThermistorCold = 0xD6,			/* DF IOF - Battery System Thermistor Cold */
	kHIDUsage_BS_ThermistorOverRange = 0xD7,	/* DF IOF - Battery System Thermistor Over Range */
	kHIDUsage_BS_VoltageOutOfRange = 0xD8,		/* DF IOF - Battery System Voltage Out Of Range */
	kHIDUsage_BS_CurrentOutOfRange = 0xD9,		/* DF IOF - Battery System Current Out Of Range */
	kHIDUsage_BS_CurrentNotRegulated = 0xDA,	/* DF IOF - Battery System Current Not Regulated */
	kHIDUsage_BS_VoltageNotRegulated = 0xDB,	/* DF IOF - Battery System Voltage Not Regulated */
	kHIDUsage_BS_MasterMode = 0xDC,				/* DF IOF - Battery System Master Mode */
	/* Reserved 0xDD - 0xEF */
	kHIDUsage_BS_ChargerSelectorSupport = 0xF0,	/* SF F- Battery System Charger Support Selector */
	kHIDUsage_BS_ChargerSpec = 0xF1,			/* SF F- Battery System Charger Specification */
	kHIDUsage_BS_Level2 = 0xF2,					/* SF F- Battery System Charger Level 2 */
	kHIDUsage_BS_Level3 = 0xF3					/* SF F- Battery System Charger Level 3 */
	/* Reserved 0xF2 - 0xFF */
};

#ifdef __cplusplus
}
#endif

#endif /* __IOHIDPowerUsage__ */

