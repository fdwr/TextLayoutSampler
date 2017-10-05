//----------------------------------------------------------------------------
//  Author:     Dwayne Robinson (dwayner@microsoft.com)
//  History:    2012-12-14 Created
//----------------------------------------------------------------------------
#pragma once


#ifndef SetWindowStyle
#define SetWindowStyle(hwnd, value)    ((DWORD)SetWindowLong(hwnd, GWL_STYLE, value))
#endif

#ifndef LVN_GETEMPTYTEXTW
#define LVN_GETEMPTYTEXTA          (LVN_FIRST-60)
#define LVN_GETEMPTYTEXTW          (LVN_FIRST-61)

#ifdef UNICODE
#define LVN_GETEMPTYTEXT           LVN_GETEMPTYTEXTW
#else
#define LVN_GETEMPTYTEXT           LVN_GETEMPTYTEXTA
#endif
#endif

#ifndef LVM_RESETEMPTYTEXT
#define LVM_RESETEMPTYTEXT         0x1054
#endif
