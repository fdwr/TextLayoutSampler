//----------------------------------------------------------------------------
//  Author:     Dwayne Robinson
//  History:    2012-12-14 Created
//----------------------------------------------------------------------------
#include "precomp.h"


import Common.String;


////////////////////////////////////////

#pragma prefast(disable:__WARNING_ACCESSIBILITY_COLORAPI, "Shush. It's a test program.")
#pragma prefast(disable:__WARNING_HARDCODED_FONT_INFO,    "Shush. It's a test program.")


namespace
{
    struct LastError
    {
        HRESULT hr;

        LastError()
        {
            hr = S_OK;
            SetLastError(NOERROR);
        }

        // Update the HRESULT if a Win32 error occurred.
        // Otherwise, if success or already have an error,
        // leave the existing error code alone. This
        // preserves the first error code that happens, and
        // it avoids later check's of cleanup functions from
        // clobbering the error.
        HRESULT Check()
        {
            if (SUCCEEDED(hr))
            {
                DWORD lastError = GetLastError();
                if (lastError != NOERROR)
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                }
            }
            return hr;
        }
    };
}


HRESULT CopyTextToClipboard(HWND hwnd, array_ref<char16_t const> text)
{
    // Copies selected text to clipboard.
    LastError lastError;

    const uint32_t textLength = static_cast<uint32_t>(text.size());
    const char16_t* rawInputText = text.data();

    // Open and empty existing contents.
    if (OpenClipboard(nullptr))
    {
        if (EmptyClipboard())
        {
            // Allocate room for the text
            const uint32_t byteSize = sizeof(char16_t) * (textLength + 1);
            HGLOBAL hClipboardData  = GlobalAlloc(GMEM_DDESHARE | GMEM_ZEROINIT, byteSize);

            if (hClipboardData != nullptr)
            {
                void* memory = GlobalLock(hClipboardData);
                char16_t* rawOutputText = reinterpret_cast<char16_t*>(memory);

                if (memory != nullptr)
                {
                    // Copy text to memory block.
                    memcpy(rawOutputText, rawInputText, byteSize);
                    rawOutputText[textLength] = '\0'; // explicit nul terminator in case there is none
                    GlobalUnlock(hClipboardData);

                    if (SetClipboardData(CF_UNICODETEXT, hClipboardData) != nullptr)
                    {
                        hClipboardData = nullptr; // system now owns the clipboard, so don't touch it.
                                                  // lastError.hr = S_OK;
                    }
                    lastError.Check();
                }
                GlobalFree(hClipboardData); // free if failed (still non-null)
            }
            lastError.Check();
        }
        lastError.Check();
        CloseClipboard();
    }
    lastError.Check();

    return lastError.hr;
}


HRESULT PasteFromClipboard(OUT std::u16string& text)
{
    // Copy Unicode text from clipboard.

    LastError lastError;

    if (!OpenClipboard(nullptr))
    {
        HGLOBAL hClipboardData = GetClipboardData(CF_UNICODETEXT);

        if (hClipboardData != nullptr)
        {
            // Get text and size of text.
            size_t byteSize                 = GlobalSize(hClipboardData);
            void* memory                    = GlobalLock(hClipboardData); // [byteSize] in bytes
            const char16_t* rawText         = reinterpret_cast<const char16_t*>(memory);
            uint32_t textLength             = static_cast<uint32_t>(wcsnlen(reinterpret_cast<wchar_t const*>(rawText), byteSize / sizeof(char16_t)));

            if (memory != nullptr)
            {
                try
                {
                    text.assign(rawText, textLength);
                }
                catch (...)
                {
                    lastError.hr = Application::ExceptionToHResult();
                }

                GlobalUnlock(hClipboardData);
            }
            lastError.Check();
        }
        lastError.Check();
        CloseClipboard();
    }
    lastError.Check();

    return lastError.hr;
}


void CopyListTextToClipboard(
    HWND listViewHwnd,
    _In_z_ char16_t const* separator
    )
{
    std::u16string text = GetListViewText(listViewHwnd, separator);
    CopyTextToClipboard(listViewHwnd, {text.c_str(), text.length()});
}

HRESULT CopyImageToClipboard(
    HWND hwnd,
    HDC hdc,
    bool isUpsideDown
    )
{
    DIBSECTION sourceBitmapInfo = {};
    HBITMAP sourceBitmap = (HBITMAP)GetCurrentObject(hdc, OBJ_BITMAP);
    GetObject(sourceBitmap, sizeof(sourceBitmapInfo), &sourceBitmapInfo);

    LastError lastError;

    if (sourceBitmapInfo.dsBm.bmBitsPixel <= 8
        ||  sourceBitmapInfo.dsBm.bmBits == nullptr
        ||  sourceBitmapInfo.dsBm.bmWidth <= 0
        ||  sourceBitmapInfo.dsBm.bmHeight <= 0)
    {
        // Don't support paletted images, only true color.
        // Only support bitmaps where the pixels are accessible.
        return E_NOTIMPL;
    }

    if (OpenClipboard(hwnd))
    {
        if (EmptyClipboard())
        {
            const size_t pixelsByteSize = sourceBitmapInfo.dsBmih.biSizeImage;
            const size_t bufferLength = sizeof(sourceBitmapInfo.dsBmih) + pixelsByteSize;
            HGLOBAL bufferHandle = GlobalAlloc(GMEM_MOVEABLE, bufferLength);
            uint8_t* buffer = reinterpret_cast<uint8_t*>(GlobalLock(bufferHandle));

            // Copy the header.
            memcpy(buffer, &sourceBitmapInfo.dsBmih, sizeof(sourceBitmapInfo.dsBmih));

            if (isUpsideDown)
            {
                // The image is a bottom-up orientation. Though, since
                // Windows legacy bitmaps are upside down too, the two
                // upside-downs cancel out.
                memcpy(buffer + sizeof(sourceBitmapInfo.dsBmih), sourceBitmapInfo.dsBm.bmBits, pixelsByteSize);
            }
            else
            {
                // We have a standard top-down image, but DIBs when shared
                // on the clipboard are actually upside down. Simply flipping
                // the height to negative works for a few applications, but
                // it confuses most.

                const size_t bytesPerRow = sourceBitmapInfo.dsBm.bmWidthBytes;
                uint8_t* destRow         = buffer + sizeof(sourceBitmapInfo.dsBmih);
                uint8_t* sourceRow       = reinterpret_cast<uint8_t*>(sourceBitmapInfo.dsBm.bmBits)
                    + (sourceBitmapInfo.dsBm.bmHeight - 1) * bytesPerRow;

                // Copy each scanline in backwards order.
                for (long y = 0; y < sourceBitmapInfo.dsBm.bmHeight; ++y)
                {
                    memcpy(destRow, sourceRow, bytesPerRow);
                    sourceRow = PtrAddByteOffset(sourceRow, -ptrdiff_t(bytesPerRow));
                    destRow   = PtrAddByteOffset(destRow,    bytesPerRow);
                }
            }

            GlobalUnlock(bufferHandle);

            if (SetClipboardData(CF_DIB, bufferHandle) == nullptr)
            {
                lastError.Check();
                GlobalFree(bufferHandle);
            }
        }
        lastError.Check();

        CloseClipboard();
    }
    lastError.Check();

    return lastError.hr;
}


void ListView_SelectSingleVisibleItem(HWND hwnd, int i)
{
    // Deselect all items. Then select the single item.
    ListView_SetItemState(hwnd, -1, 0, LVIS_FOCUSED | LVIS_SELECTED);
    ListView_SetItemState(hwnd, i, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
    ListView_EnsureVisible(hwnd, i, false); // scroll down if needed
}


ListViewWriter::ListViewWriter(HWND listHwnd)
{
    ZeroStructure(*(LVITEM*)this);
    this->mask = LVIF_TEXT; // LVIF_PARAM | LVIF_IMAGE | LVIF_STATE; 
    this->hwnd = listHwnd;
}

void ListViewWriter::InsertItem(char16_t const* text)
{
    this->pszText = (LPWSTR)text;
    this->iSubItem = 0;
    ListView_InsertItem(this->hwnd, this);
}

void ListViewWriter::SetItemText(int j, char16_t const* text)
{
    this->pszText  = (LPWSTR)text;
    this->iSubItem = j;
    ListView_SetItem(this->hwnd, this);
}

void ListViewWriter::AdvanceItem()
{
    ++this->iItem;
}

int ListViewWriter::GetItemCount()
{
    return ListView_GetItemCount(this->hwnd);
}

void ListViewWriter::ReserveItemCount(int itemCount)
{
    // Despite the name, it doesn't actually set the item count unless LVS_OWNERDATA.
    ListView_SetItemCountEx(this->hwnd, itemCount, LVSICF_NOSCROLL);
}

void ListViewWriter::SelectAndFocusItem(int i)
{
    uint32_t newState = (i == -1) ? LVIS_SELECTED : LVIS_FOCUSED|LVIS_SELECTED;
    ListView_SetItemState(this->hwnd, i, newState, newState);
}

void ListViewWriter::SelectItem(int i)
{
    uint32_t newState = LVIS_SELECTED;
    ListView_SetItemState(this->hwnd, i, newState, newState);
}

void ListViewWriter::SelectItem()
{
    SelectItem(this->iItem);
}

void ListViewWriter::FocusItem(int i)
{
    assert(i != -1);
    uint32_t newState = LVIS_FOCUSED;
    ListView_SetItemState(this->hwnd, i, newState, newState);
}

void ListViewWriter::EnsureVisible()
{
    ListView_EnsureVisible(this->hwnd, this->iItem, false); // scroll down if needed
}

void ListViewWriter::DisableDrawing()
{
    SetWindowRedraw(this->hwnd, 0);
}

void ListViewWriter::EnableDrawing()
{
    SetWindowRedraw(this->hwnd, 1);
}

void ListViewWriter::DeleteAllItems()
{
    ListView_DeleteAllItems(this->hwnd);
}


std::u16string GetListViewText(
    HWND listViewHwnd,
    _In_z_ char16_t const* separator
    )
{
    // Copy the listview contents to text on the clipboard.
    std::u16string text;
    char16_t groupText[256];
    char16_t itemText[256];

    // Setup the item to retrieve text.
    LVITEM listViewItem = {};
    listViewItem.mask = LVIF_TEXT | LVIF_GROUPID; // LVIF_PARAM | LVIF_IMAGE | LVIF_STATE;
    listViewItem.iItem = 0;
    listViewItem.iSubItem = 0;
    listViewItem.pszText = (LPWSTR)itemText;
    listViewItem.cchTextMax = ARRAYSIZE(itemText);

    // Setup the group to retrieve text.
    LVGROUP listViewGroup;
    listViewGroup.cbSize    = sizeof(LVGROUP);
    listViewGroup.mask      = LVGF_HEADER | LVGF_GROUPID;
    listViewGroup.pszHeader = (LPWSTR)groupText;
    listViewGroup.cchHeader = ARRAYSIZE(groupText);
    listViewGroup.iGroupId  = 0;
    int previousGroupId     = -1;

    const uint32_t columnCount = Header_GetItemCount(ListView_GetHeader(listViewHwnd));
    std::vector<int32_t> columnOrdering(columnCount);
    ListView_GetColumnOrderArray(listViewHwnd, columnCount, OUT &columnOrdering.front());

    // Find the first selected row. If nothing is selected, then we'll copy the entire text.
    LVITEMINDEX listViewIndex = {-1, -1};
    uint32_t searchMask = LVNI_SELECTED;
    ListView_GetNextItemIndex(listViewHwnd, &listViewIndex, searchMask);
    if (listViewIndex.iItem < 0)
    {
        searchMask = LVNI_ALL;
        ListView_GetNextItemIndex(listViewHwnd, &listViewIndex, searchMask);
    }

    // Build up a concatenated string.
    // Append next item's column to the text. (either the next selected or all)
    while (listViewIndex.iItem >= 0)
    {
        uint32_t columnSeparatorCount = 0;

        for (uint32_t column = 0; column < columnCount; ++column)
        {
            int32_t columnIndex = columnOrdering[column];

            if (ListView_GetColumnWidth(listViewHwnd, columnIndex) <= 0)
                continue; // Skip zero-width columns.

                          // Set up fields for call. We must reset the string pointer
                          // and count for virtual listviews because while it copies the
                          // text out to our buffer, it also messes with the pointer.
                          // Otherwise the next call will crash.
            itemText[0]       = '\0';
            listViewItem.iItem      = listViewIndex.iItem;
            listViewItem.iSubItem   = columnIndex;
            listViewItem.pszText    = (LPWSTR)itemText;
            listViewItem.cchTextMax = ARRAYSIZE(itemText);
            ListView_GetItem(listViewHwnd, &listViewItem);

            // Append the group name if we changed to a different group.
            if (listViewItem.iGroupId != previousGroupId)
            {
                // Add an extra blank line between the previous group.
                if (previousGroupId >= 0)
                {
                    text += u"\r\n";
                }
                previousGroupId = listViewItem.iGroupId;

                // Add group header (assuming it belongs to a group).
                if (listViewItem.iGroupId >= 0)
                {
                    groupText[0]            = '\0';
                    listViewGroup.pszHeader = (LPWSTR)groupText;
                    listViewGroup.cchHeader = ARRAYSIZE(groupText);
                    ListView_GetGroupInfo(listViewHwnd, listViewItem.iGroupId, &listViewGroup);

                    text += u"― ";
                    text += groupText;
                    text += u" ―\r\n";
                }
            }

            // If the cell is non-empty, append the text (and any accumulated separators).
            if (wcslen(ToWChar(itemText)) > 0)
            {
                // Append a separator between columns.
                while (columnSeparatorCount > 0)
                {
                    text += separator; // usually a tab, comma, or colon for field/value pair ": "
                    --columnSeparatorCount;
                }
                text += itemText;
            }
            columnSeparatorCount++;
        } // for column

        text += u"\r\n"; // new line

        if (!ListView_GetNextItemIndex(listViewHwnd, &listViewIndex, searchMask))
            break;
    } // while iItem

    text.shrink_to_fit();

    return text;
}


std::vector<uint32_t> GetListViewMatchingIndices(
    HWND listViewHwnd,
    uint32_t searchMask,
    bool returnAllIfNoMatch
    )
{
    std::vector<uint32_t> matchingIndices;

    // Find the first selected row. If nothing is selected, then we'll copy the entire text.
    LVITEMINDEX listViewIndex = {-1, -1};
    ListView_GetNextItemIndex(listViewHwnd, &listViewIndex, searchMask);
    if (listViewIndex.iItem < 0 && returnAllIfNoMatch)
    {
        searchMask = LVNI_ALL;
        ListView_GetNextItemIndex(listViewHwnd, &listViewIndex, searchMask);
    }

    while (listViewIndex.iItem >= 0)
    {
        matchingIndices.push_back(listViewIndex.iItem);
        ListView_GetNextItemIndex(listViewHwnd, &listViewIndex, searchMask);
    }

    return matchingIndices;
}


std::vector<uint32_t> GetListViewCheckedIndices(
    HWND listViewHwnd
    )
{
    std::vector<uint32_t> matchingIndices;

    int itemCount = ListView_GetItemCount(listViewHwnd);
    for (int i = 0; i < itemCount; ++i)
    {
        if (ListView_GetCheckState(listViewHwnd, i))
        {
            matchingIndices.push_back(i);
        }
    }

    return matchingIndices;
}


void ListViewRemapIndicesToLparam(
    HWND listViewHwnd,
    _Inout_ array_ref<uint32_t> indices
    )
{
    // Remap listview indices to attribute value indices, using lparam.
    LVITEM listViewItem = {};
    listViewItem.mask = LVIF_PARAM;
    listViewItem.iItem = 0;
    listViewItem.iSubItem = 0;
    for (auto& index : indices)
    {
        listViewItem.iItem = index;
        ListView_GetItem(listViewHwnd, IN OUT &listViewItem);
        index = static_cast<uint32_t>(listViewItem.lParam);
    }
}


int32_t ListViewRemapLparamToIndex(
    HWND listViewHwnd,
    uint32_t lparamValue
    )
{
    // Remap listview indices to attribute value indices, using lparam.
    LVITEM listViewItem = {};
    listViewItem.mask = LVIF_PARAM;
    listViewItem.iItem = 0;
    listViewItem.iSubItem = 0;

    int itemCount = ListView_GetItemCount(listViewHwnd);
    for (int index = 0; index < itemCount; ++index)
    {
        listViewItem.iItem = index;
        ListView_GetItem(listViewHwnd, IN OUT &listViewItem);
        if (listViewItem.lParam == lparamValue)
            return index;
    }

    return -1;
}


int GetWindowText(HWND hwnd, _Out_ std::u16string& s)
{
    auto textLength = GetWindowTextLength(hwnd);
    s.resize(textLength);
    return ::GetWindowText(hwnd, OUT ToWChar(&s[0]), textLength + 1);
}


BOOL GetWindowPos(HWND hwnd, _Out_ RECT& rect)
{
    return GetWindowRect(hwnd, &rect)
        && MapWindowPoints(HWND_DESKTOP, GetParent(hwnd), IN OUT reinterpret_cast<POINT*>(&rect), 2);
}


WindowPosition::WindowPosition()
:   hwnd(),
    options(PositionOptionsDefault),
    rect()
{
}


WindowPosition::WindowPosition(HWND newHwnd, PositionOptions newOptions)
:   hwnd(newHwnd),
    options(newOptions)
{
    GetWindowPos(newHwnd, OUT rect);

    // Ignore the window if not visible. Explicitly use style instead of
    // IsVisible() because the parent window may not be visible yet if
    // child windows are being laid out before showing the main window.
    if (!(GetWindowStyle(newHwnd) & WS_VISIBLE))
    {
        options |= PositionOptionsInvisible;
    }
}


WindowPosition::WindowPosition(HWND newHwnd, PositionOptions newOptions, const RECT& newRect)
:   hwnd(newHwnd),
    options(newOptions),
    rect(newRect)
{
}


long WindowPosition::GetWidth()
{
    return rect.right - rect.left;
}


long WindowPosition::GetHeight()
{
    return rect.bottom - rect.top;
}



void WindowPosition::Update(HDWP hdwp)
{
    if (hwnd == nullptr)
        return;

    int width = GetWidth();
    int height = GetHeight();

    if (width < 0)  width = 0;
    if (height < 0) height = 0;

    if (hdwp != nullptr)
    {
        DeferWindowPos(
            hdwp,
            hwnd,
            nullptr,
            rect.left,
            rect.top,
            width,
            height,
            SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER
            );
    }
    else
    {
        SetWindowPos(
            hwnd,
            nullptr,
            rect.left,
            rect.top,
            width,
            height,
            SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER
            );
    }
}


void WindowPosition::Update(
    __inout_ecount(windowPositionsCount) WindowPosition* windowPositions,
    __inout uint32_t windowPositionsCount
    )
{
    HDWP hdwp = BeginDeferWindowPos(windowPositionsCount);
    for (uint32_t i = 0; i < windowPositionsCount; ++i)
    {
        windowPositions[i].Update(hdwp);
    }
    EndDeferWindowPos(hdwp);
}


RECT WindowPosition::GetBounds(
    __inout_ecount(windowPositionsCount) WindowPosition* windowPositions,
    uint32_t windowPositionsCount
    )
{
    RECT bounds;
    bounds.left = INT_MAX;
    bounds.right = -INT_MAX;
    bounds.top = INT_MAX;
    bounds.bottom = -INT_MAX;

    for (uint32_t i = 0; i < windowPositionsCount; ++i)
    {
        UnionRect(OUT &bounds, &bounds, &windowPositions[i].rect);
    }

    return bounds;
}


void WindowPosition::Offset(
    __inout_ecount(windowPositionsCount) WindowPosition* windowPositions,
    uint32_t windowPositionsCount,
    long dx,
    long dy
    )
{
    for (uint32_t i = 0; i < windowPositionsCount; ++i)
    {
        OffsetRect(IN OUT &windowPositions[i].rect, dx, dy);
    }
}


void WindowPosition::SetOptions(PositionOptions setOptions, PositionOptions clearOptions)
{
    this->options = (this->options & ~clearOptions) | setOptions;
}


void WindowPosition::ClampRect(
    SIZE size
    )
{
    // If the current width is larger than given width, clamp and align horizontally.
    if (this->rect.right > this->rect.left + size.cx)
    {
        this->rect.right = this->rect.left + size.cx;
        this->options &= ~(PositionOptionsAlignHMask | PositionOptionsUseSlackWidth);
        this->options |= PositionOptionsAlignLeft;
    }

    // If the current height is larger than given height, clamp and align vertically.
    if (this->rect.bottom > this->rect.top + size.cy)
    {
        this->rect.bottom = this->rect.top + size.cy;
        this->options &= ~(PositionOptionsAlignVMask | PositionOptionsUseSlackHeight);
        this->options |= PositionOptionsAlignTop;
    }
}


// Align the window to the given rectangle, using the window
// rectangle and positioning flags.
void WindowPosition::AlignToRect(
    RECT rect
    )
{
    const PositionOptions horizontalAlignment = this->options & PositionOptionsAlignHMask;
    const PositionOptions verticalAlignment = this->options & PositionOptionsAlignVMask;

    long windowWidth = this->rect.right - this->rect.left;
    long windowHeight = this->rect.bottom - this->rect.top;
    long rectWidth = rect.right - rect.left;
    long rectHeight = rect.bottom - rect.top;

    long x = 0;
    switch (horizontalAlignment)
    {
    case PositionOptionsAlignHStretch:
        windowWidth = rectWidth;
        // fall through
    case PositionOptionsAlignLeft:
    default:
        x = 0;
        break;
    case PositionOptionsAlignRight:
        x = rectWidth - windowWidth;
        break;
    case PositionOptionsAlignHCenter:
        x = (rectWidth - windowWidth) / 2;
        break;
    }

    long y = 0;
    switch (verticalAlignment)
    {
    case PositionOptionsAlignVStretch:
        windowHeight = rectHeight;
        // fall through
    case PositionOptionsAlignTop:
    default:
        y = 0;
        break;
    case PositionOptionsAlignBottom:
        y = rectHeight - windowHeight;
        break;
    case PositionOptionsAlignVCenter:
        y = (rectHeight - windowHeight) / 2;
        break;
    }

    this->rect.left = x + rect.left;
    this->rect.top = y + rect.top;
    this->rect.right = this->rect.left + windowWidth;
    this->rect.bottom = this->rect.top + windowHeight;
}


struct GridDimension
{
    PositionOptions options;    // combined options for this grid cell (they come from the respective windows)
    long offset;                // x in horizontal, y in vertical
    long size;                  // width in horizontal, height in vertical
};


void WindowPosition::DistributeGridDimensions(
    __inout_ecount(gridDimensionsCount) GridDimension* gridDimensions,
    uint32_t gridDimensionsCount,
    long clientSize,
    long spacing,
    PositionOptions optionsMask // can be PositionOptionsUseSlackHeight or PositionOptionsUseSlackWidth
    )
{
    if (gridDimensionsCount <= 0)
    {
        return; // nothing to do
    }

    // Determine the maximum size along a single dimension,
    // and assign all offsets.

    long accumulatedSize = 0;
    uint32_t gridFillCount = 0;

    for (uint32_t i = 0; i < gridDimensionsCount; ++i)
    {
        if (gridDimensions[i].options & PositionOptionsIgnoredOrInvisible)
            continue;

        // Add spacing between all controls, but not before the first control,
        // not after zero width controls.
        if (i > 0)
            accumulatedSize += spacing;
        gridDimensions[i].offset = accumulatedSize;
        accumulatedSize += gridDimensions[i].size;

        if (gridDimensions[i].options & optionsMask)
            ++gridFillCount; // found another grid cell to fill
    }

    // Check if we still have leftover space inside the parent's client rect.

    const long slackSizeToRedistribute = clientSize - accumulatedSize;
    if (gridFillCount <= 0 || slackSizeToRedistribute <= 0)
    {
        return; // nothing more to do
    }

    // Distribute any slack space remaining throughout windows that want it.

    uint32_t gridFillIndex = 0;
    long offsetAdjustment = 0;

    for (uint32_t i = 0; i < gridDimensionsCount; ++i)
    {
        if (gridDimensions[i].options & PositionOptionsIgnoredOrInvisible)
            continue;

        // Shift the offset over from any earlier slack space contribution.
        gridDimensions[i].offset += offsetAdjustment;

        // Contribute the slack space into this row/col.
        if (gridDimensions[i].options & optionsMask)
        {
            long sizeForGridCell = (slackSizeToRedistribute * (gridFillIndex + 1) / gridFillCount)
                - (slackSizeToRedistribute *  gridFillIndex / gridFillCount);

            // Increase the size of subsequent offsets.
            gridDimensions[i].size += sizeForGridCell;
            offsetAdjustment += sizeForGridCell;
            ++gridFillIndex;
        }
    }
}

RECT WindowPosition::ReflowGrid(
    __inout_ecount(windowPositionsCount) WindowPosition* windowPositions,
    uint32_t windowPositionsCount,
    const RECT& clientRect,
    long spacing,
    uint32_t maxCols,
    PositionOptions parentOptions,
    bool queryOnly
    )
{
    // There can be up to n rows and n columns, so allocate up to 2n.
    // Rather than allocate two separate arrays, use the first half
    // for columns and second half for rows. Note the interpretation
    // of rows and columns flips if the layout is vertical.
    std::vector<GridDimension> gridDimensions(windowPositionsCount * 2);

    bool const isVertical  = (parentOptions & PositionOptionsFlowVertical)  != 0;
    bool const isReverseDu = (parentOptions & PositionOptionsReverseFlowDu) != 0;
    bool const isReverseDv = (parentOptions & PositionOptionsReverseFlowDv) != 0;
    bool const isWrapped   = (parentOptions & PositionOptionsUnwrapped)     == 0;

    if (maxCols < windowPositionsCount)
    {
        // Set the flag to make logic easier in the later loop.
        gridDimensions[maxCols].options |= PositionOptionsPreNewLine;
    }
    else
    {
        // Clamp to actual count.
        maxCols = windowPositionsCount;
    }

    ////////////////////////////////////////
    // Arrange all the children, flowing left-to-right for horizontal,
    // top-to-bottom for vertical (flipping at the end if RTL). Note
    // that all the coordinates are rotated 90 degrees when vertical
    // (so x is really y).

    long colOffset = 0;    // current column offset
    long rowOffset = 0;    // current column offset
    long totalColSize = 0; // total accumulated column size
    long totalRowSize = 0; // total accumulated row size
    long maxColSize = clientRect.right - clientRect.left;
    long maxRowSize = clientRect.bottom - clientRect.top;
    if (isVertical)
        std::swap(maxColSize, maxRowSize);

    uint32_t colCount = 0; // number of columns (reset per row)
    uint32_t rowCount = 0; // number of rows
    uint32_t gridStartingColIndex = 0; // starting column index on current row
    uint32_t gridColIndex = 0; // array index, not column offset
    uint32_t gridRowIndex = windowPositionsCount; // array index, not row offset

    PositionOptions previousOptions = PositionOptionsNone;

    for (size_t windowIndex = 0; windowIndex < windowPositionsCount; ++windowIndex)
    {
        const WindowPosition& window = windowPositions[windowIndex];
        PositionOptions options = window.options;

        // Skip any windows that are invisible, floating, or resized by the caller.
        if (options & PositionOptionsIgnoredOrInvisible)
        {
            continue;
        }

        // Expand the grid if the window is larger than any previous.
        long colSize = window.rect.right - window.rect.left;
        long rowSize = window.rect.bottom - window.rect.top;

        // If the row or column should be stretched, zero the size so that it
        // does not prematurely increase the size. Otherwise resizing the same
        // control repeatedly would end up only ever growing the control.
        if ((options & PositionOptionsAlignHMask) == PositionOptionsAlignHStretch)
        {
            colSize = 0;
        }
        if ((options & PositionOptionsAlignVMask) == PositionOptionsAlignVStretch)
        {
            rowSize = 0;
        }

        if (isVertical)
            std::swap(colSize, rowSize);

        // Flow down to a new line if:
        // - equals the maximum column count.
        // - wider than maximum column size.
        // - if current window options says it should be on a new line.
        // - previous window option says it should be followed by a new line.
        if (colCount > 0)
        {
            if ((options & PositionOptionsPreNewLine)
            ||  (previousOptions & PositionOptionsPostNewLine)
            ||  (maxCols > 0 && colCount >= maxCols)
            ||  (isWrapped && colOffset + colSize > maxColSize))
            {
                // Distribute the columns. For a regular grid, we'll distribute
                // later once we've looked at all the rows. For irregular flow,
                // distribute now while we still know the range of windows on
                // the current row.
                if (maxCols == 0)
                {
                    DistributeGridDimensions(
                        &gridDimensions[gridStartingColIndex],
                        colCount,
                        maxColSize,
                        spacing,
                        isVertical ? PositionOptionsUseSlackHeight : PositionOptionsUseSlackWidth
                        );
                    gridStartingColIndex += colCount;
                }
                else
                {
                    gridColIndex = 0;
                }

                // Reset and advance.
                ++rowCount;
                ++gridRowIndex;
                colCount = 0;
                colOffset = 0;
                rowOffset = totalRowSize + spacing;

                // Set this flag to make later logic easier so that it doesn't
                // need to recheck a number of wrapping conditions.
                options |= PositionOptionsPreNewLine;
            }
        }

        // Combine the positioning options of the current window into the grid column/row.
        gridDimensions[gridColIndex].options |= options;
        gridDimensions[gridRowIndex].options |= options;

        // If this window is larger than any previous, increase the grid row/column size,
        // and add to the accumulated total rect.
        if (colSize > gridDimensions[gridColIndex].size)
        {
            gridDimensions[gridColIndex].size = std::min(colSize, maxColSize);
        }
        if (rowSize > gridDimensions[gridRowIndex].size)
        {
            gridDimensions[gridRowIndex].size = std::min(rowSize, maxRowSize);
        }

        totalColSize = std::max(colOffset + colSize, totalColSize);
        totalRowSize = std::max(rowOffset + rowSize, totalRowSize);

        previousOptions = options;

        // Next column...
        colOffset = colOffset + colSize + spacing;
        ++colCount;
        ++gridColIndex;
    }
    if (colCount > 0)
    {
        ++rowCount; // count the last row
    }

    ////////////////////////////////////////
    // Return the bounding rect if wanted.

    RECT resultRect = { 0,0,totalColSize,totalRowSize };
    if (isVertical)
        std::swap(resultRect.right, resultRect.bottom);

    if (queryOnly)
        return resultRect;

    ////////////////////////////////////////
    // Distribute the any remaining slack space into rows and columns.

    DistributeGridDimensions(
        &gridDimensions.data()[gridStartingColIndex],
        (maxCols > 0) ? maxCols : colCount,
        maxColSize,
        spacing,
        isVertical ? PositionOptionsUseSlackHeight : PositionOptionsUseSlackWidth
        );

    gridRowIndex = windowPositionsCount; // array index, not row offset
    DistributeGridDimensions(
        &gridDimensions.data()[gridRowIndex],
        rowCount,
        maxRowSize,
        spacing,
        !isVertical ? PositionOptionsUseSlackHeight : PositionOptionsUseSlackWidth
        );

    ////////////////////////////////////////
    // Update window rects with the new positions from the grid.

    // Reset for second loop.
    colCount = 0;
    gridColIndex = 0;
    gridRowIndex = windowPositionsCount;

    for (size_t windowIndex = 0; windowIndex < windowPositionsCount; ++windowIndex)
    {
        WindowPosition& window = windowPositions[windowIndex];

        // Skip any invisible/floating windows.
        if (window.options & PositionOptionsIgnoredOrInvisible)
        {
            continue;
        }

        const bool isNewLine = !!(gridDimensions[gridColIndex].options & PositionOptionsPreNewLine);
        if (isNewLine && colCount > 0)
        {
            ++gridRowIndex;
            if (maxCols > 0)
            {
                gridColIndex = 0;
            }
        }

        RECT gridRect = {
            gridDimensions[gridColIndex].offset,
            gridDimensions[gridRowIndex].offset,
            gridRect.left + gridDimensions[gridColIndex].size,
            gridRect.top  + gridDimensions[gridRowIndex].size,
        };
        if (isVertical)
        {
            std::swap(gridRect.left, gridRect.top);
            std::swap(gridRect.right, gridRect.bottom);
        }
        gridRect.left    += clientRect.left;
        gridRect.right   += clientRect.left;
        gridRect.top     += clientRect.top;
        gridRect.bottom  += clientRect.top;

        window.AlignToRect(gridRect);

        ++colCount;
        ++gridColIndex;
    }

    if (isReverseDu)
    {
        long clientRectWidth = clientRect.right + clientRect.left;
        for (size_t windowIndex = 0; windowIndex < windowPositionsCount; ++windowIndex)
        {
            WindowPosition& window = windowPositions[windowIndex];
            long adjustment = clientRectWidth - window.rect.right - window.rect.left;
            window.rect.left += adjustment;
            window.rect.right += adjustment;
        }
    }

    if (isReverseDv)
    {
        long clientRectHeight = clientRect.bottom + clientRect.top;
        for (size_t windowIndex = 0; windowIndex < windowPositionsCount; ++windowIndex)
        {
            WindowPosition& window = windowPositions[windowIndex];
            long adjustment = clientRectHeight - window.rect.bottom - window.rect.top;
            window.rect.top += adjustment;
            window.rect.bottom += adjustment;
        }
    }

    return resultRect;
}


// Seriously, why does GetSubMenu not support either by-position or by-id like GetMenuItemInfo?
HMENU WINAPI GetSubMenu(
    __in HMENU parentMenu,
    __in UINT item,
    __in BOOL byPosition
    )
{
    MENUITEMINFO menuItemInfo;
    menuItemInfo.cbSize = sizeof(menuItemInfo);
    menuItemInfo.fMask = MIIM_ID | MIIM_SUBMENU;
    if (!GetMenuItemInfo(parentMenu, item, byPosition, OUT &menuItemInfo))
        return nullptr;

    return menuItemInfo.hSubMenu;
}


// Why isn't this a core Windows function!?
// Returns a new position based on the scroll code plus the delta.
int MapScrollBarCodeToPosition(HWND hwnd, int barType, UINT code, int smallStep, _Out_ int& delta)
{
    delta = 0;

    SCROLLINFO scrollInfo = {};
    scrollInfo.cbSize = sizeof(SCROLLINFO);
    scrollInfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
    GetScrollInfo(hwnd, barType, &scrollInfo);

    const int minPos = scrollInfo.nMin;
    const int maxPos = scrollInfo.nMax - (scrollInfo.nPage - 1); // todo::: check the -1
    int newDelta = 0;
    int newPosition = 0;

    // Get the desired delta or position.
    switch (code)
    {
    case SB_LINEUP:         newDelta = -smallStep; break;
    case SB_LINEDOWN:       newDelta =  smallStep; break;
    case SB_PAGEUP:         newDelta = -static_cast<int>(scrollInfo.nPage); break;
    case SB_PAGEDOWN:       newDelta =  static_cast<int>(scrollInfo.nPage); break;
    case SB_THUMBTRACK:     newPosition = scrollInfo.nTrackPos; break;
    case SB_TOP:            newPosition = minPos; break;
    case SB_BOTTOM:         newPosition = maxPos; break;
    default:
        return 0;
        // Do nothing for these or unrecognized codes:
        // SB_THUMBPOSITION
        // SB_ENDSCROLL
    }

    if (newDelta != 0)
    {
        newPosition = std::min(std::max(scrollInfo.nPos + newDelta, minPos), maxPos);
    }

    // Determine actual delta after clamping.
    delta = newPosition - scrollInfo.nPos;
    return newPosition;
}


int TrackPopupMenu(
    array_ref<TrackPopupMenu_Item const> items,
    HWND controlHwnd,
    HWND parentHwnd
    )
{
    // Insert an array of menu items, or a separator if '-'.
    HMENU menu = CreatePopupMenu();
    for (auto const& item : items)
    {
        if (item.name[0] == '-')
        {
            AppendMenu(menu, MF_SEPARATOR, item.id, ToWChar(item.name));
        }
        else
        {
            AppendMenu(menu, MF_STRING, item.id, ToWChar(item.name));
        }
    }
    auto menuId = TrackPopupMenu(menu, controlHwnd, parentHwnd);
    DestroyMenu(menu);
    return menuId;
}


int TrackPopupMenu(
    HMENU menu,
    HWND controlHwnd,
    HWND parentHwnd
    )
{
    RECT rect = {};
    GetWindowRect(controlHwnd, OUT &rect);
    TPMPARAMS trackPopupMenuParams = {};
    trackPopupMenuParams.cbSize = sizeof(trackPopupMenuParams);
    trackPopupMenuParams.rcExclude = rect;

    int menuId = TrackPopupMenuEx(
        menu,
        TPM_VERTICAL | TPM_LEFTALIGN | TPM_NOANIMATION | TPM_RETURNCMD | TPM_RIGHTBUTTON,
        trackPopupMenuParams.rcExclude.left,
        trackPopupMenuParams.rcExclude.top,
        parentHwnd,
        &trackPopupMenuParams
        );

    return menuId;
}


bool GetOpenFileName(
    HWND hwnd,
    _In_opt_z_ char16_t const* fileTypes,
    _Out_ std::u16string& filePath,
    char16_t const* title
    )
{
    filePath.clear();

    // Buffer that receives the file name.
    char16_t filePathBuffer[MAX_PATH * 4];
    filePathBuffer[0] = L'\0';

    // Initialize the OPENFILENAME structure.
    OPENFILENAME ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = ToWChar(fileTypes);
    ofn.nFilterIndex = 0;
    ofn.lpstrFile = ToWChar(filePathBuffer);
    ofn.nMaxFile = uint32_t(std::size(filePathBuffer));
    ofn.lpstrTitle = ToWChar(title);
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOTESTFILECREATE | OFN_EXPLORER | OFN_ENABLESIZING;

    if (!GetOpenFileName(IN OUT &ofn))
        return false;

    filePath.assign(filePathBuffer);
    return true;
}


bool GetSaveFileName(
    HWND hwnd,
    _In_opt_z_ char16_t const* defaultFileName,
    _In_opt_z_ char16_t const* fileTypes,
    _In_opt_z_ char16_t const* defaultExtension,
    _Out_ std::u16string& filePath,
    char16_t const* title
    )
{
    // Buffer that receives the file name.
    char16_t filePathBuffer[MAX_PATH * 4];
    filePathBuffer[0] = '\0';
    if (defaultFileName != nullptr)
    {
        wcscpy_s(ToWChar(filePathBuffer), std::size(filePathBuffer), ToWChar(defaultFileName));
    }

    filePath.clear();

    // Initialize the OPENFILENAME structure.
    OPENFILENAME ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = ToWChar(fileTypes);
    ofn.nFilterIndex = 0;
    ofn.lpstrFile = ToWChar(filePathBuffer);
    ofn.nMaxFile = uint32_t(std::size(filePathBuffer));
    ofn.lpstrDefExt = ToWChar(defaultExtension);
    ofn.lpstrTitle = ToWChar(title);
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_NOTESTFILECREATE | OFN_EXPLORER | OFN_ENABLESIZING;

    if (!GetSaveFileName(IN OUT &ofn))
        return false;

    filePath.assign(filePathBuffer);
    return true;
}
