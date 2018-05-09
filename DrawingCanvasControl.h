//----------------------------------------------------------------------------
//  History:    2008-02-11 Dwayne Robinson - Created
//              2015-06-24 Split into base class and Windows control.
//----------------------------------------------------------------------------
#pragma once


#if USE_MODULES
import DrawingCanvas;
#else
#include "DrawingCanvas.h"
#endif


class DrawingCanvasControl : public DrawingCanvas
{
////////////////////////////////////////
// Definitions (enums and structs)

public:
    using RectF = D2D_RECT_F;
    using PointF = D2D_POINT_2F;

////////////////////////////////////////
// Initialization/finalization

public:
    DrawingCanvasControl(HWND hwnd)
    :   hwnd_(hwnd)
    {
        refCount_ = INT_MAX;
    }

////////////////////////////////////////
// Windowing related

public:
    // Call before first instantiation
    static bool RegisterWindowClass(HINSTANCE hModule);

    // Thunk to the real one.
    static LRESULT CALLBACK StaticWindowProc(
        HWND hWnd, 
        UINT message, 
        WPARAM wParam, 
        LPARAM lParam
        );

    // From hwnd to real object
    static inline DrawingCanvasControl* GetClass(HWND hwnd)
    {
        return reinterpret_cast<DrawingCanvasControl*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    void NeedRepaint()
    {
        InvalidateRect(hwnd_, nullptr, false);
    }

    void SendMouseNotification();

    void Pan(float xDif, float yDif);

    void ResetView();

    void CalculateViewMatrix(_Out_ DX_MATRIX_3X2F& matrix) const;

    D2D_POINT_2F RemapPoint(D2D_POINT_2F point, bool fromCanvasToWorld);

    enum CommandId : uint32_t
    {
        CommandIdCopyImage = 1,
        CommandIdResetView = 2,
    };

protected:
    LRESULT CALLBACK WindowProc(
        HWND hWnd, 
        UINT message, 
        WPARAM wParam, 
        LPARAM lParam
        );

    HWND hwnd_;
    POINT lastMousePosition_ = {INT_MAX, INT_MAX};

    float translateX_ = 0;
    float translateY_ = 0;
    float angle_ = 0;
    float shearX_ = 0;
    float shearY_ = 0;
    float scaleX_ = 1.0f;
    float scaleY_ = 1.0f;


////////////////////////////////////////
// Static IUnknown interface because there will only be one instance, directly
// tied to the lifetime of the window class.

public:
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(IID const& iid, _Out_ void** object) throw() override
    {
        *object = nullptr;
        return E_NOINTERFACE;
    }

    virtual unsigned long STDMETHODCALLTYPE AddRef() throw() override
    {
        return 1;
    }

    virtual unsigned long STDMETHODCALLTYPE Release() throw() override
    {
        return 1;
    }

////////////////////////////////////////
// Drawing helper methods

public:
    void Paint(HDC displayHdc, RECT const& rect);
    bool CopyToClipboard();
};
