//----------------------------------------------------------------------------
//  Author:     Dwayne Robinson
//  History:    2008-02-11 Created
//              2015-06-24 Split into base class and Windows control.
//----------------------------------------------------------------------------
#include "precomp.h"

MODULE(DrawingCanvasControl)

EXPORT_BEGIN
    #include "DrawingCanvasControl.h"
EXPORT_END

////////////////////////////////////////

#pragma prefast(disable:__WARNING_ACCESSIBILITY_COLORAPI, "Shush. It's a test program.")
#pragma prefast(disable:__WARNING_HARDCODED_FONT_INFO,    "Shush. It's a test program.")
#pragma warning(disable:4351)   // Yes, we're aware that POD's are default initialized.
                                // This warning is just for those upgrading 2005 to 2008.

bool DrawingCanvasControl::RegisterWindowClass(HINSTANCE hModule)
{
    WNDCLASSEX wcex = {sizeof(WNDCLASSEX)};

    wcex.style = CS_OWNDC | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = &StaticWindowProc;
    wcex.hInstance      = hModule;
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.lpszClassName  = L"D2DWDrawingCanvas";

    return RegisterClassEx(&wcex) != 0;
}


LRESULT CALLBACK DrawingCanvasControl::StaticWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    DrawingCanvasControl* window = GetClass(hwnd);
    if (window == nullptr)
    {
        window = new(std::nothrow) DrawingCanvasControl(hwnd);
        if (window == nullptr)
        {
            return -1; // failed creation
        }

        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)window);
    }
    return window->WindowProc(hwnd, message, wParam, lParam);
}


LRESULT CALLBACK DrawingCanvasControl::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
        {
            // Subclasses may completely override this, using PaintPrepare and PaintFinish themselves.
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            Paint(ps.hdc, ps.rcPaint);
            EndPaint(hwnd, &ps);
        }
        break;

    case WM_ERASEBKGND: // return true here, but user32 still sometimes ignores it, causing flicker :/
        return true;

    case WM_SIZE:
        if (target_ != nullptr)
        {
            RECT rect;
            GetClientRect(hwnd, &rect);

            // Resize the target bitmap to the window size.
            auto hr = ResizeRenderTargets({std::max<int>(rect.right, 1), std::max<int>(rect.bottom, 1)});

            if (FAILED(hr))
                target_.Clear();
        }
        break;

    case WM_DESTROY:
        {
            LRESULT result = DefWindowProc(hwnd, message, wParam, lParam);
            delete this;
            return result;
        }
        // MUST not touch the class anymore...

    case WM_GETDLGCODE:
        if (wParam == VK_RETURN)
            return DLGC_WANTALLKEYS;

        return DLGC_WANTARROWS | DLGC_WANTCHARS;

    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
    case WM_XBUTTONDBLCLK:
        break;

    case WM_RBUTTONUP:
        {
            HMENU menu = CreatePopupMenu();
            auto x = GET_X_LPARAM(lParam);
            auto y = GET_Y_LPARAM(lParam);
            AppendMenu(menu, MF_STRING, CommandIdCopyImage, L"Copy image");
            AppendMenu(menu, MF_STRING, CommandIdResetView, L"Reset view");

            POINT cursorPoint;
            GetCursorPos(&cursorPoint);
            TrackPopupMenu(menu, TPM_LEFTALIGN|TPM_NOANIMATION|TPM_RIGHTBUTTON, cursorPoint.x, cursorPoint.y, 0, hwnd, 0);
            DestroyMenu(menu);

            SendMouseNotification();
        }
        break;

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
        {
            SetFocus(hwnd);
            SetCapture(hwnd);

            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);
            lastMousePosition_.x = xPos;
            lastMousePosition_.y = yPos;

            if (message == WM_LBUTTONDOWN)
                SendMouseNotification();
        }
        break;

    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
        ReleaseCapture();
        SendMouseNotification();
        break;

    case WM_CAPTURECHANGED:
        lastMousePosition_.x = INT_MAX;
        lastMousePosition_.y = INT_MAX;
        break;

    case WM_MOUSEMOVE:
        if ((wParam & MK_MBUTTON) != 0)
        {
            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);
            int xPrev = (lastMousePosition_.x < INT_MAX) ? lastMousePosition_.x : xPos;
            int yPrev = (lastMousePosition_.y < INT_MAX) ? lastMousePosition_.y : yPos;
            int xDif = xPos - xPrev;
            int yDif = yPos - yPrev;

            // Avoid spurious mouse move messages on triple clicking.
            if (xDif == 0 && yDif == 0)
                break;

            lastMousePosition_.x = xPos;
            lastMousePosition_.y = yPos;

            bool heldShift = (GetKeyState(VK_SHIFT) & 0x80) != 0;
            bool heldControl = (GetKeyState(VK_CONTROL) & 0x80) != 0;

            if (heldControl)
            {
                angle_ += (atan2(float(xPrev), float(yPrev)) - atan2(float(xPos), float(yPos))) * float(180.0 / M_PI);
                InvalidateRect(hwnd_, nullptr, false);
            }
            else
            {
                Pan(float(xDif), float(yDif));
            }
        }
        break;

    case WM_MOUSEWHEEL:
    case WM_MOUSEHWHEEL:
        {
            float const scrollMultiple = 64;
            float xDif = 0;
            float yDif = GET_WHEEL_DELTA_WPARAM(wParam) * scrollMultiple / float(WHEEL_DELTA);
            bool heldShift = (GetKeyState(VK_SHIFT) & 0x80) != 0;
            bool heldControl = (GetKeyState(VK_CONTROL) & 0x80) != 0;
            if ((message == WM_MOUSEHWHEEL) ^ heldShift)
                std::swap(xDif, yDif);

            if (heldControl)
            {
                bool isIncrease = (signed)wParam > 0;
                struct ScaleFactorHelper
                {
                    static void Adjust(float& number, bool isIncrease)
                    {
                        if (abs(number) < 1 / 8.0f)
                            number += isIncrease ? .25f : -.25f;
                        else
                            number *= (isIncrease ^ (number < 0)) ? 1.25f : 0.8f;
                    }
                };
                ScaleFactorHelper::Adjust(scaleX_, isIncrease);
                ScaleFactorHelper::Adjust(scaleY_, isIncrease);
                InvalidateRect(hwnd_, nullptr, false);
            }
            else
            {
                Pan(xDif, yDif);
            }

            SendMouseNotification();
        }
        break;

    case WM_KEYDOWN:
        {
            float constexpr scrollMultipleDefault = 64;
            float constexpr scrollPageDefault = 256;
            bool heldControl = (GetKeyState(VK_CONTROL) & 0x80) != 0;
            bool heldShift   = (GetKeyState(VK_SHIFT)   & 0x80) != 0;
            auto getScrollMultiplier = [&]() -> float { return (heldControl && heldShift) ? 1.0f / 8 : heldControl ? 1 : scrollMultipleDefault; };

            switch (wParam)
            {
            case 'C':       if (heldControl) CopyToClipboard(); break;
            case VK_UP:     Pan(0,  getScrollMultiplier()); break;
            case VK_DOWN:   Pan(0, -getScrollMultiplier()); break;
            case VK_LEFT:   Pan( getScrollMultiplier(), 0); break;
            case VK_RIGHT:  Pan(-getScrollMultiplier(), 0); break;
            case VK_PRIOR:  Pan(0,  scrollPageDefault); break;
            case VK_NEXT:   Pan(0, -scrollPageDefault); break;
            case VK_HOME:   if (heldControl) ResetView(); else Pan(scrollPageDefault, 0); break;
            case VK_END:    Pan(-scrollPageDefault, 0); break;
            default:        return DefWindowProc(hwnd, message, wParam, lParam);
            } // switch
        }
        break;

    case WM_COMMAND:
        switch (wParam)
        {
        case CommandIdCopyImage:
            CopyToClipboard();
            break;
        case CommandIdResetView:
            ResetView();
            break;
        } // switch

        break;

    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0;
}


void DrawingCanvasControl::SendMouseNotification()
{
    NMMOUSE mouse = {};
    mouse.hdr.code = NM_CLICK;
    mouse.hdr.idFrom = GetDlgCtrlID(hwnd_);
    mouse.hdr.hwndFrom = hwnd_;
    mouse.pt = lastMousePosition_;
    SendMessage(GetParent(hwnd_), WM_NOTIFY, mouse.hdr.idFrom, reinterpret_cast<LPARAM>(&mouse));
}


////////////////////////////////////////
// Drawing related


void DrawingCanvasControl::Paint(HDC displayHdc, RECT const& rect)
{
    RECT clientRect = {};
    GetClientRect(hwnd_, OUT &clientRect);
    if (!DrawingCanvas::PaintPrepare(displayHdc, clientRect))
        return;

    // Give the parent control an opportunity to paint custom content.
    // Subclassed controls may just draw directly.

    NMCUSTOMDRAW customDraw = {};
    customDraw.hdr.code = NM_CUSTOMDRAW;
    customDraw.hdr.idFrom = GetDlgCtrlID(hwnd_);
    customDraw.hdr.hwndFrom = hwnd_;
    customDraw.dwDrawStage = CDDS_PREERASE;
    customDraw.hdc = displayHdc;
    customDraw.rc = rect;
    customDraw.dwItemSpec = 0;
    customDraw.uItemState = 0;
    customDraw.lItemlParam = 0;
    auto result = SendMessage(GetParent(hwnd_), WM_NOTIFY, customDraw.hdr.idFrom, reinterpret_cast<LPARAM>(&customDraw));
    if (result == CDRF_DODEFAULT || result == CDRF_DOERASE)
    {
        DrawingCanvas::ClearBackground(0x00FFFFFF);
    }
    customDraw.dwDrawStage = CDDS_POSTERASE;
    SendMessage(GetParent(hwnd_), WM_NOTIFY, customDraw.hdr.idFrom, reinterpret_cast<LPARAM>(&customDraw));

    DrawingCanvas::PaintFinish(displayHdc, rect);
}


////////////////////////////////////////
// UI interaction


bool DrawingCanvasControl::CopyToClipboard()
{
    return DrawingCanvas::CopyToClipboard(hwnd_);
}


void DrawingCanvasControl::Pan(float xDif, float yDif)
{
    DX_MATRIX_3X2F viewMatrix, inverseMatrix;
    CalculateViewMatrix(OUT viewMatrix);
    ComputeInverseMatrix(viewMatrix, OUT inverseMatrix);

    translateX_ += (xDif * inverseMatrix.xx + yDif * inverseMatrix.yx);
    translateY_ += (xDif * inverseMatrix.xy + yDif * inverseMatrix.yy);

    InvalidateRect(hwnd_, nullptr, false);
}


void DrawingCanvasControl::ResetView()
{
    translateX_ = 0;
    translateY_ = 0;
    angle_      = 0;
    shearX_     = 0;
    shearY_     = 0;
    scaleX_     = 1.0f;
    scaleY_     = 1.0f;
    InvalidateRect(hwnd_, nullptr, false);
}


void DrawingCanvasControl::CalculateViewMatrix(_Out_ DX_MATRIX_3X2F& viewMatrix) const
{
    float scaleX = scaleX_;
    float scaleY = scaleY_;

    DX_MATRIX_3X2F translationMatrix = {
        1, 0,
        0, 1,
        translateX_, translateY_
    };

    // Scale and rotate
    double radians = DegreesToRadians(fmod(angle_, 360.0f));
    double cosValue = cos(radians);
    double sinValue = sin(radians);
    if (fmod(angle_, 90.0f) == 0)
    {
        cosValue = floor(cosValue + .5);
        sinValue = floor(sinValue + .5);
    }
    DX_MATRIX_3X2F shearMatrix = {
        1,          shearY_,
        shearX_,    1,
        0, 0
    };

    DX_MATRIX_3X2F rotationMatrix = {
        float( cosValue * scaleX), float(sinValue * scaleX),
        float(-sinValue * scaleY), float(cosValue * scaleY),
        0,0,
    };

    if (rotationMatrix.xx == -0.0f) rotationMatrix.xx = 0.0f;
    if (rotationMatrix.xy == -0.0f) rotationMatrix.xy = 0.0f;
    if (rotationMatrix.yx == -0.0f) rotationMatrix.yx = 0.0f;
    if (rotationMatrix.yy == -0.0f) rotationMatrix.yy = 0.0f;

    // Set the origin in the center of the window
    //RECT clientRect;
    //GetClientRect(hwnd_, &clientRect);

    DX_MATRIX_3X2F centerMatrix = {
        1, 0,
        0, 1,
        0, 0,
    };

    DX_MATRIX_3X2F resultA = CombineMatrix(shearMatrix, rotationMatrix);
    DX_MATRIX_3X2F resultB = CombineMatrix(translationMatrix, resultA);
    viewMatrix = CombineMatrix(resultB, centerMatrix);
}


D2D_POINT_2F DrawingCanvasControl::RemapPoint(D2D_POINT_2F point, bool fromCanvasToWorld)
{
    DX_MATRIX_3X2F viewMatrix;
    CalculateViewMatrix(OUT viewMatrix);
    if (fromCanvasToWorld)
    {
        DX_MATRIX_3X2F inverseMatrix;
        ComputeInverseMatrix(viewMatrix, OUT inverseMatrix);
        return TransformPoint(inverseMatrix, point);
    }
    else
    {
        return TransformPoint(viewMatrix, point);
    }
}
