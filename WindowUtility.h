//----------------------------------------------------------------------------
//  Author:     Dwayne Robinson (dwayner@microsoft.com)
//  History:    2012-12-14 Created
//----------------------------------------------------------------------------
#pragma once


template<typename T>
T* GetClassFromWindow(HWND hwnd)
{
    return reinterpret_cast<T*>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
}

template<typename T>
T* GetClassFromDialog(HWND hwnd)
{
    return reinterpret_cast<T*>(::GetWindowLongPtr(hwnd, DWLP_USER));
}

HRESULT CopyTextToClipboard(HWND hwnd, array_ref<char16_t const> text);

HRESULT PasteFromClipboard(OUT std::u16string& text);

int GetWindowText(HWND hwnd, _Out_ std::u16string& s);

// Satisfy functionality hole, a complement to SetWindowPos.
BOOL GetWindowPos(HWND hwnd, _Out_ RECT& rect);

HMENU WINAPI GetSubMenu(
    HMENU parentMenu,
    UINT item,
    BOOL byPosition
    );

struct TrackPopupMenu_Item
{
    UINT_PTR id;
    char16_t const* name;
};

// Creates the menu given the item list.
int TrackPopupMenu(
    array_ref<TrackPopupMenu_Item const> items,
    HWND controlHwnd,
    HWND parentHwnd,
    uint32_t defaultMenuItemId = ~0
    );

// More useful version that places the menu where it should go to begin with,
// around the pertinent rect of the control.
int TrackPopupMenu(
    HMENU menu,
    HWND controlHwnd,
    HWND parentHwnd
    );

bool GetOpenFileName(
    HWND hwnd,
    _In_opt_z_ char16_t const* fileTypes,
    _Out_ std::u16string& fileName,
    char16_t const* title = nullptr
    );

bool GetSaveFileName(
    HWND hwnd,
    _In_opt_z_ char16_t const* fileTypes,
    _In_opt_z_ char16_t const* defaultExtension,
    _In_opt_z_ char16_t const* defaultFileName,
    _Out_ std::u16string& fileName,
    char16_t const* title = nullptr
    );

// A more recallable and sensible name, since the function is not dialog-specific.
inline HWND WINAPI GetWindowFromId(_In_opt_ HWND parentWindowHandle, _In_ int windowId) { return GetDlgItem(parentWindowHandle, windowId); }


enum PositionOptions : uint32_t
{
    PositionOptionsNone                 = 0,

    // Per-window options.
    PositionOptionsUseSlackWidth        = 1,        // use any remaining width of parent's client rect (use FillWidth unless you want specific alignment)
    PositionOptionsUseSlackHeight       = 2,        // use any remaining height of parent's client rect (use FillHeight unless you want specific alignment)
    PositionOptionsAlignHCenter         = 0,        // center horizontally within allocated space
    PositionOptionsAlignLeft            = 16,       // align left within allocated space
    PositionOptionsAlignRight           = 32,       // align right within allocated space
    PositionOptionsAlignHStretch        = 16|32,    // stretch left within allocated space
    PositionOptionsAlignHMask           = 16|32,    // mask for horizontal alignment
    PositionOptionsAlignVCenter         = 0,        // center vertically within allocated space
    PositionOptionsAlignTop             = 64,       // align top within allocated space
    PositionOptionsAlignBottom          = 128,      // align bottom within allocated space
    PositionOptionsAlignVStretch        = 64|128,   // stretch within allocated space
    PositionOptionsAlignVMask           = 64|128,   // mask for vertical alignment

    PositionOptionsPreNewLine           = 256,      // start this control on a new line
    PositionOptionsPostNewLine          = 512,      // new line after this control

    PositionOptionsIgnored              = 1024,     // window will not be adjusted (either hidden or floating or out-of-band)
    PositionOptionsInvisible            = 2048,     // window will not be adjusted because it is invisible

    // General behavior options (ignored on individual controls).
    PositionOptionsUnwrapped            = 8192,     // do not wrap / allow controls to flow off the edge even if larger than containing parent width/height
    PositionOptionsReverseFlowDu        = 16384,    // reverse the primary direction (ltr->rtl in horz, ttb->btt in vert)
    PositionOptionsReverseFlowDv        = 32768,    // reverse the secondary direction (ttb->btt in horz, ltr->rtl in vert)
    PositionOptionsFlowHorizontal       = 0,        // flow primarily horizontal, then vertical
    PositionOptionsFlowVertical         = 65536,    // flow primarily vertical, then horizontal

    PositionOptionsDefault              = PositionOptionsAlignLeft|PositionOptionsAlignTop,
    PositionOptionsFillWidth            = PositionOptionsUseSlackWidth|PositionOptionsAlignHStretch,
    PositionOptionsFillHeight           = PositionOptionsUseSlackHeight|PositionOptionsAlignVStretch,
    PositionOptionsIgnoredOrInvisible   = PositionOptionsIgnored|PositionOptionsInvisible,
};
DEFINE_ENUM_FLAG_OPERATORS(PositionOptions);


struct WindowPosition
{
    HWND hwnd;
    PositionOptions options;
    RECT rect;

    WindowPosition();
    WindowPosition(HWND newHwnd, PositionOptions newOptions = PositionOptionsDefault);
    WindowPosition(HWND newHwnd, PositionOptions newOptions, const RECT& newRect);

    inline long GetWidth();
    inline long GetHeight();

    // Updates the real system window to match the rectangle in the structure.
    // If a deferred window position handle is passed, it will apply it to that,
    // otherwise the effect is immediate.
    void Update(HDWP hdwp = nullptr);

    static void Update(
        __inout_ecount(windowPositionsCount) WindowPosition* windowPositions,
        __inout uint32_t windowPositionsCount
        );

    void SetOptions(PositionOptions setOptions, PositionOptions clearOptions);

    // Clamp the size of the rect.
    void ClampRect(
        SIZE size
        );

        // Align the window to the given rectangle, using the window
    // rectangle and positioning flags.
    void AlignToRect(
        RECT rect
        );

    static RECT GetBounds(
        __inout_ecount(windowPositionsCount) WindowPosition* windowPositions,
        uint32_t windowPositionsCount
        );

    static void Offset(
        __inout_ecount(windowPositionsCount) WindowPosition* windowPositions,
        uint32_t windowPositionsCount,
        long dx,
        long dy
        );

    static RECT ReflowGrid(
        __inout_ecount(windowPositionsCount) WindowPosition* windowPositions,
        uint32_t windowPositionsCount,
        const RECT& clientRect,
        long spacing,
        uint32_t maxCols,
        PositionOptions parentOptions,
        bool queryOnly = false
        );

private:
    struct GridDimension
    {
        PositionOptions options;    // combined options for this grid cell (they come from the respective windows)
        long offset;                // x in horizontal, y in vertical
        long size;                  // width in horizontal, height in vertical
    };

    static void DistributeGridDimensions(
        __inout_ecount(gridDimensionsCount) GridDimension* gridDimensions,
        uint32_t gridDimensionsCount,
        long clientSize,
        long spacing,
        PositionOptions optionsMask // can be PositionOptionsUseSlackHeight or PositionOptionsUseSlackWidth
        );
};


struct DialogProcResult
{
    INT_PTR handled; // whether it was handled or not
    LRESULT value;   // the return value

    DialogProcResult(INT_PTR newHandled, LRESULT newValue)
    :   handled(newHandled),
        value(newValue)
    { }

    DialogProcResult(bool newHandled)
    :   handled(newHandled),
        value()
    { }
};


// Why doesn't CommCtrl.h have a helper function like this?
void ListView_SelectSingleVisibleItem(HWND hwnd, int i);


struct ListViewWriter : LVITEM
{
public:
    HWND hwnd;

public:
    ListViewWriter(HWND listHwnd);
    void InsertItem(char16_t const* text = u"");
    void SetItemText(int j, char16_t const* text);
    void AdvanceItem();
    int  GetItemCount();
    void ReserveItemCount(int itemCount);
    void SelectAndFocusItem(int i);
    void SelectItem(int i);
    void SelectItem();
    void FocusItem(int i);
    void EnsureVisible();
    void DisableDrawing();
    void EnableDrawing();
    void DeleteAllItems();
};

std::u16string GetListViewText(
    HWND listViewHwnd,
    _In_z_ char16_t const* separator
    );

void CopyListTextToClipboard(
    HWND listViewHwnd,
    _In_z_ char16_t const* separator
    );

std::vector<uint32_t> GetListViewMatchingIndices(
    HWND hwnd,
    uint32_t searchMask = LVNI_SELECTED,
    bool returnAllIfNoMatch = false
    );

std::vector<uint32_t> GetListViewCheckedIndices(
    HWND listViewHwnd
    );

void ListViewRemapIndicesToLparam(
    HWND listViewHwnd,
    _Inout_ array_ref<uint32_t> indices
    );

int32_t ListViewRemapLparamToIndex(
    HWND listViewHwnd,
    uint32_t lparamValue
    );

int MapScrollBarCodeToPosition(HWND hwnd, int barType, UINT code, int smallStep, _Out_ int& delta);
