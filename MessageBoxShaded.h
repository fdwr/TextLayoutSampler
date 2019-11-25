//+---------------------------------------------------------------------------
//  Window layout functions.
//
//  History:    2008-02-11   Dwayne Robinson - Created
//----------------------------------------------------------------------------

////////////////////////////////////////

#if USE_CPP_MODULES
import Common.ArrayRef;
import Common.String;
import WindowUtility;
#else
#include "Common.ArrayRef.h"
#include "Common.String.h"
#include "WindowUtility.h"
#endif

EXPORT_BEGIN

class MessageBoxShaded
{
    using Self = MessageBoxShaded;

private:
    HWND hwnd_;

public:
    static int32_t Show(
        HWND ownerHwnd,
        _In_z_ const char16_t* text,
        _In_z_ const char16_t* caption,
        uint32_t type
    );

    MessageBoxShaded(HWND hwnd);

    INT_PTR Initialize(MSGBOXPARAMS const& params);
    static INT_PTR CALLBACK StaticDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    DialogProcResult CALLBACK DialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    DialogProcResult CALLBACK ProcessCommand(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    DialogProcResult CALLBACK ProcessNotification(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void Resize();
};

EXPORT_END
