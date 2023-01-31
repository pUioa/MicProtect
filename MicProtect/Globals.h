#include <ntddk.h>
#include <windef.h>
#include <stdlib.h>


#define DEVICE_NAME L"\\Device\\MicProtect"
#define LINK_NAME L"\\DosDevices\\MicProtect"
#define LINK_GLOBAL_NAME L"\\DosDevices\\Global\\MicProtect"

typedef enum WindowsVersion {
	Windows_UnKnow,
	Windows_7_7600,     //Windows 7
	Windows_7_7601,     //Windows 7
	Windows_10_10240,	//Windows10 1507
	Windows_10_10586,	//Windows10 1511
	Windows_10_14393,	//Windows10 1607
	Windows_10_15063,	//Windows10 1703
	Windows_10_16299,	//Windows10 1709
	Windows_10_17134,	//Windows10 1803
	Windows_10_17763,	//Windows10 1809
	Windows_10_18362,	//Windows10 1903
	Windows_10_18363,	//Windows10 1909
	Windows_10_19041,	//Windows10 2004
	Windows_10_19042,	//Windows10 2009
	Windows_10_19043,	//Windows10 2104
	Windows_10_19044,	//Windows10 2109
	Windows_10_22000,	//Windows11
}WindowsVersion;
WindowsVersion GetWindowsVersion();