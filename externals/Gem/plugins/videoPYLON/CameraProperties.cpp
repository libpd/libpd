////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file:
//  handling of getting/setting pylon/baslergigecam properties
//
//    Copyright (c) 2010-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "LICENSE.txt"
//
/////////////////////////////////////////////////////////


// get GigE-Camera attributes
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_PYLON
#include "CameraProperties.h"
#include <map>
#include <sstream>

namespace gem{namespace pylon{namespace cameraproperties{
  static gem::Properties writeprops, readprops;

  typedef Pylon::CBaslerGigECamera DEVICE;


  /* GenApi::IBoolean */
  typedef bool (*t_getbool)(DEVICE*device);
  std::map<std::string, t_getbool>map_getbool;
  typedef void (*t_setbool)(DEVICE*device, const bool);
  std::map<std::string, t_setbool>map_setbool;
#define GETSETBOOL(T)                                                   \
  static bool get##T(DEVICE*device) { return device->T.GetValue(); }    \
    static void set##T(DEVICE*device, const bool v) { device->T.SetValue(v); }

  GETSETBOOL(GammaEnable);
  GETSETBOOL(ExposureTimeBaseAbsEnable);
  GETSETBOOL(AcquisitionFrameRateEnable);
  GETSETBOOL(LineInverter);
  GETSETBOOL(LineTermination);
  GETSETBOOL(LineStatus);
  GETSETBOOL(UserOutputValue);
  GETSETBOOL(TimerSequenceEnable);
  GETSETBOOL(TimerSequenceTimerEnable);
  GETSETBOOL(TimerSequenceTimerInverter);
  GETSETBOOL(LUTEnable);
  GETSETBOOL(ShadingEnable);
  GETSETBOOL(RemoveLimits);
  GETSETBOOL(ExpertFeatureEnable);
  GETSETBOOL(PixelStepCorrectionEnable);
  GETSETBOOL(ChunkModeActive);
  GETSETBOOL(ChunkEnable);
  GETSETBOOL(GevDeviceModeIsBigEndian);
  GETSETBOOL(GevSupportedIPConfigurationLLA);
  GETSETBOOL(GevSupportedIPConfigurationDHCP);
  GETSETBOOL(GevSupportedIPConfigurationPersistentIP);
  GETSETBOOL(GevSupportedOptionalCommandsEVENTDATA);
  GETSETBOOL(GevSupportedOptionalCommandsEVENT);
  GETSETBOOL(GevSupportedOptionalCommandsPACKETRESEND);
  GETSETBOOL(GevSupportedOptionalCommandsWRITEMEM);
  GETSETBOOL(GevSupportedOptionalCommandsConcatenation);




  /* GenApi::IInteger */
  typedef int64_t (*t_getint)(DEVICE*device);
  std::map<std::string, t_getint>map_getint;
  typedef void (*t_setint)(DEVICE*device, const int64_t);
  std::map<std::string, t_setint>map_setint;


#define GETSETINT(T)                                                    \
  static int64_t get##T(DEVICE*device) { return device->T.GetValue(); } \
    static void set##T(DEVICE*device, const int64_t v) { device->T.SetValue(v); }

  GETSETINT(GainRaw);
  GETSETINT(BlackLevelRaw);
  GETSETINT(BalanceRatioRaw);
  GETSETINT(DigitalShift);
  GETSETINT(PixelDynamicRangeMin);
  GETSETINT(PixelDynamicRangeMax);
  GETSETINT(SpatialCorrection);
  GETSETINT(SpatialCorrectionAmount);
  GETSETINT(Width);
  GETSETINT(Height);
  GETSETINT(OffsetX);
  GETSETINT(OffsetY);
  GETSETINT(BinningVertical);
  GETSETINT(BinningHorizontal);
  GETSETINT(ExposureTimeRaw);
  GETSETINT(AveragingNumberOfFrames);
  GETSETINT(LineDebouncerTimeRaw);
  GETSETINT(LineStatusAll);
  GETSETINT(UserOutputValueAll);
  GETSETINT(UserOutputValueAllMask);
  GETSETINT(ShaftEncoderModuleCounter);
  GETSETINT(ShaftEncoderModuleCounterMax);
  GETSETINT(ShaftEncoderModuleReverseCounterMax);
  GETSETINT(TimerDelayRaw);
  GETSETINT(TimerDurationRaw);
  GETSETINT(TimerSequenceLastEntryIndex);
  GETSETINT(TimerSequenceCurrentEntryIndex);
  GETSETINT(TimerSequenceTimerDelayRaw);
  GETSETINT(TimerSequenceTimerDurationRaw);
  GETSETINT(LUTIndex);
  GETSETINT(LUTValue);
  GETSETINT(AutoTargetValue);
  GETSETINT(AutoGainRawLowerLimit);
  GETSETINT(AutoGainRawUpperLimit);
  GETSETINT(AutoFunctionAOIWidth);
  GETSETINT(AutoFunctionAOIHeight);
  GETSETINT(AutoFunctionAOIOffsetX);
  GETSETINT(AutoFunctionAOIOffsetY);
  GETSETINT(UserDefinedValue);
  GETSETINT(SensorWidth);
  GETSETINT(SensorHeight);
  GETSETINT(WidthMax);
  GETSETINT(HeightMax);
  GETSETINT(ExpertFeatureAccessKey);
  GETSETINT(PixelStepCorrectionValueRaw);
  GETSETINT(PixelStepCorrectionBusy);
  GETSETINT(ChunkStride);
  GETSETINT(ChunkOffsetX);
  GETSETINT(ChunkOffsetY);
  GETSETINT(ChunkWidth);
  GETSETINT(ChunkHeight);
  GETSETINT(ChunkDynamicRangeMin);
  GETSETINT(ChunkDynamicRangeMax);
  GETSETINT(ChunkTimestamp);
  GETSETINT(ChunkFramecounter);
  GETSETINT(ChunkLineStatusAll);
  GETSETINT(ChunkTriggerinputcounter);
  GETSETINT(ChunkLineTriggerIgnoredCounter);
  GETSETINT(ChunkFrameTriggerIgnoredCounter);
  GETSETINT(ChunkFrameTriggerCounter);
  GETSETINT(ChunkFramesPerTriggerCounter);
  GETSETINT(ChunkLineTriggerEndToEndCounter);
  GETSETINT(ChunkPayloadCRC16);
  GETSETINT(ExposureEndEventStreamChannelIndex);
  GETSETINT(ExposureEndEventFrameID);
  GETSETINT(ExposureEndEventTimestamp);
  GETSETINT(LineStartOvertriggerEventStreamChannelIndex);
  GETSETINT(LineStartOvertriggerEventTimestamp);
  GETSETINT(FrameStartOvertriggerEventStreamChannelIndex);
  GETSETINT(FrameStartOvertriggerEventTimestamp);
  GETSETINT(EventOverrunEventStreamChannelIndex);
  GETSETINT(EventOverrunEventFrameID);
  GETSETINT(EventOverrunEventTimestamp);
  GETSETINT(FileAccessOffset);
  GETSETINT(FileAccessLength);
  GETSETINT(FileOperationResult);
  GETSETINT(FileSize);
  GETSETINT(PayloadSize);
  GETSETINT(GevVersionMajor);
  GETSETINT(GevVersionMinor);
  GETSETINT(GevDeviceModeCharacterSet);
  GETSETINT(GevMACAddress);
  GETSETINT(GevCurrentIPConfiguration);
  GETSETINT(GevCurrentIPAddress);
  GETSETINT(GevCurrentSubnetMask);
  GETSETINT(GevCurrentDefaultGateway);
  GETSETINT(GevPersistentIPAddress);
  GETSETINT(GevPersistentSubnetMask);
  GETSETINT(GevPersistentDefaultGateway);
  GETSETINT(GevLinkSpeed);
  GETSETINT(GevNumberOfInterfaces);
  GETSETINT(GevMessageChannelCount);
  GETSETINT(GevStreamChannelCount);
  GETSETINT(GevHeartbeatTimeout);
  GETSETINT(GevTimestampTickFrequency);
  GETSETINT(GevTimestampValue);
  GETSETINT(GevSCPInterfaceIndex);
  GETSETINT(GevSCDA);
  GETSETINT(GevSCPHostPort);
  GETSETINT(GevSCPSPacketSize);
  GETSETINT(GevSCPD);
  GETSETINT(GevSCFTD);
  GETSETINT(GevSCBWR);
  GETSETINT(GevSCBWRA);
  GETSETINT(GevSCBWA);
  GETSETINT(GevSCDMT);
  GETSETINT(GevSCDCT);
  GETSETINT(GevSCFJM);
  GETSETINT(TLParamsLocked);



  /* GenApi::IFloat */
  typedef double (*t_getfloat)(DEVICE*device);
  std::map<std::string, t_getfloat>map_getfloat;
  typedef void (*t_setfloat)(DEVICE*device, const double);
  std::map<std::string, t_setfloat>map_setfloat;

#define GETSETFLOAT(T)                                                  \
  static double get##T(DEVICE*device) { return device->T.GetValue(); }  \
    static void set##T(DEVICE*device, const double v) { device->T.SetValue(v); }

  GETSETFLOAT(GainAbs);
  GETSETFLOAT(BlackLevelAbs);
  GETSETFLOAT(BalanceRatioAbs);
  GETSETFLOAT(Gamma);
  GETSETFLOAT(ExposureTimeAbs);
  GETSETFLOAT(ExposureTimeBaseAbs);
  GETSETFLOAT(AcquisitionLineRateAbs);
  GETSETFLOAT(ResultingLineRateAbs);
  GETSETFLOAT(AcquisitionFrameRateAbs);
  GETSETFLOAT(ResultingFrameRateAbs);
  GETSETFLOAT(LineDebouncerTimeAbs);
  GETSETFLOAT(TimerDelayTimebaseAbs);
  GETSETFLOAT(TimerDurationTimebaseAbs);
  GETSETFLOAT(TimerDelayAbs);
  GETSETFLOAT(TimerDurationAbs);
  GETSETFLOAT(AutoExposureTimeAbsLowerLimit);
  GETSETFLOAT(AutoExposureTimeAbsUpperLimit);
  GETSETFLOAT(TemperatureAbs);
  GETSETFLOAT(PixelStepCorrectionValueAbs);

  /* GenApi::IString */
  typedef GenICam::gcstring (*t_getstring)(DEVICE*device);
  std::map<std::string, t_getstring>map_getstring;
  typedef void (*t_setstring)(DEVICE*device, GenICam::gcstring);
  std::map<std::string, t_setstring>map_setstring;
#define GETSETSTRING(T)                                                 \
  static void set##T(DEVICE*device, const GenICam::gcstring v) { device->T.SetValue(v); } \
    static GenICam::gcstring get##T(DEVICE*device) { return device->T.GetValue(); }

  GETSETSTRING(DeviceVendorName);
  GETSETSTRING(DeviceModelName);
  GETSETSTRING(DeviceManufacturerInfo);
  GETSETSTRING(DeviceVersion);
  GETSETSTRING(DeviceFirmwareVersion);
  GETSETSTRING(DeviceID);
  GETSETSTRING(DeviceUserID);
  GETSETSTRING(GevFirstURL);
  GETSETSTRING(GevSecondURL);


  /* GenApi::ICommand */
  // only setting(=calling), no getting
  typedef void (*t_setcommand)(DEVICE*device);
  std::map<std::string, t_setcommand>map_setcommand;
#define GETSETCOMMAND(T)                                      \
  static void set##T(DEVICE*device) { device->T.Execute(); }
  GETSETCOMMAND(AcquisitionStart);
  GETSETCOMMAND(AcquisitionStop);
  GETSETCOMMAND(AcquisitionAbort);
  GETSETCOMMAND(TriggerSoftware);
  GETSETCOMMAND(ShaftEncoderModuleCounterReset);
  GETSETCOMMAND(ShaftEncoderModuleReverseCounterReset);
  GETSETCOMMAND(UserSetLoad);
  GETSETCOMMAND(UserSetSave);
  GETSETCOMMAND(ShadingSetActivate);
  GETSETCOMMAND(ShadingSetCreate);
  GETSETCOMMAND(DeviceReset);
  GETSETCOMMAND(SavePixelStepCorrection);
  GETSETCOMMAND(CreatePixelStepCorrection);
  GETSETCOMMAND(FileOperationExecute);
  GETSETCOMMAND(GevTimestampControlLatch);
  GETSETCOMMAND(GevTimestampControlReset);
  GETSETCOMMAND(GevTimestampControlLatchReset);

  /* GenApi::IRegister */
  // skip this, it's too lowlevel
  
  /* GenApi::IEnumerationT */
  // do this manually!

#define GETSETENUMVAL(T)                                                \
  std::map<std::string, enum Basler_GigECameraParams::T##Enums> enumap_##T; \
    static std::string get##T (DEVICE*device) {                         \
      const enum Basler_GigECameraParams::T##Enums v=device->T.GetValue(); \
      std::map<std::string, Basler_GigECameraParams::T##Enums>::iterator it; \
      for (it=enumap_##T.begin(); it != enumap_##T.end(); ++it) {       \
        if(v==it->second)                                               \
          return (it->first);                                           \
      }                                                                 \
      return std::string("unknown");                                    \
    }                                                                   \
    static void setS_##T (DEVICE*device, const std::string s) {         \
      std::map<std::string, Basler_GigECameraParams:: T##Enums>::iterator it=enumap_##T.find(s); \
      if(it!=enumap_##T.end())                                          \
        device->T.SetValue(it->second);                                 \
      else throw OUT_OF_RANGE_EXCEPTION(s.c_str());                     \
    }                                                                   \
    static void setI_##T (DEVICE*device, const int i) {                 \
      if(i<0 || i>=enumap_##T.size()) {                                 \
        std::stringstream ss;                                           \
        ss << "Out of range: " << i << " must be within [0.."<<enumap_##T.size()<<")"; \
        throw OUT_OF_RANGE_EXCEPTION(ss.str());                         \
      }                                                                 \
      else {                                                            \
        const enum Basler_GigECameraParams::T##Enums v=static_cast<Basler_GigECameraParams:: T##Enums>(i); \
        device->T.SetValue(v);                                          \
      }                                                                 \
    }

  typedef std::string (*t_getenum)(DEVICE*device);
  typedef void (*t_setenum)(DEVICE*device, const std::string);
  typedef void (*t_setenumi)(DEVICE*device, const int);
  std::map<std::string, t_getenum>map_getenum; 
  std::map<std::string, t_setenum>map_setenum; 
  std::map<std::string, t_setenumi>map_setenumi;

  GETSETENUMVAL(AcquisitionMode);
  GETSETENUMVAL(AutoFunctionAOISelector);
  GETSETENUMVAL(BalanceRatioSelector);
  GETSETENUMVAL(BalanceWhiteAuto);
  GETSETENUMVAL(BlackLevelSelector);
  GETSETENUMVAL(ChunkPixelFormat);
  GETSETENUMVAL(ChunkSelector);
  GETSETENUMVAL(DeviceScanType);
  GETSETENUMVAL(EventNotification);
  GETSETENUMVAL(EventSelector);
  GETSETENUMVAL(ExpertFeatureAccessSelector);
  GETSETENUMVAL(ExposureAuto);
  GETSETENUMVAL(ExposureMode);
  GETSETENUMVAL(FileOpenMode);
  GETSETENUMVAL(FileOperationSelector);
  GETSETENUMVAL(FileOperationStatus);
  GETSETENUMVAL(FileSelector);
  GETSETENUMVAL(GainAuto);
  GETSETENUMVAL(GainSelector);
  GETSETENUMVAL(GevCCP);
  GETSETENUMVAL(GevInterfaceSelector);
  GETSETENUMVAL(GevStreamChannelSelector);
  GETSETENUMVAL(LegacyBinningVertical);
  GETSETENUMVAL(LineFormat);
  GETSETENUMVAL(LineMode);
  GETSETENUMVAL(LineSelector);
  GETSETENUMVAL(LineSource);
  GETSETENUMVAL(LUTSelector);
  GETSETENUMVAL(ParameterSelector);
  GETSETENUMVAL(PixelCoding);
  GETSETENUMVAL(PixelColorFilter);
  GETSETENUMVAL(PixelFormat);
  GETSETENUMVAL(PixelSize);
  GETSETENUMVAL(PixelStepCorrectionSelector);
  GETSETENUMVAL(ShadingSelector);
  GETSETENUMVAL(ShadingSetDefaultSelector);
  GETSETENUMVAL(ShadingSetSelector);
  GETSETENUMVAL(ShadingStatus);
  GETSETENUMVAL(ShaftEncoderModuleCounterMode);
  GETSETENUMVAL(ShaftEncoderModuleLineSelector);
  GETSETENUMVAL(ShaftEncoderModuleLineSource);
  GETSETENUMVAL(ShaftEncoderModuleMode);
  GETSETENUMVAL(SpatialCorrectionStartingLine);
  GETSETENUMVAL(TemperatureSelector);
  GETSETENUMVAL(TestImageSelector);
  GETSETENUMVAL(TimerSelector);
  GETSETENUMVAL(TimerSequenceEntrySelector);
  GETSETENUMVAL(TimerSequenceTimerSelector);
  GETSETENUMVAL(TimerTriggerActivation);
  GETSETENUMVAL(TimerTriggerSource);
  GETSETENUMVAL(TriggerActivation);
  GETSETENUMVAL(TriggerMode);
  GETSETENUMVAL(TriggerSelector);
  GETSETENUMVAL(TriggerSource);
  GETSETENUMVAL(UserDefinedValueSelector);
  GETSETENUMVAL(UserOutputSelector);
  GETSETENUMVAL(UserSetDefaultSelector);
  GETSETENUMVAL(UserSetSelector);
  

};};};

using namespace gem::pylon::cameraproperties;


//typedef t_int *(*t_perfroutine)(t_int *args);

//typedef GenApi::IInteger&(pylprop_int_tf) (Pylon::CBaslerGigECamera::*) (bool);


typedef GenApi::IInteger& bla_t;

static std::map<std::string, GenApi::IInteger&>s_intfun;

void   gem::pylon::cameraproperties::init() {
  static bool initialized=false;  if(initialized){return;} else {initialized=true;}

#define MAP_GETSETBOOL(T)                       \
  map_getbool[ #T ]=get##T;                     \
  map_setbool[ #T ]=set##T

#define MAP_GETSETINT(T)                        \
  map_getint[ #T ]=get##T;                      \
  map_setint[ #T ]=set##T

#define MAP_GETSETFLOAT(T)                      \
  map_getfloat[ #T ]=get##T;                    \
  map_setfloat[ #T ]=set##T

#define MAP_GETSETSTRING(T)                     \
  map_getstring[ #T ]=get##T;                   \
  map_setstring[ #T ]=set##T

#define MAP_GETSETCOMMAND(T)                    \
  map_setcommand[ #T ]=set##T

#define MAP_GETSETENUM(T)                       \
  map_getenum[ #T ]=get##T;                     \
  map_setenum[ #T ]=setS_##T;                   \
  map_setenumi[ #T ]=setI_##T

  map_getbool.clear();
  map_setbool.clear();
  MAP_GETSETBOOL(GammaEnable);
  MAP_GETSETBOOL(ExposureTimeBaseAbsEnable);
  MAP_GETSETBOOL(AcquisitionFrameRateEnable);
  MAP_GETSETBOOL(LineInverter);
  MAP_GETSETBOOL(LineTermination);
  MAP_GETSETBOOL(LineStatus);
  MAP_GETSETBOOL(UserOutputValue);
  MAP_GETSETBOOL(TimerSequenceEnable);
  MAP_GETSETBOOL(TimerSequenceTimerEnable);
  MAP_GETSETBOOL(TimerSequenceTimerInverter);
  MAP_GETSETBOOL(LUTEnable);
  MAP_GETSETBOOL(ShadingEnable);
  MAP_GETSETBOOL(RemoveLimits);
  MAP_GETSETBOOL(ExpertFeatureEnable);
  MAP_GETSETBOOL(PixelStepCorrectionEnable);
  MAP_GETSETBOOL(ChunkModeActive);
  MAP_GETSETBOOL(ChunkEnable);
  MAP_GETSETBOOL(GevDeviceModeIsBigEndian);
  MAP_GETSETBOOL(GevSupportedIPConfigurationLLA);
  MAP_GETSETBOOL(GevSupportedIPConfigurationDHCP);
  MAP_GETSETBOOL(GevSupportedIPConfigurationPersistentIP);
  MAP_GETSETBOOL(GevSupportedOptionalCommandsEVENTDATA);
  MAP_GETSETBOOL(GevSupportedOptionalCommandsEVENT);
  MAP_GETSETBOOL(GevSupportedOptionalCommandsPACKETRESEND);
  MAP_GETSETBOOL(GevSupportedOptionalCommandsWRITEMEM);
  MAP_GETSETBOOL(GevSupportedOptionalCommandsConcatenation);


  map_getint.clear();
  map_setint.clear();
  MAP_GETSETINT(GainRaw);
  MAP_GETSETINT(BlackLevelRaw);
  MAP_GETSETINT(BalanceRatioRaw);
  MAP_GETSETINT(DigitalShift);
  MAP_GETSETINT(PixelDynamicRangeMin);
  MAP_GETSETINT(PixelDynamicRangeMax);
  MAP_GETSETINT(SpatialCorrection);
  MAP_GETSETINT(SpatialCorrectionAmount);
  MAP_GETSETINT(Width);
  MAP_GETSETINT(Height);
  MAP_GETSETINT(OffsetX);
  MAP_GETSETINT(OffsetY);
  MAP_GETSETINT(BinningVertical);
  MAP_GETSETINT(BinningHorizontal);
  MAP_GETSETINT(ExposureTimeRaw);
  MAP_GETSETINT(AveragingNumberOfFrames);
  MAP_GETSETINT(LineDebouncerTimeRaw);
  MAP_GETSETINT(LineStatusAll);
  MAP_GETSETINT(UserOutputValueAll);
  MAP_GETSETINT(UserOutputValueAllMask);
  MAP_GETSETINT(ShaftEncoderModuleCounter);
  MAP_GETSETINT(ShaftEncoderModuleCounterMax);
  MAP_GETSETINT(ShaftEncoderModuleReverseCounterMax);
  MAP_GETSETINT(TimerDelayRaw);
  MAP_GETSETINT(TimerDurationRaw);
  MAP_GETSETINT(TimerSequenceLastEntryIndex);
  MAP_GETSETINT(TimerSequenceCurrentEntryIndex);
  MAP_GETSETINT(TimerSequenceTimerDelayRaw);
  MAP_GETSETINT(TimerSequenceTimerDurationRaw);
  MAP_GETSETINT(LUTIndex);
  MAP_GETSETINT(LUTValue);
  MAP_GETSETINT(AutoTargetValue);
  MAP_GETSETINT(AutoGainRawLowerLimit);
  MAP_GETSETINT(AutoGainRawUpperLimit);
  MAP_GETSETINT(AutoFunctionAOIWidth);
  MAP_GETSETINT(AutoFunctionAOIHeight);
  MAP_GETSETINT(AutoFunctionAOIOffsetX);
  MAP_GETSETINT(AutoFunctionAOIOffsetY);
  MAP_GETSETINT(UserDefinedValue);
  MAP_GETSETINT(SensorWidth);
  MAP_GETSETINT(SensorHeight);
  MAP_GETSETINT(WidthMax);
  MAP_GETSETINT(HeightMax);
  MAP_GETSETINT(ExpertFeatureAccessKey);
  MAP_GETSETINT(PixelStepCorrectionValueRaw);
  MAP_GETSETINT(PixelStepCorrectionBusy);
  MAP_GETSETINT(ChunkStride);
  MAP_GETSETINT(ChunkOffsetX);
  MAP_GETSETINT(ChunkOffsetY);
  MAP_GETSETINT(ChunkWidth);
  MAP_GETSETINT(ChunkHeight);
  MAP_GETSETINT(ChunkDynamicRangeMin);
  MAP_GETSETINT(ChunkDynamicRangeMax);
  MAP_GETSETINT(ChunkTimestamp);
  MAP_GETSETINT(ChunkFramecounter);
  MAP_GETSETINT(ChunkLineStatusAll);
  MAP_GETSETINT(ChunkTriggerinputcounter);
  MAP_GETSETINT(ChunkLineTriggerIgnoredCounter);
  MAP_GETSETINT(ChunkFrameTriggerIgnoredCounter);
  MAP_GETSETINT(ChunkFrameTriggerCounter);
  MAP_GETSETINT(ChunkFramesPerTriggerCounter);
  MAP_GETSETINT(ChunkLineTriggerEndToEndCounter);
  MAP_GETSETINT(ChunkPayloadCRC16);
  MAP_GETSETINT(ExposureEndEventStreamChannelIndex);
  MAP_GETSETINT(ExposureEndEventFrameID);
  MAP_GETSETINT(ExposureEndEventTimestamp);
  MAP_GETSETINT(LineStartOvertriggerEventStreamChannelIndex);
  MAP_GETSETINT(LineStartOvertriggerEventTimestamp);
  MAP_GETSETINT(FrameStartOvertriggerEventStreamChannelIndex);
  MAP_GETSETINT(FrameStartOvertriggerEventTimestamp);
  MAP_GETSETINT(EventOverrunEventStreamChannelIndex);
  MAP_GETSETINT(EventOverrunEventFrameID);
  MAP_GETSETINT(EventOverrunEventTimestamp);
  MAP_GETSETINT(FileAccessOffset);
  MAP_GETSETINT(FileAccessLength);
  MAP_GETSETINT(FileOperationResult);
  MAP_GETSETINT(FileSize);
  MAP_GETSETINT(PayloadSize);
  MAP_GETSETINT(GevVersionMajor);
  MAP_GETSETINT(GevVersionMinor);
  MAP_GETSETINT(GevDeviceModeCharacterSet);
  MAP_GETSETINT(GevMACAddress);
  MAP_GETSETINT(GevCurrentIPConfiguration);
  MAP_GETSETINT(GevCurrentIPAddress);
  MAP_GETSETINT(GevCurrentSubnetMask);
  MAP_GETSETINT(GevCurrentDefaultGateway);
  MAP_GETSETINT(GevPersistentIPAddress);
  MAP_GETSETINT(GevPersistentSubnetMask);
  MAP_GETSETINT(GevPersistentDefaultGateway);
  MAP_GETSETINT(GevLinkSpeed);
  MAP_GETSETINT(GevNumberOfInterfaces);
  MAP_GETSETINT(GevMessageChannelCount);
  MAP_GETSETINT(GevStreamChannelCount);
  MAP_GETSETINT(GevHeartbeatTimeout);
  MAP_GETSETINT(GevTimestampTickFrequency);
  MAP_GETSETINT(GevTimestampValue);
  MAP_GETSETINT(GevSCPInterfaceIndex);
  MAP_GETSETINT(GevSCDA);
  MAP_GETSETINT(GevSCPHostPort);
  MAP_GETSETINT(GevSCPSPacketSize);
  MAP_GETSETINT(GevSCPD);
  MAP_GETSETINT(GevSCFTD);
  MAP_GETSETINT(GevSCBWR);
  MAP_GETSETINT(GevSCBWRA);
  MAP_GETSETINT(GevSCBWA);
  MAP_GETSETINT(GevSCDMT);
  MAP_GETSETINT(GevSCDCT);
  MAP_GETSETINT(GevSCFJM);
  MAP_GETSETINT(TLParamsLocked);


  map_getfloat.clear();
  map_setfloat.clear();
  MAP_GETSETFLOAT(GainAbs);
  MAP_GETSETFLOAT(BlackLevelAbs);
  MAP_GETSETFLOAT(BalanceRatioAbs);
  MAP_GETSETFLOAT(Gamma);
  MAP_GETSETFLOAT(ExposureTimeAbs);
  MAP_GETSETFLOAT(ExposureTimeBaseAbs);
  MAP_GETSETFLOAT(AcquisitionLineRateAbs);
  MAP_GETSETFLOAT(ResultingLineRateAbs);
  MAP_GETSETFLOAT(AcquisitionFrameRateAbs);
  MAP_GETSETFLOAT(ResultingFrameRateAbs);
  MAP_GETSETFLOAT(LineDebouncerTimeAbs);
  MAP_GETSETFLOAT(TimerDelayTimebaseAbs);
  MAP_GETSETFLOAT(TimerDurationTimebaseAbs);
  MAP_GETSETFLOAT(TimerDelayAbs);
  MAP_GETSETFLOAT(TimerDurationAbs);
  MAP_GETSETFLOAT(AutoExposureTimeAbsLowerLimit);
  MAP_GETSETFLOAT(AutoExposureTimeAbsUpperLimit);
  MAP_GETSETFLOAT(TemperatureAbs);
  MAP_GETSETFLOAT(PixelStepCorrectionValueAbs);




  map_getstring.clear();
  map_setstring.clear();
  MAP_GETSETSTRING(DeviceVendorName);
  MAP_GETSETSTRING(DeviceModelName);
  MAP_GETSETSTRING(DeviceManufacturerInfo);
  MAP_GETSETSTRING(DeviceVersion);
  MAP_GETSETSTRING(DeviceFirmwareVersion);
  MAP_GETSETSTRING(DeviceID);
  MAP_GETSETSTRING(DeviceUserID);
  MAP_GETSETSTRING(GevFirstURL);
  MAP_GETSETSTRING(GevSecondURL);



  map_setcommand.clear();
  MAP_GETSETCOMMAND(AcquisitionStart);
  MAP_GETSETCOMMAND(AcquisitionStop);
  MAP_GETSETCOMMAND(AcquisitionAbort);
  MAP_GETSETCOMMAND(TriggerSoftware);
  MAP_GETSETCOMMAND(ShaftEncoderModuleCounterReset);
  MAP_GETSETCOMMAND(ShaftEncoderModuleReverseCounterReset);
  MAP_GETSETCOMMAND(UserSetLoad);
  MAP_GETSETCOMMAND(UserSetSave);
  MAP_GETSETCOMMAND(ShadingSetActivate);
  MAP_GETSETCOMMAND(ShadingSetCreate);
  MAP_GETSETCOMMAND(DeviceReset);
  MAP_GETSETCOMMAND(SavePixelStepCorrection);
  MAP_GETSETCOMMAND(CreatePixelStepCorrection);
  MAP_GETSETCOMMAND(FileOperationExecute);
  MAP_GETSETCOMMAND(GevTimestampControlLatch);
  MAP_GETSETCOMMAND(GevTimestampControlReset);
  MAP_GETSETCOMMAND(GevTimestampControlLatchReset);


  map_getenum.clear();
  map_setenum.clear();
  map_setenumi.clear();
  MAP_GETSETENUM(AcquisitionMode);
  MAP_GETSETENUM(AutoFunctionAOISelector);
  MAP_GETSETENUM(BalanceRatioSelector);
  MAP_GETSETENUM(BalanceWhiteAuto);
  MAP_GETSETENUM(BlackLevelSelector);
  MAP_GETSETENUM(ChunkPixelFormat);
  MAP_GETSETENUM(ChunkSelector);
  MAP_GETSETENUM(DeviceScanType);
  MAP_GETSETENUM(EventNotification);
  MAP_GETSETENUM(EventSelector);
  MAP_GETSETENUM(ExpertFeatureAccessSelector);
  MAP_GETSETENUM(ExposureAuto);
  MAP_GETSETENUM(ExposureMode);
  MAP_GETSETENUM(FileOpenMode);
  MAP_GETSETENUM(FileOperationSelector);
  MAP_GETSETENUM(FileOperationStatus);
  MAP_GETSETENUM(FileSelector);
  MAP_GETSETENUM(GainAuto);
  MAP_GETSETENUM(GainSelector);
  MAP_GETSETENUM(GevCCP);
  MAP_GETSETENUM(GevInterfaceSelector);
  MAP_GETSETENUM(GevStreamChannelSelector);
  MAP_GETSETENUM(LegacyBinningVertical);
  MAP_GETSETENUM(LineFormat);
  MAP_GETSETENUM(LineMode);
  MAP_GETSETENUM(LineSelector);
  MAP_GETSETENUM(LineSource);
  MAP_GETSETENUM(LUTSelector);
  MAP_GETSETENUM(ParameterSelector);
  MAP_GETSETENUM(PixelCoding);
  MAP_GETSETENUM(PixelColorFilter);
  MAP_GETSETENUM(PixelFormat);
  MAP_GETSETENUM(PixelSize);
  MAP_GETSETENUM(PixelStepCorrectionSelector);
  MAP_GETSETENUM(ShadingSelector);
  MAP_GETSETENUM(ShadingSetDefaultSelector);
  MAP_GETSETENUM(ShadingSetSelector);
  MAP_GETSETENUM(ShadingStatus);
  MAP_GETSETENUM(ShaftEncoderModuleCounterMode);
  MAP_GETSETENUM(ShaftEncoderModuleLineSelector);
  MAP_GETSETENUM(ShaftEncoderModuleLineSource);
  MAP_GETSETENUM(ShaftEncoderModuleMode);
  MAP_GETSETENUM(SpatialCorrectionStartingLine);
  MAP_GETSETENUM(TemperatureSelector);
  MAP_GETSETENUM(TestImageSelector);
  MAP_GETSETENUM(TimerSelector);
  MAP_GETSETENUM(TimerSequenceEntrySelector);
  MAP_GETSETENUM(TimerSequenceTimerSelector);
  MAP_GETSETENUM(TimerTriggerActivation);
  MAP_GETSETENUM(TimerTriggerSource);
  MAP_GETSETENUM(TriggerActivation);
  MAP_GETSETENUM(TriggerMode);
  MAP_GETSETENUM(TriggerSelector);
  MAP_GETSETENUM(TriggerSource);
  MAP_GETSETENUM(UserDefinedValueSelector);
  MAP_GETSETENUM(UserOutputSelector);
  MAP_GETSETENUM(UserSetDefaultSelector);
  MAP_GETSETENUM(UserSetSelector);


  using namespace  Basler_GigECameraParams;
  enumap_AcquisitionMode.clear();
  enumap_AutoFunctionAOISelector.clear();
  enumap_BalanceRatioSelector.clear();
  enumap_BalanceWhiteAuto.clear();
  enumap_BlackLevelSelector.clear();
  enumap_ChunkPixelFormat.clear();
  enumap_ChunkSelector.clear();
  enumap_DeviceScanType.clear();
  enumap_EventNotification.clear();
  enumap_EventSelector.clear();
  enumap_ExpertFeatureAccessSelector.clear();
  enumap_ExposureAuto.clear();
  enumap_ExposureMode.clear();
  enumap_FileOpenMode.clear();
  enumap_FileOperationSelector.clear();
  enumap_FileOperationStatus.clear();
  enumap_FileSelector.clear();
  enumap_GainAuto.clear();
  enumap_GainSelector.clear();
  enumap_GevCCP.clear();
  enumap_GevInterfaceSelector.clear();
  enumap_GevStreamChannelSelector.clear();
  enumap_LegacyBinningVertical.clear();
  enumap_LineFormat.clear();
  enumap_LineMode.clear();
  enumap_LineSelector.clear();
  enumap_LineSource.clear();
  enumap_LUTSelector.clear();
  enumap_ParameterSelector.clear();
  enumap_PixelCoding.clear();
  enumap_PixelColorFilter.clear();
  enumap_PixelFormat.clear();
  enumap_PixelSize.clear();
  enumap_PixelStepCorrectionSelector.clear();
  enumap_ShadingSelector.clear();
  enumap_ShadingSetDefaultSelector.clear();
  enumap_ShadingSetSelector.clear();
  enumap_ShadingStatus.clear();
  enumap_ShaftEncoderModuleCounterMode.clear();
  enumap_ShaftEncoderModuleLineSelector.clear();
  enumap_ShaftEncoderModuleLineSource.clear();
  enumap_ShaftEncoderModuleMode.clear();
  enumap_SpatialCorrectionStartingLine.clear();
  enumap_TemperatureSelector.clear();
  enumap_TestImageSelector.clear();
  enumap_TimerSelector.clear();
  enumap_TimerSequenceEntrySelector.clear();
  enumap_TimerSequenceTimerSelector.clear();
  enumap_TimerTriggerActivation.clear();
  enumap_TimerTriggerSource.clear();
  enumap_TriggerActivation.clear();
  enumap_TriggerMode.clear();
  enumap_TriggerSelector.clear();
  enumap_TriggerSource.clear();
  enumap_UserDefinedValueSelector.clear();
  enumap_UserOutputSelector.clear();
  enumap_UserSetDefaultSelector.clear();
  enumap_UserSetSelector.clear();

  enumap_GainAuto["Off"]=GainAuto_Off;
  enumap_GainAuto["Once"]=GainAuto_Once;
  enumap_GainAuto["Continuous"]=GainAuto_Continuous;
  enumap_GainSelector["All"]=GainSelector_All;
  enumap_GainSelector["AnalogAll"]=GainSelector_AnalogAll;
  enumap_GainSelector["DigitalAll"]=GainSelector_DigitalAll;
  enumap_GainSelector["Tap1"]=GainSelector_Tap1;
  enumap_GainSelector["Tap2"]=GainSelector_Tap2;
  enumap_GainSelector["Tap3"]=GainSelector_Tap3;
  enumap_GainSelector["Tap4"]=GainSelector_Tap4;
  enumap_GainSelector["Red"]=GainSelector_Red;
  enumap_GainSelector["Green"]=GainSelector_Green;
  enumap_GainSelector["Blue"]=GainSelector_Blue;
  enumap_BlackLevelSelector["All"]=BlackLevelSelector_All;
  enumap_BlackLevelSelector["AnalogAll"]=BlackLevelSelector_AnalogAll;
  enumap_BlackLevelSelector["DigitalAll"]=BlackLevelSelector_DigitalAll;
  enumap_BlackLevelSelector["Tap1"]=BlackLevelSelector_Tap1;
  enumap_BlackLevelSelector["Tap2"]=BlackLevelSelector_Tap2;
  enumap_BlackLevelSelector["Tap3"]=BlackLevelSelector_Tap3;
  enumap_BlackLevelSelector["Tap4"]=BlackLevelSelector_Tap4;
  enumap_BlackLevelSelector["Red"]=BlackLevelSelector_Red;
  enumap_BlackLevelSelector["Green"]=BlackLevelSelector_Green;
  enumap_BlackLevelSelector["Blue"]=BlackLevelSelector_Blue;
  enumap_BalanceWhiteAuto["Off"]=BalanceWhiteAuto_Off;
  enumap_BalanceWhiteAuto["Once"]=BalanceWhiteAuto_Once;
  enumap_BalanceWhiteAuto["Continuous"]=BalanceWhiteAuto_Continuous;
  enumap_BalanceRatioSelector["Red"]=BalanceRatioSelector_Red;
  enumap_BalanceRatioSelector["Green"]=BalanceRatioSelector_Green;
  enumap_BalanceRatioSelector["Blue"]=BalanceRatioSelector_Blue;
  enumap_PixelFormat["Mono8"]=PixelFormat_Mono8;
  enumap_PixelFormat["Mono8Signed"]=PixelFormat_Mono8Signed;
  enumap_PixelFormat["Mono10"]=PixelFormat_Mono10;
  enumap_PixelFormat["Mono12"]=PixelFormat_Mono12;
  enumap_PixelFormat["Mono16"]=PixelFormat_Mono16;
  enumap_PixelFormat["Mono16Signed"]=PixelFormat_Mono16Signed;
  enumap_PixelFormat["Mono10Packed"]=PixelFormat_Mono10Packed;
  enumap_PixelFormat["Mono12Packed"]=PixelFormat_Mono12Packed;
  enumap_PixelFormat["BayerGR8"]=PixelFormat_BayerGR8;
  enumap_PixelFormat["BayerRG8"]=PixelFormat_BayerRG8;
  enumap_PixelFormat["BayerGB8"]=PixelFormat_BayerGB8;
  enumap_PixelFormat["BayerBG8"]=PixelFormat_BayerBG8;
  enumap_PixelFormat["BayerGR10"]=PixelFormat_BayerGR10;
  enumap_PixelFormat["BayerRG10"]=PixelFormat_BayerRG10;
  enumap_PixelFormat["BayerGB10"]=PixelFormat_BayerGB10;
  enumap_PixelFormat["BayerBG10"]=PixelFormat_BayerBG10;
  enumap_PixelFormat["BayerGR12"]=PixelFormat_BayerGR12;
  enumap_PixelFormat["BayerRG12"]=PixelFormat_BayerRG12;
  enumap_PixelFormat["BayerGB12"]=PixelFormat_BayerGB12;
  enumap_PixelFormat["BayerBG12"]=PixelFormat_BayerBG12;
  enumap_PixelFormat["RGB8Packed"]=PixelFormat_RGB8Packed;
  enumap_PixelFormat["BGR8Packed"]=PixelFormat_BGR8Packed;
  enumap_PixelFormat["RGBA8Packed"]=PixelFormat_RGBA8Packed;
  enumap_PixelFormat["BGRA8Packed"]=PixelFormat_BGRA8Packed;
  enumap_PixelFormat["RGB10Packed"]=PixelFormat_RGB10Packed;
  enumap_PixelFormat["BGR10Packed"]=PixelFormat_BGR10Packed;
  enumap_PixelFormat["RGB12Packed"]=PixelFormat_RGB12Packed;
  enumap_PixelFormat["BGR12Packed"]=PixelFormat_BGR12Packed;
  enumap_PixelFormat["RGB10V1Packed"]=PixelFormat_RGB10V1Packed;
  enumap_PixelFormat["RGB10V2Packed"]=PixelFormat_RGB10V2Packed;
  enumap_PixelFormat["YUV411Packed"]=PixelFormat_YUV411Packed;
  enumap_PixelFormat["YUV422Packed"]=PixelFormat_YUV422Packed;
  enumap_PixelFormat["YUV444Packed"]=PixelFormat_YUV444Packed;
  enumap_PixelFormat["RGB8Planar"]=PixelFormat_RGB8Planar;
  enumap_PixelFormat["RGB10Planar"]=PixelFormat_RGB10Planar;
  enumap_PixelFormat["RGB12Planar"]=PixelFormat_RGB12Planar;
  enumap_PixelFormat["RGB16Planar"]=PixelFormat_RGB16Planar;
  enumap_PixelFormat["YUV422_YUYV_Packed"]=PixelFormat_YUV422_YUYV_Packed;
  enumap_PixelFormat["BayerGB12Packed"]=PixelFormat_BayerGB12Packed;
  enumap_PixelFormat["BayerGR12Packed"]=PixelFormat_BayerGR12Packed;
  enumap_PixelFormat["BayerRG12Packed"]=PixelFormat_BayerRG12Packed;
  enumap_PixelFormat["BayerBG12Packed"]=PixelFormat_BayerBG12Packed;
  enumap_PixelFormat["BayerGR16"]=PixelFormat_BayerGR16;
  enumap_PixelFormat["BayerRG16"]=PixelFormat_BayerRG16;
  enumap_PixelFormat["BayerGB16"]=PixelFormat_BayerGB16;
  enumap_PixelFormat["BayerBG16"]=PixelFormat_BayerBG16;
  enumap_PixelFormat["RGB12V1Packed"]=PixelFormat_RGB12V1Packed;
  enumap_PixelCoding["Mono8"]=PixelCoding_Mono8;
  enumap_PixelCoding["Mono8Signed"]=PixelCoding_Mono8Signed;
  enumap_PixelCoding["Mono16"]=PixelCoding_Mono16;
  enumap_PixelCoding["Mono10Packed"]=PixelCoding_Mono10Packed;
  enumap_PixelCoding["Mono12Packed"]=PixelCoding_Mono12Packed;
  enumap_PixelCoding["Raw8"]=PixelCoding_Raw8;
  enumap_PixelCoding["Raw16"]=PixelCoding_Raw16;
  enumap_PixelCoding["RGB8"]=PixelCoding_RGB8;
  enumap_PixelCoding["BGR8"]=PixelCoding_BGR8;
  enumap_PixelCoding["RGBA8"]=PixelCoding_RGBA8;
  enumap_PixelCoding["BGRA8"]=PixelCoding_BGRA8;
  enumap_PixelCoding["RGB16"]=PixelCoding_RGB16;
  enumap_PixelCoding["BGR16"]=PixelCoding_BGR16;
  enumap_PixelCoding["RGB10V1Packed"]=PixelCoding_RGB10V1Packed;
  enumap_PixelCoding["RGB10V2Packed"]=PixelCoding_RGB10V2Packed;
  enumap_PixelCoding["YUV411"]=PixelCoding_YUV411;
  enumap_PixelCoding["YUV422"]=PixelCoding_YUV422;
  enumap_PixelCoding["YUV444"]=PixelCoding_YUV444;
  enumap_PixelCoding["RGB8Planar"]=PixelCoding_RGB8Planar;
  enumap_PixelCoding["RGB16Planar"]=PixelCoding_RGB16Planar;
  enumap_PixelSize["Bpp8"]=PixelSize_Bpp8;
  enumap_PixelSize["Bpp10"]=PixelSize_Bpp10;
  enumap_PixelSize["Bpp12"]=PixelSize_Bpp12;
  enumap_PixelSize["Bpp16"]=PixelSize_Bpp16;
  enumap_PixelSize["Bpp24"]=PixelSize_Bpp24;
  enumap_PixelSize["Bpp32"]=PixelSize_Bpp32;
  enumap_PixelSize["Bpp36"]=PixelSize_Bpp36;
  enumap_PixelSize["Bpp48"]=PixelSize_Bpp48;
  enumap_PixelSize["Bpp64"]=PixelSize_Bpp64;
  enumap_PixelColorFilter["None"]=PixelColorFilter_None;
  enumap_PixelColorFilter["Bayer_RG"]=PixelColorFilter_Bayer_RG;
  enumap_PixelColorFilter["Bayer_GB"]=PixelColorFilter_Bayer_GB;
  enumap_PixelColorFilter["Bayer_GR"]=PixelColorFilter_Bayer_GR;
  enumap_PixelColorFilter["Bayer_BG"]=PixelColorFilter_Bayer_BG;
  enumap_SpatialCorrectionStartingLine["LineRed"]=SpatialCorrectionStartingLine_LineRed;
  enumap_SpatialCorrectionStartingLine["LineGreen"]=SpatialCorrectionStartingLine_LineGreen;
  enumap_SpatialCorrectionStartingLine["LineBlue"]=SpatialCorrectionStartingLine_LineBlue;
  enumap_TestImageSelector["Off"]=TestImageSelector_Off;
  enumap_TestImageSelector["Black"]=TestImageSelector_Black;
  enumap_TestImageSelector["White"]=TestImageSelector_White;
  enumap_TestImageSelector["GreyHorizontalRamp"]=TestImageSelector_GreyHorizontalRamp;
  enumap_TestImageSelector["GreyVerticalRamp"]=TestImageSelector_GreyVerticalRamp;
  enumap_TestImageSelector["GreyHorizontalRampMoving"]=TestImageSelector_GreyHorizontalRampMoving;
  enumap_TestImageSelector["GreyVerticalRampMoving"]=TestImageSelector_GreyVerticalRampMoving;
  enumap_TestImageSelector["HorzontalLineMoving"]=TestImageSelector_HorzontalLineMoving;
  enumap_TestImageSelector["VerticalLineMoving"]=TestImageSelector_VerticalLineMoving;
  enumap_TestImageSelector["ColorBar"]=TestImageSelector_ColorBar;
  enumap_TestImageSelector["FrameCounter"]=TestImageSelector_FrameCounter;
  enumap_TestImageSelector["DeviceSpecific"]=TestImageSelector_DeviceSpecific;
  enumap_TestImageSelector["FixedDiagonalGrayGradient_8Bit"]=TestImageSelector_FixedDiagonalGrayGradient_8Bit;
  enumap_TestImageSelector["MovingDiagonalGrayGradient_8Bit"]=TestImageSelector_MovingDiagonalGrayGradient_8Bit;
  enumap_TestImageSelector["MovingDiagonalGrayGradient_12Bit"]=TestImageSelector_MovingDiagonalGrayGradient_12Bit;
  enumap_TestImageSelector["MovingDiagonalGrayGradientFeatureTest_8Bit"]=TestImageSelector_MovingDiagonalGrayGradientFeatureTest_8Bit;
  enumap_TestImageSelector["MovingDiagonalGrayGradientFeatureTest_12Bit"]=TestImageSelector_MovingDiagonalGrayGradientFeatureTest_12Bit;
  enumap_TestImageSelector["MovingDiagonalColorGradient"]=TestImageSelector_MovingDiagonalColorGradient;
  enumap_TestImageSelector["Testimage1"]=TestImageSelector_Testimage1;
  enumap_TestImageSelector["Testimage2"]=TestImageSelector_Testimage2;
  enumap_TestImageSelector["Testimage3"]=TestImageSelector_Testimage3;
  enumap_TestImageSelector["Testimage4"]=TestImageSelector_Testimage4;
  enumap_TestImageSelector["Testimage5"]=TestImageSelector_Testimage5;
  enumap_TestImageSelector["Testimage6"]=TestImageSelector_Testimage6;
  enumap_TestImageSelector["Testimage7"]=TestImageSelector_Testimage7;
  enumap_LegacyBinningVertical["Off"]=LegacyBinningVertical_Off;
  enumap_LegacyBinningVertical["Two_Rows"]=LegacyBinningVertical_Two_Rows;
  enumap_AcquisitionMode["SingleFrame"]=AcquisitionMode_SingleFrame;
  enumap_AcquisitionMode["MultiFrame"]=AcquisitionMode_MultiFrame;
  enumap_AcquisitionMode["Continuous"]=AcquisitionMode_Continuous;
  enumap_TriggerSelector["AcquisitionStart"]=TriggerSelector_AcquisitionStart;
  enumap_TriggerSelector["AcquisitionEnd"]=TriggerSelector_AcquisitionEnd;
  enumap_TriggerSelector["AcquisitionActive"]=TriggerSelector_AcquisitionActive;
  enumap_TriggerSelector["FrameStart"]=TriggerSelector_FrameStart;
  enumap_TriggerSelector["FrameEnd"]=TriggerSelector_FrameEnd;
  enumap_TriggerSelector["FrameActive"]=TriggerSelector_FrameActive;
  enumap_TriggerSelector["LineStart"]=TriggerSelector_LineStart;
  enumap_TriggerSelector["ExposureStart"]=TriggerSelector_ExposureStart;
  enumap_TriggerSelector["ExposureEnd"]=TriggerSelector_ExposureEnd;
  enumap_TriggerSelector["ExposureActive"]=TriggerSelector_ExposureActive;
  enumap_TriggerMode["Off"]=TriggerMode_Off;
  enumap_TriggerMode["On"]=TriggerMode_On;
  enumap_TriggerSource["Line1"]=TriggerSource_Line1;
  enumap_TriggerSource["Line2"]=TriggerSource_Line2;
  enumap_TriggerSource["Line3"]=TriggerSource_Line3;
  enumap_TriggerSource["Line4"]=TriggerSource_Line4;
  enumap_TriggerSource["Timer1Start"]=TriggerSource_Timer1Start;
  enumap_TriggerSource["Timer1End"]=TriggerSource_Timer1End;
  enumap_TriggerSource["Counter1Start"]=TriggerSource_Counter1Start;
  enumap_TriggerSource["Counter1End"]=TriggerSource_Counter1End;
  enumap_TriggerSource["UserOutput1"]=TriggerSource_UserOutput1;
  enumap_TriggerSource["UserOutput2"]=TriggerSource_UserOutput2;
  enumap_TriggerSource["Software"]=TriggerSource_Software;
  enumap_TriggerSource["ShaftEncoderModuleOut"]=TriggerSource_ShaftEncoderModuleOut;
  enumap_TriggerActivation["RisingEdge"]=TriggerActivation_RisingEdge;
  enumap_TriggerActivation["FallingEdge"]=TriggerActivation_FallingEdge;
  enumap_TriggerActivation["AnyEdge"]=TriggerActivation_AnyEdge;
  enumap_TriggerActivation["LevelHigh"]=TriggerActivation_LevelHigh;
  enumap_TriggerActivation["LevelLow"]=TriggerActivation_LevelLow;
  enumap_ExposureMode["Off"]=ExposureMode_Off;
  enumap_ExposureMode["Timed"]=ExposureMode_Timed;
  enumap_ExposureMode["TriggerWidth"]=ExposureMode_TriggerWidth;
  enumap_ExposureMode["TriggerControlled"]=ExposureMode_TriggerControlled;
  enumap_ExposureAuto["Off"]=ExposureAuto_Off;
  enumap_ExposureAuto["Once"]=ExposureAuto_Once;
  enumap_ExposureAuto["Continuous"]=ExposureAuto_Continuous;
  enumap_LineSelector["Line1"]=LineSelector_Line1;
  enumap_LineSelector["Line2"]=LineSelector_Line2;
  enumap_LineSelector["Line3"]=LineSelector_Line3;
  enumap_LineSelector["Line4"]=LineSelector_Line4;
  enumap_LineSelector["In1"]=LineSelector_In1;
  enumap_LineSelector["In2"]=LineSelector_In2;
  enumap_LineSelector["In3"]=LineSelector_In3;
  enumap_LineSelector["In4"]=LineSelector_In4;
  enumap_LineSelector["Out1"]=LineSelector_Out1;
  enumap_LineSelector["Out2"]=LineSelector_Out2;
  enumap_LineSelector["Out3"]=LineSelector_Out3;
  enumap_LineSelector["Out4"]=LineSelector_Out4;
  enumap_LineMode["Input"]=LineMode_Input;
  enumap_LineMode["Output"]=LineMode_Output;
  enumap_LineFormat["NoConnect"]=LineFormat_NoConnect;
  enumap_LineFormat["TriState"]=LineFormat_TriState;
  enumap_LineFormat["TTL"]=LineFormat_TTL;
  enumap_LineFormat["LVDS"]=LineFormat_LVDS;
  enumap_LineFormat["RS422"]=LineFormat_RS422;
  enumap_LineFormat["OptoCoupled"]=LineFormat_OptoCoupled;
  enumap_LineSource["Off"]=LineSource_Off;
  enumap_LineSource["ExposureActive"]=LineSource_ExposureActive;
  enumap_LineSource["FrameTriggerWait"]=LineSource_FrameTriggerWait;
  enumap_LineSource["LineTriggerWait"]=LineSource_LineTriggerWait;
  enumap_LineSource["Timer1Active"]=LineSource_Timer1Active;
  enumap_LineSource["Timer2Active"]=LineSource_Timer2Active;
  enumap_LineSource["Timer3Active"]=LineSource_Timer3Active;
  enumap_LineSource["Timer4Active"]=LineSource_Timer4Active;
  enumap_LineSource["TimerActive"]=LineSource_TimerActive;
  enumap_LineSource["UserOutput1"]=LineSource_UserOutput1;
  enumap_LineSource["UserOutput2"]=LineSource_UserOutput2;
  enumap_LineSource["UserOutput3"]=LineSource_UserOutput3;
  enumap_LineSource["UserOutput4"]=LineSource_UserOutput4;
  enumap_LineSource["UserOutput"]=LineSource_UserOutput;
  enumap_LineSource["TriggerReady"]=LineSource_TriggerReady;
  enumap_LineSource["SerialTx"]=LineSource_SerialTx;
  enumap_UserOutputSelector["UserOutput1"]=UserOutputSelector_UserOutput1;
  enumap_UserOutputSelector["UserOutput2"]=UserOutputSelector_UserOutput2;
  enumap_UserOutputSelector["UserOutput3"]=UserOutputSelector_UserOutput3;
  enumap_UserOutputSelector["UserOutput4"]=UserOutputSelector_UserOutput4;
  enumap_ShaftEncoderModuleLineSelector["PhaseA"]=ShaftEncoderModuleLineSelector_PhaseA;
  enumap_ShaftEncoderModuleLineSelector["PhaseB"]=ShaftEncoderModuleLineSelector_PhaseB;
  enumap_ShaftEncoderModuleLineSource["Line1"]=ShaftEncoderModuleLineSource_Line1;
  enumap_ShaftEncoderModuleLineSource["Line2"]=ShaftEncoderModuleLineSource_Line2;
  enumap_ShaftEncoderModuleLineSource["Line3"]=ShaftEncoderModuleLineSource_Line3;
  enumap_ShaftEncoderModuleLineSource["Line4"]=ShaftEncoderModuleLineSource_Line4;
  enumap_ShaftEncoderModuleMode["AnyDirection"]=ShaftEncoderModuleMode_AnyDirection;
  enumap_ShaftEncoderModuleMode["ForwardOnly"]=ShaftEncoderModuleMode_ForwardOnly;
  enumap_ShaftEncoderModuleCounterMode["FollowDirection"]=ShaftEncoderModuleCounterMode_FollowDirection;
  enumap_ShaftEncoderModuleCounterMode["IgnoreDirection"]=ShaftEncoderModuleCounterMode_IgnoreDirection;
  enumap_TimerSelector["Timer1"]=TimerSelector_Timer1;
  enumap_TimerSelector["Timer2"]=TimerSelector_Timer2;
  enumap_TimerSelector["Timer3"]=TimerSelector_Timer3;
  enumap_TimerSelector["Timer4"]=TimerSelector_Timer4;
  enumap_TimerTriggerSource["Off"]=TimerTriggerSource_Off;
  enumap_TimerTriggerSource["ExposureStart"]=TimerTriggerSource_ExposureStart;
  enumap_TimerTriggerActivation["RisingEdge"]=TimerTriggerActivation_RisingEdge;
  enumap_TimerTriggerActivation["FallingEdge"]=TimerTriggerActivation_FallingEdge;
  enumap_TimerTriggerActivation["LevelHigh"]=TimerTriggerActivation_LevelHigh;
  enumap_TimerTriggerActivation["LevelLow"]=TimerTriggerActivation_LevelLow;
  enumap_TimerSequenceEntrySelector["Entry1"]=TimerSequenceEntrySelector_Entry1;
  enumap_TimerSequenceEntrySelector["Entry2"]=TimerSequenceEntrySelector_Entry2;
  enumap_TimerSequenceEntrySelector["Entry3"]=TimerSequenceEntrySelector_Entry3;
  enumap_TimerSequenceEntrySelector["Entry4"]=TimerSequenceEntrySelector_Entry4;
  enumap_TimerSequenceEntrySelector["Entry5"]=TimerSequenceEntrySelector_Entry5;
  enumap_TimerSequenceEntrySelector["Entry6"]=TimerSequenceEntrySelector_Entry6;
  enumap_TimerSequenceEntrySelector["Entry7"]=TimerSequenceEntrySelector_Entry7;
  enumap_TimerSequenceEntrySelector["Entry8"]=TimerSequenceEntrySelector_Entry8;
  enumap_TimerSequenceEntrySelector["Entry9"]=TimerSequenceEntrySelector_Entry9;
  enumap_TimerSequenceEntrySelector["Entry10"]=TimerSequenceEntrySelector_Entry10;
  enumap_TimerSequenceEntrySelector["Entry11"]=TimerSequenceEntrySelector_Entry11;
  enumap_TimerSequenceEntrySelector["Entry12"]=TimerSequenceEntrySelector_Entry12;
  enumap_TimerSequenceEntrySelector["Entry13"]=TimerSequenceEntrySelector_Entry13;
  enumap_TimerSequenceEntrySelector["Entry14"]=TimerSequenceEntrySelector_Entry14;
  enumap_TimerSequenceEntrySelector["Entry15"]=TimerSequenceEntrySelector_Entry15;
  enumap_TimerSequenceEntrySelector["Entry16"]=TimerSequenceEntrySelector_Entry16;
  enumap_TimerSequenceTimerSelector["Timer1"]=TimerSequenceTimerSelector_Timer1;
  enumap_TimerSequenceTimerSelector["Timer2"]=TimerSequenceTimerSelector_Timer2;
  enumap_TimerSequenceTimerSelector["Timer3"]=TimerSequenceTimerSelector_Timer3;
  enumap_TimerSequenceTimerSelector["Timer4"]=TimerSequenceTimerSelector_Timer4;
  enumap_LUTSelector["Luminance"]=LUTSelector_Luminance;
  enumap_UserSetSelector["Default"]=UserSetSelector_Default;
  enumap_UserSetSelector["UserSet1"]=UserSetSelector_UserSet1;
  enumap_UserSetSelector["UserSet2"]=UserSetSelector_UserSet2;
  enumap_UserSetSelector["UserSet3"]=UserSetSelector_UserSet3;
  enumap_UserSetDefaultSelector["Default"]=UserSetDefaultSelector_Default;
  enumap_UserSetDefaultSelector["UserSet1"]=UserSetDefaultSelector_UserSet1;
  enumap_UserSetDefaultSelector["UserSet2"]=UserSetDefaultSelector_UserSet2;
  enumap_UserSetDefaultSelector["UserSet3"]=UserSetDefaultSelector_UserSet3;
  enumap_AutoFunctionAOISelector["AOI1"]=AutoFunctionAOISelector_AOI1;
  enumap_AutoFunctionAOISelector["AOI2"]=AutoFunctionAOISelector_AOI2;
  enumap_AutoFunctionAOISelector["AOI3"]=AutoFunctionAOISelector_AOI3;
  enumap_AutoFunctionAOISelector["AOI4"]=AutoFunctionAOISelector_AOI4;
  enumap_AutoFunctionAOISelector["AOI5"]=AutoFunctionAOISelector_AOI5;
  enumap_AutoFunctionAOISelector["AOI6"]=AutoFunctionAOISelector_AOI6;
  enumap_AutoFunctionAOISelector["AOI7"]=AutoFunctionAOISelector_AOI7;
  enumap_AutoFunctionAOISelector["AOI8"]=AutoFunctionAOISelector_AOI8;
  enumap_ShadingSelector["OffsetShading"]=ShadingSelector_OffsetShading;
  enumap_ShadingSelector["GainShading"]=ShadingSelector_GainShading;
  enumap_ShadingStatus["NoError"]=ShadingStatus_NoError;
  enumap_ShadingStatus["StartupSetError"]=ShadingStatus_StartupSetError;
  enumap_ShadingStatus["ActivateError"]=ShadingStatus_ActivateError;
  enumap_ShadingStatus["CreateError"]=ShadingStatus_CreateError;
  enumap_ShadingSetDefaultSelector["DefaultShadingSet"]=ShadingSetDefaultSelector_DefaultShadingSet;
  enumap_ShadingSetDefaultSelector["UserShadingSet1"]=ShadingSetDefaultSelector_UserShadingSet1;
  enumap_ShadingSetDefaultSelector["UserShadingSet2"]=ShadingSetDefaultSelector_UserShadingSet2;
  enumap_ShadingSetSelector["DefaultShadingSet"]=ShadingSetSelector_DefaultShadingSet;
  enumap_ShadingSetSelector["UserShadingSet1"]=ShadingSetSelector_UserShadingSet1;
  enumap_ShadingSetSelector["UserShadingSet2"]=ShadingSetSelector_UserShadingSet2;
  enumap_UserDefinedValueSelector["Value1"]=UserDefinedValueSelector_Value1;
  enumap_UserDefinedValueSelector["Value2"]=UserDefinedValueSelector_Value2;
  enumap_UserDefinedValueSelector["Value3"]=UserDefinedValueSelector_Value3;
  enumap_UserDefinedValueSelector["Value4"]=UserDefinedValueSelector_Value4;
  enumap_DeviceScanType["Areascan"]=DeviceScanType_Areascan;
  enumap_DeviceScanType["Linescan"]=DeviceScanType_Linescan;
  enumap_TemperatureSelector["Sensorboard"]=TemperatureSelector_Sensorboard;
  enumap_TemperatureSelector["Coreboard"]=TemperatureSelector_Coreboard;
  enumap_TemperatureSelector["Framegrabberboard"]=TemperatureSelector_Framegrabberboard;
  enumap_ParameterSelector["Gain"]=ParameterSelector_Gain;
  enumap_ParameterSelector["Brightness"]=ParameterSelector_Brightness;
  enumap_ParameterSelector["ExposureTime"]=ParameterSelector_ExposureTime;
  enumap_ExpertFeatureAccessSelector["ExpertFeature1"]=ExpertFeatureAccessSelector_ExpertFeature1;
  enumap_ExpertFeatureAccessSelector["ExpertFeature2"]=ExpertFeatureAccessSelector_ExpertFeature2;
  enumap_ExpertFeatureAccessSelector["ExpertFeature3"]=ExpertFeatureAccessSelector_ExpertFeature3;
  enumap_ExpertFeatureAccessSelector["ExpertFeature4"]=ExpertFeatureAccessSelector_ExpertFeature4;
  enumap_PixelStepCorrectionSelector["Tap1"]=PixelStepCorrectionSelector_Tap1;
  enumap_PixelStepCorrectionSelector["Tap2"]=PixelStepCorrectionSelector_Tap2;
  enumap_PixelStepCorrectionSelector["Tap3"]=PixelStepCorrectionSelector_Tap3;
  enumap_PixelStepCorrectionSelector["Tap4"]=PixelStepCorrectionSelector_Tap4;
  enumap_ChunkSelector["Image"]=ChunkSelector_Image;
  enumap_ChunkSelector["OffsetX"]=ChunkSelector_OffsetX;
  enumap_ChunkSelector["OffsetY"]=ChunkSelector_OffsetY;
  enumap_ChunkSelector["Width"]=ChunkSelector_Width;
  enumap_ChunkSelector["Height"]=ChunkSelector_Height;
  enumap_ChunkSelector["PixelFormat"]=ChunkSelector_PixelFormat;
  enumap_ChunkSelector["DynamicRangeMax"]=ChunkSelector_DynamicRangeMax;
  enumap_ChunkSelector["DynamicRangeMin"]=ChunkSelector_DynamicRangeMin;
  enumap_ChunkSelector["Timestamp"]=ChunkSelector_Timestamp;
  enumap_ChunkSelector["LineStatusAll"]=ChunkSelector_LineStatusAll;
  enumap_ChunkSelector["Framecounter"]=ChunkSelector_Framecounter;
  enumap_ChunkSelector["Triggerinputcounter"]=ChunkSelector_Triggerinputcounter;
  enumap_ChunkSelector["LineTriggerIgnoredCounter"]=ChunkSelector_LineTriggerIgnoredCounter;
  enumap_ChunkSelector["FrameTriggerIgnoredCounter"]=ChunkSelector_FrameTriggerIgnoredCounter;
  enumap_ChunkSelector["LineTriggerEndToEndCounter"]=ChunkSelector_LineTriggerEndToEndCounter;
  enumap_ChunkSelector["FrameTriggerCounter"]=ChunkSelector_FrameTriggerCounter;
  enumap_ChunkSelector["FramesPerTriggerCounter"]=ChunkSelector_FramesPerTriggerCounter;
  enumap_ChunkSelector["PayloadCRC16"]=ChunkSelector_PayloadCRC16;
  enumap_ChunkPixelFormat["Mono8"]=ChunkPixelFormat_Mono8;
  enumap_ChunkPixelFormat["Mono8Signed"]=ChunkPixelFormat_Mono8Signed;
  enumap_ChunkPixelFormat["Mono10"]=ChunkPixelFormat_Mono10;
  enumap_ChunkPixelFormat["Mono10Packed"]=ChunkPixelFormat_Mono10Packed;
  enumap_ChunkPixelFormat["Mono12"]=ChunkPixelFormat_Mono12;
  enumap_ChunkPixelFormat["Mono12Packed"]=ChunkPixelFormat_Mono12Packed;
  enumap_ChunkPixelFormat["Mono16"]=ChunkPixelFormat_Mono16;
  enumap_ChunkPixelFormat["BayerGR8"]=ChunkPixelFormat_BayerGR8;
  enumap_ChunkPixelFormat["BayerRG8"]=ChunkPixelFormat_BayerRG8;
  enumap_ChunkPixelFormat["BayerGB8"]=ChunkPixelFormat_BayerGB8;
  enumap_ChunkPixelFormat["BayerBG8"]=ChunkPixelFormat_BayerBG8;
  enumap_ChunkPixelFormat["BayerGR10"]=ChunkPixelFormat_BayerGR10;
  enumap_ChunkPixelFormat["BayerRG10"]=ChunkPixelFormat_BayerRG10;
  enumap_ChunkPixelFormat["BayerGB10"]=ChunkPixelFormat_BayerGB10;
  enumap_ChunkPixelFormat["BayerBG10"]=ChunkPixelFormat_BayerBG10;
  enumap_ChunkPixelFormat["BayerGR12"]=ChunkPixelFormat_BayerGR12;
  enumap_ChunkPixelFormat["BayerRG12"]=ChunkPixelFormat_BayerRG12;
  enumap_ChunkPixelFormat["BayerGB12"]=ChunkPixelFormat_BayerGB12;
  enumap_ChunkPixelFormat["BayerBG12"]=ChunkPixelFormat_BayerBG12;
  enumap_ChunkPixelFormat["RGB8Packed"]=ChunkPixelFormat_RGB8Packed;
  enumap_ChunkPixelFormat["BGR8Packed"]=ChunkPixelFormat_BGR8Packed;
  enumap_ChunkPixelFormat["RGBA8Packed"]=ChunkPixelFormat_RGBA8Packed;
  enumap_ChunkPixelFormat["BGRA8Packed"]=ChunkPixelFormat_BGRA8Packed;
  enumap_ChunkPixelFormat["RGB10Packed"]=ChunkPixelFormat_RGB10Packed;
  enumap_ChunkPixelFormat["BGR10Packed"]=ChunkPixelFormat_BGR10Packed;
  enumap_ChunkPixelFormat["RGB12Packed"]=ChunkPixelFormat_RGB12Packed;
  enumap_ChunkPixelFormat["BGR12Packed"]=ChunkPixelFormat_BGR12Packed;
  enumap_ChunkPixelFormat["RGB10V1Packed"]=ChunkPixelFormat_RGB10V1Packed;
  enumap_ChunkPixelFormat["RGB10V2Packed"]=ChunkPixelFormat_RGB10V2Packed;
  enumap_ChunkPixelFormat["YUV411Packed"]=ChunkPixelFormat_YUV411Packed;
  enumap_ChunkPixelFormat["YUV422Packed"]=ChunkPixelFormat_YUV422Packed;
  enumap_ChunkPixelFormat["YUV444Packed"]=ChunkPixelFormat_YUV444Packed;
  enumap_ChunkPixelFormat["RGB8Planar"]=ChunkPixelFormat_RGB8Planar;
  enumap_ChunkPixelFormat["RGB10Planar"]=ChunkPixelFormat_RGB10Planar;
  enumap_ChunkPixelFormat["RGB12Planar"]=ChunkPixelFormat_RGB12Planar;
  enumap_ChunkPixelFormat["RGB16Planar"]=ChunkPixelFormat_RGB16Planar;
  enumap_ChunkPixelFormat["YUV422Packed_AlternateByteOrder"]=ChunkPixelFormat_YUV422Packed_AlternateByteOrder;
  enumap_ChunkPixelFormat["BayerGB12Packed"]=ChunkPixelFormat_BayerGB12Packed;
  enumap_ChunkPixelFormat["BayerGR12Packed"]=ChunkPixelFormat_BayerGR12Packed;
  enumap_ChunkPixelFormat["BayerRG12Packed"]=ChunkPixelFormat_BayerRG12Packed;
  enumap_ChunkPixelFormat["BayerBG12Packed"]=ChunkPixelFormat_BayerBG12Packed;
  enumap_ChunkPixelFormat["RGB12V1Packed"]=ChunkPixelFormat_RGB12V1Packed;
  enumap_EventSelector["ExposureEnd"]=EventSelector_ExposureEnd;
  enumap_EventSelector["LineStartOvertrigger"]=EventSelector_LineStartOvertrigger;
  enumap_EventSelector["FrameStartOvertrigger"]=EventSelector_FrameStartOvertrigger;
  enumap_EventSelector["EventOverrun"]=EventSelector_EventOverrun;
  enumap_EventNotification["Off"]=EventNotification_Off;
  enumap_EventNotification["GenICamEvent"]=EventNotification_GenICamEvent;
  enumap_FileSelector["UserSet1"]=FileSelector_UserSet1;
  enumap_FileSelector["UserSet2"]=FileSelector_UserSet2;
  enumap_FileSelector["UserSet3"]=FileSelector_UserSet3;
  enumap_FileSelector["UserGainShading1"]=FileSelector_UserGainShading1;
  enumap_FileSelector["UserGainShading2"]=FileSelector_UserGainShading2;
  enumap_FileOperationSelector["Open"]=FileOperationSelector_Open;
  enumap_FileOperationSelector["Close"]=FileOperationSelector_Close;
  enumap_FileOperationSelector["Read"]=FileOperationSelector_Read;
  enumap_FileOperationSelector["Write"]=FileOperationSelector_Write;
  enumap_FileOpenMode["Read"]=FileOpenMode_Read;
  enumap_FileOpenMode["Write"]=FileOpenMode_Write;
  enumap_FileOperationStatus["Success"]=FileOperationStatus_Success;
  enumap_FileOperationStatus["Failure"]=FileOperationStatus_Failure;
  enumap_GevInterfaceSelector["NetworkInterface0"]=GevInterfaceSelector_NetworkInterface0;
  enumap_GevCCP["Exclusive"]=GevCCP_Exclusive;
  enumap_GevCCP["Control"]=GevCCP_Control;
  enumap_GevCCP["ExclusiveControl"]=GevCCP_ExclusiveControl;
  enumap_GevStreamChannelSelector["StreamChannel0"]=GevStreamChannelSelector_StreamChannel0;

}
gem::Properties&gem::pylon::cameraproperties::getKeys(void) {
  gem::Properties&result=readprops; result.clear();
  gem::pylon::cameraproperties::init();

  do {
    gem::any typevalue=1;
    std::map<std::string, t_getbool>::iterator it;
    for(it=map_getbool.begin(); it!=map_getbool.end(); ++it) {
      result.set(it->first, typevalue);
    }
  } while(0);

  do {
    gem::any typevalue=0;
    std::map<std::string, t_getint>::iterator it;
    for(it=map_getint.begin(); it!=map_getint.end(); ++it) {
      result.set(it->first, typevalue);
    }
  } while(0);

  do {
    gem::any typevalue=0;
    std::map<std::string, t_getfloat>::iterator it;
    for(it=map_getfloat.begin(); it!=map_getfloat.end(); ++it) {
      result.set(it->first, typevalue);
    }
  } while(0);

  do {
    gem::any typevalue=std::string("symbol");
    std::map<std::string, t_getstring>::iterator it;
    for(it=map_getstring.begin(); it!=map_getstring.end(); ++it) {
      result.set(it->first, typevalue);
    }
  } while(0);

#if 0
  do {
    gem::any typevalue;
    std::map<std::command, t_getcommand>::iterator it;
    for(it=map_getcommand.begin(); it!=map_getcommand.end(); ++it) {
      result.set(it->first, typevalue);
    }
  } while(0);
#endif

  do {
    gem::any typevalue=std::string("symbol");
    std::map<std::string, t_getenum>::iterator it;
    for(it=map_getenum.begin(); it!=map_getenum.end(); ++it) {
      result.set(it->first, typevalue);
    }
  } while(0);

  return result;
}
gem::Properties&gem::pylon::cameraproperties::setKeys(void) {
  gem::Properties&result=writeprops; result.clear();
  gem::pylon::cameraproperties::init();

  do {
    gem::any typevalue=1;
    std::map<std::string, t_setbool>::iterator it;
    for(it=map_setbool.begin(); it!=map_setbool.end(); ++it) {
      result.set(it->first, typevalue);
    }
  } while(0);

  do {
    gem::any typevalue=0;
    std::map<std::string, t_setint>::iterator it;
    for(it=map_setint.begin(); it!=map_setint.end(); ++it) {
      result.set(it->first, typevalue);
    }
  } while(0);

  do {
    gem::any typevalue=0;
    std::map<std::string, t_setfloat>::iterator it;
    for(it=map_setfloat.begin(); it!=map_setfloat.end(); ++it) {
      result.set(it->first, typevalue);
    }
  } while(0);

  do {
    gem::any typevalue=std::string("symbol");
    std::map<std::string, t_setstring>::iterator it;
    for(it=map_setstring.begin(); it!=map_setstring.end(); ++it) {
      result.set(it->first, typevalue);
    }
  } while(0);

  do {
    gem::any typevalue;
    std::map<std::string, t_setcommand>::iterator it;
    for(it=map_setcommand.begin(); it!=map_setcommand.end(); ++it) {
      result.set(it->first, typevalue);
    }
  } while(0);

  do {
    gem::any typevalue=std::string("symbol");
    std::map<std::string, t_setenum>::iterator it;
    for(it=map_setenum.begin(); it!=map_setenum.end(); ++it) {
      result.set(it->first, typevalue);
    }
  } while(0);

  return result;
}

void gem::pylon::cameraproperties::get(Pylon::CBaslerGigECamera*device, 
                                       std::string key,
                                       gem::any&result)
{
  gem::pylon::cameraproperties::init();

  /* compatibility with other backends */
  if (key=="width") { /* for compat with other backends */
    result=static_cast<double>(device->Width.GetValue());
    return;

  } else if (key=="height") { /* for compat with other backends */
    result=static_cast<double>(device->Height.GetValue());
    return;
  }


  do {
    std::map<std::string, t_getbool>::iterator it=map_getbool.find(key);
    if(it!=map_getbool.end()) {
      result=static_cast<double>(it->second(device));
    }
  } while(0);
  do {
    std::map<std::string, t_getint>::iterator it=map_getint.find(key);
    if(it!=map_getint.end()) {
      result=static_cast<double>(it->second(device));
    }
  } while(0);
  do {
    std::map<std::string, t_getfloat>::iterator it=map_getfloat.find(key);
    if(it!=map_getfloat.end()) {
      result=static_cast<double>(it->second(device));
    }
  } while(0);
  do {
    std::map<std::string, t_getstring>::iterator it=map_getstring.find(key);
    if(it!=map_getstring.end()) {
      result=std::string((it->second(device)).c_str());
    }
  } while(0);
  do {
    std::map<std::string, t_getenum>::iterator it=map_getenum.find(key);
    if(it!=map_getenum.end()) {
      result=it->second(device);
    }
  } while(0);
}


bool gem::pylon::cameraproperties::set(Pylon::CBaslerGigECamera*device, 
                                       std::string key,
                                       gem::Properties&props)
{
  gem::pylon::cameraproperties::init();
  double d;
  std::string s;

  /* compatibility with other APIs */
  if (key=="width") {
    if(props.get(key, d)) {
      device->Width.SetValue(d);
    }
    return true;
  } else if (key=="height") {
    if(props.get(key, d)) {
      device->Height.SetValue(d);
    }
    return true;
  }


  std::map<std::string, t_setbool>::iterator it_b=map_setbool.find(key);
  if(it_b != map_setbool.end()) {
    if(props.get(key, d)) {
      it_b->second(device, static_cast<bool>(d));
      return true;
    } else {
      return false;
    }
  }
  std::map<std::string, t_setint>::iterator it_i=map_setint.find(key);
  if(it_i != map_setint.end()) {
    if(props.get(key, d)) {
      it_i->second(device, static_cast<int64_t>(d));
      return true;
    } else {
      return false;
    }
  }
  std::map<std::string, t_setfloat>::iterator it_f=map_setfloat.find(key);
  if(it_f != map_setfloat.end()) {
    if(props.get(key, d)) {
      it_f->second(device, static_cast<double>(d));
      return true;
    } else {
      return false;
    }
  }

  std::map<std::string, t_setstring>::iterator it_s=map_setstring.find(key);
  if(it_s != map_setstring.end()) {
    if(props.get(key, s)) {
      GenICam::gcstring gs(s.c_str());
      it_s->second(device, gs);
      return true;
    } else {
      return false;
    }
  }
  std::map<std::string, t_setcommand>::iterator it_c=map_setcommand.find(key);
  if(it_c != map_setcommand.end()) {
    it_c->second(device);
    props.erase(key);
  }

  std::map<std::string, t_setenum>::iterator it_e=map_setenum.find(key);
  if(it_e != map_setenum.end()) {
    if(props.get(key, s)) {
      it_e->second(device, s);
    } else if(props.get(key, d)) {
      std::map<std::string, t_setenumi>::iterator it_ei=map_setenumi.find(key);
      if(it_ei != map_setenumi.end()) {
        it_ei->second(device, d);
      }   
    }
    return true;
  }

  return false;
}
#endif /* HAVE_PYLON */
