//+---------------------------------------------------------------------------
//
//  Copyright (c) Microsoft, 2008. All rights reserved.
//
//  Contents:   Window layout functions.
//
//  Author:     Dwayne Robinson (dwayner@microsoft.com)
//
//  History:    2008-02-11   dwayner    Created
//
//----------------------------------------------------------------------------

#include "precomp.h"
#include "resource.h"
#include <string>
#include <Windows.h>

MODULE(MessageBoxShaded)
EXPORT_BEGIN
    #include "MessageBoxShaded.h"
EXPORT_END

////////////////////////////////////////

namespace
{
    static RECT GetStaticDefaultSize(
        HWND hwnd,
        RECT rect
        )
    {
        RECT defaultRect = rect;

        // Measure the text height and width.
        int32_t textLength = Static_GetTextLength(hwnd);
        if (textLength >= 0)
        {
            std::u16string text;
            text.resize(textLength + 1);
            Static_GetText(hwnd, OUT ToWChar(&text[0]), textLength + 1);
            HFONT font = GetWindowFont(hwnd);

            HDC hdc = GetDC(hwnd); // Get the static control's DC.
            HFONT oldFont = SelectFont(hdc, font);
            DrawText(hdc, ToWChar(text.data()), textLength, IN OUT &defaultRect, DT_CALCRECT|DT_NOPREFIX|DT_WORDBREAK);
            SelectObject(hdc, oldFont);
            ReleaseDC(hwnd, hdc); // free the DC
        }

        return defaultRect;
    }
}


int32_t MessageBoxShaded::Show(
    HWND ownerHwnd,
    _In_z_ const char16_t* text,
    _In_z_ const char16_t* caption,
    uint32_t type
    )
{
    // Note the type parameter exists to simplify using this as a drop-in
    // substitute, but it actually ignores most of the flags.
    // It only accepts MB_OK, none of the other Yes/No/Cancel variants.
    HMODULE moduleHandle = nullptr;
    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<const wchar_t*>(&MessageBoxShaded::Show), OUT &moduleHandle);
    MSGBOXPARAMS params = {};
    params.hwndOwner = ownerHwnd;
    params.lpszText = ToWChar(text);
    params.lpszCaption = ToWChar(caption);
    params.dwStyle = type;
    auto result = ::DialogBoxParam(moduleHandle, MAKEINTRESOURCE(IddMessageDialog), ownerHwnd, &MessageBoxShaded::StaticDialogProc, (LPARAM)&params);
    return static_cast<int32_t>(result);
}


MessageBoxShaded::MessageBoxShaded(HWND hwnd)
:   hwnd_(hwnd)
{
}


INT_PTR MessageBoxShaded::Initialize(MSGBOXPARAMS const& params)
{
    HWND iconHwnd = GetDlgItem(hwnd_, IdcMessageDialogIcon);

    HICON icon = nullptr;
    if (params.dwStyle & MB_USERICON)
    {
        icon = LoadIcon(nullptr, IDI_WARNING);
    }
    else
    {
        // Convert the message box type enum into an icon id.
        INT32 iconId = int(reinterpret_cast<ULONG_PTR>(IDI_APPLICATION)) + ((params.dwStyle & 0x00000070) >> 4);
        icon = LoadIcon(nullptr, MAKEINTRESOURCE(iconId));
    }

    Static_SetIcon(iconHwnd, icon);
    Static_SetText(GetDlgItem(hwnd_, IdcMessageDialogMessage), params.lpszText);

    // Check for either right to left (RLM) or left to right mark (LRM).
    if ((params.lpszText[0] & 0x200E) == 0x200E)
    {
        UINT32 extendedStyle = GetWindowLong(hwnd_, GWL_EXSTYLE);
        if (params.lpszText[0] == 0x200E && params.lpszText[1] == 0x200E) extendedStyle &= ~WS_EX_LAYOUTRTL;
        if (params.lpszText[0] == 0x200F && params.lpszText[1] == 0x200F) extendedStyle |= WS_EX_LAYOUTRTL;
        SetWindowLong(hwnd_, GWL_EXSTYLE, extendedStyle);
    }

    // Overlay the modal dialog atop the owner window with the same size.
    RECT ownerRect = {};
    GetWindowRect(params.hwndOwner, OUT &ownerRect);

    if (IsMaximized(params.hwndOwner))
    {
        // Avoid the border from spilling over onto the adjacent monitors.
        // This compensates for a GetWindowRect backwards compat annoyance
        // where it returns a size larger than the actual size because back
        // before the days of multiple monitors, it didn't matter. So to
        // get the true size, clamp to the work area.
        // todo::: Consider using DWMWA_EXTENDED_FRAME_BOUNDS instead.
        MONITORINFO monitorInfo = {};
        monitorInfo.cbSize = sizeof(monitorInfo);
        HMONITOR monitor = MonitorFromWindow(params.hwndOwner, MONITOR_DEFAULTTONEAREST);
        GetMonitorInfo(monitor, OUT &monitorInfo);
        IntersectRect(OUT &ownerRect, &ownerRect, &monitorInfo.rcWork);
    }

    SetWindowPos(hwnd_, nullptr, ownerRect.left, ownerRect.top, ownerRect.right - ownerRect.left, ownerRect.bottom - ownerRect.top, SWP_NOZORDER|SWP_NOACTIVATE);

    SetLayeredWindowAttributes(hwnd_, 0, 224, LWA_ALPHA);

    SetFocus(hwnd_);

    return true;
}


INT_PTR CALLBACK MessageBoxShaded::StaticDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    Self* window = GetClassFromWindow<Self>(hwnd);
    if (window == nullptr)
    {
        window = new(std::nothrow) Self(hwnd);
        if (window == nullptr)
        {
            return -1; // failed creation
        }

        ::SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)window);
    }

    const DialogProcResult result = window->DialogProc(hwnd, message, wParam, lParam);
    if (result.handled)
    {
        ::SetWindowLongPtr(hwnd, DWLP_MSGRESULT, result.value);
    }
    return result.handled;
}


DialogProcResult CALLBACK MessageBoxShaded::DialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        Initialize(*reinterpret_cast<MSGBOXPARAMS*>(lParam));
        return false; // Do not attempt to set key focus.

    case WM_COMMAND:
        return ProcessCommand(hwnd, message, wParam, lParam);

    case WM_NOTIFY:
        return ProcessNotification(hwnd, message, wParam, lParam);

    case WM_SIZE:
        Resize();
        break;

    case WM_ERASEBKGND:
    {
        // Shade the background.
        RECT rect = {};
        HDC hdc = (HDC)wParam;
        GetClientRect(hwnd, OUT &rect);
        FillRect(hdc, &rect, GetSysColorBrush(COLOR_3DSHADOW));
    }
    return true;

    case WM_ACTIVATE:
        if (wParam == WA_INACTIVE)
            SetTimer(hwnd, IDCANCEL, 30, nullptr);

        // Set a short timer rather than calling EndDialog directly (which
        // confuses Windows and fails to reset the focus to the control when
        // Alt+Tabbing back) or PostMessage'ing directly (which confuses the
        // Alt+Tab order and switches right back to the current window).

        return false; // Let the default procedure handle it.

    case WM_TIMER: // A timer was set to dismiss the dialog.
        KillTimer(hwnd, wParam);
        PostMessage(hwnd, WM_COMMAND, wParam, 0);
        break;

    case WM_LBUTTONDOWN:
        PostMessage(hwnd, WM_COMMAND, IDCANCEL, 0);
        break;

    default:
        return false;
    }

    return true;
}


DialogProcResult CALLBACK MessageBoxShaded::ProcessCommand(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    uint32_t wmId    = LOWORD(wParam);
    uint32_t wmEvent = HIWORD(wParam);
    UNREFERENCED_PARAMETER(wmEvent);

    switch (wmId)
    {
    case IDOK:
    case IDCANCEL:
    case IDABORT:
    case IDRETRY:
    case IDIGNORE:
    case IDYES:
    case IDNO:
    case IDCLOSE:
    case IDHELP:
    case IDTRYAGAIN:
    case IDCONTINUE:
    {
        EndDialog(hwnd, wmId); // not DestroyWindow
                                // Note, do not reference the class after this.
    }
    break;

    default:
        return DialogProcResult(false, -1); // unhandled
    }

    return DialogProcResult(true, 0); // handled
}


DialogProcResult CALLBACK MessageBoxShaded::ProcessNotification(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int idCtrl = (int) wParam; 
    NMHDR const& nmh = *(NMHDR*)lParam;

    switch (idCtrl)
    {
    //case IdcActionsList:
    //    switch (nmh.code)
    //    {
    //    case NM_RETURN:
    //    case NM_DBLCLK:
    //        break;
    //    }
    //    break;

    default:
        return false;

    #pragma warning(suppress: 4065) // Yes, I know there are no case labels above.
    } // switch idCtrl

    return true;
}

void MessageBoxShaded::Resize()
{
    // Resize contained controls.

    long const spacing = 16;
    RECT clientRect;
    GetClientRect(hwnd_, OUT &clientRect);

    WindowPosition windowPositions[] = {
        WindowPosition( GetDlgItem(hwnd_, IdcMessageDialogBackground), PositionOptionsIgnored),
        WindowPosition( GetDlgItem(hwnd_, IdcMessageDialogIcon), PositionOptionsAlignLeft|PositionOptionsAlignTop),
        WindowPosition( GetDlgItem(hwnd_, IdcMessageDialogMessage), PositionOptionsAlignTop),
        WindowPosition( GetDlgItem(hwnd_, IDOK), PositionOptionsUseSlackWidth|PositionOptionsPreNewLine),
    };

    // Measure the text height and width.

    windowPositions[2].rect = GetStaticDefaultSize(
        windowPositions[2].hwnd,
        windowPositions[2].rect
        );

    // Call once to compute size only.
    RECT groupRect = WindowPosition::ReflowGrid(windowPositions, int(countof(windowPositions)), clientRect, spacing, 0, PositionOptionsFlowHorizontal|PositionOptionsUnwrapped, /*queryOnly*/ true);

    // Align all the controls in the middle so the horizontal band is centered.
    WindowPosition groupPosition(nullptr, PositionOptionsAlignHCenter|PositionOptionsAlignVCenter, groupRect);
    IntersectRect(OUT &groupPosition.rect, &groupPosition.rect, &clientRect);
    groupPosition.AlignToRect(clientRect);

    // Compute final layout.
    WindowPosition::ReflowGrid(windowPositions, int(countof(windowPositions)), groupPosition.rect, spacing, 0, PositionOptionsFlowHorizontal|PositionOptionsUnwrapped);

    // Assign the background layer's rect, and give it padding.
    InflateRect(IN OUT &groupPosition.rect, spacing, spacing);
    windowPositions[0].rect = groupPosition.rect;

    WindowPosition::Update(windowPositions, int(countof(windowPositions)));
}
