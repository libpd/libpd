#ifdef NT
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the MJLIB_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// MJLIB_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef MJLIB_EXPORTS
#define MJLIB_API __declspec(dllexport)
#else
#define MJLIB_API __declspec(dllimport)
#endif

// This class is exported from the mjLib.dll
//class MJLIB_API CMjLib {
//public:
//	CMjLib(void);
	// TODO: add your methods here.
//};

//extern MJLIB_API int nMjLib;

//MJLIB_API int fnMjLib(void);

 __declspec(dllexport) void mjLib_setup( void );
 
 #endif
 
 