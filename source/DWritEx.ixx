﻿//+---------------------------------------------------------------------------
//  DWrite helper functions for commonly needed tasks. These are mostly
//  convenience functions that help interop with standard STL containers,
//  but make some processes clearer (such as getting the path to a font file
//  or retrieving the black box of a glyph run) which otherwise take multiple
//  steps and are not obvious how to achieve.
//
//  The minimum OS is Windows 7, but some of these functions require later
//  versions of Windows to work fully. The library tries to gracefully fall back
//  to reasonable functionality where possible, such as DrawColorGlyphRun
//  drawing at least monochrome if color is not supported.
//
//  History:    2009-09-09  Dwayne Robinson - Created
//----------------------------------------------------------------------------

#if USE_CPP_MODULES
    module;
#endif

#include "precomp.h"

#include <Windows.h>
#include <DWrite_3.h>

#pragma comment(lib, "DWrite.lib")

#if USE_CPP_MODULES
    export module DWritEx;
    import Common.ArrayRef;
    import Common.String;
    import FileHelpers;
    import Common.AutoResource.Windows;
    export
    {
        #include "DWritEx.h"
    }
#else
    #include "Common.ArrayRef.h"
    #include "Common.String.h"
    #include "FileHelpers.h"
    #include "Common.AutoResource.h"
    #include "Common.AutoResource.Windows.h"
    #include "DWritEx.h"
#endif


////////////////////////////////////////

void CombineMatrix(
    _In_  DX_MATRIX_3X2F const& a,
    _In_  DX_MATRIX_3X2F const& b,
    _Out_ DX_MATRIX_3X2F& result
    )
{
    DEBUG_ASSERT(&a != &result);
    DEBUG_ASSERT(&b != &result);

    // Common transposed dot product (as opposed to any of the other numerous
    // forms of matrix 'multiplication' like the element-wise Hadamard product)
    // such that:
    //
    //  If you transpose <10,0> and rotate 45 degrees, you'll be at <.7,.7>
    //  If you rotate 45 degrees and transpose <10,0>, you'll be at <10,0>
    //
    // This is similar to XNA and opposite of OpenGL (so, easier to understand).

    result.xx = a.xx * b.xx + a.xy * b.yx;
    result.xy = a.xx * b.xy + a.xy * b.yy;
    result.yx = a.yx * b.xx + a.yy * b.yx;
    result.yy = a.yx * b.xy + a.yy * b.yy;
    result.dx = a.dx * b.xx + a.dy * b.yx + b.dx;
    result.dy = a.dx * b.xy + a.dy * b.yy + b.dy;
}


DX_MATRIX_3X2F CombineMatrix(
    _In_  DX_MATRIX_3X2F const& a,
    _In_  DX_MATRIX_3X2F const& b
    )
{
    DX_MATRIX_3X2F result;
    CombineMatrix(a, b, OUT result);
    return result;
}


////////////////////////////////////////

void DWritExGlyphMetrics::Set(DWRITE_GLYPH_METRICS const& glyphMetrics)
{
    verticalOriginX = glyphMetrics.advanceWidth >> 1; // the x v-origin is not stored in the font, and is assumed to be half the width.
    verticalOriginY = glyphMetrics.verticalOriginY;
    advanceWidth    = glyphMetrics.advanceWidth;
    advanceHeight   = glyphMetrics.advanceHeight;
    top             = int32_t(glyphMetrics.topSideBearing - glyphMetrics.verticalOriginY);
    left            = int32_t(glyphMetrics.leftSideBearing);
    right           = int32_t(glyphMetrics.advanceWidth - glyphMetrics.rightSideBearing);
    bottom          = int32_t(glyphMetrics.advanceHeight - glyphMetrics.bottomSideBearing - glyphMetrics.verticalOriginY);
}


namespace
{
    const static DX_MATRIX_3X2F g_identityTransform = {1,0,0,1,0,0};

    DWRITE_GLYPH_IMAGE_FORMATS g_allMonochromaticOutlineGlyphImageFormats =
        DWRITE_GLYPH_IMAGE_FORMATS_TRUETYPE |
        DWRITE_GLYPH_IMAGE_FORMATS_CFF |
        DWRITE_GLYPH_IMAGE_FORMATS_COLR
        // DWRITE_GLYPH_IMAGE_FORMATS_SVG |
        // DWRITE_GLYPH_IMAGE_FORMATS_PNG |
        // DWRITE_GLYPH_IMAGE_FORMATS_TIFF |
        // DWRITE_GLYPH_IMAGE_FORMATS_JPEG |
        // DWRITE_GLYPH_IMAGE_FORMATS_PREMULTIPLIED_B8G8R8A8
        ;

    DWRITE_GLYPH_IMAGE_FORMATS g_allNonOutlineGlyphImageFormats =
        // DWRITE_GLYPH_IMAGE_FORMATS_TRUETYPE |
        // DWRITE_GLYPH_IMAGE_FORMATS_CFF |
        // DWRITE_GLYPH_IMAGE_FORMATS_COLR
        DWRITE_GLYPH_IMAGE_FORMATS_SVG |
        DWRITE_GLYPH_IMAGE_FORMATS_PNG |
        DWRITE_GLYPH_IMAGE_FORMATS_TIFF |
        DWRITE_GLYPH_IMAGE_FORMATS_JPEG |
        DWRITE_GLYPH_IMAGE_FORMATS_PREMULTIPLIED_B8G8R8A8
        ;

    DWRITE_GLYPH_IMAGE_FORMATS g_allColorGlyphImageFormats =
        // DWRITE_GLYPH_IMAGE_FORMATS_TRUETYPE |
        // DWRITE_GLYPH_IMAGE_FORMATS_CFF |
        DWRITE_GLYPH_IMAGE_FORMATS_COLR |
        DWRITE_GLYPH_IMAGE_FORMATS_SVG |
        DWRITE_GLYPH_IMAGE_FORMATS_PNG |
        DWRITE_GLYPH_IMAGE_FORMATS_TIFF |
        DWRITE_GLYPH_IMAGE_FORMATS_JPEG |
        DWRITE_GLYPH_IMAGE_FORMATS_PREMULTIPLIED_B8G8R8A8
        ;
}


// Load it dynamically.
HRESULT LoadDWrite(
    _In_z_ const wchar_t* dllPath,
    DWRITE_FACTORY_TYPE factoryType, // DWRITE_FACTORY_TYPE_SHARED
    _COM_Outptr_ IDWriteFactory** factory,
    _Out_ HMODULE& moduleHandle
    ) noexcept
{
    using DWriteCreateFactory_t = HRESULT (__stdcall)(
        __in DWRITE_FACTORY_TYPE,
        __in IID const&,
        _COM_Outptr_ IUnknown**
        );

    *factory = nullptr;
    moduleHandle = nullptr;

    ModuleHandle dwriteModule = LoadLibrary(dllPath);
    if (dwriteModule == nullptr)
        return HRESULT_FROM_WIN32(GetLastError());

    DWriteCreateFactory_t* factoryFunction = (DWriteCreateFactory_t*) GetProcAddress(dwriteModule, "DWriteCreateFactory");
    if (factoryFunction == nullptr)
        return HRESULT_FROM_WIN32(GetLastError());

    // Use an isolated factory to prevent polluting the global cache.
    HRESULT hr = factoryFunction(
                    DWRITE_FACTORY_TYPE_ISOLATED,
                    __uuidof(IDWriteFactory),
                    OUT (IUnknown**)factory
                    );

    if (SUCCEEDED(hr))
    {
        moduleHandle = dwriteModule.Detach();
    }

    return hr;
}


#if 0
// Helper to return multiple supported interfaces.
//
// Example:
//
//  STDMETHOD(QueryInterface)(IID const& iid, __out void** object) override
//  {
//      COM_BASE_RETURN_INTERFACE(iid, IUnknown, object);
//      COM_BASE_RETURN_INTERFACE(iid, IDWriteInlineObject, object);
//      COM_BASE_RETURN_NO_INTERFACE(object);
//  }
//
#define COM_BASE_RETURN_INTERFACE(iid, U, object) \
    if (iid == __uuidof(U)) \
    { \
        U* p = static_cast<U*>(this); \
        p->AddRef(); \
        *object = p; \
        return S_OK; \
    }

// For those cases when diamond inheritance causes the ambiguous cast compilation error.
#define COM_BASE_RETURN_INTERFACE_AMBIGUOUS(iid, U, object, subthis) \
    if (iid == __uuidof(U)) \
    { \
        U* p = static_cast<U*>(subthis); \
        p->AddRef(); \
        *object = p; \
        return S_OK; \
    }

#define COM_BASE_RETURN_NO_INTERFACE(object) \
        *object = nullptr; \
        return E_NOINTERFACE;
#endif

// RefCountBase implementation for local reference-counted objects.
class RefCountBase
{
public:
    explicit RefCountBase() noexcept
        :   refValue_()
    { }

    explicit RefCountBase(ULONG refValue) noexcept
        :   refValue_(refValue)
    { }

    unsigned long IncrementRef() noexcept
    {
        return InterlockedIncrement(&refValue_);
    }

    unsigned long DecrementRef() noexcept
    {
        ULONG newCount = InterlockedDecrement(&refValue_);
        if (newCount == 0)
            delete this;
        return newCount;
    }

    // Ensure we have a v-table pointer and that the destructor is always
    // called on the most derived class.
    virtual ~RefCountBase()
    { }

protected:
    unsigned long refValue_;
};

class RefCountBaseStatic : public RefCountBase
{
public:
    using Base = RefCountBase;

    explicit RefCountBaseStatic() noexcept
    :   Base()
    { }

    explicit RefCountBaseStatic(ULONG refValue) noexcept
    :   Base(refValue)
    { }

    // Just use inherited IncrementRef.

    // Do not delete the reference.
    unsigned long DecrementRef() noexcept
    {
        return InterlockedDecrement(&refValue_);
    }
};

// COM base implementation for IUnknown.
//
// Example:
//
//  class RenderTarget : public ComBase<IDWriteTextRenderer>
//
template <typename T = IUnknown, typename RCB = RefCountBase>
class ComBase : public RCB, public T
{
public:
    using Base = RCB;
    using BaseInterface = T;

    /*
    ** Leave the definition of QI to the subclass.
    **
    // IUnknown interface
    STDMETHOD(QueryInterface)(IID const& iid, __out void** object) override
    {
        COM_BASE_RETURN_INTERFACE(iid, IUnknown, object);
        COM_BASE_RETURN_NO_INTERFACE(object);
    }
    */

    IFACEMETHODIMP_(unsigned long) AddRef()
    {
        return RCB::IncrementRef();
    }

    IFACEMETHODIMP_(unsigned long) Release()
    {
        return RCB::DecrementRef();
    }
};


class CustomCollectionLocalFontFileEnumerator : public ComBase<IDWriteFontFileEnumerator>
{
protected:
    IFACEMETHODIMP QueryInterface(IID const& iid, __out void** object)
    {
        COM_BASE_RETURN_INTERFACE(iid, IDWriteFontFileEnumerator, object);
        COM_BASE_RETURN_INTERFACE(iid, IUnknown, object);
        COM_BASE_RETURN_NO_INTERFACE(object);
    }


public:  
    CustomCollectionLocalFontFileEnumerator()
    :   remainingFontFileNames_(nullptr),
        findHandle_(INVALID_HANDLE_VALUE)
    { }  


    ~CustomCollectionLocalFontFileEnumerator()
    {
        if (findHandle_ != INVALID_HANDLE_VALUE)
        {
            FindClose(findHandle_);
        }
    }


    HRESULT Initialize(
        _In_ IDWriteFactory* factory,
        _In_reads_(fontFileNamesSize) const wchar_t* fontFileNames,
        uint32_t fontFileNamesSize
        ) noexcept
    {  
        if (factory == nullptr || fontFileNames == nullptr || !fontFileNames[0])
            return E_INVALIDARG;

        factory_ = factory;
        remainingFontFileNames_ = ToChar16(fontFileNames);
        remainingFontFileNamesEnd_ = ToChar16(fontFileNames) + fontFileNamesSize;
        return S_OK;
    }


    IFACEMETHODIMP MoveNext(_Out_ BOOL* hasCurrentFile)  
    {  
        HRESULT hr = S_OK;
        *hasCurrentFile = false;
        currentFontFile_ = nullptr;

        try
        {
            // Get next filename.
            for (;;)
            {
                // Check for the end of the list, either reaching the end
                // of the key or double-nul termination.
                if (remainingFontFileNames_ >= remainingFontFileNamesEnd_
                ||  remainingFontFileNames_[0] == '\0')
                {
                    return hr;
                }

                if (findHandle_ == INVALID_HANDLE_VALUE)
                {
                    // Get the first file matching the mask.
                    findHandle_ = FindFirstFile(ToWChar(remainingFontFileNames_), OUT &findData_);
                    if (findHandle_ == INVALID_HANDLE_VALUE)
                    {
                        auto errorCode = GetLastError();
                        if (errorCode == ERROR_FILE_NOT_FOUND)
                            return S_OK;

                        return HRESULT_FROM_WIN32(errorCode);
                    }
                }
                else if (!FindNextFile(findHandle_, OUT &findData_))
                {
                    // Move to next filename (skipping the nul).
                    remainingFontFileNames_ += wcslen(ToWChar(remainingFontFileNames_)) + 1;

                    FindClose(findHandle_);
                    findHandle_ = INVALID_HANDLE_VALUE;

                    continue; // Move to next file mask.
                }

                if (!(findData_.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) // Skip directories.
                    break; // Have our file.
            }

            // Concatenate the path and current file name.
            char16_t const* fileNameStart = FindFileNameStart({ remainingFontFileNames_, remainingFontFileNamesEnd_ });
            fullPath_.assign(remainingFontFileNames_, fileNameStart);
            fullPath_ += ToChar16(findData_.cFileName);

            #if 0
            wprintf(L"%s\n", fullPath_.c_str());
            #endif

            currentFontFile_.Clear();
            IFR(factory_->CreateFontFileReference(
                ToWChar(fullPath_.c_str()),
                &findData_.ftLastWriteTime,
                &currentFontFile_
                ));

            *hasCurrentFile = true;
        }
        catch (std::bad_alloc const&)
        {
            return E_OUTOFMEMORY; // This is the only exception type we need to worry about.
        }

        return hr;
    }  

    IFACEMETHODIMP GetCurrentFontFile(_COM_Outptr_ IDWriteFontFile** fontFile)  
    {  
        *fontFile = currentFontFile_;
        currentFontFile_->AddRef();
        return S_OK;
    }  

private:  
    ComPtr<IDWriteFactory> factory_;
    ComPtr<IDWriteFontFile> currentFontFile_;
    const char16_t* remainingFontFileNames_;
    const char16_t* remainingFontFileNamesEnd_;
    HANDLE findHandle_;
    WIN32_FIND_DATA findData_;
    std::u16string fullPath_;
};


class CustomCollectionFontFileEnumerator : public ComBase<IDWriteFontFileEnumerator>
{
protected:
    IFACEMETHODIMP QueryInterface(IID const& iid, __out void** object)
    {
        COM_BASE_RETURN_INTERFACE(iid, IDWriteFontFileEnumerator, object);
        COM_BASE_RETURN_INTERFACE(iid, IUnknown, object);
        COM_BASE_RETURN_NO_INTERFACE(object);
    }


public:  
    CustomCollectionFontFileEnumerator()
    :   currentFontFile_(nullptr),
        remainingFontFiles_(nullptr),
        remainingFontFilesEnd_(nullptr)
    { }


    HRESULT Initialize(
        _In_ IDWriteFactory* factory,
        _In_reads_(fontFilesCount) IDWriteFontFile* const* fontFiles,
        uint32_t fontFilesCount
        ) noexcept
    {  
        if (factory == nullptr || (fontFiles == nullptr && fontFilesCount > 0))
            return E_INVALIDARG;

        remainingFontFiles_ = fontFiles;
        remainingFontFilesEnd_ = fontFiles + fontFilesCount;
        return S_OK;
    }

    IFACEMETHODIMP MoveNext(_Out_ BOOL* hasCurrentFile)  
    {  
        *hasCurrentFile = (remainingFontFiles_ < remainingFontFilesEnd_);
        currentFontFile_ = *remainingFontFiles_;
        ++remainingFontFiles_;
        return S_OK;
    }  
  
    IFACEMETHODIMP GetCurrentFontFile(_COM_Outptr_ IDWriteFontFile** fontFile)  
    {  
        *fontFile = currentFontFile_;
        currentFontFile_->AddRef();
        return S_OK;
    }  
  
private:  
    IDWriteFontFile* currentFontFile_;
    IDWriteFontFile* const* remainingFontFiles_;
    IDWriteFontFile* const* remainingFontFilesEnd_;
};


class CustomFontCollectionLoader : public ComBase<IDWriteFontCollectionLoader, RefCountBaseStatic>
{
protected:
    IFACEMETHODIMP QueryInterface(IID const& iid, __out void** object)
    {
        COM_BASE_RETURN_INTERFACE(iid, IDWriteFontCollectionLoader, object);
        COM_BASE_RETURN_INTERFACE(iid, IUnknown, object);
        COM_BASE_RETURN_NO_INTERFACE(object);
    }

public:
    IFACEMETHODIMP CreateEnumeratorFromKey(
        _In_ IDWriteFactory* factory,
        _In_bytecount_(collectionKeySize) void const* collectionKey,
        uint32_t collectionKeySize,
        _COM_Outptr_ IDWriteFontFileEnumerator** fontFileEnumerator
        )
    {
        if (collectionKey == nullptr || collectionKeySize < sizeof(IDWriteFontFileEnumerator))
            return E_INVALIDARG;

        // The collectionKey actually points to the address of the custom IDWriteFontFileEnumerator.
        IDWriteFontFileEnumerator* enumerator = *reinterpret_cast<IDWriteFontFileEnumerator* const*>(collectionKey);
        enumerator->AddRef();

        *fontFileEnumerator = enumerator;
  
        return S_OK;
    }  
  
    static IDWriteFontCollectionLoader* GetInstance()  
    {  
        return &singleton_;
    }  
  
private:  
    static CustomFontCollectionLoader singleton_;
};

CustomFontCollectionLoader CustomFontCollectionLoader::singleton_;


HRESULT CreateFontCollection(
    _In_ IDWriteFactory* factory,
    DWRITE_FONT_FAMILY_MODEL fontFamilyModel,
    IDWriteFontFileEnumerator* fontFileEnumerator,
    _COM_Outptr_ IDWriteFontCollection** fontCollection
    ) noexcept
{
    RETURN_ON_ZERO(CustomFontCollectionLoader::GetInstance(), E_FAIL);

    HRESULT hr = S_OK;

    ComPtr<IDWriteFactory6> factory6;
    factory->QueryInterface(OUT &factory6);
    if (factory6 != nullptr)
    {
        ComPtr<IDWriteFontSet> fontSet;
        ComPtr<IDWriteFontSetBuilder1> fontSetBuilder;
        IFR(factory6->CreateFontSetBuilder(OUT &fontSetBuilder));
        BOOL hasCurrentFile = false;
        while (SUCCEEDED(fontFileEnumerator->MoveNext(OUT &hasCurrentFile)) && hasCurrentFile)
        {
            ComPtr<IDWriteFontFile> fontFile;
            IFR(fontFileEnumerator->GetCurrentFontFile(OUT &fontFile));
            IFR(fontSetBuilder->AddFontFile(fontFile));
        }
        IFR(fontSetBuilder->CreateFontSet(OUT &fontSet));
        IFR(factory6->CreateFontCollectionFromFontSet(fontSet, fontFamilyModel, OUT reinterpret_cast<IDWriteFontCollection2**>(fontCollection)));
    }
    else
    {
        IDWriteFontFileEnumerator** enumeratorAddress = &fontFileEnumerator;

        // Pass the address of the enumerator as the unique key.
        IFR(factory->RegisterFontCollectionLoader(CustomFontCollectionLoader::GetInstance()));
        hr = factory->CreateCustomFontCollection(CustomFontCollectionLoader::GetInstance(), enumeratorAddress, sizeof(enumeratorAddress), OUT fontCollection);
        IFR(factory->UnregisterFontCollectionLoader(CustomFontCollectionLoader::GetInstance()));
    }

    return hr;
}


HRESULT CreateFontCollection(
    _In_ IDWriteFactory* factory,
    DWRITE_FONT_FAMILY_MODEL fontFamilyModel,
    _In_reads_(fontFileNamesSize) const wchar_t* fontFileNames, // Each file name null terminated.
    _In_ uint32_t fontFileNamesSize, // Number of wchar_t's, not number file name count
    _COM_Outptr_ IDWriteFontCollection** fontCollection
    ) noexcept
{  
    RETURN_ON_ZERO(CustomFontCollectionLoader::GetInstance(), E_FAIL);

    CustomCollectionLocalFontFileEnumerator enumerator;
    IFR(enumerator.Initialize(factory, fontFileNames, fontFileNamesSize));
    enumerator.AddRef();

    return CreateFontCollection(factory, fontFamilyModel, &enumerator, OUT fontCollection);
}


HRESULT CreateFontCollection(
    _In_ IDWriteFactory* factory,
    DWRITE_FONT_FAMILY_MODEL fontFamilyModel,
    _In_reads_(fontFilesCount) IDWriteFontFile* const* fontFiles,
    uint32_t fontFilesCount,
    _COM_Outptr_ IDWriteFontCollection** fontCollection
    ) noexcept
{  
    RETURN_ON_ZERO(CustomFontCollectionLoader::GetInstance(), E_FAIL);

    CustomCollectionFontFileEnumerator enumerator;
    IFR(enumerator.Initialize(factory, fontFiles, fontFilesCount));
    enumerator.AddRef();

    return CreateFontCollection(factory, fontFamilyModel, &enumerator, OUT fontCollection);
}


class LocalFontFileStream : public IDWriteFontFileStream
{
public:
    LocalFontFileStream(_Inout_ std::vector<uint8_t>& streamData)
    {
        streamData_.swap(streamData);
    }

    IFACEMETHODIMP_(HRESULT) QueryInterface(IID const& iid, void** object)
    {
        HRESULT hr = S_OK;
        if ((iid == __uuidof(IUnknown)) || (iid == __uuidof(IDWriteFontFileStream)))
        {
            *object = this;
            AddRef();
        }
        else
        {
            *object = nullptr;
            hr = E_NOINTERFACE;
        }
        return hr;
    }

    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&refCount_);
    }

    IFACEMETHODIMP_(ULONG) Release()
    {
        ULONG newCount = InterlockedDecrement(&refCount_);
        if (newCount == 0)
        {
            delete this;
        }

        return newCount;
    }

    // IDWriteFontFileStream methods
    IFACEMETHODIMP_(HRESULT) ReadFileFragment(
        __in void const** fragmentStart,
        uint64_t fileOffset,
        uint64_t fragmentSize, // in bytes
        __deref_out void** fragmentContext
        )
    {
        uint64_t streamByteCount_ = streamData_.size();
        if ((fileOffset <= streamByteCount_) && 
            (fragmentSize <= streamByteCount_ - fileOffset))
        {
            *fragmentStart = streamData_.data() + static_cast<size_t>(fileOffset);
            *fragmentContext = nullptr;
        }
        else
        {
            *fragmentStart = nullptr;
            *fragmentContext = nullptr;
            return E_FAIL;
        }
        return S_OK;
    }

    IFACEMETHODIMP_(void) ReleaseFileFragment(
        void* fragmentContext
        )
    {
    }

    IFACEMETHODIMP_(HRESULT) GetFileSize(
        __out uint64_t* fileSize
        )
    {
        *fileSize = streamData_.size();
        return S_OK;
    }

    IFACEMETHODIMP_(HRESULT) GetLastWriteTime(
        __out uint64_t* lastWriteTime
        )
    {
        *lastWriteTime = 0;
        return S_OK;
    }

    bool IsInitialized()
    {
        return !streamData_.empty();
    }

private:
    ULONG refCount_ = 0;
    std::vector<uint8_t> streamData_;
};


// Dumb file loader that just reads byte contents from a file.
class LocalFontFileLoader : public IDWriteFontFileLoader
{
public:
    LocalFontFileLoader()
    {
    }

    // IUnknown methods
    IFACEMETHODIMP_(HRESULT) QueryInterface(IID const& iid, __deref_out void** object)
    {
        if ((iid == __uuidof(IUnknown)) || (iid == __uuidof(IDWriteFontFileLoader)))
        {
            *object = this;
            AddRef();
        }
        else
        {
            *object = nullptr;
            return E_NOINTERFACE;
        }
        return S_OK;
    }

    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&refCount_);
    }

    IFACEMETHODIMP_(ULONG) Release()
    {
        ULONG newCount = InterlockedDecrement(&refCount_);
        if (newCount == 0)
        {
            delete this;
        }
        return newCount;
    }

    // IDWriteFontFileLoader method
    IFACEMETHODIMP_(HRESULT) CreateStreamFromKey(
        __in void const* fontFileReferenceKey,
        uint32_t fontFileReferenceKeySize,
        __deref_out IDWriteFontFileStream** fontFileStream
        )
    {
        *fontFileStream = nullptr;

        auto* filePath = reinterpret_cast<wchar_t const*>(fontFileReferenceKey);
        auto filePathSize = fontFileReferenceKeySize / sizeof(wchar_t);
        std::vector<uint8_t> fileBytes;
        IFR(ReadBinaryFile(ToChar16(filePath), OUT fileBytes));
        ComPtr<IDWriteFontFileStream> stream(new LocalFontFileStream(IN OUT fileBytes));
        *fontFileStream = stream.Detach();

        return S_OK;
    }

private:
    ULONG refCount_ = 0;
};


HRESULT CreateFontFaceFromFile(
    IDWriteFactory* factory,
    _In_z_ const wchar_t* fontFilePath,
    uint32_t fontFaceIndex,
    DWRITE_FONT_FACE_TYPE fontFaceType,
    DWRITE_FONT_SIMULATIONS fontSimulations,
    array_ref<DWRITE_FONT_AXIS_VALUE> fontAxisValues,
    _COM_Outptr_ IDWriteFontFace** fontFace
    ) noexcept
{
    ComPtr<IDWriteFontFile> fontFile;

    // Create a dummy local file loader to work around the issue that DWrite
    // on Windows 10 won't let you use DWRITE_FONT_FACE_TYPE_RAW_CFF with the
    // standard file loader. Temporarily register it just long enough to create
    // the file reference.

    if (fontFaceType == DWRITE_FONT_FACE_TYPE_RAW_CFF)
    {
        ComPtr<IDWriteFontFileLoader> localFileLoader(new LocalFontFileLoader());
        IFR(factory->RegisterFontFileLoader(localFileLoader));
        auto hr = factory->CreateCustomFontFileReference(
            fontFilePath,
            uint32_t((wcslen(fontFilePath) + 1) * sizeof(wchar_t)),
            localFileLoader,
            OUT &fontFile
            );
        factory->UnregisterFontFileLoader(localFileLoader);
        IFR(hr);
    }
    else // Normal easy path.
    {
        IFR(factory->CreateFontFileReference(
            fontFilePath,
            nullptr, 
            OUT &fontFile
            ));
    }

    // Call the newer overload font axis values.
    if (!fontAxisValues.empty())
    {
        ComPtr<IDWriteFactory6> dwriteFactory6;
        ComPtr<IDWriteFontFaceReference1> fontFaceReference;
        factory->QueryInterface(OUT &dwriteFactory6);
        if (dwriteFactory6 != nullptr)
        {
            dwriteFactory6->CreateFontFaceReference(
                fontFile,
                fontFaceIndex,
                fontSimulations,
                fontAxisValues.data(),
                static_cast<uint32_t>(fontAxisValues.size()), // variationAxisCount
                OUT &fontFaceReference
            );
            return fontFaceReference->CreateFontFace(OUT reinterpret_cast<IDWriteFontFace5**>(fontFace));
        }
    }

    // If unknown file type, analyze it.
    if (fontFaceType == DWRITE_FONT_FACE_TYPE_UNKNOWN)
    {
        BOOL isSupportedFontType;
        uint32_t numberOfFaces;
        DWRITE_FONT_FILE_TYPE fontFileType;

        IFR(fontFile->Analyze(
            OUT &isSupportedFontType,
            OUT &fontFileType,
            OUT &fontFaceType,
            OUT &numberOfFaces
            ));

        if (!isSupportedFontType)
        {
            return DWRITE_E_FILEFORMAT;
        }
    }

    IDWriteFontFile* fontFileArray[] = {fontFile};
    IFR(factory->CreateFontFace(
        fontFaceType,
        ARRAYSIZE(fontFileArray),
        fontFileArray,
        fontFaceIndex ,
        fontSimulations,
        OUT fontFace
        ));

    return S_OK;
}


HRESULT CreateFontFaceFromTextFormat(
    IDWriteTextFormat* textFormat,
    _COM_Outptr_ IDWriteFontFace** fontFace
    ) noexcept
{
    // Get font family name
    uint32_t const fontFamilyNameLength = textFormat->GetFontFamilyNameLength();
    std::vector<wchar_t> fontFamilyName(fontFamilyNameLength + 1);
    ComPtr<IDWriteFontCollection> fontCollection;

    IFR(textFormat->GetFontFamilyName(OUT fontFamilyName.data(), fontFamilyNameLength + 1));
    IFR(textFormat->GetFontCollection(OUT &fontCollection));

    return CreateFontFace(
        fontCollection,
        fontFamilyName.data(),
        textFormat->GetFontWeight(),
        textFormat->GetFontStretch(),
        textFormat->GetFontStyle(),
        OUT fontFace
        );
}


HRESULT CreateFontFace(
    IDWriteFontCollection* fontCollection,
    _In_z_ const wchar_t* fontFamilyName,
    DWRITE_FONT_WEIGHT fontWeight,
    DWRITE_FONT_STRETCH fontStretch,
    DWRITE_FONT_STYLE fontSlope,
    _COM_Outptr_ IDWriteFontFace** fontFace
    )
{
    ComPtr<IDWriteFontFamily> fontFamily;
    ComPtr<IDWriteFont> font;

    uint32_t index = 0;
    BOOL exists = false;
    IFR(fontCollection->FindFamilyName(fontFamilyName, OUT &index, OUT &exists));
    if (!exists)
        return DWRITE_E_NOFONT;

    IFR(fontCollection->GetFontFamily(index, OUT &fontFamily));
    IFR(fontFamily->GetFirstMatchingFont(
        fontWeight,
        fontStretch,
        fontSlope,
        OUT &font
        ));

    return font->CreateFontFace(OUT fontFace);
}


HRESULT RecreateFontFace(
    IDWriteFactory* factory,
    IDWriteFontFace* originalFontFace,
    DWRITE_FONT_SIMULATIONS fontSimulations,
    array_ref<const DWRITE_FONT_AXIS_VALUE> fontAxisValues,
    _COM_Outptr_ IDWriteFontFace** newFontFace
    )
{
    ComPtr<IDWriteFontFile> fontFile;
    IFR(GetFontFile(originalFontFace, OUT &fontFile));
    auto fontFaceType = originalFontFace->GetType();
    auto fontFaceIndex = originalFontFace->GetIndex();

    if (!fontAxisValues.empty())
    {
        ComPtr<IDWriteFactory6> factory6;
        if SUCCEEDED(factory->QueryInterface(OUT &factory6))
        {
            ComPtr<IDWriteFontFaceReference1> fontFaceReference;
            IFR(factory6->CreateFontFaceReference(
                fontFile,
                originalFontFace->GetIndex(),
                fontSimulations,
                fontAxisValues.data(),
                static_cast<uint32_t>(fontAxisValues.size()),
                OUT &fontFaceReference
            ));
            return fontFaceReference->CreateFontFace(OUT reinterpret_cast<IDWriteFontFace5**>(newFontFace));
        }
    }

    IDWriteFontFile* fontFileArray[] = {fontFile};
    return factory->CreateFontFace(
        fontFaceType,
        ARRAYSIZE(fontFileArray),
        fontFileArray,
        fontFaceIndex ,
        fontSimulations,
        OUT newFontFace
        );
}


HRESULT CreateTextLayout(
    IDWriteFactory* factory,
    __in_ecount(textLength) wchar_t const* text,
    uint32_t textLength,
    IDWriteTextFormat* textFormat,
    float maxWidth,
    float maxHeight,
    DWRITE_MEASURING_MODE measuringMode,
    __out IDWriteTextLayout** textLayout
    ) noexcept
{
    if (measuringMode == DWRITE_MEASURING_MODE_NATURAL)
    {
        return factory->CreateTextLayout(
                            text,
                            textLength,
                            textFormat,
                            maxWidth,
                            maxHeight,
                            OUT textLayout
                            );
    }
    else
    {
        return factory->CreateGdiCompatibleTextLayout(
                            text,
                            textLength,
                            textFormat,
                            maxWidth,
                            maxHeight,
                            1, // pixels per DIP
                            nullptr, // no transform
                            (measuringMode == DWRITE_MEASURING_MODE_GDI_NATURAL) ? true : false,
                            OUT textLayout
                            );
    }
}


HRESULT GetFontFaceMetrics(
    IDWriteFontFace* fontFace,
    float fontEmSize,
    DWRITE_MEASURING_MODE measuringMode,
    __out DWRITE_FONT_METRICS* fontMetrics
    ) noexcept
{
    switch (measuringMode)
    {
    case DWRITE_MEASURING_MODE_GDI_CLASSIC:
    case DWRITE_MEASURING_MODE_GDI_NATURAL:
        return fontFace->GetGdiCompatibleMetrics(
            fontEmSize,
            1.0f, // pixelsPerDip
            nullptr, // transform
            OUT fontMetrics
            );

    case DWRITE_MEASURING_MODE_NATURAL:
    default:
        fontFace->GetMetrics(OUT fontMetrics);
        return S_OK;
    }
}


HRESULT GetFontFaceDesignAdvances(
    IDWriteFontFace* fontFace,
    float fontEmSize,
    array_ref<uint16_t const> glyphIds,
    DWRITE_MEASURING_MODE measuringMode,
    bool isSideways,
    _Out_ array_ref<int32_t> glyphAdvances
    ) noexcept
{
    if (glyphIds.size() != glyphAdvances.size())
        return E_INVALIDARG;

    uint32_t glyphCount = static_cast<uint32_t>(glyphIds.size());

    ComPtr<IDWriteFontFace1> fontFace1;

    // Use the lighter Get*GlyphAdvances functions if they exist.
    if (SUCCEEDED(fontFace->QueryInterface(OUT &fontFace1)))
    {
        switch (measuringMode)
        {
        case DWRITE_MEASURING_MODE_GDI_CLASSIC:
        case DWRITE_MEASURING_MODE_GDI_NATURAL:
            IFR(fontFace1->GetGdiCompatibleGlyphAdvances(
                fontEmSize,
                1.0f, // pixelsPerDip
                nullptr, // transform
                measuringMode == DWRITE_MEASURING_MODE_GDI_NATURAL,
                isSideways,
                glyphCount,
                glyphIds.data(),
                OUT glyphAdvances.data()
                ));
            break;

        case DWRITE_MEASURING_MODE_NATURAL:
        default:
            IFR(fontFace1->GetDesignGlyphAdvances(glyphCount, glyphIds.data(), OUT glyphAdvances.data(), isSideways));
            break;
        }
    }
    // else using slower DWRITE_GLYPH_METRICS approach for Windows 7.
    else
    {
        std::vector<DWRITE_GLYPH_METRICS> glyphMetricsBuffer(glyphIds.size());
        switch (measuringMode)
        {
        case DWRITE_MEASURING_MODE_GDI_CLASSIC:
        case DWRITE_MEASURING_MODE_GDI_NATURAL:
            IFR(fontFace1->GetGdiCompatibleGlyphMetrics(
                fontEmSize,
                1.0f, // pixelsPerDip
                nullptr, // transform
                measuringMode == DWRITE_MEASURING_MODE_GDI_NATURAL,
                glyphIds.data(),
                glyphCount,
                OUT glyphMetricsBuffer.data(),
                isSideways
                ));
            break;

        case DWRITE_MEASURING_MODE_NATURAL:
        default:
            IFR(fontFace->GetDesignGlyphMetrics(glyphIds.data(), glyphCount, OUT glyphMetricsBuffer.data(), isSideways));
            break;
        }

        for (uint32_t i = 0; i < glyphCount; ++i)
        {
            auto const& metrics = glyphMetricsBuffer[i];
            glyphAdvances[i] = isSideways ? metrics.advanceHeight : metrics.advanceWidth;
        }
    }

    return S_OK;
}


HRESULT GetFontFaceAdvances(
    IDWriteFontFace* fontFace,
    float fontEmSize,
    array_ref<uint16_t const> glyphIds,
    DWRITE_MEASURING_MODE measuringMode,
    bool isSideways,
    _Out_ array_ref<float> glyphAdvances
    ) noexcept
{
    std::vector<int32_t> glyphAdvancesBuffer(glyphIds.size());
    IFR(GetFontFaceDesignAdvances(
        fontFace,
        fontEmSize,
        glyphIds,
        measuringMode,
        isSideways,
        OUT glyphAdvancesBuffer
        ));

    uint32_t glyphCount = static_cast<uint32_t>(glyphIds.size());
    DWRITE_FONT_METRICS fontMetrics;
    fontFace->GetMetrics(OUT &fontMetrics);

    for (uint32_t i = 0; i < glyphCount; ++i)
    {
        int32_t advance = glyphAdvancesBuffer[i];
        glyphAdvances[i] = advance * fontEmSize / fontMetrics.designUnitsPerEm;
    }

    return S_OK;
}


HRESULT PlacementsToAbsolutePoints(
    IDWriteFontFace* fontFace,
    float fontEmSize,
    float baselineOriginX,
    float baselineOriginY,
    bool isSideways,
    bool isRtl,
    uint32_t glyphCount,
    _In_reads_(glyphCount) const uint16_t* glyphIndices,
    _In_reads_(glyphCount) const float* glyphAdvances,
    _In_reads_opt_(glyphCount) const DWRITE_GLYPH_OFFSET* glyphOffsets,
    _Out_writes_(glyphCount) D2D_POINT_2F* absoluteGlyphPoints
    ) noexcept
{
    static_assert(sizeof(D2D_POINT_2F) == sizeof(DWRITE_GLYPH_OFFSET), "");

    return PlacementsToAbsoluteOffsets(
        fontFace,
        fontEmSize,
        baselineOriginX,
        baselineOriginY,
        isSideways,
        isRtl,
        true, // invertYCoordinate
        glyphCount,
        glyphIndices,
        glyphAdvances,
        glyphOffsets,
        reinterpret_cast<DWRITE_GLYPH_OFFSET*>(absoluteGlyphPoints)
        );
}

HRESULT PlacementsToAbsoluteOffsets(
    IDWriteFontFace* fontFace,
    float fontEmSize,
    float baselineOriginX,
    float baselineOriginY,
    bool isSideways,
    bool isRtl,
    bool invertYCoordinate,
    uint32_t glyphCount,
    _In_reads_(glyphCount) const uint16_t* glyphIndices,
    _In_reads_(glyphCount) const float* glyphAdvances,
    _In_reads_opt_(glyphCount) const DWRITE_GLYPH_OFFSET* glyphOffsets,
    _Out_writes_(glyphCount) DWRITE_GLYPH_OFFSET* absoluteGlyphOffsets
    ) noexcept
{
    // isSideways does not affect the interpretation of the u,v offsets.

    DWRITE_FONT_METRICS fontMetrics = {};
    fontMetrics.designUnitsPerEm = 2048; // Set a default to avoid division by zero
    std::vector<int32_t> designGlyphAdvances(glyphCount);

    if (fontFace != nullptr)
    {
        fontFace->GetMetrics(&fontMetrics);

        GetFontFaceDesignAdvances(
            fontFace,
            fontEmSize,
            {glyphIndices, glyphIndices + glyphCount},
            DWRITE_MEASURING_MODE_NATURAL, // measuringMode
            isSideways,
            OUT designGlyphAdvances
            );
    }

    float x = baselineOriginX;
    const static DWRITE_GLYPH_OFFSET glyphOffsetZero = {};

    bool shouldComputeDesignGlyphAdvance = (glyphAdvances != nullptr) || isRtl;

    for (uint32_t gi = 0; gi < glyphCount; ++gi)
    {
        float designGlyphAdvance;
        if (shouldComputeDesignGlyphAdvance)
        {
            designGlyphAdvance = designGlyphAdvances[gi] * fontEmSize / fontMetrics.designUnitsPerEm;
        }
        const float glyphAdvance = (glyphAdvances != nullptr) ? glyphAdvances[gi] : designGlyphAdvance;
        DWRITE_GLYPH_OFFSET glyphOffset = (glyphOffsets != nullptr) ? glyphOffsets[gi] : glyphOffsetZero;

        // Turn advances and offsets into pure positions.
        if (isRtl)
        {
            // RTL runs reverse the interpretation of the glyph offset,
            // so flip them to the typical Cartesian x orientation.
            glyphOffset.advanceOffset = x - designGlyphAdvance - glyphOffset.advanceOffset;
            x -= glyphAdvance;
        }
        else
        {
            // Post-adjust glyph's position
            glyphOffset.advanceOffset = x + glyphOffset.advanceOffset;
            x += glyphAdvance;
        }

        auto y = glyphOffset.ascenderOffset;
        if (invertYCoordinate) y = -y;
        glyphOffset.ascenderOffset = baselineOriginY - y;

        absoluteGlyphOffsets[gi] = glyphOffset;
    }

    return S_OK;
}


HRESULT GetLocalFileLoaderAndKey(
    IDWriteFontFile* fontFile,
    _Out_ void const** fontFileReferenceKey,
    _Out_ uint32_t& fontFileReferenceKeySize,
    _Out_ IDWriteLocalFontFileLoader** localFontFileLoader
    )
{
    *localFontFileLoader = nullptr;
    *fontFileReferenceKey = nullptr;
    fontFileReferenceKeySize = 0;

    if (fontFile == nullptr)
        return E_INVALIDARG;

    ComPtr<IDWriteFontFileLoader> fontFileLoader;
    IFR(fontFile->GetLoader(OUT &fontFileLoader));
    IFR(fontFileLoader->QueryInterface(OUT localFontFileLoader));

    IFR(fontFile->GetReferenceKey(fontFileReferenceKey, OUT &fontFileReferenceKeySize));

    return S_OK;
}


HRESULT GetFilePath(
    IDWriteFontFile* fontFile,
    OUT std::u16string& filePath
    ) noexcept
{
    std::u16string tempFilePath;
    std::swap(tempFilePath, filePath);

    if (fontFile == nullptr)
        return E_INVALIDARG;

    ComPtr<IDWriteLocalFontFileLoader> localFileLoader;
    uint32_t fontFileReferenceKeySize = 0;
    const void* fontFileReferenceKey = nullptr;

    IFR(GetLocalFileLoaderAndKey(fontFile, OUT &fontFileReferenceKey, OUT fontFileReferenceKeySize, OUT &localFileLoader));

    uint32_t filePathLength = 0;
    IFR(localFileLoader->GetFilePathLengthFromKey(
        fontFileReferenceKey,
        fontFileReferenceKeySize,
        OUT &filePathLength
    ));

    try
    {
        tempFilePath.resize(filePathLength);
    }
    catch (std::bad_alloc const&)
    {
        return E_OUTOFMEMORY; // This is the only exception type we need to worry about.
    }

    IFR(localFileLoader->GetFilePathFromKey(
        fontFileReferenceKey,
        fontFileReferenceKeySize,
        OUT ToWChar(&tempFilePath[0]),
        filePathLength + 1
    ));
    std::swap(tempFilePath, filePath);

    return S_OK;
}


HRESULT GetFontFile(
    IDWriteFontFace* fontFace,
    OUT IDWriteFontFile** fontFile
    )
{
    *fontFile = nullptr;

    if (fontFace == nullptr)
        return E_INVALIDARG;

    ComPtr<IDWriteFontFile> fontFiles[8];
    uint32_t fontFileCount = 0;

    IFR(fontFace->GetFiles(OUT &fontFileCount, nullptr));
    if (fontFileCount > ARRAYSIZE(fontFiles))
        return E_NOT_SUFFICIENT_BUFFER;

    IFR(fontFace->GetFiles(IN OUT &fontFileCount, OUT &fontFiles[0]));

    if (fontFileCount == 0 || fontFiles[0] == nullptr)
        return E_FAIL;

    *fontFile = fontFiles[0].Detach();
    return S_OK;
}


HRESULT GetFilePath(
    IDWriteFontFace* fontFace,
    OUT std::u16string& filePath
    ) noexcept
{
    filePath.clear();

    if (fontFace == nullptr)
        return E_INVALIDARG;

    ComPtr<IDWriteFontFile> fontFile;
    IFR(GetFontFile(fontFace, OUT &fontFile));

    return GetFilePath(fontFile, OUT filePath);
}


HRESULT GetFileModifiedDate(
    IDWriteFontFace* fontFace,
    _Out_ FILETIME& fileTime
    ) noexcept
{
    const static FILETIME zeroFileTime = {};
    fileTime = zeroFileTime;
    ComPtr<IDWriteLocalFontFileLoader> localFileLoader;
    uint32_t fontFileReferenceKeySize = 0;
    const void* fontFileReferenceKey = nullptr;

    ComPtr<IDWriteFontFile> fontFile;
    IFR(GetFontFile(fontFace, OUT &fontFile));

    IFR(GetLocalFileLoaderAndKey(fontFile, &fontFileReferenceKey, OUT fontFileReferenceKeySize, OUT &localFileLoader));

    return localFileLoader->GetLastWriteTimeFromKey(
            fontFileReferenceKey,
            fontFileReferenceKeySize,
            OUT &fileTime
            );
}


HRESULT SaveDWriteFontFile(
    IDWriteFontFileStream* fontFileStream,
    _In_z_ char16_t const* filePath
    )
{
    void const* fragment = nullptr;
    void* fragmentContext = nullptr;
    uint64_t fileSize = 0;

    auto fragmentCleanup = DeferCleanup([&] { fontFileStream->ReleaseFileFragment(fragmentContext); });
    IFR(fontFileStream->GetFileSize(OUT &fileSize));
    IFR(fontFileStream->ReadFileFragment(OUT &fragment, 0, fileSize, OUT &fragmentContext));
    return WriteBinaryFile(filePath, fragment, static_cast<uint32_t>(fileSize));
}


HRESULT SaveDWriteFontFile(
    IDWriteFontFace* fontFace,
    _In_z_ char16_t const* filePath
    )
{
    ComPtr<IDWriteFontFile> fontFile;
    ComPtr<IDWriteFontFileStream> fontFileStream;
    ComPtr<IDWriteFontFileLoader> fontFileLoader;
    void const* fontFileReferenceKey = nullptr;
    uint32_t fontFileReferenceKeySize = 0;

    IFR(GetFontFile(fontFace, &fontFile));
    IFR(fontFile->GetLoader(OUT &fontFileLoader));
    IFR(fontFile->GetReferenceKey(OUT &fontFileReferenceKey, OUT &fontFileReferenceKeySize));
    IFR(fontFileLoader->CreateStreamFromKey(fontFileReferenceKey, fontFileReferenceKeySize, OUT &fontFileStream));
    return SaveDWriteFontFile(fontFileStream, filePath);
}


HRESULT GetLocalizedStringLanguage(
    IDWriteLocalizedStrings* strings,
    uint32_t stringIndex,
    OUT std::u16string& stringValue
    ) noexcept
{
    stringValue.clear();

    if (strings == nullptr)
        return E_INVALIDARG;

    uint32_t length = 0;
    strings->GetLocaleNameLength(stringIndex, OUT &length);
    if (length == 0 || length == UINT32_MAX)
        return S_OK;

    try
    {
        stringValue.resize(length);
    }
    catch (std::bad_alloc const&)
    {
        return E_OUTOFMEMORY; // This is the only exception type we need to worry about.
    }
    return strings->GetLocaleName(stringIndex, OUT ToWChar(&stringValue[0]), length + 1);
}


HRESULT GetLocalizedString(
    IDWriteLocalizedStrings* strings,
    _In_opt_z_ const wchar_t* preferredLanguage,
    OUT std::u16string& stringValue
    ) noexcept
{
    stringValue.clear();

    if (strings == nullptr)
        return E_INVALIDARG;

    uint32_t stringIndex = 0;
    BOOL exists = false;
    if (preferredLanguage != nullptr)
    {
        strings->FindLocaleName(preferredLanguage, OUT &stringIndex, OUT &exists);
    }

    if (!exists)
    {
        strings->FindLocaleName(L"en-us", OUT &stringIndex, OUT &exists);
    }

    if (!exists)
    {
        stringIndex = 0; // Just try the first index.
    }

    return GetLocalizedString(strings, stringIndex, OUT stringValue);
}


HRESULT GetLocalizedString(
    IDWriteLocalizedStrings* strings,
    uint32_t stringIndex,
    OUT std::u16string& stringValue
    ) noexcept
{
    stringValue.clear();
    if (strings == nullptr || strings->GetCount() == 0)
        return S_OK;

    uint32_t length = 0;
    IFR(strings->GetStringLength(stringIndex, OUT &length));
    if (length == 0 || length == UINT32_MAX)
        return S_OK;

    try
    {
        stringValue.resize(length);
    }
    catch (std::bad_alloc const&)
    {
        return E_OUTOFMEMORY; // This is the only exception type we need to worry about.
    }
    IFR(strings->GetString(stringIndex, OUT ToWChar(&stringValue[0]), length + 1));

    return S_OK;
}


HRESULT GetFontFaceName(
    IDWriteFont* font,
    _In_opt_z_ wchar_t const* languageName,
    OUT std::u16string& stringValue
    )
{
    stringValue.clear();
    if (font == nullptr)
        return E_INVALIDARG;

    ComPtr<IDWriteLocalizedStrings> fontFaceNames;
    IFR(font->GetFaceNames(OUT &fontFaceNames));
    IFR(GetLocalizedString(fontFaceNames, languageName, OUT stringValue));

    return S_OK;
}


HRESULT GetFontFamilyName(
    IDWriteFont* font,
    _In_opt_z_ wchar_t const* languageName,
    OUT std::u16string& stringValue
    )
{
    stringValue.clear();
    if (font == nullptr)
        return E_INVALIDARG;

    ComPtr<IDWriteFontFamily> fontFamily;
    IFR(font->GetFontFamily(OUT &fontFamily));
    return GetFontFamilyName(fontFamily, languageName, OUT stringValue);
}


HRESULT GetFontFamilyName(
    IDWriteFontFamily* fontFamily,
    _In_opt_z_ wchar_t const* languageName,
    OUT std::u16string& stringValue
    )
{
    stringValue.clear();
    if (fontFamily == nullptr)
        return E_INVALIDARG;

    ComPtr<IDWriteLocalizedStrings> fontFamilyNames;
    IFR(fontFamily->GetFamilyNames(OUT &fontFamilyNames));
    return GetLocalizedString(fontFamilyNames, languageName, OUT stringValue);
}


HRESULT GetInformationalString(
    IDWriteFont* font,
    DWRITE_INFORMATIONAL_STRING_ID informationalStringID,
    _In_opt_z_ wchar_t const* languageName,
    _Out_ std::u16string& stringValue
    )
{
    // Handle the multiple intermediate steps and just return the string.
    stringValue.clear();

    BOOL stringExists;
    ComPtr<IDWriteLocalizedStrings> localizedStrings;
    IFR(font->GetInformationalStrings(
        informationalStringID,
        OUT &localizedStrings,
        OUT &stringExists
        ));
    if (stringExists == false)
    {
        return S_OK; // Just return empty string.
    }

    return GetLocalizedString(localizedStrings, languageName, OUT stringValue);
}


HRESULT GetInformationalString(
    IDWriteFontFace* fontFace,
    DWRITE_INFORMATIONAL_STRING_ID informationalStringID,
    _In_opt_z_ wchar_t const* languageName,
    _Out_ std::u16string& stringValue
    )
{
    // Handle the multiple intermediate steps and just return the string.
    // If the string does not exist, just return emptiness.
    stringValue.clear();

    BOOL stringExists;
    ComPtr<IDWriteFontFace3> fontFace3;
    ComPtr<IDWriteLocalizedStrings> localizedStrings;
    IFR(fontFace->QueryInterface(OUT &fontFace3));
    IFR(fontFace3->GetInformationalStrings(
        informationalStringID,
        OUT &localizedStrings,
        OUT &stringExists
        ));
    if (stringExists == false)
    {
        return S_OK; // Just return empty string.
    }

    return GetLocalizedString(localizedStrings, languageName, OUT stringValue);
}


HRESULT GetFontAxisValues(
    IDWriteFontFaceReference* fontFaceReference,
    _Out_ std::vector<DWRITE_FONT_AXIS_VALUE>& fontAxisValues
    )
{
    fontAxisValues.clear();
    ComPtr<IDWriteFontFaceReference1> fontFaceReference1;
    IFR(fontFaceReference->QueryInterface(OUT &fontFaceReference1));
    auto valueCount = fontFaceReference1->GetFontAxisValueCount();
    fontAxisValues.resize(valueCount);
    return fontFaceReference1->GetFontAxisValues(OUT fontAxisValues.data(), valueCount);
}


HRESULT GetFontAxisValues(
    IDWriteFontFace* fontFace,
    _Out_ std::vector<DWRITE_FONT_AXIS_VALUE>& fontAxisValues
    )
{
    fontAxisValues.clear();
    ComPtr<IDWriteFontFace3> fontFace3;
    ComPtr<IDWriteFontFaceReference> fontFaceReference;
    IFR(fontFace->QueryInterface(OUT &fontFace3));
    IFR(fontFace3->GetFontFaceReference(OUT &fontFaceReference));
    return GetFontAxisValues(fontFaceReference, OUT fontAxisValues);
}


HRESULT GetFontAxisValues(
    IDWriteFont* font,
    _Out_ std::vector<DWRITE_FONT_AXIS_VALUE>& fontAxisValues
    )
{
    fontAxisValues.clear();
    ComPtr<IDWriteFont3> font3;
    ComPtr<IDWriteFontFaceReference> fontFaceReference;
    IFR(font->QueryInterface(OUT &font3));
    IFR(font3->GetFontFaceReference(OUT &fontFaceReference));
    return GetFontAxisValues(fontFaceReference, OUT fontAxisValues);
}


float GetFontAxisValue(
    array_ref<DWRITE_FONT_AXIS_VALUE const> fontAxisValues,
    DWRITE_FONT_AXIS_TAG axisTag,
    float defaultValue
    )
{
    for (auto& fontAxisValue : fontAxisValues)
    {
        if (fontAxisValue.axisTag == axisTag)
            return fontAxisValue.value;
    }

    return defaultValue;
}


bool IsKnownFontFileExtension(_In_z_ const wchar_t* fileExtension) noexcept
{
    return (_wcsicmp(fileExtension, L".otf") == 0
        ||  _wcsicmp(fileExtension, L".ttf") == 0
        ||  _wcsicmp(fileExtension, L".ttc") == 0
        ||  _wcsicmp(fileExtension, L".tte") == 0
            );
}


void GetGlyphOrientationTransform(
    DWRITE_GLYPH_ORIENTATION_ANGLE glyphOrientationAngle,
    bool isSideways,
    float originX,
    float originY,
    _Out_ DWRITE_MATRIX* transform
    ) noexcept
{
    uint32_t quadrant = glyphOrientationAngle;
    if (isSideways)
    {
        // The world transform is an additional 90 degrees clockwise from the
        // glyph orientation when the sideways flag is set.
        ++quadrant;
    }

    const static DWRITE_MATRIX quadrantMatrices[4] = {
        /* I0   */{  1, 0, 0, 1, 0, 0 }, // translation is always zero
        /* C90  */{  0, 1,-1, 0, 0, 0 },
        /* C180 */{ -1, 0, 0,-1, 0, 0 },
        /* C270 */{  0,-1, 1, 0, 0, 0 }
    };

    auto const& matrix = quadrantMatrices[quadrant & 3];
    *transform = matrix;

    if (quadrant != 0)
    {
        // Compute the translation necessary to rotate around the given origin.
        // (if the angle is zero, the equation would produce zero translation
        // values anyway, so skip it in that case).
        transform->dx = originX * (1 - matrix.m11) - originY * matrix.m21;
        transform->dy = originY * (1 - matrix.m22) - originX * matrix.m12;
    }
}


class BitmapRenderTargetTextRenderer : public IDWriteTextRenderer1
{
public:
    BitmapRenderTargetTextRenderer(
        IDWriteFactory* dwriteFactory,
        IDWriteBitmapRenderTarget* renderTarget,
        IDWriteRenderingParams* renderingParams,
        COLORREF textColor = 0x00000000,
        uint32_t colorPaletteIndex = 0xFFFFFFFF,
        bool enablePixelSnapping = true
        )
    :   dwriteFactory_(dwriteFactory),
        renderTarget_(renderTarget),
        renderingParams_(renderingParams),
        textColor_(textColor),
        colorPaletteIndex_(colorPaletteIndex),
        enablePixelSnapping_(enablePixelSnapping)
    { }

    class TransformSetter
    {
    public:
        TransformSetter(
            IDWriteBitmapRenderTarget* renderTarget,
            DWRITE_GLYPH_ORIENTATION_ANGLE orientationAngle,
            float originX,
            float originY,
            bool isSideways = false
            )
        {
            renderTarget_ = renderTarget;
            needToSetTransform_ = (orientationAngle != DWRITE_GLYPH_ORIENTATION_ANGLE_0_DEGREES || isSideways);
            renderTarget_->GetCurrentTransform(OUT &previousTransform_.dwrite);
            if (needToSetTransform_)
            {
                DX_MATRIX_3X2F runTransform;
                GetGlyphOrientationTransform(
                    orientationAngle,
                    isSideways,
                    originX,
                    originY,
                    OUT &runTransform.dwrite
                    );
                CombineMatrix(runTransform, previousTransform_, OUT currentTransform_);
                renderTarget_->SetCurrentTransform(&currentTransform_.dwrite);
            }
            else
            {
                currentTransform_ = previousTransform_;
            }
        }

        ~TransformSetter()
        {
            if (needToSetTransform_)
                renderTarget_->SetCurrentTransform(&previousTransform_.dwrite);
        }

    public:
        DX_MATRIX_3X2F currentTransform_;
        DX_MATRIX_3X2F previousTransform_;

    protected:
        IDWriteBitmapRenderTarget* renderTarget_; // Weak pointers because class is stack local.
        bool needToSetTransform_;
    };

    HRESULT STDMETHODCALLTYPE DrawGlyphRun(
        _In_ void* clientDrawingContext,
        _In_ float baselineOriginX,
        _In_ float baselineOriginY,
        DWRITE_MEASURING_MODE measuringMode,
        _In_ DWRITE_GLYPH_RUN const* glyphRun,
        _In_ DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
        _In_ IUnknown* clientDrawingEffect
        ) noexcept override
    {
        // Forward to newer overload.
        return DrawGlyphRun(
            clientDrawingContext,
            baselineOriginX,
            baselineOriginY,
            DWRITE_GLYPH_ORIENTATION_ANGLE_0_DEGREES,
            measuringMode,
            glyphRun,
            glyphRunDescription,
            clientDrawingEffect
            );
    }

    HRESULT STDMETHODCALLTYPE DrawGlyphRun(
        _In_ void* clientDrawingContext,
        _In_ FLOAT baselineOriginX,
        _In_ FLOAT baselineOriginY,
        _In_ DWRITE_GLYPH_ORIENTATION_ANGLE orientationAngle,
        DWRITE_MEASURING_MODE measuringMode,
        _In_ DWRITE_GLYPH_RUN const* glyphRun,
        _In_ DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
        _In_ IUnknown* clientDrawingEffect
        ) noexcept override
    {
        if (glyphRun->glyphCount <= 0)
            return S_OK;

        TransformSetter transformSetter(renderTarget_, orientationAngle, baselineOriginX, baselineOriginY, !!glyphRun->isSideways);

        #if 0 // Non color version.
        hr = target_->DrawGlyphRun(
            baselineX,
            baselineY,
            measuringMode,
            &partialGlyphRun,
            renderingParams_,
            foregroundColor,
            &rect
            );
        #else
        DrawColorGlyphRun(
            dwriteFactory_,
            renderTarget_,
            *glyphRun,
            transformSetter.currentTransform_.dwrite,
            measuringMode,
            baselineOriginX,
            baselineOriginY,
            renderingParams_,
            textColor_,
            colorPaletteIndex_
            );
        #endif

        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE DrawUnderline(
        _In_ void* clientDrawingContext,
        _In_ FLOAT baselineOriginX,
        _In_ FLOAT baselineOriginY,
        _In_ DWRITE_GLYPH_ORIENTATION_ANGLE orientationAngle,
        _In_ DWRITE_UNDERLINE const* underline,
        _In_ IUnknown* clientDrawingEffect
        ) noexcept override
    {
        DrawLine(
            clientDrawingContext,
            baselineOriginX,
            baselineOriginY,
            clientDrawingEffect,
            textColor_,
            underline->width,
            underline->offset,
            underline->thickness,
            orientationAngle
            );

        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE DrawUnderline(
        _In_ void* clientDrawingContext,
        _In_ float baselineOriginX,
        _In_ float baselineOriginY,
        _In_ DWRITE_UNDERLINE const* underline,
        _In_ IUnknown* clientDrawingEffect
        ) noexcept override
    {
        return DrawUnderline(
            clientDrawingContext,
            baselineOriginX,
            baselineOriginY,
            DWRITE_GLYPH_ORIENTATION_ANGLE_0_DEGREES,
            underline,
            clientDrawingEffect
            );
    }

    HRESULT STDMETHODCALLTYPE DrawStrikethrough(
        _In_ void* clientDrawingContext,
        _In_ float baselineOriginX,
        _In_ float baselineOriginY,
        _In_ DWRITE_GLYPH_ORIENTATION_ANGLE orientationAngle,
        _In_ DWRITE_STRIKETHROUGH const* strikethrough,
        _In_ IUnknown* clientDrawingEffect
        ) noexcept override
    {
        DrawLine(
            clientDrawingContext,
            baselineOriginX,
            baselineOriginY,
            clientDrawingEffect,
            textColor_,
            strikethrough->width,
            strikethrough->offset,
            strikethrough->thickness,
            orientationAngle
            );
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE DrawStrikethrough(
        _In_ void* clientDrawingContext,
        _In_ float baselineOriginX,
        _In_ float baselineOriginY,
        _In_ DWRITE_STRIKETHROUGH const* strikethrough,
        _In_ IUnknown* clientDrawingEffect
        ) noexcept override
    {
        return DrawStrikethrough(
            clientDrawingContext,
            baselineOriginX,
            baselineOriginY,
            DWRITE_GLYPH_ORIENTATION_ANGLE_0_DEGREES,
            strikethrough,
            clientDrawingEffect
            );
    }

    HRESULT STDMETHODCALLTYPE DrawInlineObject(
        _In_ void* clientDrawingContext,
        _In_ FLOAT originX,
        _In_ FLOAT originY,
        _In_ DWRITE_GLYPH_ORIENTATION_ANGLE orientationAngle,
        _In_ IDWriteInlineObject* inlineObject,
        _In_ BOOL isSideways,
        _In_ BOOL isRightToLeft,
        _In_ IUnknown* clientDrawingEffect
        ) noexcept override
    {
        return inlineObject->Draw(clientDrawingContext, this, originX, originY, isSideways, isRightToLeft, clientDrawingEffect);
    }

    HRESULT STDMETHODCALLTYPE DrawInlineObject(
        _In_ void* clientDrawingContext,
        _In_ float originX,
        _In_ float originY,
        _In_ IDWriteInlineObject* inlineObject,
        _In_ BOOL isSideways,
        _In_ BOOL isRightToLeft,
        _In_ IUnknown* clientDrawingEffect
        ) noexcept override
    {
        return inlineObject->Draw(clientDrawingContext, this, originX, originY, isSideways, isRightToLeft, clientDrawingEffect);
    }

    void DrawLine(
        _In_ void* clientDrawingContext,
        _In_ FLOAT baselineOriginX,
        _In_ FLOAT baselineOriginY,
        _In_ IUnknown* clientDrawingEffects,
        _In_ COLORREF defaultColor,
        _In_ float width,
        _In_ float offset,
        _In_ float thickness,
        _In_ DWRITE_GLYPH_ORIENTATION_ANGLE orientationAngle
        )
    {
        TransformSetter transformSetter(renderTarget_, orientationAngle, baselineOriginX, baselineOriginY);

        // We will always get a strikethrough or underline as a LTR rectangle with the baseline origin snapped.
        D2D1_RECT_F rectangle = {baselineOriginX + 0, baselineOriginY + offset, baselineOriginX + width, baselineOriginY + offset + thickness};

        // use GDI to draw line.
        RECT rect = {int(rectangle.left), int(rectangle.top), int(rectangle.right), int(rectangle.bottom),};
        if (rect.bottom <= rect.top)
        {
            rect.bottom = rect.top + 1;
        }

        // Draw the line
        HDC hdc = renderTarget_->GetMemoryDC();
        XFORM previousTransform;
        GetWorldTransform(hdc, OUT &previousTransform);
        SetWorldTransform(hdc, &transformSetter.currentTransform_.gdi);
        SetBkColor(hdc, defaultColor);
        ExtTextOut(
            hdc,
            0, 0,
            ETO_OPAQUE,
            &rect,
            L"",
            0,
            nullptr
            );
        SetWorldTransform(hdc, &previousTransform);
    }

    HRESULT STDMETHODCALLTYPE IsPixelSnappingDisabled(
        _In_opt_ void* clientDrawingContext,
        _Out_ BOOL* isDisabled
        ) noexcept override
    {
        *isDisabled = !enablePixelSnapping_;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE GetCurrentTransform(
        _In_opt_ void* clientDrawingContext,
        _Out_ DWRITE_MATRIX* transform
        ) noexcept override
    {
        *transform = g_identityTransform.dwrite;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE GetPixelsPerDip(
        _In_opt_ void* clientDrawingContext,
        _Out_ float* pixelsPerDip
        ) noexcept override
    {
        *pixelsPerDip = 1.0;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(IID const& iid, _Out_ void** object) noexcept override
    {
        if (iid == __uuidof(IDWriteTextRenderer1)
        ||  iid == __uuidof(IDWriteTextRenderer)
        ||  iid == __uuidof(IDWritePixelSnapping)
        ||  iid == __uuidof(IUnknown))
        {
            AddRef();
            *object = static_cast<IUnknown*>(this);
            return S_OK;
        }
        *object = nullptr;
        return E_NOINTERFACE;
    }

    unsigned long STDMETHODCALLTYPE AddRef() noexcept override
    {
        return 1; // Static stack class
    }

    unsigned long STDMETHODCALLTYPE Release() noexcept override
    {
        return 1; // Static stack class
    }

    IDWriteFactory* dwriteFactory_; // Weak pointers because class is stack local.
    IDWriteBitmapRenderTarget* renderTarget_; // Weak pointers because class is stack local.
    IDWriteRenderingParams* renderingParams_;
    COLORREF textColor_;
    uint32_t colorPaletteIndex_;
    bool enablePixelSnapping_;
};


HRESULT DrawTextLayout(
    IDWriteFactory* dwriteFactory, // Needed for TranslateColorGlyphRun.
    IDWriteBitmapRenderTarget* renderTarget,
    IDWriteRenderingParams* renderingParams,
    IDWriteTextLayout* textLayout,
    float x,
    float y,
    COLORREF textColor,
    uint32_t colorPaletteIndex,
    bool enablePixelSnapping
    ) noexcept
{
    if (renderTarget == nullptr || renderingParams == nullptr || textLayout == nullptr)
        return E_INVALIDARG;

    BitmapRenderTargetTextRenderer textRenderer(dwriteFactory, renderTarget, renderingParams, textColor, colorPaletteIndex, enablePixelSnapping);
    return textLayout->Draw(nullptr, &textRenderer, x, y);
}


namespace
{
    inline uint8_t FloatToColorByte(float c)
    {
        return static_cast<uint8_t >(floorf(c * 255 + 0.5f));
    }

    COLORREF ToCOLORREF(DWRITE_COLOR_F const& color)
    {
        return RGB(
            FloatToColorByte(color.r),
            FloatToColorByte(color.g),
            FloatToColorByte(color.b)
            );
    }

    HRESULT GetColorGlyphRunEnumerator(
        IDWriteFactory* dwriteFactory,
        DWRITE_GLYPH_RUN const& glyphRun,
        DWRITE_MATRIX const& transform,
        float baselineOriginX,
        float baselineOriginY,
        uint32_t colorPalette,
        _COM_Outptr_ IDWriteColorGlyphRunEnumerator** colorEnumerator
        )
    {
        *colorEnumerator = nullptr;
        HRESULT hr = DWRITE_E_NOCOLOR;

        ComPtr<IDWriteFontFace2> fontFace2;
        if (colorPalette != 0xFFFFFFFF && SUCCEEDED(glyphRun.fontFace->QueryInterface(OUT &fontFace2)))
        {
            uint32_t colorPaletteCount = fontFace2->GetColorPaletteCount();
            if (colorPalette >= colorPaletteCount)
                colorPalette = 0;

            ComPtr<IDWriteFactory2> factory2;
            ComPtr<IDWriteFactory4> factory4;

            // Try the newer DWrite which supports multiple glyph image formats.
            // Otherwise try the old one.
            hr = dwriteFactory->QueryInterface(OUT &factory4);
            if (SUCCEEDED(hr))
            {
                IFR(factory4->TranslateColorGlyphRun(
                    { baselineOriginX, baselineOriginY },
                    &glyphRun,
                    nullptr,
                    g_allMonochromaticOutlineGlyphImageFormats,
                    DWRITE_MEASURING_MODE_NATURAL,
                    &transform,
                    colorPalette,
                    OUT reinterpret_cast<IDWriteColorGlyphRunEnumerator1**>(colorEnumerator)
                    ));
            }
            else
            {
                IFR(dwriteFactory->QueryInterface(OUT &factory2));

                // Perform color translation.
                // Fall back to the default palette if the current palette index is out of range.
                IFR(factory2->TranslateColorGlyphRun(
                    baselineOriginX,
                    baselineOriginY,
                    &glyphRun,
                    nullptr,
                    DWRITE_MEASURING_MODE_NATURAL,
                    &transform,
                    colorPalette,
                    OUT colorEnumerator
                    ));
            }
        }
        return hr;
    }
}


HRESULT DrawColorGlyphRun(
    IDWriteFactory* dwriteFactory,
    IDWriteBitmapRenderTarget* renderTarget,
    DWRITE_GLYPH_RUN const& glyphRun,
    DWRITE_MATRIX const& transform,
    DWRITE_MEASURING_MODE measuringMode,
    float baselineOriginX,
    float baselineOriginY,
    IDWriteRenderingParams* renderingParams,
    COLORREF textColor,
    uint32_t colorPalette // 0xFFFFFFFF if none
    ) noexcept
{
    textColor &= 0x00FFFFFF; // GDI may render nothing in outline mode if alpha byte is set.

    ComPtr<IDWriteColorGlyphRunEnumerator> colorEnumerator;
    auto hr = GetColorGlyphRunEnumerator(
        dwriteFactory,
        glyphRun,
        transform,
        baselineOriginX,
        baselineOriginY,
        colorPalette,
        OUT &colorEnumerator
        );

    if (hr == DWRITE_E_NOCOLOR)
    {
        // No color information; draw the top line with no color translation.
        IFR(renderTarget->DrawGlyphRun(
                baselineOriginX,
                baselineOriginY,
                measuringMode,
                &glyphRun,
                renderingParams,
                textColor,
                nullptr // don't need blackBoxRect
                ));
        hr = S_OK;
    }
    else
    {
        ComPtr<IDWriteColorGlyphRunEnumerator1> colorEnumerator1;
        colorEnumerator->QueryInterface(OUT &colorEnumerator1);
        DWRITE_GLYPH_IMAGE_FORMATS glyphImageFormat = DWRITE_GLYPH_IMAGE_FORMATS_TRUETYPE | DWRITE_GLYPH_IMAGE_FORMATS_CFF;

        while (SUCCEEDED(hr))
        {
            BOOL haveRun;
            IFR(colorEnumerator->MoveNext(OUT &haveRun));
            if (!haveRun)
                break;

            // Get the old run or new one.
            DWRITE_COLOR_GLYPH_RUN const* colorRun = nullptr;
            if (colorEnumerator1 != nullptr)
            {
                DWRITE_COLOR_GLYPH_RUN1 const* colorRun1 = nullptr;
                IFR(colorEnumerator1->GetCurrentRun(OUT &colorRun1));
                colorRun = colorRun1;
                glyphImageFormat = colorRun1->glyphImageFormat;
            }
            else
            {
                IFR(colorEnumerator->GetCurrentRun(OUT &colorRun));
            }

            COLORREF runColor = (colorRun->paletteIndex == 0xFFFF) ? textColor : ToCOLORREF(colorRun->runColor);

            ComPtr<IDWriteColorGlyphRunEnumerator> colorLayers;

            switch (glyphImageFormat)
            {
            case DWRITE_GLYPH_IMAGE_FORMATS_TRUETYPE:
            case uint32_t(DWRITE_GLYPH_IMAGE_FORMATS_TRUETYPE)|uint32_t(DWRITE_GLYPH_IMAGE_FORMATS_COLR):
            case uint32_t(DWRITE_GLYPH_IMAGE_FORMATS_CFF)|uint32_t(DWRITE_GLYPH_IMAGE_FORMATS_COLR):
            case uint32_t(DWRITE_GLYPH_IMAGE_FORMATS_TRUETYPE)|uint32_t(DWRITE_GLYPH_IMAGE_FORMATS_CFF):
            case DWRITE_GLYPH_IMAGE_FORMATS_CFF:
                IFR(renderTarget->DrawGlyphRun(
                    colorRun->baselineOriginX,
                    colorRun->baselineOriginY,
                    measuringMode,
                    &colorRun->glyphRun,
                    renderingParams,
                    runColor,
                    nullptr // don't need blackBoxRect
                    ));
            case DWRITE_GLYPH_IMAGE_FORMATS_PNG:
                // todo:::
                break;
            }
        }
    }

    return hr;
}


HRESULT DrawColorGlyphRun(
    IDWriteFactory* dwriteFactory,
    ID2D1RenderTarget* renderTarget,
    DWRITE_GLYPH_RUN const& glyphRun,
    DWRITE_MATRIX const& transform,
    DWRITE_MEASURING_MODE measuringMode,
    float baselineOriginX,
    float baselineOriginY,
    ID2D1Brush* brush,
    uint32_t colorPalette // 0xFFFFFFFF if none
    ) noexcept
{
    ComPtr<IDWriteColorGlyphRunEnumerator> colorLayers;
    auto hr = GetColorGlyphRunEnumerator(
            dwriteFactory,
            glyphRun,
            transform,
            baselineOriginX,
            baselineOriginY,
            colorPalette,
            OUT &colorLayers
            );

    if (hr == DWRITE_E_NOCOLOR)
    {
        // No color information; draw the top line with no color translation.
        renderTarget->DrawGlyphRun(
            { baselineOriginX, baselineOriginY },
            &glyphRun,
            brush,
            measuringMode
            );
        hr = S_OK; // Remap since non-fatal.
    }
    else if (SUCCEEDED(hr))
    {
        ComPtr<ID2D1SolidColorBrush> tempBrush;
        IFR(renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), OUT &tempBrush));

        while (SUCCEEDED(hr))
        {
            BOOL haveRun;
            IFR(colorLayers->MoveNext(OUT &haveRun));
            if (!haveRun)
                break;

            DWRITE_COLOR_GLYPH_RUN const* colorRun;
            IFR(colorLayers->GetCurrentRun(OUT &colorRun));

            auto* currentBrush = brush;
            if (colorRun->paletteIndex != 0xFFFF)
            {
                tempBrush->SetColor(colorRun->runColor);
                currentBrush = tempBrush;
            }

            renderTarget->DrawGlyphRun(
                { colorRun->baselineOriginX, colorRun->baselineOriginY },
                &colorRun->glyphRun,
                currentBrush,
                measuringMode
                );
        }
    }

    return hr;
}


// Return a per-character (0..UnicodeTotal-1) coverage count for all the
// font faces, where the array index corresponds to each Unicode code point
// and is incremented once for each font that supports it. If a specific
// string of characters is passed, the counts for each character are returned.
HRESULT GetFontCharacterCoverageCounts(
    array_ref<IDWriteFontFace* const> fontFaces,
    array_ref<char32_t const> unicodeCharactersIn,
    bool getOnlyColorFontCharacters,
    std::function<void(uint32_t i, uint32_t total)> progress,
    _Out_ std::vector<uint16_t>& coverageCounts
    ) // todo: make noexcept.
{
    coverageCounts.clear();

    uint32_t unicodeCharactersCount = static_cast<char32_t>(unicodeCharactersIn.size());
    char32_t const* unicodeCharacters = unicodeCharactersIn.data();
    const bool useEntireUnicodeRange = (unicodeCharactersCount == 0);
    if (useEntireUnicodeRange)
    {
        unicodeCharactersCount = UnicodeTotal; // All Unicode characters.
    }

    coverageCounts.resize(unicodeCharactersCount);
    std::vector<char32_t> allUnicodeCharacters;
    std::vector<uint16_t> glyphIds(unicodeCharactersCount);

    if (useEntireUnicodeRange)
    {
        // No list of characters given, so return the whole Unicode array.
        allUnicodeCharacters.resize(unicodeCharactersCount);
        std::iota(allUnicodeCharacters.begin(), allUnicodeCharacters.end(), 0);
        unicodeCharacters = allUnicodeCharacters.data();
    }

    // Get all the glyphs the font faces support, and increment for each covered character.
    uint32_t fontFacesCount = static_cast<uint32_t>(fontFaces.size());
    for (uint32_t i = 0; i < fontFacesCount; ++i)
    {
        ComPtr<IDWriteFontFace4> fontFace4;
        auto* counts = coverageCounts.data();
        auto* fontFace = fontFaces[i];

        IFR(fontFace->GetGlyphIndices(reinterpret_cast<uint32_t const*>(unicodeCharacters), unicodeCharactersCount, glyphIds.data()));
        if (getOnlyColorFontCharacters)
        {
            IFR(fontFace->QueryInterface(OUT &fontFace4));
        }
        
        for (char32_t ch = 0; ch < unicodeCharactersCount; ++ch)
        {
            if (glyphIds[ch] == 0)
                continue;

            if (getOnlyColorFontCharacters)
            {
                DWRITE_GLYPH_IMAGE_FORMATS glyphImageFormats = DWRITE_GLYPH_IMAGE_FORMATS_NONE;
                IFR(fontFace4->GetGlyphImageFormats(glyphIds[ch], 0, UINT32_MAX, OUT &glyphImageFormats));
                if (!(glyphImageFormats & g_allColorGlyphImageFormats))
                {
                    continue;
                }
            }

            // Found another character supported by the font.
            if (++counts[ch] == 0)
                counts[ch] = UINT16_MAX;
        }

        progress(i, fontFacesCount);
    }

    return S_OK;
}


// Return a UTF-16 string of all the characters supported in the coverage
// count array. The character coverage range acts as filter to essentially
// apply boolean operations (union, intersection, uniqueness) across font
// coverage arrays, as if each font had its own separate parallel coverage
// array instead of being summed into a single count per character.
HRESULT GetStringFromCoverageCount(
    array_ref<uint16_t const> characterCounts,
    uint32_t lowCount,
    uint32_t highCount,
    _Out_ std::u16string& text
    ) // todo: make noexcept.
{
    // Calculate size first, determining needed size for UTF-16.
    uint32_t countsSize = static_cast<uint32_t>(characterCounts.size());
    size_t textSize = 0;
    for (char32_t ch = 1; ch < countsSize; ++ch)
    {
        auto count = characterCounts[ch];
        if (count >= lowCount && count <= highCount)
        {
            textSize += IsCharacterBeyondBmp(ch) ? 2 : 1;
        }
    }

    // Convert the UTF-32 string (with nul's) to UTF-16.
    // The code points are already in ascending order.
    text.resize(textSize);

    size_t textOffset = 0;
    for (char32_t ch = 1; ch < countsSize; ++ch)
    {
        auto count = characterCounts[ch];
        if (count >= lowCount && count <= highCount)
        {
            textOffset += ConvertUtf32ToUtf16(
                { &ch, 1 },
                { &text[textOffset], textSize - textOffset }
                );
        }
    }

    return S_OK;
}


D2D1_RECT_F GetBlackBox(const DWRITE_OVERHANG_METRICS& overhangMetrics, const DWRITE_TEXT_METRICS& textMetrics)
{
    return
        {
        /*left  */ -overhangMetrics.left,
        /*top   */ -overhangMetrics.top,
        /*right */  overhangMetrics.right + textMetrics.layoutWidth,
        /*bottom*/  overhangMetrics.bottom + textMetrics.layoutHeight
        };
}


HRESULT GetGlyphMetrics(
    IDWriteFontFace* fontFace,
    float fontEmSize,
    float pixelsPerDip,
    const DWRITE_MATRIX* transform,
    DWRITE_MEASURING_MODE measuringMode,
    bool isSideways,
    uint32_t glyphCount,
    _In_reads_(glyphCount) const uint16_t* glyphIndices,
    _Out_writes_(glyphCount) DWRITE_GLYPH_METRICS* glyphMetrics
    )
{
    switch (measuringMode)
    {
    case DWRITE_MEASURING_MODE_GDI_CLASSIC:
    case DWRITE_MEASURING_MODE_GDI_NATURAL:
            IFR(fontFace->GetGdiCompatibleGlyphMetrics(
                fontEmSize,
                pixelsPerDip,
                transform,
                (measuringMode == DWRITE_MEASURING_MODE_GDI_NATURAL),
                glyphIndices,
                glyphCount,
                OUT glyphMetrics,
                isSideways
                ));
        break;

    default:
        DEBUG_ASSERT("A new measuring mode has been added. Handle it here.");
        __fallthrough;

    case DWRITE_MEASURING_MODE_NATURAL:
        IFR(fontFace->GetDesignGlyphMetrics(
                glyphIndices,
                glyphCount,
                OUT glyphMetrics,
                isSideways
                ));
        break;
    }

    return S_OK;
}


DWRITE_GLYPH_ORIENTATION_ANGLE GetRelativeOrientation(bool isSideways, bool isFlippedOrientation) noexcept
{
    // Determine a relative angle from the flags.
    //
    //  isSideways  isFlipped ->    Angle
    //      F           F           0
    //      T           T           90
    //      F           T           180
    //      T           F           270

    DWRITE_GLYPH_ORIENTATION_ANGLE          rotationAmount = DWRITE_GLYPH_ORIENTATION_ANGLE_0_DEGREES;
    if (isSideways)                         rotationAmount = DWRITE_GLYPH_ORIENTATION_ANGLE(rotationAmount + 1);
    if (isSideways != isFlippedOrientation) rotationAmount = DWRITE_GLYPH_ORIENTATION_ANGLE(rotationAmount + 2);

    return rotationAmount;
}


void AccumulateRect(_Inout_ D2D1_RECT_F& modifiedRect, D2D1_RECT_F const& otherRect)
{
    if (otherRect.left   < modifiedRect.left)    modifiedRect.left = otherRect.left;
    if (otherRect.right  > modifiedRect.right)   modifiedRect.right = otherRect.right;
    if (otherRect.top    < modifiedRect.top)     modifiedRect.top = otherRect.top;
    if (otherRect.bottom > modifiedRect.bottom)  modifiedRect.bottom = otherRect.bottom;
}


inline void OffsetRect(_Inout_ D2D1_RECT_F& modifiedRect, float x, float y)
{
    modifiedRect.left   += x;
    modifiedRect.top    += y;
    modifiedRect.right  += x;
    modifiedRect.bottom += y;
}


template<typename RectType>
void RotateRect(
    _Inout_ RectType& modifiedRect,
    DWRITE_GLYPH_ORIENTATION_ANGLE rotationAmount
    )
{
    // Rotate the corners around the zero point <0,0>, whether it be a glyph
    // or inline object.
    switch (rotationAmount)
    {
    case DWRITE_GLYPH_ORIENTATION_ANGLE_0_DEGREES:
        break; // do nothing

    case DWRITE_GLYPH_ORIENTATION_ANGLE_90_DEGREES:
        {
            auto oldLeft          =  modifiedRect.left;
            modifiedRect.left     =  modifiedRect.bottom;
            modifiedRect.bottom   = -modifiedRect.right;
            modifiedRect.right    =  modifiedRect.top;
            modifiedRect.top      = -oldLeft;
        }
        break;

    case DWRITE_GLYPH_ORIENTATION_ANGLE_180_DEGREES: // for stacked Arabic
        {
            std::swap(modifiedRect.left, modifiedRect.right);
            std::swap(modifiedRect.top, modifiedRect.bottom);
            modifiedRect.left     = -modifiedRect.left;
            modifiedRect.right    = -modifiedRect.right;
            modifiedRect.top      = -modifiedRect.top;
            modifiedRect.bottom   = -modifiedRect.bottom;
        }
        break;

    case DWRITE_GLYPH_ORIENTATION_ANGLE_270_DEGREES: // for ideographs (isSideways = true)
        {
            auto oldLeft          =  modifiedRect.left;
            modifiedRect.left     =  modifiedRect.top;
            modifiedRect.top      = -modifiedRect.right;
            modifiedRect.right    =  modifiedRect.bottom;
            modifiedRect.bottom   = -oldLeft;
        }
        break;

    default:
        DEBUG_ASSERT("Update the code to handle the new orientation.");
    }
}


// Checks whether a glyph has a non-empty black box.
inline bool GlyphHasSize(DWRITE_GLYPH_METRICS const& glyphMetrics) noexcept
{
    return static_cast<int64_t>(glyphMetrics.advanceWidth)  - glyphMetrics.leftSideBearing - glyphMetrics.rightSideBearing  > 0
        && static_cast<int64_t>(glyphMetrics.advanceHeight) - glyphMetrics.topSideBearing  - glyphMetrics.bottomSideBearing > 0;
}


HRESULT GetGlyphRunBlackBox(
    float baselineOriginX,
    float baselineOriginY,
    IDWriteFontFace* fontFace,
    float fontEmSize,
    float pixelsPerDip,
    const DWRITE_MATRIX* transform,
    DWRITE_MEASURING_MODE measuringMode,
    bool isRtlContent,
    bool isRtlSpan,
    bool isSideways,
    uint32_t glyphCount,
    _In_reads_(glyphCount) const uint16_t* glyphIndices,
    _In_reads_(glyphCount) const float* glyphAdvances,
    _In_reads_opt_(glyphCount) const DWRITE_GLYPH_OFFSET* glyphOffsets,
    _Inout_ D2D1_RECT_F& blackBox
)
{
    if (glyphCount <= 0)
    {
        return S_OK;
    }

    DWRITE_FONT_METRICS fontMetrics = {};
    fontFace->GetMetrics(&fontMetrics);

    DWRITE_GLYPH_METRICS smallBuffer[120];
    std::vector<DWRITE_GLYPH_METRICS> bigBuffer;
    DWRITE_GLYPH_METRICS* glyphRunMetrics = smallBuffer;

    if (glyphCount > std::size(smallBuffer))
    {
        bigBuffer.resize(glyphCount);
        glyphRunMetrics = &bigBuffer[0];
    }

    IFR(GetGlyphMetrics(
        fontFace,
        fontEmSize,
        pixelsPerDip,
        transform,
        measuringMode,
        isSideways,
        glyphCount,
        glyphIndices,
        OUT glyphRunMetrics
    ));

    D2D1_RECT_F accumulatedBounds;
    float       baselineX = baselineOriginX;
    float const baselineY = baselineOriginY;
    float const fontScale = fontEmSize / fontMetrics.designUnitsPerEm;

    // Determine how to rotate the blackbox of each glyph from the flags.

    bool const isFlippedOrientation = (isRtlContent != isRtlSpan);
    DWRITE_GLYPH_ORIENTATION_ANGLE const rotationAmount = GetRelativeOrientation(isSideways, isFlippedOrientation);

    for (uint32_t j = 0; j < glyphCount; ++j)
    {
        DWRITE_GLYPH_METRICS const& glyphMetrics = glyphRunMetrics[j];
        float const nominalGlyphAdvance = (isSideways ? glyphMetrics.advanceHeight : glyphMetrics.advanceWidth) * fontScale;
        float const glyphAdvance = (glyphAdvances != nullptr) ? glyphAdvances[j] : nominalGlyphAdvance;

        float glyphX = baselineX, glyphY = baselineY;

        // When glyphs are drawn RTL, the glyph origin is on the opposite
        // side of the pen. So add the nominal advance to get the glyph origin
        // (the nominal advance, not the user supplied advance). The sign
        // depends on whether the glyphs are increasing/decreasing in
        // coordinate space which depends on the direction of the span
        // rather than the content (since upside-down RTL text is effectively
        // LTR).
        if (isRtlContent)
        {
            glyphX += isRtlSpan ? -nominalGlyphAdvance : nominalGlyphAdvance;
        }
        // Advance the pen.
        baselineX += isRtlSpan ? -glyphAdvance : glyphAdvance;

        // Skip any blank glyphs like the space character, which has no black box.
        if (!GlyphHasSize(glyphMetrics))
            continue;

        // For sideways glyphs like ideographs in vertical, move from the pen
        // position (which is relative to the vertical origin) to the glyph
        // origin at the bottom left (0,0 in font design space). The delta
        // from the vertical origin to the horizontal origin is the vertical
        // origin distance and half the advance width.
        if (isSideways)
        {
            // If the glyph run is flipped relative to the line, then reverse
            // the signs, since the glyph is turned 180 degrees relative to
            // the coordinate space.
            int32_t originY = glyphMetrics.verticalOriginY;
            int32_t advance = glyphMetrics.advanceWidth;
            glyphX += (isFlippedOrientation ? -originY : originY) * fontScale;
            glyphY += (isFlippedOrientation ? -advance : advance) * fontScale * 0.5f;
        }

        // Determine ink edges of glyph (in font design units but Cartesian coordinates).
        // The four corners are relative to the horizontal glyph origin now.
        D2D_RECT_L intBounds = {
            int32_t(glyphMetrics.leftSideBearing),
            int32_t(glyphMetrics.topSideBearing - glyphMetrics.verticalOriginY),
            int32_t(glyphMetrics.advanceWidth - glyphMetrics.rightSideBearing),
            int32_t(glyphMetrics.advanceHeight - glyphMetrics.bottomSideBearing - glyphMetrics.verticalOriginY)
        };

        RotateRect(IN OUT intBounds, rotationAmount);

        // Add the glyph offset from shaping/character spacing/justification.
        if (glyphOffsets != nullptr)
        {
            float advanceOffset = glyphOffsets[j].advanceOffset;
            float ascenderOffset = glyphOffsets[j].ascenderOffset;
            glyphX += isRtlSpan ? -advanceOffset : advanceOffset;
            glyphY += isFlippedOrientation ? ascenderOffset : -ascenderOffset;
        }

        // Scale the font design units by the font size to get DIPs
        // and add the glyph position to move from glyph space to layout space.
        D2D1_RECT_F glyphBounds = {
            float(intBounds.left)   * fontScale,
            float(intBounds.top)    * fontScale,
            float(intBounds.right)  * fontScale,
            float(intBounds.bottom) * fontScale
        };
        OffsetRect(IN OUT glyphBounds, glyphX, glyphY);

        AccumulateRect(IN OUT accumulatedBounds, glyphBounds);
    }

    AccumulateRect(IN OUT blackBox, accumulatedBounds);

    return S_OK;
}


HRESULT GetGlyphRunBlackBox(
    DWRITE_GLYPH_RUN const& glyphRun,
    float baselineOriginX,
    float baselineOriginY,
    _Inout_ D2D1_RECT_F& blackBox
    )
{
    return GetGlyphRunBlackBox(
        baselineOriginX,
        baselineOriginY,
        glyphRun.fontFace,
        glyphRun.fontEmSize,
        1, // pixelsPerDip
        nullptr, // transform
        DWRITE_MEASURING_MODE_NATURAL,
        glyphRun.bidiLevel & 1,
        glyphRun.bidiLevel & 1,
        glyphRun.isSideways,
        glyphRun.glyphCount,
        glyphRun.glyphIndices,
        glyphRun.glyphAdvances,
        glyphRun.glyphOffsets,
        OUT blackBox
    );
}
