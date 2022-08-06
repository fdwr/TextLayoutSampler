//----------------------------------------------------------------------------
//  History:    2008-02-11 Dwayne Robinson - Created
//              2015-06-24 Split into base class and Windows control.
//----------------------------------------------------------------------------

#if USE_CPP_MODULES
    module;
#endif

#include "precomp.h"

#pragma prefast(disable:__WARNING_ACCESSIBILITY_COLORAPI, "Shush. It's a test program.")
#pragma prefast(disable:__WARNING_HARDCODED_FONT_INFO,    "Shush. It's a test program.")
#pragma warning(disable:4351)   // Yes, we're aware that POD's are default initialized.
                                // This warning is just for those upgrading 2005 to 2008.

#pragma comment(lib, "DWrite.lib")
#pragma comment(lib, "D2D1.lib")
#pragma comment(lib, "GdiPlus.lib")
#pragma comment(lib, "WindowsCodecs.lib")

#if USE_CPP_MODULES
    export module DrawingCanvas;
    import Common.String;
    import Common.AutoResource;
    import Common.AutoResource.Windows;
    import DWritEx;
    export
    {
        #include "DrawingCanvas.h"
    }
#else
    #include "Common.ArrayRef.h"
    #include "Common.String.h"
    #include "Common.AutoResource.h"
    #include "Common.AutoResource.Windows.h"
    #include "DWritEx.h"
    #include "DrawingCanvas.h"
#endif


////////////////////////////////////////

// WinCodec_Proxy.h appears to be missing from Visual Studio's include path??
extern "C"
{
    HRESULT WINAPI WICCreateImagingFactory_Proxy(
        __in UINT32 sdkVersion,
        __deref_out IWICImagingFactory** wicImagingFactory
        );
}


const DX_MATRIX_3X2F DrawingCanvas::g_identityMatrix = {1,0,0,1,0,0};
const GUID DrawingCanvas::g_guid = { 0x74868E11, 0xF1CF, 0x461A, 0xAE,0xAF,0x17,0x52,0x16,0xDF,0xF0,0xBA };


void ComputeInverseMatrix(
    _In_  DX_MATRIX_3X2F const& matrix,
    _Out_ DX_MATRIX_3X2F& result
    )
{
    DEBUG_ASSERT(&matrix != &result);

    float invdet = 1.f / GetDeterminant(matrix);
    result.xx =  matrix.yy * invdet;
    result.xy = -matrix.xy * invdet;
    result.yx = -matrix.yx * invdet;
    result.yy =  matrix.xx * invdet;
    result.dx = (matrix.yx * matrix.dy - matrix.dx * matrix.yy) * invdet;
    result.dy = (matrix.dx * matrix.xy - matrix.xx * matrix.dy) * invdet;
}


// todo::: consider moving out of here to more generic shared location.

// In and out rects are allowed to be the same variable.
void TransformRect(D2D_MATRIX_3X2_F const& transform, D2D_RECT_F const& rectIn, _Out_ D2D_RECT_F& rectOut)
{
    // Equivalent to calling D2D1::Matrix3x2F.TransformPoint on each
    // corner of the rect, but half the multiplies and additions.

    float leftX   = rectIn.left   * transform._11;
    float leftY   = rectIn.left   * transform._12;
    float rightX  = rectIn.right  * transform._11;
    float rightY  = rectIn.right  * transform._12;
    float topX    = rectIn.top    * transform._21;
    float topY    = rectIn.top    * transform._22;
    float bottomX = rectIn.bottom * transform._21;
    float bottomY = rectIn.bottom * transform._22;

    D2D_POINT_2F points[4];
    points[0].x = leftX  + topX   ;
    points[0].y = leftY  + topY   ;
    points[1].x = rightX + topX   ;
    points[1].y = rightY + topY   ;
    points[2].x = leftX  + bottomX;
    points[2].y = leftY  + bottomY;
    points[3].x = rightX + bottomX;
    points[3].y = rightY + bottomY;

    rectOut.left = rectOut.right = points[0].x;
    rectOut.top = rectOut.bottom = points[0].y;
    for (uint32_t i = 1; i < 4; ++i)
    {
        if (points[i].x < rectOut.left)   rectOut.left   = points[i].x;
        if (points[i].x > rectOut.right)  rectOut.right  = points[i].x;
        if (points[i].y < rectOut.top)    rectOut.top    = points[i].y;
        if (points[i].y > rectOut.bottom) rectOut.bottom = points[i].y;
    }
    rectOut.left   += transform._31;
    rectOut.right  += transform._31;
    rectOut.top    += transform._32;
    rectOut.bottom += transform._32;
}


float GetDeterminant(_In_ DX_MATRIX_3X2F const& matrix)
{
    return matrix.xx * matrix.yy - matrix.xy * matrix.yx;
}


D2D_POINT_2F TransformPoint(DX_MATRIX_3X2F const& matrix, D2D_POINT_2F point)
{
    return {
        point.x * matrix.xx + point.y * matrix.yx + matrix.dx,
        point.y * matrix.yy + point.x * matrix.xy + matrix.dy
        };
}


void UnionRect(D2D_RECT_F const& rectIn, _Inout_ D2D_RECT_F& rectInOut)
{
    if (rectIn.left   < rectInOut.left)   rectInOut.left = rectIn.left;
    if (rectIn.right  > rectInOut.right)  rectInOut.right = rectIn.right;
    if (rectIn.top    < rectInOut.top)    rectInOut.top = rectIn.top;
    if (rectIn.bottom > rectInOut.bottom) rectInOut.bottom = rectIn.bottom;
}


void PixelAlignRect(_Inout_ D2D_RECT_F& rectInOut)
{
    rectInOut.left   = floor(rectInOut.left);
    rectInOut.top    = floor(rectInOut.top);
    rectInOut.right  = ceil(rectInOut.right);
    rectInOut.bottom = ceil(rectInOut.bottom);
}


void TranslateRect(D2D_POINT_2F offset, _Inout_ D2D_RECT_F& rectInOut)
{
    rectInOut.left   += offset.x;
    rectInOut.top    += offset.y;
    rectInOut.right  += offset.x;
    rectInOut.bottom += offset.y;
}


void ConvertRect(D2D_RECT_F const& rectIn, _Out_ RECT& rectOut)
{
    rectOut.left   = LONG(floor(rectIn.left));
    rectOut.top    = LONG(floor(rectIn.top));
    rectOut.right  = LONG(ceil(rectIn.right));
    rectOut.bottom = LONG(ceil(rectIn.bottom));
}


void ConvertRect(RECT const& rectIn, _Out_ D2D_RECT_F& rectOut)
{
    rectOut.left   = float(rectIn.left);
    rectOut.top    = float(rectIn.top);
    rectOut.right  = float(rectIn.right);
    rectOut.bottom = float(rectIn.bottom);
}


bool IsPointInRect(D2D_RECT_F const& rectIn, float x, float y)
{
    return x >= rectIn.left && x < rectIn.right
        && y >= rectIn.top  && y < rectIn.bottom;
}


D2D1_COLOR_F ToD2DColor(/*BGRA*/uint32_t color)
{
    D2D1_COLOR_F convertedColor = {
        static_cast<FLOAT>((color >> 16) & 255) / 255.f,
        static_cast<FLOAT>((color >>  8) & 255) / 255.f,
        static_cast<FLOAT>((color >>  0) & 255) / 255.f,
        static_cast<FLOAT>((color >> 24) & 255) / 255.f
    };
    return convertedColor;
}


////////////////////////////////////////
// Lifetime


DrawingCanvas::~DrawingCanvas()
{
    UninitializeForRendering();
}


void DrawingCanvas::UninitializeForRendering()
{
    target_.Clear();
    targetD2D_.Clear();
    brush_.Clear(); // reusable scratch brush for current color
    renderingParams_.Clear();
    gdiInterop_.Clear();
    dwriteFactory_.Clear();
    d2dFactory_.Clear();
    gdiplusToken_.Clear();
    wicFactory_.Clear();
}


void DrawingCanvas::Clone(_COM_Outptr_ DrawingCanvas** newDrawingCanvas)
{
    auto* drawingCanvas = new DrawingCanvas();

    // Copy over selected items to reduce cost,
    // but not unshareable state like render targets.
    drawingCanvas->AddRef();
    drawingCanvas->d2dFactory_ = d2dFactory_;
    drawingCanvas->dwriteFactory_ = dwriteFactory_;
    drawingCanvas->renderingParams_ = renderingParams_;
    drawingCanvas->gdiInterop_ = gdiInterop_;

    *newDrawingCanvas = drawingCanvas;
}


void DrawingCanvas::SetDWriteFactory(IDWriteFactory* factory)
{
    dwriteFactory_.Set(factory);
}


void DrawingCanvas::SetD2DFactory(ID2D1Factory* factory)
{
    d2dFactory_.Set(factory);
}


void DrawingCanvas::SetWicFactory(IWICImagingFactory* factory)
{
    wicFactory_.Set(factory);
}


////////////////////////////////////////
// Drawing related


bool DrawingCanvas::PaintPrepare(HDC displayHdc, RECT const& rect)
{
    // Create render target on demand
    if (FAILED(CreateRenderTargetsOnDemand(displayHdc, {rect.right - rect.left, rect.bottom - rect.top})))
    {
        return false;
    }

    DEBUG_ASSERT(target_    != nullptr);
    DEBUG_ASSERT(targetD2D_ != nullptr);

    HDC memoryHdc = target_->GetMemoryDC();
    SetGraphicsMode(memoryHdc, GM_ADVANCED);
    //targetD2D_->BindDC(memoryHdc, &rect); // Already done by resize.

    return true;
}


bool DrawingCanvas::PaintFinish(HDC displayHdc, RECT const& rect)
{
    if (target_ == nullptr)
        return false;

    HDC memoryHdc = target_->GetMemoryDC();
    if (displayHdc == memoryHdc)
        return true; // No work.

    // Restore any modified transform to display final bitmap,
    // so the bitmap isn't placed in weird places.

    SetWorldTransform(memoryHdc, (XFORM const*)&g_identityMatrix);
    auto oldMode = SetGraphicsMode(memoryHdc, GM_COMPATIBLE);
    BitBlt(
        displayHdc,
        rect.left, rect.top,
        rect.right, rect.bottom,
        memoryHdc,
        rect.left, rect.top,
        SRCCOPY
        );
    SetGraphicsMode(memoryHdc, oldMode);

    return true;
}


HRESULT DrawingCanvas::InitializeRendering()
{
    if (dwriteFactory_ == nullptr)
    {
        IFR(DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(OUT &dwriteFactory_)
            ));
    }

    if (renderingParams_ == nullptr)
    {
        IFR(dwriteFactory_->CreateRenderingParams(OUT &renderingParams_));

        /*
        IFR(dwriteFactory_->CreateCustomRenderingParams(
            1.8f,   // Default values used by DWrite.
            0.5f,
            0.5f,
            g_PixelGeometry,
            g_RenderingMode,
            &renderingParams_
            ));
            */
    }

    if (gdiInterop_ == nullptr)
    {
        IFR(dwriteFactory_->GetGdiInterop(OUT &gdiInterop_));
    }

    if (d2dFactory_ == nullptr)
    {
        IFR(D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED,
            &d2dFactory_
            ));
    }

    if (wicFactory_ == nullptr)
    {
        IFR(WICCreateImagingFactory_Proxy(WINCODEC_SDK_VERSION1, OUT &wicFactory_));
    }

    if (gdiplusToken_ == NULL)
    {
        Gdiplus::GdiplusStartupInputEx gdiplusStartupInput(Gdiplus::GdiplusStartupDefault);
        Gdiplus::GdiplusStartup(OUT &gdiplusToken_, &gdiplusStartupInput, nullptr);
        if (gdiplusToken_ == NULL)
        {
            return E_FAIL;
        }
    }

    return S_OK;
}


HRESULT DrawingCanvas::CreateRenderTargetsOnDemand(_In_opt_ HDC templateHdc, SIZE size)
{
    if (target_ != nullptr && targetD2D_ != nullptr)
    {
        return S_OK;
    }

    // Get dimensions of window and layout to test.
    size.cx = std::max<int>(size.cx, 1);
    size.cy = std::max<int>(size.cy, 1);

    IFR(InitializeRendering());

    if (target_ == nullptr)
    {
        IFR(gdiInterop_->CreateBitmapRenderTarget(templateHdc, size.cx, size.cy, OUT &target_));

        // Don't let this additional factor complicate everything from hit-testing to drawing.
        // Fold this scaling into the transform.
        target_->SetPixelsPerDip(1.0);
    }

    if (targetD2D_ == nullptr)
    {
        // Create a D2D render target.
        D2D1_RENDER_TARGET_PROPERTIES targetProperties =
            D2D1::RenderTargetProperties(
                D2D1_RENDER_TARGET_TYPE_SOFTWARE,
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED), // D2D1_ALPHA_MODE_IGNORE
                0,0,
                D2D1_RENDER_TARGET_USAGE_NONE
                );

        IFR(d2dFactory_->CreateDCRenderTarget(
                        &targetProperties,
                        &targetD2D_
                        ));

        // Any scaling will be combined into matrix transforms rather than an
        // additional DPI scaling. This simplifies the logic for rendering
        // and hit-testing. If an application does not use matrices, then
        // using the scaling factor directly is simpler.

        targetD2D_->SetDpi(96.0, 96.0);
        targetD2D_->SetTextRenderingParams(renderingParams_);

        HDC memoryHdc = target_->GetMemoryDC();
        RECT bindRect = {0,0,size.cx, size.cy};
        targetD2D_->BindDC(memoryHdc, &bindRect);

        // Clear the old brush from any previous render target.
        brush_.Clear();
    }

    // Create a reusable scratch brush, rather than allocating one for
    // each new color.
    if (brush_ == nullptr)
    {
        IFR(targetD2D_->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &brush_));
    }

    return S_OK;
}


HRESULT DrawingCanvas::ResizeRenderTargets(SIZE size)
{
    if (target_ == nullptr || targetD2D_ == nullptr)
    {
        return E_NOT_VALID_STATE;
    }

    target_->Resize(size.cx, size.cy);
    RECT bindRect = {0,0,size.cx, size.cy};
    targetD2D_->BindDC(target_->GetMemoryDC(), &bindRect);
    return S_OK;
}


void DrawingCanvas::SwitchRenderingAPI(CurrentRenderingApi currentRenderingApi)
{
    if (currentRenderingApi == currentRenderingApi_)
        return;

    switch (currentRenderingApi_)
    {
    case CurrentRenderingApiD2D:
        if (targetD2D_ != nullptr)
        {
            targetD2D_->EndDraw();
        }
        break;

    case CurrentRenderingApiGdi:
    case CurrentRenderingApiGdiPlus:
        GdiFlush();
        break;

    default:
        break; // No other API's currently have special Begin/End needs.
    }

    currentRenderingApi_ = currentRenderingApi;

    switch (currentRenderingApi)
    {
    case CurrentRenderingApiD2D:
        if (targetD2D_ != nullptr)
        {
            targetD2D_->BeginDraw();
        }
        break;

    default:
        break; // No other API's currently have special Begin/End needs.
    }
}


void DrawingCanvas::SetDirectWriteRenderingParams(IDWriteRenderingParams* renderingParams)
{
    renderingParams_.Set(renderingParams);

    if (targetD2D_ != nullptr)
    {
        targetD2D_->SetTextRenderingParams(renderingParams_);
    }
}


HRESULT DrawingCanvas::GetSharedResource(
    UUID const& typeUuid,
    _In_z_ char16_t const* name,
    _COM_Outptr_ IUnknown** resource
    )
{
    for (auto& sharedResource : sharedResources_)
    {
        if (sharedResource.typeUuid == typeUuid
        &&  sharedResource.name.compare(name) == 0
        &&  sharedResource.resource != nullptr)
        {
            auto hr = sharedResource.resource->QueryInterface(typeUuid, OUT reinterpret_cast<void**>(resource));
            if (*resource != nullptr)
            {
                sharedResource.freshnessCount = std::max(sharedResource.freshnessCount, 1u);
            }
            return hr;
        }
    }

    return E_NOT_SET;
}


HRESULT DrawingCanvas::SetSharedResource(
    UUID const& typeUuid,
    _In_z_ char16_t const* name,
    IUnknown* resource
    )
{
    SharedResource* matchingSharedResource = nullptr;
    for (auto& sharedResource : sharedResources_)
    {
        if (sharedResource.typeUuid == typeUuid && sharedResource.name.compare(name) == 0)
        {
            matchingSharedResource = &sharedResource;
            break;
        }
    }

    // No existing resource was found.
    if (matchingSharedResource == nullptr)
    {
        if (resource == nullptr)
            return S_OK; // Nothing was set. Return early.

        // Allocate a new one at the end.
        sharedResources_.resize(sharedResources_.size() + 1);
        matchingSharedResource = &sharedResources_.back();
        matchingSharedResource->typeUuid = typeUuid;
        matchingSharedResource->name = name;
    }

    // Increment the access freshness.
    matchingSharedResource->freshnessCount = std::max(matchingSharedResource->freshnessCount, 1u);
    matchingSharedResource->resource.Set(resource);

    return S_OK;
}


HRESULT DrawingCanvas::ClearSharedResources()
{
    sharedResources_.clear();
    return S_OK;
}


HRESULT DrawingCanvas::RetireStaleSharedResources()
{
    for (auto sharedResource = sharedResources_.begin(); sharedResource != sharedResources_.end(); )
    {
        if (sharedResource->freshnessCount == 0)
        {
            sharedResource = sharedResources_.erase(sharedResource);
        }
        else
        {
            sharedResource->freshnessCount--;
            ++sharedResource;
        }
    }
    return S_OK;
}


DrawingCanvas::RawPixels DrawingCanvas::GetRawPixels()
{
    DrawingCanvas::RawPixels rawPixels = {};
    if (target_ == nullptr)
        return rawPixels;

    HDC memoryHdc = target_->GetMemoryDC();
    if (memoryHdc == nullptr)
    {
        return rawPixels;
    }

    DIBSECTION destBitmapInfo = {};

    // Display the alpha channel in-place.
    HBITMAP destBitmap = (HBITMAP)GetCurrentObject(memoryHdc, OBJ_BITMAP);
    GetObject(destBitmap, sizeof(destBitmapInfo), &destBitmapInfo);

    rawPixels.width = destBitmapInfo.dsBm.bmWidth;
    rawPixels.height = destBitmapInfo.dsBm.bmHeight;
    rawPixels.byteStride = destBitmapInfo.dsBm.bmWidthBytes;
    rawPixels.bitsPerPixel = destBitmapInfo.dsBm.bmBitsPixel;
    rawPixels.pixels = destBitmapInfo.dsBm.bmBits;
    if (rawPixels.pixels == nullptr)
    {
        rawPixels.width = 0;
        rawPixels.height = 0;
    }

    return rawPixels;
}


void DrawingCanvas::ClearBackground(uint32_t color)
{
    DEBUG_ASSERT(target_ != nullptr); // should have called PaintPrepare

    RawPixels rawPixels = GetRawPixels();
    if (rawPixels.bitsPerPixel != 32)
        return;

    uint8_t* destPixels = reinterpret_cast<uint8_t*>(rawPixels.pixels);
    uint32_t* destRow   = reinterpret_cast<uint32_t*>(destPixels);

    // Clear each scanline in-place.
    for (uint32_t y = 0; y < rawPixels.height; ++y)
    {
        for (uint32_t x = 0; x < rawPixels.width; ++x)
        {
            destRow[x] = color;
        }
        destRow = PtrAddByteOffset(destRow, rawPixels.byteStride);
    }
}


void DrawingCanvas::DrawAlphaChannel()
{
    DEBUG_ASSERT(target_ != nullptr); // should have called PaintPrepare

    RawPixels rawPixels = GetRawPixels();
    if (rawPixels.bitsPerPixel != 32)
        return;

    uint8_t* destPixels = reinterpret_cast<uint8_t*>(rawPixels.pixels);
    uint32_t* destRow   = reinterpret_cast<uint32_t*>(destPixels);

    // Modify each scanline in-place.
    for (uint32_t y = 0; y < rawPixels.height; ++y)
    {
        for (uint32_t x = 0; x < rawPixels.width; ++x)
        {
            // Duplicate the alpha channel in the other three color channels.
            uint32_t alpha = (destRow[x] >> 24);
            destRow[x] = (alpha<<0) | (alpha<<8) | (alpha<<16) | (alpha<<24);
        }
        destRow = PtrAddByteOffset(destRow, rawPixels.byteStride);
    }
}


void DrawingCanvas::DrawGrid(uint32_t color, uint32_t step)
{
    DEBUG_ASSERT(target_ != nullptr); // should have called PaintPrepare

    RawPixels rawPixels = GetRawPixels();
    if (rawPixels.bitsPerPixel != 32)
        return;

    uint8_t* destPixels = reinterpret_cast<uint8_t*>(rawPixels.pixels);
    uint32_t* destRow   = reinterpret_cast<uint32_t*>(destPixels);

    // Modify each scanline in-place.
    uint32_t yMod = 0;
    uint32_t yLineMod = 0;
    for (uint32_t y = 0; y < rawPixels.height; ++y)
    {
        // Horizontal line
        if (yMod == 0)
        {
            uint32_t pixelColor = (yLineMod == 0) ? 0 : color;
            ++yLineMod;
            if (yLineMod >= step)
            {
                yLineMod = 0;
            }
            for (uint32_t x = 0; x < rawPixels.width; ++x)
            {
                destRow[x] = pixelColor;
            }
        }
        // Segments of vertical line.
        uint32_t xLineMod = 0;
        for (uint32_t x = 0; x < rawPixels.width; x += step)
        {
            uint32_t pixelColor = (xLineMod == 0) ? 0 : color;
            ++xLineMod;
            if (xLineMod >= step)
            {
                xLineMod = 0;
            }
            // Duplicate the alpha channel in the other three color channels.
            destRow[x] = pixelColor;
        }
        ++yMod;
        if (yMod >= step)
            yMod = 0;
        destRow = PtrAddByteOffset(destRow, rawPixels.byteStride);
    }
}


namespace
{
    // The often copy&pasted code for loading an
    // image from a resource into D2D...

    HRESULT LoadAndLockResource(
        const char16_t* resourceName,
        const char16_t* resourceType,
        OUT UINT8** fileData,
        OUT DWORD* fileSize
        )
    {
        HRSRC resourceHandle = nullptr;
        HGLOBAL resourceDataHandle = nullptr;
        *fileData = nullptr;
        *fileSize = 0;

        HMODULE moduleHandle = nullptr;
        GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)&LoadAndLockResource, &moduleHandle);

        // Locate the resource handle in our DLL.
        resourceHandle = FindResourceW(
            moduleHandle,
            ToWChar(resourceName),
            ToWChar(resourceType)
            );
        if (resourceHandle == nullptr)
        {
            return E_FAIL;
        }

        // Load the resource.
        resourceDataHandle = LoadResource(moduleHandle, resourceHandle);

        if (resourceDataHandle == nullptr)
        {
            return E_FAIL;
        }

        // Lock it to get a system memory pointer.
        *fileData = (BYTE*)LockResource(resourceDataHandle);
        if (*fileData == nullptr)
        {
            return E_FAIL;
        }

        // Calculate the size.
        *fileSize = SizeofResource(moduleHandle, resourceHandle);
        if (*fileSize == 0)
        {
            return E_FAIL;
        }

        return S_OK;
    }


    HRESULT LoadImageFromResource(
        const char16_t* resourceName,
        const char16_t* resourceType,
        IWICImagingFactory* wicFactory,
        OUT IWICBitmapSource** bitmap
        )
    {
        // Loads an image from a resource into the given bitmap.

        DWORD fileSize;
        UINT8* fileData; // [fileSize]

        ComPtr<IWICStream> stream;
        ComPtr<IWICBitmapDecoder> decoder;
        ComPtr<IWICBitmapFrameDecode> source;
        ComPtr<IWICFormatConverter> converter;

        IFR(LoadAndLockResource(resourceName, resourceType, &fileData, &fileSize));

        // Create a WIC stream to map onto the memory.
        IFR(wicFactory->CreateStream(&stream));

        // Initialize the stream with the memory pointer and size.
        IFR(stream->InitializeFromMemory(reinterpret_cast<BYTE*>(fileData), fileSize));

        // Create a decoder for the stream.
        IFR(wicFactory->CreateDecoderFromStream(
                stream,
                nullptr,
                WICDecodeMetadataCacheOnLoad,
                &decoder
                ));

        // Create the initial frame.
        IFR(decoder->GetFrame(0, &source));

        // Convert format to 32bppPBGRA - which D2D expects.
        IFR(wicFactory->CreateFormatConverter(&converter));

        IFR(converter->Initialize(
                source,
                GUID_WICPixelFormat32bppPBGRA,
                WICBitmapDitherTypeNone,
                nullptr,
                0.f,
                WICBitmapPaletteTypeMedianCut
                ));

        *bitmap = converter.Detach();

        return S_OK;
    }


    HRESULT LoadImageFromFile(
        const char16_t* fileName,
        IWICImagingFactory* wicFactory,
        OUT IWICBitmapSource** bitmap
        )
    {
        // Loads an image from a file into the given bitmap.

        // create a decoder for the stream
        ComPtr<IWICBitmapDecoder>      decoder;
        ComPtr<IWICBitmapFrameDecode>  source;
        ComPtr<IWICFormatConverter>    converter;

        IFR(wicFactory->CreateDecoderFromFilename(
                ToWChar(fileName),
                nullptr,
                GENERIC_READ,
                WICDecodeMetadataCacheOnLoad,
                &decoder
                ));

        // Create the initial frame.
        IFR(decoder->GetFrame(0, &source));

        // Convert format to 32bppPBGRA - which D2D expects.
        IFR(wicFactory->CreateFormatConverter(&converter));

        IFR(converter->Initialize(
            source,
            GUID_WICPixelFormat32bppPBGRA,
            WICBitmapDitherTypeNone,
            nullptr,
            0.f,
            WICBitmapPaletteTypeMedianCut
            ));

        *bitmap = converter.Detach();

        return S_OK;
    }

    uint32_t GetSolidLineCount(
        uint32_t* pixels,
        size_t bytesPerRow,
        uint32_t width,
        uint32_t height,
        uint32_t maxCount,
        uint32_t direction, // 0-top, 1-bottom, 2-left, 3-right
        uint32_t mask = 0x00FFFFFF // default mask ignores alpha to only compare color channels
        )
    {
        uint32_t* sPixel = pixels;
        ptrdiff_t sDelta = bytesPerRow;
        ptrdiff_t tDelta = sizeof(*pixels);

        switch (direction)
        {
        default:
        case 0: // top
        case 2: // left
            break;

        case 1: // bottom
            sPixel = PtrAddByteOffset(pixels, (height - 1) * bytesPerRow);
            sDelta = -sDelta;
            break;

        case 3: // right
            sPixel = pixels + width - 1;
            tDelta = -tDelta;
            break;
        }

        // Get the first pixel color.
        uint32_t pixelColor = *sPixel & mask;

        // Swap s and t if checking vertical lines instead of horizontal.
        uint32_t sMax = height;
        uint32_t tMax = width;
        if (direction & 2)
        {
            std::swap(sMax, tMax);
            std::swap(sDelta, tDelta);
        }

        sMax = std::min(sMax, maxCount);

        uint32_t s = 0;
        for ( ; s < sMax; ++s)
        {
            uint32_t* tPixel = sPixel;
            for (uint32_t t = 0; t < tMax; ++t)
            {
                if ((*tPixel & mask) != pixelColor)
                    return s;

                tPixel = PtrAddByteOffset(tPixel, tDelta);
            }
            sPixel = PtrAddByteOffset(sPixel, sDelta);
        }

        return s;
    }


    bool CopyToClipboard(
        HWND hwnd,
        HDC hdc,
        bool isUpsideDown = false,
        bool shouldTrimEdges = true,
        uint32_t padding = 0
        )
    {
        bool succeeded = false;

        DIBSECTION sourceBitmapInfo = {};
        HBITMAP sourceBitmap = (HBITMAP)GetCurrentObject(hdc, OBJ_BITMAP);
        GetObject(sourceBitmap, sizeof(sourceBitmapInfo), &sourceBitmapInfo);

        if (sourceBitmapInfo.dsBm.bmBitsPixel <= 8
        ||  sourceBitmapInfo.dsBm.bmBits == nullptr
        ||  sourceBitmapInfo.dsBm.bmPlanes != 1
        ||  sourceBitmapInfo.dsBm.bmWidth <= 0
        ||  sourceBitmapInfo.dsBm.bmHeight <= 0)
        {
            // Don't support paletted images, only true color.
            // Only support bitmaps where the pixels are accessible.
            return false;
        }

        const uint32_t bitmapWidth  = sourceBitmapInfo.dsBm.bmWidth;
        const uint32_t bitmapHeight = std::abs(sourceBitmapInfo.dsBm.bmHeight);
        uint32_t top    = 0;
        uint32_t left   = 0;
        uint32_t right = bitmapWidth;
        uint32_t bottom = bitmapHeight;

        if (shouldTrimEdges)
        {
            const size_t bytesPerRow = sourceBitmapInfo.dsBm.bmWidthBytes;
            uint32_t* pixels = reinterpret_cast<uint32_t*>(sourceBitmapInfo.dsBm.bmBits);
            top     = GetSolidLineCount(pixels, bytesPerRow, bitmapWidth, bitmapHeight, bitmapHeight, 0);
            bottom -= GetSolidLineCount(pixels, bytesPerRow, bitmapWidth, bitmapHeight, bitmapHeight - top, 1);
            left    = GetSolidLineCount(pixels, bytesPerRow, bitmapWidth, bitmapHeight, bitmapWidth, 2);
            right  -= GetSolidLineCount(pixels, bytesPerRow, bitmapWidth, bitmapHeight, bitmapWidth - left, 3);

            top     = std::max(int32_t(top    - padding), 0);
            bottom  = std::min(int32_t(bottom + padding), int32_t(bitmapHeight));
            left    = std::max(int32_t(left   - padding), 0);
            right   = std::min(int32_t(right  + padding), int32_t(bitmapWidth));

            DEBUG_ASSERT(right >= left);
            DEBUG_ASSERT(bottom >= top);
        }

        const uint32_t width  = right  - left;
        const uint32_t height = bottom - top;

        if (OpenClipboard(hwnd))
        {
            if (EmptyClipboard())
            {
                const size_t singlePixelByteCount = sizeof(uint32_t);
                const size_t destRowByteCount = width * singlePixelByteCount;
                const size_t destPixelsByteCount = height * destRowByteCount;
                DEBUG_ASSERT(destPixelsByteCount < sourceBitmapInfo.dsBmih.biSizeImage);
                const size_t destByteCount = sizeof(BITMAPV5HEADER) + destPixelsByteCount;
                HGLOBAL destHandle = GlobalAlloc(GMEM_MOVEABLE, destByteCount);
                uint8_t* destBytes = reinterpret_cast<uint8_t*>(GlobalLock(destHandle));

                // Copy the header.
                BITMAPV5HEADER& header = *reinterpret_cast<BITMAPV5HEADER*>(destBytes);
                memcpy(&header, &sourceBitmapInfo.dsBmih, sizeof(sourceBitmapInfo.dsBmih));
                // Handled by the memcpy...
                // LONG         bV5Width;
                // LONG         bV5Height;
                // WORD         bV5Planes;
                // WORD         bV5BitCount;
                // DWORD        bV5Compression;
                // DWORD        bV5SizeImage;
                // LONG         bV5XPelsPerMeter;
                // LONG         bV5YPelsPerMeter;
                // DWORD        bV5ClrUsed;
                // DWORD        bV5ClrImportant;

                header.bV5Size          = sizeof(header);
                header.bV5Compression   = BI_RGB;
                header.bV5RedMask       = 0x00FF0000;
                header.bV5GreenMask     = 0x0000FF00;
                header.bV5BlueMask      = 0x000000FF;
                header.bV5AlphaMask     = 0xFF000000;
                header.bV5CSType        = LCS_sRGB;
                memset(&header.bV5Endpoints, 0, sizeof(header.bV5Endpoints));
                header.bV5GammaRed      = 0; // ignored
                header.bV5GammaGreen    = 0; // ignored
                header.bV5GammaBlue     = 0; // ignored
                header.bV5Intent        = LCS_GM_IMAGES;
                header.bV5ProfileData   = 0; // ignored
                header.bV5ProfileSize   = 0; // ignored
                header.bV5Reserved      = 0; // ignored

                if (isUpsideDown && !shouldTrimEdges)
                {
                    // Header already copied.

                    // The image is a non-standard bottom-up orientation.
                    // Though, since Windows legacy bitmaps are upside down,
                    // the two upside-downs cancel out.
                    memcpy(destBytes + sizeof(header), sourceBitmapInfo.dsBm.bmBits, destPixelsByteCount);
                }
                else
                {
                    // We have a standard top-down image, but DIBs shared on the
                    // clipboard are actually upside down. Simply flipping the
                    // height negative doesn't work for many applications. So
                    // manually copy the rows flipped.

                    // Header already copied, so just fix different values.
                    header.bV5Width = width;
                    header.bV5Height = height;
                    header.bV5SizeImage = static_cast<DWORD>(destPixelsByteCount);

                    // Copy the pixels from source to destination.
                    const size_t sourceByteStride   = sourceBitmapInfo.dsBm.bmWidthBytes;
                    uint8_t* destRow                = destBytes + sizeof(header);
                    uint8_t* sourceRow              = reinterpret_cast<uint8_t*>(sourceBitmapInfo.dsBm.bmBits)
                                                    + (bottom - 1) * sourceByteStride
                                                    + (left * singlePixelByteCount);

                    // Copy each scanline in backwards order.
                    for (uint32_t i = height; i > 0; --i)
                    {
                        memcpy(destRow, sourceRow, destRowByteCount);
                        sourceRow = PtrAddByteOffset(sourceRow, -ptrdiff_t(sourceByteStride));
                        destRow   = PtrAddByteOffset(destRow, destRowByteCount);
                    }
                }

                GlobalUnlock(destHandle);

                if (SetClipboardData(CF_DIBV5, destHandle) != nullptr)
                {
                    succeeded = true;
                }
                else
                {
                    GlobalFree(destHandle);
                }
            }
            CloseClipboard();
        }

        return succeeded;
    }
}


bool DrawingCanvas::CopyToClipboard(HWND hwnd)
{
    if (target_ == nullptr)
    {
        return false;
    }

    return ::CopyToClipboard(hwnd, target_->GetMemoryDC(), /*isUpsideDown*/false, /*shouldTrimEdges*/true, /*padding*/4);
}
