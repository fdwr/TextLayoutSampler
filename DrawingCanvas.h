//----------------------------------------------------------------------------
//  Author:     Dwayne Robinson
//  History:    2008-02-11 Created
//              2015-06-24 Split into base class and Windows control.
//----------------------------------------------------------------------------
#pragma once

#pragma warning(disable:4201)   // Nameless unions should be fine.

////////////////////
// Matrix helpers

#if 0
// For older systems where it is not defined, such as Windows 7/Vista.
__if_not_exists(DX_MATRIX_3X2F)
{
    // Union of D2D and DWrite's matrix to facilitate
    // usage between them, while not breaking existing
    // applications that use one or the other.
    union DX_MATRIX_3X2F
    {
        // Explicity named fields for clarity.
        struct { // Win8
            FLOAT xx; // x affects x (horizontal scaling / cosine of rotation)
            FLOAT xy; // x affects y (vertical shear     / sine of rotation)
            FLOAT yx; // y affects x (horizontal shear   / negative sine of rotation)
            FLOAT yy; // y affects y (vertical scaling   / cosine of rotation)
            FLOAT dx; // displacement of x, always orthogonal regardless of rotation
            FLOAT dy; // displacement of y, always orthogonal regardless of rotation
        };
        struct { // D2D Win7
            FLOAT _11;
            FLOAT _12;
            FLOAT _21;
            FLOAT _22;
            FLOAT _31;
            FLOAT _32;
        };
        struct { // DWrite Win7
            FLOAT m11;
            FLOAT m12;
            FLOAT m21;
            FLOAT m22;
            FLOAT m31;
            FLOAT m32;
        };
        float m[6]; // Would [3][2] be better, more useful?

        DWRITE_MATRIX dwrite;
        D2D1_MATRIX_3X2_F d2d;
        XFORM gdi;
    };
}

void CombineMatrix(
    _In_  DX_MATRIX_3X2F const& a,
    _In_  DX_MATRIX_3X2F const& b,
    _Out_ DX_MATRIX_3X2F& result
    );

DX_MATRIX_3X2F CombineMatrix(
    _In_  DX_MATRIX_3X2F const& a,
    _In_  DX_MATRIX_3X2F const& b
    );
#endif

void ComputeInverseMatrix(
    _In_  DX_MATRIX_3X2F const& matrix,
    _Out_ DX_MATRIX_3X2F& result
    );

float GetDeterminant(_In_ DX_MATRIX_3X2F const& matrix);

D2D_POINT_2F TransformPoint(DX_MATRIX_3X2F const& matrix, D2D_POINT_2F point);

////////////////////
// Rect helpers

void TransformRect(D2D_MATRIX_3X2_F const& transform, D2D_RECT_F const& rectIn, _Out_ D2D_RECT_F& rectOut);

void UnionRect(D2D_RECT_F const& rectIn, _Inout_ D2D_RECT_F& rectInOut);

// Rounds to nearest integer outward.
void PixelAlignRect(_Inout_ D2D_RECT_F& rectInOut);

void TranslateRect(D2D_POINT_2F offset, _Inout_ D2D_RECT_F& rectInOut);

void ConvertRect(D2D_RECT_F const& rectIn, _Out_ RECT& rectOut);

void ConvertRect(RECT const& rectIn, _Out_ D2D_RECT_F& rectOut);

// Inclusive to left and top edge. Exclusive to right and bottom edge.
bool IsPointInRect(D2D_RECT_F const& rectIn, float x, float y);

////////////////////
// Color helpers

//                     Byte 0 1 2 3
// Windows 32-bit DIBs:     B G R A
// GDI RGBQUAD:             B G R A
// GDI COLORREF:            R G B A
// GDI+:                    B G R A
// D2D ColorF parameter:    B G R X
// D2D Color internal:      R G B A
// D3D RGB/RGBA/CI 8x4:     B G R A
// D3DCOLORVALUE floatx4:   R G B A

D2D_COLOR_F ToD2DColor(/*BGRA*/uint32_t color);

inline /*BGRA*/uint32_t ToBGRA(/*RGBA*/uint32_t color)
{
    return (color & 0xFF00FF00)
        | ((color & 0x00FF0000) >> 16)
        | ((color & 0x000000FF) << 16);
}

inline /*RGBA*/uint32_t ToRGBA(/*BGRA*/uint32_t color)
{
    return ToBGRA(color);
}

inline COLORREF ToColorRef(/*BGRA*/uint32_t color)
{
    // Strip alpha, since GDI can't use it anyway, and it causes GDI to
    // draw nothing depending on the function called.
    return ToRGBA(color) & 0x00FFFFFF;
}

////////////////////

inline double DegreesToRadians(float degrees)
{
    return degrees * M_PI * 2.0f / 360.0f;
}

inline float Lerp(float first, float last, float fraction)
{
    return first + fraction * (last - first); // equivalent to (first * (1-fraction)) + (last * fraction)
}


////////////////////

using GdiPlusStartupAutoResource = AutoResource<ULONG_PTR, HandleResourceTypePolicy<ULONG_PTR, void (WINAPI*)(ULONG_PTR), &Gdiplus::GdiplusShutdown>, ULONG_PTR>;


class __declspec(uuid("74868E11-F1CF-461A-AEAF-175216DFF0BA")) DrawingCanvas : public ComObject
{
    ////////////////////////////////////////
    // Definitions (enums and structs)

public:
    using RectF = D2D_RECT_F;
    using PointF = D2D_POINT_2F;

    struct RawPixels
    {
        void* pixels;
        uint32_t width;
        uint32_t height;
        uint32_t bitsPerPixel;
        uint32_t byteStride;
    };

    const static DX_MATRIX_3X2F g_identityMatrix;
    const static GUID g_guid;

    static void SetIdentityMatrix(_Out_ DX_MATRIX_3X2F& result)
    {
        result = g_identityMatrix;
    }

    template <typename T>
    static inline T* AddBitmapByteOffset(T* p, size_t offset)
    {
        return reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(p) + offset);
    }

    template <typename T>
    static inline T const* AddBitmapByteOffset(T const* p, size_t offset)
    {
        return reinterpret_cast<T const*>(reinterpret_cast<uint8_t const*>(p) + offset);
    }

    enum CurrentRenderingApi
    {
        CurrentRenderingApiAny,
        CurrentRenderingApiDWrite,
        CurrentRenderingApiD2D,
        CurrentRenderingApiGdi,
        CurrentRenderingApiGdiPlus,
    };

    struct SharedResource
    {
        UUID typeUuid;              // Type of the resource.
        std::u16string name;        // Distinct name if there can be multiple of the same type.
        uint32_t freshnessCount;    // Resource may be released upon reaching 0.
        ComPtr<IUnknown> resource;  // Pointer to generic resource.
    };

    ////////////////////////////////////////
    // Initialization/finalization

public:
    DrawingCanvas()
    { }

    ~DrawingCanvas();

    IDWriteFactory* GetDWriteFactoryWeakRef() { return dwriteFactory_; };
    ID2D1Factory* GetD2DFactoryWeakRef() { return d2dFactory_; };
    IWICImagingFactory* GetWicFactoryWeakRef() { return wicFactory_; };
    void SetDWriteFactory(IDWriteFactory* factory);
    void SetD2DFactory(ID2D1Factory* factory);
    void SetWicFactory(IWICImagingFactory* factory);
    void Clone(_COM_Outptr_ DrawingCanvas** newDrawingCanvas);

    void UninitializeForRendering();

    ////////////////////////////////////////
    // Rendering

protected:
    ComPtr<IDWriteBitmapRenderTarget>   target_;
    ComPtr<ID2D1DCRenderTarget>         targetD2D_;
    ComPtr<ID2D1SolidColorBrush>        brush_; // reusable scratch brush for current color
    ComPtr<IDWriteFactory>              dwriteFactory_;
    ComPtr<ID2D1Factory>                d2dFactory_;
    ComPtr<IWICImagingFactory>          wicFactory_;
    ComPtr<IDWriteRenderingParams>      renderingParams_;
    ComPtr<IDWriteGdiInterop>           gdiInterop_;
    GdiPlusStartupAutoResource          gdiplusToken_;
    std::vector<SharedResource>         sharedResources_;

    CurrentRenderingApi currentRenderingApi_ = CurrentRenderingApiAny;

public:
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(IID const& iid, _Out_ void** object) throw() override
    {
        *object = nullptr;
        if (iid == __uuidof(DrawingCanvas)
        ||  iid == __uuidof(IUnknown))
        {
            *object = this;
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }

    ////////////////////////////////////////
    // Drawing helper methods

public:
    IDWriteBitmapRenderTarget* GetDWriteBitmapRenderTargetWeakRef() { return target_; }
    ID2D1DCRenderTarget* GetD2DRenderTargetWeakRef() { return targetD2D_; }
    ID2D1SolidColorBrush* GetD2DBrushWeakRef() { return brush_; }
    HDC GetHDC() { return target_ == nullptr ? 0 : target_->GetMemoryDC(); }
    DrawingCanvas::RawPixels GetRawPixels();

    IDWriteRenderingParams* GetDirectWriteRenderingParamsWeakRef() { return renderingParams_; }
    void SetDirectWriteRenderingParams(IDWriteRenderingParams* renderingParams);

    HRESULT GetSharedResource(UUID const& typeUuid, _In_z_ char16_t const* name, _COM_Outptr_ IUnknown** resource);
    HRESULT SetSharedResource(UUID const& typeUuid, _In_z_ char16_t const* name, IUnknown* resource);
    HRESULT RetireStaleSharedResources();
    HRESULT ClearSharedResources();

    template <typename T>
    HRESULT GetSharedResource(_In_z_ char16_t const* name, _COM_Outptr_ T** resource)
    {
        return GetSharedResource(__uuidof(T), name, OUT reinterpret_cast<IUnknown**>(resource));
    }

    //template <typename T>
    //HRESULT GetSharedResource(_In_z_ char16_t const* name, std::function<HRESULT(IUnknown**)> const& missingResourceCallback, _COM_Outptr_ T** resource)
    //{
    //    return GetSharedResource(__uuidof(T), name, missingResourceCallback, OUT reinterpret_cast<IUnknown**>(resource));
    //}

    template <typename T>
    HRESULT SetSharedResource(_In_z_ char16_t const* name, T* resource)
    {
        return SetSharedResource(__uuidof(T), name, resource);
    }

    bool PaintPrepare(HDC displayHdc, RECT const& rect); // Create and bind render targets
    bool PaintFinish(HDC displayHdc, RECT const& rect); // Blit to given display HDC.
    void ClearBackground(uint32_t color);
    void DrawAlphaChannel();
    void DrawGrid(uint32_t color, uint32_t step);

    bool CopyToClipboard(HWND hwnd);

    HRESULT CreateRenderTargetsOnDemand(_In_opt_ HDC templateHdc, SIZE size);
    HRESULT ResizeRenderTargets(SIZE size);
    HRESULT InitializeRendering();
    void SwitchRenderingAPI(CurrentRenderingApi currentRenderingApi);
};
