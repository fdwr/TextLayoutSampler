//----------------------------------------------------------------------------
//  History:    2015-06-19 Dwayne Robinson - Created
//----------------------------------------------------------------------------
#include "precomp.h"
#include <specstrings.h>
#include "Application.macros.h"

#if USE_CPP_MODULES
import Common.String;
#else
#include "Common.String.h"
#endif

MODULE(Application)


////////////////////////////////////////

EXPORT class Application
{
public:
    static HINSTANCE g_hModule;
    static MSG g_msg;
    static HWND g_mainHwnd;

    static void Dispatch();
    static int DisplayError(__in_z const char16_t* message, __in_z_opt const char16_t* formatString, int functionResult);
    static void Fail(__in_z const char16_t* message, __in_z_opt const char16_t* formatString, int functionResult);
    static void DebugLog(const char16_t* logMessage, ...);
    static HRESULT ExceptionToHResult() noexcept;
};
