//----------------------------------------------------------------------------
//  Author:     Dwayne Robinson
//  History:    2016-10-26 Created
//----------------------------------------------------------------------------

#include <Windows.h>

#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "Gdi32.lib")


////////////////////////////////////////


int APIENTRY wWinMain(
    __in HINSTANCE      hInstance, 
    __in_opt HINSTANCE  hPrevInstance,
    __in LPWSTR         commandLine,
    __in int            nCmdShow
    )
{
    auto commandLineLength = wcslen(commandLine);
    if (commandLineLength == 0 || (commandLineLength == 1 && commandLine[0] == ' '))
    {
        MessageBox(nullptr, L"Usage:\r\n\r\n" L"RemoveFontResource.exe PathAndFileName.ttf", L"RemoveFontResource", MB_OK);
        return (int)-1;
    }

    RemoveFontResource(commandLine);
    MessageBox(nullptr, commandLine, L"Called RemoveFontResource", MB_OK);
    return 0;
}
