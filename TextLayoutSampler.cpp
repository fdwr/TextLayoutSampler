//----------------------------------------------------------------------------
//  Author:     Dwayne Robinson
//  History:    2015-06-19 Created
//----------------------------------------------------------------------------
#include "precomp.h"
#include "MainWindow.h"
#include <specstrings.h>
#include <winspool.h>


#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "DWrite.lib")
#pragma comment(lib, "D2D1.lib")
#pragma comment(lib, "GdiPlus.lib")
#pragma comment(lib, "ComCtl32.lib")


////////////////////////////////////////
// UI related

#pragma prefast(disable:__WARNING_HARD_CODED_STRING_TO_UI_FN, "It's an internal test program.")


HINSTANCE Application::g_hModule = nullptr;
MSG Application::g_msg;
HWND Application::g_mainHwnd;


////////////////////////////////////////


int APIENTRY wWinMain(
    __in HINSTANCE      hInstance, 
    __in_opt HINSTANCE  hPrevInstance,
    __in LPWSTR         commandLine,
    __in int            nCmdShow
    )
{
    Application::g_hModule = hInstance;

    _wsetlocale(LC_ALL, L""); // Unicode, not ANSI!

    ////////////////////
    // Read command line parameters.

    std::u16string trimmedCommandLine(ToChar16(commandLine));
    TrimSpaces(IN OUT trimmedCommandLine);

    if (!trimmedCommandLine.empty())
    {
        if (_wcsicmp(ToWChar(trimmedCommandLine.c_str()), L"/?"    ) == 0
        ||  _wcsicmp(ToWChar(trimmedCommandLine.c_str()), L"/help" ) == 0
        ||  _wcsicmp(ToWChar(trimmedCommandLine.c_str()), L"-h"    ) == 0
        ||  _wcsicmp(ToWChar(trimmedCommandLine.c_str()), L"--help") == 0
            )
        {
            MessageBox(nullptr, L"TextLayoutSampler.exe [SomeFile.TextLayoutSamplerSettings].", APPLICATION_TITLE, MB_OK);
            return (int)0;
        }
        else if (trimmedCommandLine[0] == '/')
        {
            Application::Fail(trimmedCommandLine.c_str(), u"Unknown command line option.\r\n\r\n\"%s\"", 0);
        }
        UnquoteString(IN OUT trimmedCommandLine);
        // Else just pass the command line to the main window.
    }

    ////////////////////
    // Create user interface elements.

    Application::g_mainHwnd = MainWindow::Create();
    if (Application::g_mainHwnd == nullptr)
    {
        Application::Fail(u"Could not create main window.", u"%s = %08X", HRESULT_FROM_WIN32(GetLastError()));
    }
    ShowWindow(Application::g_mainHwnd, SW_SHOWNORMAL);
    SendMessage(Application::g_mainHwnd, WM_CHANGEUISTATE, UIS_CLEAR | UISF_HIDEACCEL | UISF_HIDEFOCUS, (LPARAM)nullptr); // Always shows the focus rectangle.

    MainWindow& mainWindow = *MainWindow::GetClass(Application::g_mainHwnd);

    if (!trimmedCommandLine.empty())
    {
        mainWindow.LoadDrawableObjectsSettings(trimmedCommandLine.data());
    }

    while (GetMessage(&Application::g_msg, nullptr, 0, 0) > 0)
    {
        Application::Dispatch();
    }

    return static_cast<int>(Application::g_msg.wParam);
}


void Application::Dispatch()
{
    // Fixup any messages.
    switch (Application::g_msg.message)
    {
    case WM_MOUSEWHEEL:
        // Mouse wheel messages inconsistently go to the control with
        // keyboard focus instead of mouse focus, unlike every other
        // mouse message. So fix it to behave more sensibly like IE.
        POINT pt = {GET_X_LPARAM(Application::g_msg.lParam), GET_Y_LPARAM(Application::g_msg.lParam)};
        HWND mouseHwnd = WindowFromPoint(pt);
        if (mouseHwnd != nullptr)
        {
            // Don't send to a different process by mistake.
            DWORD pid = 0;
            if (GetWindowThreadProcessId(mouseHwnd, &pid) == GetCurrentThreadId())
            {
                Application::g_msg.hwnd = mouseHwnd;
            }
        }
        break;
    }

    // Get the actual dialog window handle.
    // If it's a child, get the root window.

    DWORD style = GetWindowStyle(Application::g_msg.hwnd);
    HWND dialog = Application::g_msg.hwnd;

    if (style & WS_CHILD)
        dialog = GetAncestor(Application::g_msg.hwnd, GA_ROOT);

    // Dispatch the message, trying the child and parent.

    bool messageHandled = false;
    if (Application::g_msg.message == WM_SYSCHAR)
    {
        // If Alt+Key is pressed, give the control priority, sending the
        // message to it first, then handling it as a menu accelerator
        // if it does not.
        TranslateMessage(&Application::g_msg);
        messageHandled = !DispatchMessage(&Application::g_msg);
    }

    // Ask the dialog itself to first check for accelerators.
    if (!messageHandled && Application::g_msg.message == WM_KEYDOWN)
    {
        messageHandled = !!SendMessage(dialog, Application::g_msg.message, Application::g_msg.wParam, Application::g_msg.lParam);
    }

    // Send the Return key to the right control (one with focus) so that
    // we get a NM_RETURN from that control, not the less useful IDOK.
    if (!messageHandled && Application::g_msg.message == WM_KEYDOWN && Application::g_msg.wParam == VK_RETURN)
    {
        messageHandled = !SendMessage(Application::g_msg.hwnd, Application::g_msg.message, Application::g_msg.wParam, Application::g_msg.lParam);
    }

    if (!messageHandled)
    {
        // Let the default dialog processing check it.
        messageHandled = !!IsDialogMessage(dialog, &Application::g_msg);
    }
    if (!messageHandled)
    {
        // Not any of the above, so just handle it.
        TranslateMessage(&Application::g_msg);
        DispatchMessage(&Application::g_msg);
    }
}


void Application::Fail(__in_z const char16_t* message, __in_z_opt const char16_t* formatString, int functionResult)
{
    Application::DisplayError(message, formatString, functionResult);
    ExitProcess(functionResult);
}


int Application::DisplayError(__in_z const char16_t* message, __in_z_opt const char16_t* formatString, int functionResult)
{
    wchar_t buffer[1000];
    buffer[0] = 0;

    if (formatString == nullptr)
        formatString = u"%s\r\nError code = %X";

    StringCchPrintf(
        buffer,
        countof(buffer),
        ToWChar(formatString),
        ToWChar(message),
        functionResult
        );

    MessageBox(
        nullptr, 
        buffer,
        APPLICATION_TITLE,
        MB_OK|MB_ICONEXCLAMATION
        );

    return -1;
}


void Application::DebugLog(const char16_t* logMessage, ...)
{
    va_list argList;
    va_start(argList, logMessage);

    wchar_t buffer[1000];
    buffer[0] = 0;
    StringCchVPrintf(
        buffer,
        countof(buffer),
        ToWChar(logMessage),
        argList
        );

    OutputDebugString(buffer);
}


// Maps exceptions to equivalent HRESULTs,
HRESULT Application::ExceptionToHResult() throw()
{
    try
    {
        throw;  // Rethrow previous exception.
    }
    catch (std::bad_alloc const&)
    {
        return E_OUTOFMEMORY;
    }
    catch (...)
    {
        return E_FAIL;
    }
}
