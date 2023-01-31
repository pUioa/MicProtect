#include "Globals.h"
//Windows_UnKnow,
//Windows_7_7600,     //Windows 7
//Windows_7_7601,     //Windows 7
//Windows_10_10240,	//Windows10 1507
//Windows_10_10586,	//Windows10 1511
//Windows_10_14393,	//Windows10 1607
//Windows_10_15063,	//Windows10 1703
//Windows_10_16299,	//Windows10 1709
//Windows_10_17134,	//Windows10 1803
//Windows_10_17763,	//Windows10 1809
//Windows_10_18362,	//Windows10 1903
//Windows_10_18363,	//Windows10 1909
//Windows_10_19041,	//Windows10 2004
//Windows_10_19042,	//Windows10 2009
//Windows_10_19043,	//Windows10 2104
//Windows_10_19044,	//Windows10 2109
//Windows_10_22000,	//Windows11
WindowsVersion GetWindowsVersion() {
	WindowsVersion Version = Windows_UnKnow;

	RTL_OSVERSIONINFOW VersionInfo;
	if (NT_SUCCESS(RtlGetVersion(&VersionInfo))) {
		switch (VersionInfo.dwBuildNumber)
		{
		case 7600:
			Version = Windows_7_7600;
			break;
		case 7601:
			Version = Windows_7_7601;
			break;
		case 10240:
			Version = Windows_10_10240;
			break;
		case 10586:
			Version = Windows_10_10586;
			break;
		case 14393:
			Version = Windows_10_14393;
			break;
		case 15063:
			Version = Windows_10_15063;
			break;
		case 16299:
			Version = Windows_10_16299;
		case 17134:
			Version = Windows_10_17134;
		case 17763:
			Version = Windows_10_17763;
		case 18362:
			Version = Windows_10_18362;
		case 18363:
			Version = Windows_10_18363;
		case 19041:
			Version = Windows_10_19041;
		case 19042:
			Version = Windows_10_19042;
		case 19043:
			Version = Windows_10_19043;
		case 19044:
			Version = Windows_10_19044;
		case 22000:
			Version = Windows_10_22000;
		default:
			break;
		}

	}

	return Version;
}