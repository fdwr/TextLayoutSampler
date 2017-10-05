//----------------------------------------------------------------------------
//  Author:     Dwayne Robinson
//  History:    2015-06-19 Created
//----------------------------------------------------------------------------
#pragma once

#ifndef BUILD_OPTIMIZATION_STRING
#if defined(DEBUG) || defined(_DEBUG)
#define BUILD_OPTIMIZATION_STRING L"Debug"
#else
#define BUILD_OPTIMIZATION_STRING L"Release"
#endif
#endif

#if defined(_M_IX86)
#define BUILD_ARCHITECTURE_STRING L"x86 32-bit"
#elif defined(_M_X64)
#define BUILD_ARCHITECTURE_STRING L"64-bit"
#elif defined(_M_ARM_FP)
#define BUILD_ARCHITECTURE_STRING L"ARM"
#else
#define BUILD_ARCHITECTURE_STRING L"Unknown"
#endif

#ifndef NDEBUG
#define DEBUG_MESSAGE(...) Application::DebugLog(__VA_ARGS__)
#else
#define DEBUG_MESSAGE(...) (0)
#endif

#if !defined(APPLICATION_TITLE)
#define APPLICATION_TITLE L"TextLayoutSampler (displays text with various text layout/rendering API's)"
#endif

#if !defined(BUILD_TITLE_STRING)
#define BUILD_TITLE_STRING \
        APPLICATION_TITLE L", "\
        TEXT(__DATE__) L", "\
        BUILD_ARCHITECTURE_STRING L", "\
        BUILD_OPTIMIZATION_STRING
#endif
