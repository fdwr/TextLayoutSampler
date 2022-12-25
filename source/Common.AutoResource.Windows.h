//----------------------------------------------------------------------------
//
//  2015-12-16   dwayner    Split Windows specific stuff from AutoResource.h
//
//----------------------------------------------------------------------------
#pragma once

template <
    typename ResourceType,                      // underlying type of the resource held onto (e.g. HGDIOBJ instead of HFONT)
    typename ResourceReleaserSignature,         // function prototype of the releasing function
    ResourceReleaserSignature ResourceReleaser  // address of function to release object
>
struct AutoResourceHandlePolicy : public AutoResourceDefaultPolicy<ResourceType>
{
    inline static void Release(ResourceType resource) noexcept
    {
        if (resource != 0)
        {
            ResourceReleaser(resource);
        }
    }

    // Handles are owned by exactly one owner.
    static const bool AllowsMultipleReferences = false;
};

// Releasing function for WaitHandleResource to pass to use with HandleResourceTypePolicy.
inline void WaitHandleUnregister(HANDLE handle) noexcept
{
    if (handle != nullptr)
    {
        UnregisterWaitEx(handle, nullptr); // return value unactionable
    }
}

using GdiDeviceContext      = AutoResource<HDC,     AutoResourceHandlePolicy<HDC,     BOOL (WINAPI*)(HDC),           &DeleteDC>, HDC>;
using GdiPenHandle          = AutoResource<HPEN,    AutoResourceHandlePolicy<HGDIOBJ, BOOL (WINAPI*)(HGDIOBJ),       &DeleteObject>, HGDIOBJ>;
using GdiFontHandle         = AutoResource<HFONT,   AutoResourceHandlePolicy<HGDIOBJ, BOOL (WINAPI*)(HGDIOBJ),       &DeleteObject>, HGDIOBJ>;
using GdiBitmapHandle       = AutoResource<HBITMAP, AutoResourceHandlePolicy<HGDIOBJ, BOOL (WINAPI*)(HGDIOBJ),       &DeleteObject>, HGDIOBJ>;
using GdiRegionHandle       = AutoResource<HRGN,    AutoResourceHandlePolicy<HGDIOBJ, BOOL (WINAPI*)(HGDIOBJ),       &DeleteObject>, HGDIOBJ>;
using GlobalMemoryResource  = AutoResource<HGLOBAL, AutoResourceHandlePolicy<HGLOBAL, HGLOBAL (WINAPI*)(HGLOBAL),    &GlobalFree>, HGLOBAL>;
using LocalMemoryResource   = AutoResource<HLOCAL,  AutoResourceHandlePolicy<HLOCAL,  HLOCAL (WINAPI*)(HLOCAL),      &LocalFree>, HLOCAL>;
using FileHandle            = AutoResource<HANDLE,  AutoResourceHandlePolicy<HANDLE,  BOOL (WINAPI*)(HANDLE),        &CloseHandle>, HANDLE>;
using CstdioFileHandle      = AutoResource<FILE*,   AutoResourceHandlePolicy<FILE*,   int (__cdecl *)(FILE*),        &fclose>, FILE*>;
using ScopedMemory          = AutoResource<FILE*,   AutoResourceHandlePolicy<void*,   void (__cdecl *)(void*),       &free>, FILE*>;
using ModuleHandle          = AutoResource<HMODULE, AutoResourceHandlePolicy<HMODULE, BOOL (WINAPI*)(HMODULE),       &FreeLibrary>, HMODULE>;
using WindowHandle          = AutoResource<HWND,    AutoResourceHandlePolicy<HWND,    BOOL (WINAPI*)(HWND),          &DestroyWindow>, HWND>;
using MemoryViewResource    = AutoResource<void*,   AutoResourceHandlePolicy<void*,   BOOL (WINAPI*)(void const*),   &UnmapViewOfFile>, void*>;
using MemorySectionResource = AutoResource<HANDLE,  AutoResourceHandlePolicy<HANDLE,  BOOL (WINAPI*)(HANDLE),        &CloseHandle>, HANDLE>;

using GdiDeviceContext      = AutoResource<HDC,     AutoResourceHandlePolicy<HDC,     BOOL (WINAPI*)(HDC),           &DeleteDC>, HDC>;
using GdiPenHandle          = AutoResource<HPEN,    AutoResourceHandlePolicy<HGDIOBJ, BOOL (WINAPI*)(HGDIOBJ),       &DeleteObject>, HGDIOBJ>;
using GdiFontHandle         = AutoResource<HFONT,   AutoResourceHandlePolicy<HGDIOBJ, BOOL (WINAPI*)(HGDIOBJ),       &DeleteObject>, HGDIOBJ>;
using GdiBitmapHandle       = AutoResource<HBITMAP, AutoResourceHandlePolicy<HGDIOBJ, BOOL (WINAPI*)(HGDIOBJ),       &DeleteObject>, HGDIOBJ>;
using GdiRegionHandle       = AutoResource<HRGN,    AutoResourceHandlePolicy<HGDIOBJ, BOOL (WINAPI*)(HGDIOBJ),       &DeleteObject>, HGDIOBJ>;
using GlobalMemoryResource  = AutoResource<HGLOBAL, AutoResourceHandlePolicy<HGLOBAL, HGLOBAL (WINAPI*)(HGLOBAL),    &GlobalFree>, HGLOBAL>;
using LocalMemoryResource   = AutoResource<HLOCAL,  AutoResourceHandlePolicy<HLOCAL,  HLOCAL (WINAPI*)(HLOCAL),      &LocalFree>, HLOCAL>;
using FileHandle            = AutoResource<HANDLE,  AutoResourceHandlePolicy<HANDLE,  BOOL (WINAPI*)(HANDLE),        &CloseHandle>, HANDLE>;
using CstdioFileHandle      = AutoResource<FILE*,   AutoResourceHandlePolicy<FILE*,   int (__cdecl *)(FILE*),        &fclose>, FILE*>;
using ScopedMemory          = AutoResource<FILE*,   AutoResourceHandlePolicy<void*,   void (__cdecl *)(void*),       &free>, FILE*>;
using ModuleHandle          = AutoResource<HMODULE, AutoResourceHandlePolicy<HMODULE, BOOL (WINAPI*)(HMODULE),       &FreeLibrary>, HMODULE>;
using WindowHandle          = AutoResource<HWND,    AutoResourceHandlePolicy<HWND,    BOOL (WINAPI*)(HWND),          &DestroyWindow>, HWND>;
using MemoryViewResource    = AutoResource<void*,   AutoResourceHandlePolicy<void*,   BOOL (WINAPI*)(void const*),   &UnmapViewOfFile>, void*>;
using MemorySectionResource = AutoResource<HANDLE,  AutoResourceHandlePolicy<HANDLE,  BOOL (WINAPI*)(HANDLE),        &CloseHandle>, HANDLE>;

////////////////////////////////////////
// Basic COM pointer.

struct ComResourceTypePolicy : public AutoResourceDefaultPolicy<IUnknown*>
{
    inline static void Acquire(_Inout_opt_ IUnknown* resource) noexcept
    {
        if (resource != nullptr)
        {
            resource->AddRef();
        }
    }

    inline static void Release(_Inout_opt_ IUnknown* resource) noexcept
    {
        if (resource != nullptr)
        {
            resource->Release();
        }
    }

    static const bool AllowsMultipleReferences = true;
};

template<typename ResourceType>
using ComPtr = AutoResource<ResourceType*, ComResourceTypePolicy, IUnknown*>;
