////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file:
//  handling of getting/setting pylon/baslergigestreamgrabber properties
//
//    Copyright (c) 2010-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "LICENSE.txt"
//
/////////////////////////////////////////////////////////

// get StreamGrabber attributes
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_PYLON
#include "StreamGrabberProperties.h"
#include "map"

namespace gem{namespace pylon{namespace streamgrabberproperties{
  static gem::Properties writeprops, readprops;

  typedef Pylon::CBaslerGigEStreamGrabber DEVICE;

#define GETSETBOOL(T)                                                   \
  static bool get##T(DEVICE*device) { return device->T.GetValue(); }    \
    static void set##T(DEVICE*device, const bool v) { device->T.SetValue(v); }

#define GETSETINT(T)                                                    \
  static int64_t get##T(DEVICE*device) { return device->T.GetValue(); } \
    static void set##T(DEVICE*device, const int64_t v) { device->T.SetValue(v); }


#define GETSETFLOAT(T)                                                  \
  static double get##T(DEVICE*device) { return device->T.GetValue(); }  \
    static void set##T(DEVICE*device, const double v) { device->T.SetValue(v); }

#define GETSETSTRING(T)                                                 \
  static void set##T(DEVICE*device, const GenICam::gcstring v) { device->T.SetValue(v); } \
    static GenICam::gcstring get##T(DEVICE*device) { return device->T.GetValue(); }

#define GETSETCOMMAND(T)                                      \
  static void set##T(DEVICE*device) { device->T.Execute(); }

#define GETSETENUM(T)                                                   \
  std::map<std::string, enum T##Enums> enumap_##T;                      \
    static std::string get##T (DEVICE*device) {                         \
      const enum T##Enums v=device->T.GetValue();                       \
      std::map<std::string, T##Enums>::iterator it;                     \
      for (it=enumap_##T.begin(); it != enumap_##T.end(); ++it) {       \
        if(v==it->second)                                               \
          return (it->first);                                           \
      }                                                                 \
      return std::string("unknown");                                    \
    }                                                                   \
    static void setS_##T (DEVICE*device, const std::string s) {         \
      std::map<std::string,  T##Enums>::iterator it=enumap_##T.find(s); \
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
        const enum T##Enums v=static_cast<T##Enums>(i);                 \
        device->T.SetValue(v);                                          \
      }                                                                 \
    }

  typedef bool (*t_getbool)(DEVICE*device);
  std::map<std::string, t_getbool>map_getbool;

  typedef void (*t_setbool)(DEVICE*device, const bool);
  std::map<std::string, t_setbool>map_setbool;

  GETSETBOOL(EnableResend);
  GETSETBOOL(ReceiveThreadPriorityOverride);


  typedef int64_t (*t_getint)(DEVICE*device);
  std::map<std::string, t_getint>map_getint;
  typedef void (*t_setint)(DEVICE*device, const int64_t);
  std::map<std::string, t_setint>map_setint;

  GETSETINT(MaxNumBuffer);
  GETSETINT(MaxBufferSize);
  GETSETINT(PacketTimeout);
  GETSETINT(ReceiveWindowSize);
  GETSETINT(ResendRequestThreshold);
  GETSETINT(ResendRequestBatching);
  GETSETINT(ResendTimeout);
  GETSETINT(ResendRequestResponseTimeout);
  GETSETINT(MaximumNumberResendRequests);
  GETSETINT(FrameRetention);
  GETSETINT(ReceiveThreadPriority);
  GETSETINT(SocketBufferSize);
  GETSETINT(TypeIsWindowsIntelPerformanceDriverAvailable);
  GETSETINT(TypeIsWindowsFilterDriverAvailable);
  GETSETINT(TypeIsSocketDriverAvailable);
  GETSETINT(Statistic_Total_Buffer_Count);
  GETSETINT(Statistic_Failed_Buffer_Count);
  GETSETINT(Statistic_Buffer_Underrun_Count);
  GETSETINT(Statistic_Total_Packet_Count);
  GETSETINT(Statistic_Failed_Packet_Count);
  GETSETINT(Statistic_Resend_Request_Count);
  GETSETINT(Statistic_Resend_Packet_Count);
  GETSETINT(DestinationPort);


  typedef GenICam::gcstring (*t_getstring)(DEVICE*device);
  std::map<std::string, t_getstring>map_getstring;
  typedef void (*t_setstring)(DEVICE*device, GenICam::gcstring);
  std::map<std::string, t_setstring>map_setstring;
  GETSETSTRING(DestinationAddr);


  typedef std::string (*t_getenum)(DEVICE*device);
  typedef void (*t_setenum)(DEVICE*device, const std::string);
  typedef void (*t_setenumi)(DEVICE*device, const int);
  std::map<std::string, t_getenum>map_getenum; 
  std::map<std::string, t_setenum>map_setenum; 
  std::map<std::string, t_setenumi>map_setenumi;

  using namespace Basler_GigEStreamParams;

  GETSETENUM(Status);
  GETSETENUM(AccessMode);
  GETSETENUM(TransmissionType);
};};};



void gem::pylon::streamgrabberproperties::init() {
  static bool initialized=false;
  if(initialized)return;
  initialized=true;


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
  MAP_GETSETBOOL(EnableResend);
  MAP_GETSETBOOL(ReceiveThreadPriorityOverride);

  map_getint.clear();
  map_setint.clear();  
  MAP_GETSETINT(MaxNumBuffer);
  MAP_GETSETINT(MaxBufferSize);
  MAP_GETSETINT(PacketTimeout);
  MAP_GETSETINT(ReceiveWindowSize);
  MAP_GETSETINT(ResendRequestThreshold);
  MAP_GETSETINT(ResendRequestBatching);
  MAP_GETSETINT(ResendTimeout);
  MAP_GETSETINT(ResendRequestResponseTimeout);
  MAP_GETSETINT(MaximumNumberResendRequests);
  MAP_GETSETINT(FrameRetention);
  MAP_GETSETINT(ReceiveThreadPriority);
  MAP_GETSETINT(SocketBufferSize);
  MAP_GETSETINT(TypeIsWindowsIntelPerformanceDriverAvailable);
  MAP_GETSETINT(TypeIsWindowsFilterDriverAvailable);
  MAP_GETSETINT(TypeIsSocketDriverAvailable);
  MAP_GETSETINT(Statistic_Total_Buffer_Count);
  MAP_GETSETINT(Statistic_Failed_Buffer_Count);
  MAP_GETSETINT(Statistic_Buffer_Underrun_Count);
  MAP_GETSETINT(Statistic_Total_Packet_Count);
  MAP_GETSETINT(Statistic_Failed_Packet_Count);
  MAP_GETSETINT(Statistic_Resend_Request_Count);
  MAP_GETSETINT(Statistic_Resend_Packet_Count);
  MAP_GETSETINT(DestinationPort);



  map_getstring.clear();
  map_setstring.clear();
  MAP_GETSETSTRING(DestinationAddr);


  map_getenum.clear();
  map_setenum.clear();
  map_setenumi.clear();
  MAP_GETSETENUM(Status);
  MAP_GETSETENUM(AccessMode);
  MAP_GETSETENUM(TransmissionType);


  enumap_Status.clear();
  enumap_AccessMode.clear();
  enumap_TransmissionType.clear();

  enumap_Status["NotInitialized"]=Status_NotInitialized;
  enumap_Status["Closed"]=Status_Closed;
  enumap_Status["Open"]=Status_Open;
  enumap_Status["Locked"]=Status_Locked;

  enumap_AccessMode["NotInitialized"]=AccessMode_NotInitialized;
  enumap_AccessMode["Monitor"]=AccessMode_Monitor;
  enumap_AccessMode["Control"]=AccessMode_Control;
  enumap_AccessMode["Exclusive"]=AccessMode_Exclusive;

  enumap_TransmissionType["UseCameraConfig"]=TransmissionType_UseCameraConfig;
  enumap_TransmissionType["Unicast"]=TransmissionType_Unicast;
  enumap_TransmissionType["Multicast"]=TransmissionType_Multicast;
  enumap_TransmissionType["LimitedBroadcast"]=TransmissionType_LimitedBroadcast;
  enumap_TransmissionType["SubnetDirectedBroadcast"]=TransmissionType_SubnetDirectedBroadcast;


}


gem::Properties&gem::pylon::streamgrabberproperties::getKeys(void) {
  gem::Properties&result=readprops; result.clear();

  gem::pylon::streamgrabberproperties::init();
  
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
    gem::any typevalue=std::string("symbol");
    std::map<std::string, t_getstring>::iterator it;
    for(it=map_getstring.begin(); it!=map_getstring.end(); ++it) {
      result.set(it->first, typevalue);
    }
  } while(0);
  
  /* enumeration */
  do {
    gem::any typevalue=std::string("symbol");
    std::map<std::string, t_setenum>::iterator it;
    for(it=map_setenum.begin(); it!=map_setenum.end(); ++it) {
      result.set(it->first, typevalue);
    }
  } while(0);

  return result;
}
gem::Properties&gem::pylon::streamgrabberproperties::setKeys(void) {
  gem::Properties&result=writeprops; result.clear();
  gem::pylon::streamgrabberproperties::init();

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
    gem::any typevalue=std::string("symbol");
    std::map<std::string, t_setstring>::iterator it;
    for(it=map_setstring.begin(); it!=map_setstring.end(); ++it) {
      result.set(it->first, typevalue);
    }
  } while(0);

  /* enumeration */
  do {
    gem::any typevalue=std::string("symbol");
    std::map<std::string, t_setenum>::iterator it;
    for(it=map_setenum.begin(); it!=map_setenum.end(); ++it) {
      result.set(it->first, typevalue);
    }
  } while(0);

  return result;
}

void gem::pylon::streamgrabberproperties::get(DEVICE*device, 
                                              std::string key,
                                              gem::any&result)
{
  gem::pylon::streamgrabberproperties::init();

  std::map<std::string, t_getbool>::iterator it_b=map_getbool.find(key);
  if(it_b != map_getbool.end()) {
    result=static_cast<double>(it_b->second(device));
    return;
  }
  std::map<std::string, t_getint>::iterator it_i=map_getint.find(key);
  if(it_i != map_getint.end()) {
    result=static_cast<double>(it_i->second(device));
    return;
  }
  std::map<std::string, t_getstring>::iterator it_s=map_getstring.find(key);
  if(it_s != map_getstring.end()) {
    result=std::string((it_s->second(device)).c_str());
    return;
  }

  std::map<std::string, t_getenum>::iterator it=map_getenum.find(key);
  if(it!=map_getenum.end()) {
    result=it->second(device);
  }
}
// set StreamGrabber attributes

bool gem::pylon::streamgrabberproperties::set(DEVICE*device, 
                                              std::string key,
                                              gem::Properties&props)
{
  double d;
  std::string s;

  gem::pylon::streamgrabberproperties::init();


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
