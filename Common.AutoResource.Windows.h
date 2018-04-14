#pragma once


#if USE_MODULES
import Common.AutoResource;
#else
#include "Common.AutoResource.h"
#endif


template <
    typename ResourceType,                      // underlying type of the resource held onto (e.g. HGDIOBJ instead of HFONT)
    typename ResourceReleaserSignature,         // function prototype of the releasing function
    ResourceReleaserSignature ResourceReleaser  // address of function to release object
>
struct HandleResourceTypePolicy : public DefaultResourceTypePolicy<ResourceType>
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


using GdiDeviceContext = AutoResource<HDC, HandleResourceTypePolicy<HDC, BOOL(WINAPI*)(HDC), &DeleteDC> >;
using GdiPenHandle = AutoResource<HPEN, HandleResourceTypePolicy<HGDIOBJ, BOOL(WINAPI*)(HGDIOBJ), &DeleteObject>, HGDIOBJ>;
using GdiFontHandle = AutoResource<HFONT, HandleResourceTypePolicy<HGDIOBJ, BOOL(WINAPI*)(HGDIOBJ), &DeleteObject>, HGDIOBJ>;
using GdiBitmapHandle = AutoResource<HBITMAP, HandleResourceTypePolicy<HGDIOBJ, BOOL(WINAPI*)(HGDIOBJ), &DeleteObject>, HGDIOBJ>;
using GdiRegionHandle = AutoResource<HRGN, HandleResourceTypePolicy<HGDIOBJ, BOOL(WINAPI*)(HGDIOBJ), &DeleteObject>, HGDIOBJ>;
using GlobalMemoryResource = AutoResource<HGLOBAL, HandleResourceTypePolicy<HGLOBAL, HGLOBAL(WINAPI*)(HGLOBAL), &GlobalFree> >;
using LocalMemoryResource = AutoResource<HLOCAL, HandleResourceTypePolicy<HLOCAL, HLOCAL(WINAPI*)(HLOCAL), &LocalFree> >;
using FileHandle = AutoResource<HANDLE, HandleResourceTypePolicy<HANDLE, BOOL(WINAPI*)(HANDLE), &CloseHandle> >;
using CstdioFileHandle = AutoResource<FILE*, HandleResourceTypePolicy<FILE*, int(__cdecl *)(FILE*), &fclose> >;
using ScopedMemory = AutoResource<void*, HandleResourceTypePolicy<void*, void(__cdecl *)(void*), &free> >;
using ModuleHandle = AutoResource<HMODULE, HandleResourceTypePolicy<HMODULE, BOOL(WINAPI*)(HMODULE), &FreeLibrary> >;
using WindowHandle = AutoResource<HWND, HandleResourceTypePolicy<HWND, BOOL(WINAPI*)(HWND), &DestroyWindow> >;
using MemoryViewResource = AutoResource<void*, HandleResourceTypePolicy<void*, BOOL(WINAPI*)(void const*), &UnmapViewOfFile> >;
using MemorySectionResource = AutoResource<HANDLE, HandleResourceTypePolicy<HANDLE, BOOL(WINAPI*)(HANDLE), &CloseHandle> >;
using WaitHandleResource = AutoResource<HANDLE, HandleResourceTypePolicy<HANDLE, void(*)(HANDLE), &WaitHandleUnregister>>;


////////////////////////////////////////
// Basic COM pointer.

struct ComResourceTypePolicy : public DefaultResourceTypePolicy<IUnknown*>
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
