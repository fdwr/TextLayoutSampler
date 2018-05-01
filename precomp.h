// include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//#pragma once
#pragma once

// Disable unknown pragma warnings (for when when compiling directly with VC)
// and not under build.exe.
#pragma warning(disable: 4068)

// declaration of 'thing' hides class member
// Even Windows public header files have this problem. So silence the noise to reveal more pertinent warings.
#pragma warning(disable:4458)

// Flexible array members in struct/union are perfectly fine and supported in all major compilers.
#pragma warning(disable:4200)

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER              // Allow use of features specific to Windows XP or later.
#define WINVER NTDDI_WINBLUE// 0x0601       // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT        // Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT NTDDI_WIN7// 0x0601 // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINDOWS      // Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS NTDDI_WINBLUE// 0x0601 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE           // Allow use of features specific to IE 7.0 or later.
#define _WIN32_IE 0x0700    // Change this to the appropriate value to target other versions of IE.
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#define NOMINMAX
#endif

#ifndef GDIPVER
// Want the latest GDI+ 1.1 version.
#define GDIPVER 0x0110
#endif

// The C++0x keyword is supported in Visual Studio 2010
#if _MSC_VER < 1600
#ifndef nullptr
#define nullptr 0
#endif
#endif

#define _USE_MATH_DEFINES

//////////////////////////////
// C RunTime Header Files
#define _SCL_SECURE_NO_WARNINGS // I hate doing this, but Visual Studio offers no substitute for std::uninitialized_copy.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <math.h>
#include <new>
#include <algorithm>
#include <numeric>
#include <strsafe.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <wchar.h>
#include <vector>
#include <string>
#include <functional>
#include <map>

//////////////////////////////
// Windows Header Files:

#include <sdkddkver.h>

#ifndef NTDDI_WIN10_RS3
#define NTDDI_WIN10_RS3                     0x0A000004  /* ABRACADABRA_WIN10_RS3 */
#endif

#include <windows.h>
#include <windowsx.h>
#include <shlobj.h>
#include <Knownfolders.h> // for font folder path
#include <CommDlg.h>
#include <ShellApi.h>

#if NTDDI_VERSION >= NTDDI_WIN10_RS3

#include <DWrite_3.h>
#include <D2D1_3.h>
#include <D2D1_3Helper.h>
#include <D2D1Helper.h>
#include <WinCodec.h>

#include <DCommon.h>
#include <DWrite_1.h>
#include <DWrite_2.h>
#include <DWrite_3.h>
#include <D2DBaseTypes.h>
#include <D2D1_1.h>
#include <D2D1_2.h>
#include <D2D1_3.h>
#include <D2D1_1helper.h>
#include <D2D1_2helper.h>
#include <D2D1_3helper.h>

#else // Older SDK. So use newer versions of the 2D headers.

#undef NTDDI_VERSION
#define NTDDI_VERSION NTDDI_WIN10_RS3

#include "Windows2DHeaders14393/D2D1Helper.h"
#include "Windows2DHeaders14393/D2D1_1helper.h"
#include "Windows2DHeaders14393/D2D1_2helper.h"
#include "Windows2DHeaders14393/D2D1_3helper.h"
#include "Windows2DHeaders14393/DCommon.h"
#include "Windows2DHeaders14393/DWrite_1.h"
#include "Windows2DHeaders14393/DWrite_2.h"
#include "Windows2DHeaders14393/DWrite_3.h"
#include "Windows2DHeaders14393/D2DBaseTypes.h"
#include "Windows2DHeaders14393/D2D1.h"
#include "Windows2DHeaders14393/D2D1_1.h"
#include "Windows2DHeaders14393/D2D1_2.h"
#include "Windows2DHeaders14393/D2D1_3.h"

#endif

#include <DxgiFormat.h>
#include <DxgiType.h>
#include <Dxgi.h>

#include <WinCodec.h>

#include <usp10.h>

#define min std::min
#define max std::max
#include <gdiplus.h>
#undef min
#undef max

//////////////////////////////
// Generic.

#include "Common.h"

void DebugLog(const wchar_t* logMessage, ...);

//////////////////////////////

// Need this for common controls.
// Otherwise the app either looks ugly,
// or it doesn't show anything at all
// (except an empty window).
#ifdef _UNICODE
    #if defined _M_IX86
        #pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")

    #elif defined _M_IA64
        #pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")

    #elif defined _M_X64
        #pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")

    #else
        #pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

    #endif
#endif
