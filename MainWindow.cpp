/*
todo:::
Rendering mode
Custom font fallback
Custom font collection
AddFontMemResource
EDIT/RichEdit
Show red attribute on parse error
Add font selection dialog
Add font open dialog for font file
Get all characters
?Character glyph map
App translucency
-Hit test canvas
-Pan scroll canvas
-Font fallback enable/disable
-Store settings
-Load settings
-Transform drawable objects
-Context menu on drawable objects right-click
-Drawable object absolute placement
-Draw labels
-Reflow drawable objects
-GDI+ DrawString/MeasureString
-GDI+ DrawDriverString
-Show red box on drawing error
-Draw object background colors and object together
-Fix user32 DrawText for vertical
-Save selected font file
*/
//----------------------------------------------------------------------------
//  Author:     Dwayne Robinson
//  History:    2015-06-19 Created
//----------------------------------------------------------------------------

#include "precomp.h"
#include "resource.h"
#include "MainWindow.h"


import MessageBoxShaded;


#define DEBUG_SHOW_WINDOWS_MESSAGES 0


////////////////////////////////////////
// UI related


#pragma prefast(disable:__WARNING_HARD_CODED_STRING_TO_UI_FN, "It's an internal test program.")

HACCEL MainWindow::g_accelTable;
int const g_smallMouseWheelStep = 20;

char16_t const* g_openSaveFiltersList = u"All supported text layout sampler files\0" u"*.TextLayoutSamplerSettings;*.txt\0"
                                        u"Layout Sampler Settings (*.TextLayoutSamplerSettings)\0" u"*.TextLayoutSamplerSettings\0"
                                        u"Text files (*.txt)\0" u"*.txt\0"
                                        u"All files (*)\0" u"*\0";


////////////////////////////////////////


HWND MainWindow::Create()
{
    RegisterCustomClasses();
    g_accelTable = LoadAccelerators(Application::g_hModule, MAKEINTRESOURCE(IdaMainWindow));
    return CreateDialog(Application::g_hModule, MAKEINTRESOURCE(IddMainWindow), nullptr, &MainWindow::StaticDialogProc);
}


void MainWindow::RegisterCustomClasses()
{
    // Ensure that the common control DLL is loaded.
    // (probably not needed, but do it anyway)
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC  = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex); 

    DrawingCanvasControl::RegisterWindowClass(Application::g_hModule);
}


HWND MainWindow::GetHwnd() const
{
    return hwnd_;
}


MainWindow* MainWindow::GetClass(HWND hwnd)
{
    return reinterpret_cast<MainWindow*>(::GetWindowLongPtr(hwnd, DWLP_USER));
}


INT_PTR CALLBACK MainWindow::StaticDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    MainWindow* window = GetClassFromDialog<MainWindow>(hwnd);
    if (window == nullptr)
    {
        window = new(std::nothrow) MainWindow(hwnd);
        if (window == nullptr)
        {
            return -1; // failed creation
        }

        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)window);
    }

    DialogProcResult result = {FALSE, 0};
    try
    {
        result = window->DialogProc(hwnd, message, wParam, lParam);
        if (result.handled)
        {
            ::SetWindowLongPtr(hwnd, DWLP_MSGRESULT, result.value);
        }
    }
    catch (...)
    {
    }
    return result.handled;
}


MainWindow::DialogProcResult CALLBACK MainWindow::DialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (isRecursing_)
        return false;

#if DEBUG_SHOW_WINDOWS_MESSAGES
    isRecursing_ = true;
    switch (message)
    {
    case WM_PAINT:
    case WM_GETICON:
    case WM_GETOBJECT:
    case WM_MOUSEMOVE:
    case WM_SETCURSOR:
    case CB_GETCOMBOBOXINFO: // Why are we getting this message!?
    case WM_CHANGEUISTATE:
        break;
    default:
        AppendLog(L"time=%d DLGMessage=%X, wParam=%08X, lParam=%08X\r\n", GetTickCount(), message, wParam, lParam);
        break;
    }
    isRecursing_ = false;
#endif

    switch (message)
    {
    case WM_INITDIALOG:
        return !!InitializeMainDialog();

    HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
    HANDLE_MSG(hwnd, WM_NOTIFY, OnNotification);

    case WM_SIZE:
        Resize(IddMainWindow);
        break;

    case WM_KEYDOWN:
        TranslateAccelerator(hwnd, g_accelTable, &Application::g_msg);
        break;

    case WM_ENTERIDLE:
    case WM_TIMER:
        if (wParam == IdcUpdateUi)
        {
            KillTimer(hwnd, wParam);
            UpdateUi();
        }
        #if 0
        else if (wParam == IdcUpdateUi+1)// hack:::
        {
            RepaintDrawableObjects();
        }
        #endif
        else
        {
            return false;
        }
        break;

    case WM_MOVE:
        break;

    case WM_DESTROY:
        break;

    case WM_DROPFILES:
        return OnDragAndDrop(hwnd, message, wParam, lParam);

    default:
        return false;
    }

    return true;
}


LRESULT CALLBACK AttributeValueEditProcedure(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    UINT_PTR subclassId,
    DWORD_PTR data
    )
{
    // Subclass procedure for the attribute value edit, so that the edit
    // behaves like a combo box.

    MainWindow* window = reinterpret_cast<MainWindow*>(data);

    switch (message)
    {
    case WM_KEYDOWN:
        {
            // Forward certain keys to the list if single line edit.
            auto windowStyle = GetWindowStyle(hwnd);
            if (windowStyle & ES_MULTILINE)
            {
                break;
            }

            switch (wParam)
            {
            case VK_RETURN: // Enter key pressed.
                SetFocus(GetWindowFromId(window->GetHwnd(), IdcAttributesList));
                return 0;

            case VK_PRIOR:
            case VK_NEXT:
            case VK_UP:
            case VK_DOWN:
                return SendMessage(GetWindowFromId(window->GetHwnd(), IdcAttributeValuesList), message, wParam, lParam);
            }
        }
        break;

    case WM_NCDESTROY:
        RemoveWindowSubclass(hwnd, &AttributeValueEditProcedure, 0);
        break;

    case WM_PRINTCLIENT:
    case WM_PAINT:
        {
            auto textLength = GetWindowTextLength(hwnd);
            if (textLength == 0)// && GetFocus() != hwnd)
            {
                // Get the needed DC with DCX_INTERSECTUPDATE before the EDIT
                // control's WM_PAINT handler calls BeginPaint/EndPaint, which
                // validates the update rect and would otherwise lead to drawing
                // nothing later because the region is empty.
                HDC hdc = (message == WM_PRINTCLIENT)
                    ? reinterpret_cast<HDC>(wParam)
                    : GetDCEx(hwnd, nullptr, DCX_INTERSECTUPDATE | DCX_CACHE | DCX_CLIPCHILDREN | DCX_CLIPSIBLINGS);

                // Call the EDIT control so that the caret is properly handled,
                // no litter left on the screen after tabbing away.
                auto result = DefSubclassProc(hwnd, message, wParam, lParam);

                // Get the font and margin so the cue banner text has a
                // consistent appearance and placement with existing text.
                HFONT font = GetWindowFont(hwnd);
                RECT editRect;
                Edit_GetRect(hwnd, OUT &editRect);

                // Ideally we would call Edit_GetCueBannerText, but since that message
                // returns nothing when ES_MULTILINE, use a window property instead.
                auto* cueBannerText = reinterpret_cast<char16_t*>(GetProp(hwnd, L"CueBannerText"));

                HFONT previousFont = SelectFont(hdc, font);
                SetTextColor(hdc, GetSysColor(COLOR_GRAYTEXT));
                SetBkMode(hdc, TRANSPARENT);
                DrawText(hdc, ToWChar(cueBannerText), int(wcslen(ToWChar(cueBannerText))), &editRect, DT_TOP| DT_LEFT|DT_NOPREFIX|DT_NOCLIP);
                SelectFont(hdc, previousFont);

                ReleaseDC(hwnd, hdc);

                // Return the EDIT's result (could just return zero here,but
                // will relay whatever value came from the edit).
                return result;
            }
        }
        break;
    }

    return DefSubclassProc(hwnd, message, wParam, lParam);
}


INT_PTR MainWindow::InitializeMainDialog()
{
    ////////////////////
    // Re-enable drag and drop, even if running process as admin.
    // Then set icon and title.

    #ifndef WM_COPYGLOBALDATA
    #define WM_COPYGLOBALDATA 0x0049
    #endif

    ChangeWindowMessageFilterEx(hwnd_, WM_DROPFILES, MSGFLT_ALLOW, nullptr);
    ChangeWindowMessageFilterEx(hwnd_, WM_COPYDATA, MSGFLT_ALLOW, nullptr);
    ChangeWindowMessageFilterEx(hwnd_, WM_COPYGLOBALDATA, MSGFLT_ALLOW, nullptr);

    DefWindowProc(hwnd_, WM_SETICON, ICON_BIG, LPARAM(LoadIcon(Application::g_hModule, MAKEINTRESOURCE(1))));
    SetWindowText(hwnd_, BUILD_TITLE_STRING);

    #if 0 // todo::: delete
    SetWindowStyleEx(g_mainHwnd, GetWindowLong(g_mainHwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(
        g_mainHwnd,
        0,
        128,
        LWA_ALPHA
        );
    #endif

    ////////////////////
    // Initialize with initialize objects.

    char16_t* const functionNames[] = {
        u"D2D DrawTextLayout",
        u"IDWriteBitmapRenderTarget IDWriteTextLayout",
        u"User32 DrawText",
        u"GDIPlus DrawString",
        //u"GDI ExtTextOut",
        //u"IDWriteBitmapRenderTarget DrawGlyphRun",
        //u"GDIPlus DrawDriverString",
    };
    drawableObjects_.clear();
    drawableObjects_.resize(countof(functionNames));

    for (size_t i = 0; i < countof(functionNames); ++i)
    {
        auto& drawableObject = drawableObjects_[i];
        drawableObject.Set(DrawableObjectAttributeFunction, functionNames[i]);
        drawableObject.Set(DrawableObjectAttributeText, u"This is a test.");
        drawableObject.Set(DrawableObjectAttributeFontFamily, u"Segoe UI");
        drawableObject.Set(DrawableObjectAttributeFontSize, u"18");
        drawableObject.Update();
    }

    Edit_LimitText(GetWindowFromId(hwnd_, IdcLog), 1048576);

    // Subclass the values edit box for a few reasons.
    // - Forward up/down arrow key presses to the listview under it.
    // - Dynamically change between multiline and single-line behavior for return keypress.
    // - Display the cue banner text because cue banners do not work correctly for ES_MULTILINE.
    auto attributeValuesEdit = GetWindowFromId(hwnd_, IdcAttributeValuesEdit);
    SetWindowSubclass(attributeValuesEdit, &AttributeValueEditProcedure, 0, reinterpret_cast<DWORD_PTR>(this));
    SetWindowStyle(attributeValuesEdit, GetWindowStyle(attributeValuesEdit) & ~ES_MULTILINE);

    // Not only do multiline edit controls fail to display the cue banner text,
    // but they also ignore the Edit_SetCueBannerText call, meaning we can't
    // just call GetCueBannerText in the subclassed function. So store it as
    // a window property instead.
    SetProp(attributeValuesEdit, L"CueBannerText", L"<no attributes selected>");

    auto attributesEdit = GetWindowFromId(hwnd_, IdcAttributesFilterEdit);
    Edit_SetCueBannerText(attributesEdit, L"<type attribute filter here>");

    ChangeSettingsVisibility(settingsVisibility_);
    InitializeDrawableObjectsListView();
    InitializeAttributesListView();
    InitializeAttributeValuesListView();
    Button_SetCheck(GetWindowFromId(hwnd_, settingsVisibility_ + IdcSettingsFirst), true);

    Resize(IddMainWindow);

    UpdateDrawableObjectsListView();
    UpdateAttributesListView();
    UpdateAttributeValuesListView();

    AppendLog(u"Mouse wheel scrolls, middle button drag pans, right click opens menu, left click selects object. "
              u"You can select multiple objects in the list to change an attribute across all.\r\n"
              u"Double click cell to edit. Press delete key to delete selected object. Press delete in attribute list to clear value.\r\n"
              );

    uint32_t controlIdToSetFocusTo = 0;
    static_assert(SettingsVisibilityTotal == 3, "");
    switch (settingsVisibility_)
    {
    case SettingsVisibilityHidden: controlIdToSetFocusTo = IdcDrawingCanvas; break;
    case SettingsVisibilityLight:  controlIdToSetFocusTo = IdcEditText; break;
    case SettingsVisibilityFull:   controlIdToSetFocusTo = IdcDrawableObjectsList; break;
    }
    SetFocus(GetWindowFromId(hwnd_, controlIdToSetFocusTo));

#if 0
    SetTimer(hwnd_, IdcUpdateUi + 1, 10, nullptr);
#endif

    return false; // do not permit user32 code to set focus
}


namespace
{
    struct ListViewColumnInfo
    {
        int attributeType;
        int width;
        const char16_t* name;
    };

    void InitializeSysListView(
        HWND hwnd,
        __in_ecount(columnInfoCount) const ListViewColumnInfo* columnInfo,
        size_t columnInfoCount
        )
    {
        // Columns
        LVCOLUMN lc;
        lc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

        for (size_t i = 0; i < columnInfoCount; ++i)
        {
            lc.iSubItem = columnInfo[i].attributeType;
            lc.cx = columnInfo[i].width;
            lc.pszText = (LPWSTR)columnInfo[i].name;
            ListView_InsertColumn(hwnd, lc.iSubItem, &lc);
        }
    }

    bool HandleSysListViewEmptyText(LPARAM lParam, _In_z_ char16_t const* text)
    {
        LV_DISPINFO& di = *(LV_DISPINFO*)lParam;
        if (di.hdr.code != LVN_GETEMPTYTEXT)
            return false;

        if (di.item.mask & LVIF_TEXT)
        {
            StringCchCopyW(
                di.item.pszText,
                di.item.cchTextMax,
                ToWChar(text)
                );
            return true;
        }
        return false;
    }
}


void MainWindow::InitializeDrawableObjectsListView()
{
    HWND listViewHwnd = GetWindowFromId(hwnd_, IdcDrawableObjectsList);
    ListView_SetExtendedListViewStyle(listViewHwnd, LVS_EX_LABELTIP | LVS_EX_DOUBLEBUFFER | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERINALLVIEWS);

    // Columns
    LVCOLUMN lc;
    lc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    for (auto const& attribute : DrawableObject::attributeList)
    {
        // Determine column width from the type and semantic, giving wider
        // columns to data arrays (including strings) and enumerations than
        // simple numeric values.
        lc.iSubItem = attribute.id;
        lc.cx = (attribute.IsTypeArray() || attribute.id == 0) ? 120 : 80;
        lc.pszText = const_cast<LPWSTR>(ToWChar(attribute.display));
        ListView_InsertColumn(listViewHwnd, lc.iSubItem, &lc);
    }
}


void MainWindow::InitializeAttributesListView()
{
    const static ListViewColumnInfo columnInfo[] = {
        { 0, 120, u"Attribute" },
        { 1, 400, u"Value" },
    };
    auto listViewHwnd = GetWindowFromId(hwnd_, IdcAttributesList);
    ListView_SetExtendedListViewStyle(listViewHwnd, LVS_EX_LABELTIP | LVS_EX_DOUBLEBUFFER | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERINALLVIEWS);
    InitializeSysListView(listViewHwnd, columnInfo, ARRAYSIZE(columnInfo));
    FillAttributesListView();
}


void MainWindow::FillAttributesListView()
{
    // Add items to the list, respecting any filters.

    ListViewWriter lw(GetWindowFromId(hwnd_, IdcAttributesList));

    isRecursing_ = true;
    lw.DisableDrawing();

    lw.DeleteAllItems();
    lw.mask |= LVIF_PARAM;

    std::vector<uint32_t> listIndices(countof(DrawableObject::attributeList));
    ListSubstringPrioritizer substringPrioritizer(attributeFilter_, static_cast<uint32_t>(countof(DrawableObject::attributeList)));

    // Get the filtered list.
    for (uint32_t i = 0; i < countof(DrawableObject::attributeList); ++i)
    {
        auto& attribute = DrawableObject::attributeList[i];
        auto weight = substringPrioritizer.GetStringWeight(ToChar16ArrayRef(attribute.display));
        substringPrioritizer.SetItemWeight(i, weight);
    }
    auto clampedListIndices = substringPrioritizer.GetItemIndices(OUT listIndices, /*excludeMismatches*/true);

    // Add matching items to the ListView.
    for (auto& index : clampedListIndices)
    {
        auto& attribute = DrawableObject::attributeList[index];
        lw.lParam = attribute.id;
        lw.InsertItem(attribute.display);
        lw.AdvanceItem();
    }

    lw.EnableDrawing();
    isRecursing_ = false;
}


void MainWindow::InitializeAttributeValuesListView()
{
    const static ListViewColumnInfo columnInfo[] = {
        { 0, 120, u"Name" },
        { 1, 400, u"Value" },
    };
    auto listViewHwnd = GetWindowFromId(hwnd_, IdcAttributeValuesList);
    ListView_SetExtendedListViewStyle(listViewHwnd, LVS_EX_LABELTIP | LVS_EX_DOUBLEBUFFER | LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERINALLVIEWS);
    InitializeSysListView(listViewHwnd, columnInfo, ARRAYSIZE(columnInfo));
}


void MainWindow::UpdateDrawableObjectsListView()
{
    std::u16string truncatedString;
    ListViewWriter lw(GetWindowFromId(hwnd_, IdcDrawableObjectsList));
    lw.DisableDrawing();

    size_t itemCount = lw.GetItemCount();
    size_t const totalDrawableObjects = drawableObjects_.size();
    if (itemCount > totalDrawableObjects)
    {
        lw.DeleteAllItems();
        itemCount = 0;
    }
    lw.ReserveItemCount(int(totalDrawableObjects));

    for (auto const& drawableObject : drawableObjects_)
    {
        if (lw.iItem >= int(itemCount))
        {
            isRecursing_ = true; // Stop pointless LVN_ITEMCHANGED messages.
            lw.InsertItem();
            isRecursing_ = false;
            ++itemCount;
        }
    
        for (uint32_t i = 0; i < DrawableObjectAttributeTotal; ++i)
        {
            auto* text = &drawableObject.values_[i].stringValue;
            if (text->size() > 256)
            {
                truncatedString.assign(text->c_str(), 256);
                text = &truncatedString;
            }
            isRecursing_ = true; // Stop pointless LVN_ITEMCHANGED messages.
            lw.SetItemText(i, text->c_str());
            isRecursing_ = false;
        }
        lw.AdvanceItem();
    }
    lw.EnableDrawing();
    InvalidateRect(lw.hwnd, nullptr, false);
}


void MainWindow::DeferUpdateUi(NeededUiUpdate neededUiUpdate)
{
    neededUiUpdate_ |= neededUiUpdate;
    SetTimer(hwnd_, IdcUpdateUi, 50, nullptr);
}


void MainWindow::UpdateUi()
{
    auto neededUiUpdate = neededUiUpdate_;
    neededUiUpdate_ = NeededUiUpdateNone;

    if (neededUiUpdate & NeededUiUpdateDrawableObjectsListView)
    {
        UpdateDrawableObjectsListView();
    }
    if (neededUiUpdate & NeededUiFillAttributesListView)
    {
        FillAttributesListView();
    }
    if (neededUiUpdate & NeededUiUpdateAttributesListView)
    {
        UpdateAttributesListView();
    }
    if (neededUiUpdate & NeededUiUpdateAttributeValuesEdit)
    {
        UpdateAttributeValuesEdit();
    }
    if (neededUiUpdate & NeededUiUpdateAttributeValuesSlider)
    {
        UpdateAttributeValuesSlider();
    }
    if (neededUiUpdate & NeededUiUpdateAttributeValuesListView)
    {
        UpdateAttributeValuesListView();
    }
    if (neededUiUpdate & NeededUiUpdateDrawableObjectsCanvas)
    {
        RepaintDrawableObjects();
    }
    if (neededUiUpdate & NeededUiUpdateTextEdit)
    {
        UpdateTextEdit();
    }
}


void MainWindow::UpdateAttributesListView()
{
    // Update the text in the value column. This expects the items and attribute labels are already filled.

    ListViewWriter lw(GetWindowFromId(hwnd_, IdcAttributesList));

    std::vector<uint32_t> selectedDrawableObjectIndices = GetSelectedDrawableObjectIndices();

    // Get all the attribute indices.
    uint32_t attributeIndicesBuffer[DrawableObjectAttributeTotal];
    int attributeListItemCount = ListView_GetItemCount(lw.hwnd);
    array_ref<uint32_t> attributeIndices(attributeIndicesBuffer, std::min(countof(attributeIndicesBuffer), size_t(attributeListItemCount)));
    std::iota(OUT attributeIndices.begin(), OUT attributeIndices.end(), 0);
    ListViewRemapIndicesToLparam(lw.hwnd, IN OUT attributeIndices);

    lw.DisableDrawing();
    size_t const totalDrawableObjects = drawableObjects_.size();
    for (auto attributeIndex : attributeIndices)
    {
        char16_t const* stringValue = DrawableObjectAndValues::GetStringValue(
                                        drawableObjects_,
                                        selectedDrawableObjectIndices,
                                        attributeIndex,
                                        u"<mixed values>"
                                        );

        isRecursing_ = true; // Stop pointless LVN_ITEMCHANGED messages.
        lw.SetItemText(/*column*/1, stringValue);
        isRecursing_ = false;
        lw.AdvanceItem();
    }
    lw.EnableDrawing();

    InvalidateRect(lw.hwnd, nullptr, false);
}


void MainWindow::UpdateAttributeValuesListView()
{
    ListViewWriter lw(GetWindowFromId(hwnd_, IdcAttributeValuesList));
    lw.DisableDrawing();
    lw.DeleteAllItems();

    std::u16string stringValueBuffer;

    // Nop if selectedAttributeIndex_ invalid (no attribute selected).
    if (selectedAttributeIndex_ < DrawableObjectAttributeTotal)
    {
        Attribute const& attribute = DrawableObject::attributeList[selectedAttributeIndex_];
        uint32_t const predefinedValuesCount = static_cast<uint32_t>(attribute.predefinedValues.size());
        std::vector<uint32_t> orderedIndices(predefinedValuesCount);

        if (attribute.semantic == Attribute::SemanticLongText || !isTypingAttributeValueToFilter_)
        {
            // Just include all the values in order.
            std::iota(orderedIndices.begin(), orderedIndices.end(), 0);
        }
        else
        {
            // For single line, reorder the list by best match typed so far
            // (if recently editing the value).
            attribute.GetPredefinedValueIndices(selectedAttributeValue_.c_str(), OUT orderedIndices);
        }
        isTypingAttributeValueToFilter_ = false; // Reset once used.

        int selectedItem = -1; // Keep track of which value to select, if any.

        for (uint32_t i = 0; i < predefinedValuesCount; ++i)
        {
            // Get the name and value for each column.
            uint32_t valueIndex = orderedIndices[i];
            auto& valueMapping = attribute.predefinedValues[valueIndex];
            auto* name = attribute.GetPredefinedValueName(valueIndex);
            auto* stringValue = attribute.GetPredefinedValue(
                valueIndex,
                attribute.GetPredefinedValueFlagsString | attribute.GetPredefinedValueFlagsInteger,
                OUT stringValueBuffer
                );

            // Add entry.
            lw.lParam = valueIndex;
            lw.mask |= LVIF_PARAM; // Use the lparam to keep track of attribute value index, regardless of order.
            lw.InsertItem(name != nullptr ? name : u"");
            lw.mask &= ~LVIF_PARAM;
            if (stringValue != nullptr)
            {
                lw.SetItemText(/*column*/1, stringValue);
            }
            lw.AdvanceItem();

            // Keep track of the selection if the strings exactly matched.
            if (!selectedAttributeValue_.empty())
            {
                if (_wcsicmp(ToWChar(name), ToWChar(selectedAttributeValue_.c_str())) == 0
                || (stringValue != nullptr && _wcsicmp(ToWChar(stringValue), ToWChar(selectedAttributeValue_.c_str())) == 0))
                {
                    selectedItem = i;
                }
            }
        }

        if (selectedItem != -1)
        {
            isRecursing_ = true;
            lw.SelectAndFocusItem(selectedItem);
            lw.iItem = selectedItem;
            lw.EnsureVisible();
            isRecursing_ = false;
        }
    }

    lw.EnableDrawing();
    InvalidateRect(lw.hwnd, nullptr, false);
}


void MainWindow::UpdateAttributeValuesEdit()
{
    char16_t const* stringValue = u"";
    char16_t const* cueBannerText = u"<no attributes selected>";
    bool hasMultilineText = false;

    std::vector<uint32_t> attributeIndices = GetSelectedAttributeIndices();
    std::vector<uint32_t> drawableObjectIndices = GetSelectedDrawableObjectIndices();

    // Set the edit control's cue banner text based on which attributes are selected.
    if (!attributeIndices.empty())
    {
        cueBannerText = u"<multiple attributes selected>";
        if (attributeIndices.size() == 1 && selectedAttributeIndex_ < DrawableObjectAttributeTotal)
        {
            auto* text = DrawableObject::attributeList[selectedAttributeIndex_].description;
            if (text == nullptr) text = DrawableObject::attributeList[selectedAttributeIndex_].display;
            if (text != nullptr) cueBannerText = text;
        }
    }

    // Check if any text boxes are multi-line.
    for (auto index : attributeIndices)
    {
        ThrowIf(index >= countof(DrawableObject::attributeList), "Index is beyond DrawableObject::attributeList size!");
        if (DrawableObject::attributeList[index].semantic == Attribute::SemanticLongText)
        {
            hasMultilineText = true;
        }
    }

    // Get the right string for the edit based on which attribute and drawable
    // objects are selected. If mixed values are set, leave empty.
    bool isFirstItem = true;
    for (auto index : attributeIndices)
    {
        // Check if any text boxes are multi-line.
        char16_t const* currentStringValue = DrawableObjectAndValues::GetStringValue(
            drawableObjects_,
            drawableObjectIndices,
            index,
            u"" // Empty string if mixed settings between options. Display cue banner text instead.
            );

        if (isFirstItem)
        {
            stringValue = currentStringValue;
            isFirstItem = false;
        }
        else if (wcscmp(ToWChar(currentStringValue), ToWChar(stringValue)) != 0)
        {
            stringValue = u"";
            break;
        }
    }

    selectedAttributeValue_.assign(stringValue);

    // Update the edit's text, style, and size depending on whether multiline.
    // Note that ES_MULTILINE doesn't actually seem to have any effect after
    // creation, but it still sets the flag anyway, which allows other code
    // to inspect the flag and relay keypresses accordingly.
    {
        HWND attributeValueEdit = GetWindowFromId(hwnd_, IdcAttributeValuesEdit);

        // Update the cue banner text according to the attribute.
        SetProp(attributeValueEdit, L"CueBannerText", reinterpret_cast<HANDLE>(const_cast<char16_t*>(cueBannerText)));

        // Update the new size.
        RECT oldWindowRect;
        RECT newWindowRect = { 0,0,172,hasMultilineText ? 48 : 12};
        MapDialogRect(hwnd_, IN OUT &newWindowRect);
        GetWindowRect(attributeValueEdit, OUT &oldWindowRect);

        // Update the text and style, for scroll bars and return key presses.
        isRecursing_ = true;
        SetWindowText(attributeValueEdit, ToWChar(stringValue));
        DWORD oldStyle = GetWindowStyle(attributeValueEdit);
        DWORD newStyle = oldStyle & ~(ES_MULTILINE | ES_WANTRETURN | WS_VSCROLL);
        if (hasMultilineText)
            newStyle |= ES_MULTILINE | ES_WANTRETURN | WS_VSCROLL;
        SetWindowStyle(attributeValueEdit, newStyle);
        isRecursing_ = false;

        // If style change, then reflow the main window.
        if (((newStyle ^ oldStyle) & ES_MULTILINE) == ES_MULTILINE)
        {
            Resize(IddMainWindow);
        }
    }
}


void MainWindow::UpdateTextEdit()
{
    std::vector<uint32_t> drawableObjectIndices(drawableObjects_.size());
    std::iota(OUT drawableObjectIndices.begin(), OUT drawableObjectIndices.end(), 0);

    char16_t const* stringValue = DrawableObjectAndValues::GetStringValue(
                                        drawableObjects_,
                                        drawableObjectIndices,
                                        DrawableObjectAttributeText,
                                        u""
                                        );

    SetWindowText(GetWindowFromId(hwnd_, IdcEditText), ToWChar(stringValue));
}


void MainWindow::UpdateAttributeValuesSlider()
{
#if 0 // todo: finish
    auto slider = GetWindowFromId(hwnd_, IdcAttributeValueSlider);
    if (selectedAttributeIndex_ < DrawableObjectAttributeTotal)
    {
        auto& attribute = DrawableObject::attributeList[selectedAttributeIndex_];
        if (attribute.IsTypeNumeric(attribute.type)
        &&  !attribute.IsTypeArray(attribute.type)
        &&  attribute.semantic != Attribute::SemanticEnum
        &&  attribute.semantic != Attribute::SemanticEnumExclusive
            )
        {
            // todo: respond to TB_ENDTRACK
            SendMessage(slider, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, 20));  // min. & max. positions
            SendMessage(slider, TBM_SETPAGESIZE, 0, (LPARAM)4); // new page size 
            SendMessage(slider, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)7); 
        }
    }
#endif
}


void MainWindow::RepaintDrawableObjects()
{
    InvalidateRect(GetWindowFromId(hwnd_, IdcDrawingCanvas), nullptr, false);
}


void MainWindow::DeleteDrawableObjectsListViewSelected()
{
    std::vector<uint32_t> drawableObjectIndices = GetListViewMatchingIndices(
        GetWindowFromId(hwnd_, IdcDrawableObjectsList),
        LVNI_SELECTED,
        /*returnAllIfNoMatch*/false
        );

    if (drawableObjectIndices.empty())
    {
        ShowMessageAndAppendLog(u"Select at least one drawable object in the list first.");
        return;
    }

    if (drawableObjectIndices.empty())
        return;

    auto drawableObjectsTotal = drawableObjects_.size();
    // Erase from back to front.
    for (size_t i = drawableObjectIndices.size(); i > 0; --i)
    {
        auto index = drawableObjectIndices[i - 1];
        if (index >= drawableObjectsTotal)
            continue;

        drawableObjects_.erase(drawableObjects_.begin() + index);
    }

    DeferUpdateUi(
        NeededUiUpdateDrawableObjectsListView |
        NeededUiUpdateAttributesListView |
        NeededUiUpdateAttributeValuesListView |
        NeededUiUpdateAttributeValuesEdit |
        NeededUiUpdateAttributeValuesSlider |
        NeededUiUpdateDrawableObjectsCanvas
        );
}


namespace
{
    void InitializeDefaultDrawableObjectAndValues(DrawableObjectAndValues& drawableObjectAndValues)
    {
        drawableObjectAndValues.Set(DrawableObjectAttributeFunction, u"IDWriteBitmapRenderTarget IDWriteTextLayout");
        drawableObjectAndValues.Set(DrawableObjectAttributeText, u"This is a test.");
        drawableObjectAndValues.Set(DrawableObjectAttributeFontFamily, u"Segoe UI");
        drawableObjectAndValues.Set(DrawableObjectAttributeFontSize, u"18");
        drawableObjectAndValues.Update();
    }
}


void MainWindow::CreateDrawableObjectsListViewSelected()
{
    auto drawableObjectsListView = GetWindowFromId(hwnd_, IdcDrawableObjectsList);
    auto attributeValuesListView = GetWindowFromId(hwnd_, IdcAttributeValuesList);
    std::vector<uint32_t> drawableObjectIndices = GetListViewMatchingIndices(drawableObjectsListView, LVNI_SELECTED, /*returnAllIfNoMatch*/false);
    std::vector<uint32_t> attributeValueIndices = GetListViewMatchingIndices(attributeValuesListView, LVNI_SELECTED, /*returnAllIfNoMatch*/false);

    // Create a default object or duplicate a selected one.
    size_t const originalDrawableObjectsCount = drawableObjects_.size();
    if (originalDrawableObjectsCount == 0)
    {
        // Initialize new default object because there are no existing ones.
        drawableObjects_.resize(1);
        InitializeDefaultDrawableObjectAndValues(drawableObjects_.front());
    }
    else
    {
        // Duplicate the last object.
        if (drawableObjectIndices.empty())
            drawableObjectIndices.push_back(uint32_t(originalDrawableObjectsCount - 1));

        // Copy objects from existing selections.
        size_t repeatCount = std::max(attributeValueIndices.size(), size_t(1));
        for (size_t i = 0; i < repeatCount; ++i)
        {
            for (auto drawableObjectIndex : drawableObjectIndices)
            {
                if (drawableObjectIndex >= originalDrawableObjectsCount)
                    continue;

                drawableObjects_.push_back(drawableObjects_[drawableObjectIndex]);
            }
        }
    }

    size_t const newDrawableObjectsCount = drawableObjects_.size();

    // Create permutations based on selected attributes.
    if (attributeValueIndices.size() > 1 && selectedAttributeIndex_ < countof(DrawableObject::attributeList))
    {
        // Remap listview indices to attribute value indices, using lparam.
        ListViewRemapIndicesToLparam(attributeValuesListView, IN OUT attributeValueIndices);

        // Apply attribute all value permutations, for each attribute, and each new object.
        auto const& attribute = DrawableObject::attributeList[selectedAttributeIndex_];
        uint32_t const predefinedValuesCount = static_cast<uint32_t>(attribute.predefinedValues.size());
        std::u16string stringValueBuffer;
        size_t drawableObjectIndex = originalDrawableObjectsCount;

        for (auto& attributeValueIndex : attributeValueIndices)
        {
            if (attributeValueIndex >= predefinedValuesCount)
                continue;

            // Read the attribute string value.
            auto* stringValue = attribute.GetPredefinedValue(
                attributeValueIndex,
                attribute.GetPredefinedValueFlagsString | attribute.GetPredefinedValueFlagsName | attribute.GetPredefinedValueFlagsInteger,
                OUT stringValueBuffer
            );

            // And set that attribute value onto all the drawable objects
            size_t drawableObjectIndexEnd = drawableObjectIndex + drawableObjectIndices.size();
            for (; drawableObjectIndex < drawableObjectIndexEnd; ++drawableObjectIndex)
            {
                drawableObjects_[drawableObjectIndex].Set(selectedAttributeIndex_, stringValue);
            }
        }
    }

    // Update all the copied objects, since attributes have been changed.
    for (auto& drawableObject : make_iterator_range(drawableObjects_.data(), originalDrawableObjectsCount, newDrawableObjectsCount))
    {
        drawableObject.Invalidate();
        drawableObject.Update();
    }

    DeferUpdateUi(
        NeededUiUpdateDrawableObjectsListView |
        NeededUiUpdateAttributesListView |
        NeededUiUpdateAttributeValuesListView |
        NeededUiUpdateAttributeValuesEdit |
        NeededUiUpdateAttributeValuesSlider |
        NeededUiUpdateDrawableObjectsCanvas
    );
}


void MainWindow::ShiftSelectedDrawableObjects(int32_t shiftDirection /* down = positive, up = negative */)
{
    if (shiftDirection == 0)
        return; // Nop

    auto drawableObjectsListView = GetWindowFromId(hwnd_, IdcDrawableObjectsList);
    size_t const drawableObjectsCount = drawableObjects_.size();
    std::vector<uint32_t> drawableObjectIndices = GetListViewMatchingIndices(drawableObjectsListView, LVNI_SELECTED, /*returnAllIfNoMatch*/false);
    if (drawableObjectIndices.empty())
        return; // Nop

    int32_t iDelta = (/*if shifting up*/ shiftDirection < 0) ? /*swap down*/1 : /*swap up*/ -1;
    uint32_t begin = 0, end = drawableObjectIndices.size(); // if shifting up

    // Check for selected lines already being at either end, thus unabled to shift further.
    if (shiftDirection > 0)
    {
        // Shifting selected lines down, progressing up.
        begin = end - 1;
        end = 0xFFFFFFFF;
        if (drawableObjectIndices.back() == drawableObjectsCount - 1)
            return;
    }
    else // shiftDirection < 0
    {
        // Shifting selected lines up, progressing down.
        if (drawableObjectIndices.front() == 0)
            return;
    }

    for (uint32_t i = begin; i != end; i += iDelta)
    {
        auto drawableObjectIndex = drawableObjectIndices[i];
        if (drawableObjectIndex >= drawableObjectsCount)
            continue;
        std::swap(drawableObjects_[drawableObjectIndex], drawableObjects_[drawableObjectIndex - iDelta]);
        //drawableObject.Invalidate();
        //drawableObject.Update();
    }


    // Update all the copied objects, since attributes have been changed.
    // todo:delete
    //for (auto& drawableObject : make_iterator_range(drawableObjects_.data(), originalDrawableObjectsCount, newDrawableObjectsCount))
    //{
    //
    //}

    DeferUpdateUi(
        NeededUiUpdateDrawableObjectsListView |
        NeededUiUpdateAttributesListView |
        NeededUiUpdateAttributeValuesListView |
        NeededUiUpdateAttributeValuesEdit |
        NeededUiUpdateAttributeValuesSlider |
        NeededUiUpdateDrawableObjectsCanvas
    );
}


MainWindow::DialogProcResult CALLBACK MainWindow::OnDragAndDrop(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    std::u16string fileName;
    HDROP hDrop = (HDROP)wParam;
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);

    // Get the number of files being dropped.
    const uint32_t fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, nullptr, 0);

    // Accumulate all the filenames into a list to load.
    bool clearExistingItems = true;
    for (uint32_t i = 0; i < fileCount; ++i)
    {
        const uint32_t fileNameSize = DragQueryFile(hDrop, i, nullptr, 0);
        if (fileNameSize == 0)
            continue;

        fileName.resize(fileNameSize);
        array_ref<char16_t> fileNameRef(fileName);
        if (DragQueryFile(hDrop, i, ToWChar(fileNameRef.data()), fileNameSize + 1))
        {
            auto* filenameExtension = FindFileNameExtension(fileName);

            if (_wcsicmp(ToWChar(filenameExtension), L"TextLayoutSamplerSettings") == 0)
            {
                hr = LoadDrawableObjectsSettings(fileName.data(), clearExistingItems, /*merge*/false);
            }
            else if (_wcsicmp(ToWChar(filenameExtension), L"txt") == 0)
            {
                hr = LoadTextFileIntoDrawableObjects(fileName.data());
            }
            else if (_wcsicmp(ToWChar(filenameExtension), L"ttf") == 0
                ||   _wcsicmp(ToWChar(filenameExtension), L"otf") == 0
                ||   _wcsicmp(ToWChar(filenameExtension), L"tte") == 0
                ||   _wcsicmp(ToWChar(filenameExtension), L"ttc") == 0
                ||   _wcsicmp(ToWChar(filenameExtension), L"otc") == 0)
            {
                hr = LoadFontFileIntoDrawableObjects(fileName.data());
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
            }

            if (FAILED(hr))
                break;

            clearExistingItems = false;
        }
    }
    DragFinish(hDrop);

    if (hr == HRESULT_FROM_WIN32(ERROR_BAD_FORMAT))
        ShowMessageAndAppendLog(u"Unknown file format '%s', 0x%08X", fileName.c_str(), hr);
    else if (FAILED(hr))
        ShowMessageAndAppendLog(u"Failed to load file '%s', 0x%08X", fileName.c_str(), hr);

    return DialogProcResult(/*handled*/ true, /*value*/ 0);
}


std::vector<uint32_t> MainWindow::GetSelectedDrawableObjectIndices()
{
    return GetListViewMatchingIndices(GetWindowFromId(hwnd_, IdcDrawableObjectsList), LVNI_SELECTED,  /*returnAllIfNoMatch*/true);
}


std::vector<uint32_t> MainWindow::GetSelectedAttributeIndices()
{
    // The user can select multiple attributes via control+click.
    // If none are selected, use the previous selected attribute,
    // which can occur if a filter hides the attributes.
    auto attributeList = GetWindowFromId(hwnd_, IdcAttributesList);

    std::vector<uint32_t> attributeIndices = GetListViewMatchingIndices(attributeList, LVNI_SELECTED, /*returnAllIfNoMatch*/false);
    std::vector<uint32_t> drawableObjectIndices = GetSelectedDrawableObjectIndices();
    ListViewRemapIndicesToLparam(attributeList, IN OUT attributeIndices);

    if (attributeIndices.empty() && selectedAttributeIndex_ < DrawableObjectAttributeTotal)
        attributeIndices.push_back(selectedAttributeIndex_);

    return attributeIndices;
}


HRESULT MainWindow::GetLogFontFromDrawableObjects(_Out_ LOGFONT& logFont)
{
    std::vector<uint32_t> selectedDrawableObjectIndices = GetSelectedDrawableObjectIndices();
    auto const drawableObjectsTotal = drawableObjects_.size();
    uint32_t drawableObjectIndex = selectedDrawableObjectIndices.empty() ? ~0 : selectedDrawableObjectIndices[0];

    DWRITE_FONT_WEIGHT fontWeight = DWRITE_FONT_WEIGHT_NORMAL;
    DWRITE_FONT_STYLE fontSlope = DWRITE_FONT_STYLE_NORMAL;
    DWRITE_FONT_STRETCH fontStretch = DWRITE_FONT_STRETCH_NORMAL;
    float fontSize = DrawableObject::defaultFontSize;
    bool hasUnderline = false;
    bool hasStrikethrough = false;
    char16_t defaultFamilyName[] = u"Segoe UI";
    array_ref<char16_t const> familyName = defaultFamilyName;
    std::u16string gdiFamilyName;
    bool isGdiOrGdiPlusFunction = false;

    // If more than one object is selected, just pick the first one.
    if (drawableObjectIndex < drawableObjectsTotal)
    {
        auto& drawableObject = drawableObjects_[drawableObjectIndex];
        familyName = drawableObject.GetString(DrawableObjectAttributeFontFamily);
        fontSize = drawableObject.GetValue(DrawableObjectAttributeFontSize, DrawableObject::defaultFontSize);
        hasUnderline = drawableObject.GetValue(DrawableObjectAttributeUnderline, false);
        hasStrikethrough = drawableObject.GetValue(DrawableObjectAttributeStrikethrough, false);
        fontWeight = drawableObject.GetValue(DrawableObjectAttributeWeight, DWRITE_FONT_WEIGHT_NORMAL);
        fontStretch = drawableObject.GetValue(DrawableObjectAttributeStretch, DWRITE_FONT_STRETCH_NORMAL);
        fontSlope = drawableObject.GetValue(DrawableObjectAttributeSlope, DWRITE_FONT_STYLE_NORMAL);
        DrawableObjectFunction function = drawableObject.GetValue(DrawableObjectAttributeFunction, DrawableObjectFunctionNop);
        isGdiOrGdiPlusFunction = DrawableObject::IsGdiOrGdiPlusFunction(function);
    }

    logFont.lfHeight            = -static_cast<LONG>(floor(fontSize + 0.5f));
    logFont.lfWidth             = 0;
    logFont.lfEscapement        = 0;
    logFont.lfOrientation       = 0;
    logFont.lfWeight            = fontWeight;
    logFont.lfItalic            = (fontSlope > DWRITE_FONT_STYLE_NORMAL);
    logFont.lfUnderline         = static_cast<BYTE>(hasUnderline);
    logFont.lfStrikeOut         = static_cast<BYTE>(hasStrikethrough);
    logFont.lfCharSet           = DEFAULT_CHARSET;
    logFont.lfOutPrecision      = OUT_DEFAULT_PRECIS;
    logFont.lfClipPrecision     = CLIP_DEFAULT_PRECIS;
    logFont.lfQuality           = DEFAULT_QUALITY;
    logFont.lfPitchAndFamily    = DEFAULT_PITCH;

    // Map to the GDI equivalent values if they were in the DWrite model.
    if (!isGdiOrGdiPlusFunction)
    {
        // todo: Move much of this into DWritEx.cpp.
        ComPtr<IDWriteFactory> dwriteFactory;
        ComPtr<IDWriteGdiInterop> gdiInterop;
        ComPtr<IDWriteFontFace> fontFace;
        ComPtr<IDWriteFontFace3> fontFace3;
        ComPtr<IDWriteLocalizedStrings> localizedStrings;
        ComPtr<IDWriteFontCollection> fontCollection;

        IFR(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(OUT &dwriteFactory)));
        IFR(dwriteFactory->GetSystemFontCollection(OUT &fontCollection));
        IFR(dwriteFactory->GetGdiInterop(OUT &gdiInterop));
        CreateFontFace(
            fontCollection,
            ToWChar(familyName.data()),
            fontWeight,
            fontStretch,
            fontSlope,
            OUT &fontFace
            );

        if (fontFace != nullptr)
        {
            LOGFONT convertedLogFont = {};

            // Get the weight and slope.
            gdiInterop->ConvertFontFaceToLOGFONT(fontFace, &convertedLogFont);
            logFont.lfWeight = DWRITE_FONT_WEIGHT(convertedLogFont.lfWeight);
            logFont.lfItalic = DWRITE_FONT_STYLE(convertedLogFont.lfItalic);

            // Get the family name.
            fontFace->QueryInterface(OUT &fontFace3);
            if (fontFace3 != nullptr)
            {
                BOOL exists = false;
                fontFace3->GetInformationalStrings(DWRITE_INFORMATIONAL_STRING_WIN32_FAMILY_NAMES, OUT &localizedStrings, OUT &exists);
                GetLocalizedString(localizedStrings, /*languageTag*/nullptr, OUT gdiFamilyName);
                familyName = gdiFamilyName;
            }
        }
    }

    wcsncpy_s(logFont.lfFaceName, ToWChar(familyName.data()), _TRUNCATE);

    return S_OK;
}


struct MainWindow::FontFamilyNameProperties
{
    DWRITE_FONT_WEIGHT gdiFontWeight = DWRITE_FONT_WEIGHT_NORMAL;
    DWRITE_FONT_STYLE gdiFontSlope = DWRITE_FONT_STYLE_NORMAL;
    DWRITE_FONT_STRETCH gdiFontStretch = DWRITE_FONT_STRETCH_NORMAL;
    DWRITE_FONT_WEIGHT dwriteFontWeight = DWRITE_FONT_WEIGHT_NORMAL;
    DWRITE_FONT_STYLE dwriteFontSlope = DWRITE_FONT_STYLE_NORMAL;
    DWRITE_FONT_STRETCH dwriteFontStretch = DWRITE_FONT_STRETCH_NORMAL;
    std::u16string dwriteFamilyName;
    std::u16string gdiFamilyName;
    std::u16string filePath;
    float fontSize;
};


HRESULT SetFontFamilyNameProperties(LOGFONT const& logFont, _Out_ MainWindow::FontFamilyNameProperties& fontFamilyNameProperties)
{
    // Map from the GDI simple naming model to DWrite's weight-width-slope.
    ComPtr<IDWriteFactory> dwriteFactory;
    ComPtr<IDWriteGdiInterop> gdiInterop;
    ComPtr<IDWriteFont> font;

    // todo: Move much of this into DWritEx.cpp.
    IFR(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(OUT &dwriteFactory)));
    IFR(dwriteFactory->GetGdiInterop(OUT &gdiInterop));
    gdiInterop->CreateFontFromLOGFONT(&logFont, OUT &font);

    fontFamilyNameProperties.fontSize = float(-logFont.lfHeight);
    fontFamilyNameProperties.gdiFontWeight = static_cast<DWRITE_FONT_WEIGHT>(logFont.lfWeight);
    fontFamilyNameProperties.gdiFontSlope = logFont.lfItalic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL;
    fontFamilyNameProperties.gdiFontStretch = DWRITE_FONT_STRETCH_NORMAL;
    fontFamilyNameProperties.dwriteFontWeight = fontFamilyNameProperties.gdiFontWeight;
    fontFamilyNameProperties.dwriteFontSlope = fontFamilyNameProperties.gdiFontSlope;
    fontFamilyNameProperties.dwriteFontStretch = fontFamilyNameProperties.gdiFontStretch;
    fontFamilyNameProperties.dwriteFamilyName.assign(ToChar16(logFont.lfFaceName));
    fontFamilyNameProperties.gdiFamilyName.assign(fontFamilyNameProperties.dwriteFamilyName);

    if (font != nullptr)
    {
        fontFamilyNameProperties.dwriteFontWeight = font->GetWeight();
        fontFamilyNameProperties.dwriteFontStretch = font->GetStretch();
        fontFamilyNameProperties.dwriteFontSlope = font->GetStyle();
        GetFontFamilyName(font, /*languageTag*/nullptr, OUT fontFamilyNameProperties.dwriteFamilyName);
    }

    return S_OK;
}


HRESULT SetFontFamilyNameProperties(
    IDWriteFont* font,
    _In_z_ char16_t const* filePath,
    _Out_ MainWindow::FontFamilyNameProperties& fontFamilyNameProperties
)
{
    LOGFONT logFont;
    ComPtr<IDWriteFactory> dwriteFactory;
    ComPtr<IDWriteGdiInterop> gdiInterop;
    BOOL dummyBool;

    // todo: Move much of this into DWritEx.cpp.
    IFR(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(OUT &dwriteFactory)));
    IFR(dwriteFactory->GetGdiInterop(OUT &gdiInterop));
    fontFamilyNameProperties.filePath = filePath;
    fontFamilyNameProperties.dwriteFontWeight = font->GetWeight();
    fontFamilyNameProperties.dwriteFontStretch = font->GetStretch();
    fontFamilyNameProperties.dwriteFontSlope = font->GetStyle();
    fontFamilyNameProperties.gdiFontWeight = fontFamilyNameProperties.dwriteFontWeight;
    fontFamilyNameProperties.gdiFontSlope = fontFamilyNameProperties.dwriteFontSlope;
    fontFamilyNameProperties.gdiFontStretch = fontFamilyNameProperties.dwriteFontStretch;

    GetFontFamilyName(font, /*languageTag*/nullptr, OUT fontFamilyNameProperties.dwriteFamilyName);
    fontFamilyNameProperties.gdiFamilyName.assign(fontFamilyNameProperties.dwriteFamilyName);

    if (SUCCEEDED(gdiInterop->ConvertFontToLOGFONT(font, OUT &logFont, OUT &dummyBool)))
    {
        fontFamilyNameProperties.gdiFamilyName.assign(ToChar16(logFont.lfFaceName));
        fontFamilyNameProperties.gdiFontWeight = static_cast<DWRITE_FONT_WEIGHT>(logFont.lfWeight);
        fontFamilyNameProperties.gdiFontSlope = logFont.lfItalic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL;
        fontFamilyNameProperties.gdiFontStretch = DWRITE_FONT_STRETCH_NORMAL;
    }

    return S_OK;
}


HRESULT MainWindow::UpdateDrawableObjectsFromFontFamilyNameProperties(FontFamilyNameProperties const& fontFamilyNameProperties)
{
    // Set the family name and face properties of every selected drawable object.
    std::vector<uint32_t> selectedDrawableObjectIndices = GetSelectedDrawableObjectIndices();
    std::wstring fontSizeString = std::to_wstring(fontFamilyNameProperties.fontSize);
    RemoveTrailingZeroes(IN OUT reinterpret_cast<std::u16string&>(fontSizeString));

    auto const drawableObjectsTotal = drawableObjects_.size();

    for (auto drawableObjectIndex : selectedDrawableObjectIndices)
    {
        if (drawableObjectIndex >= drawableObjectsTotal)
            continue;

        auto& drawableObject = drawableObjects_[drawableObjectIndex];
        DrawableObjectFunction function = drawableObject.GetValue(DrawableObjectAttributeFunction, DrawableObjectFunctionNop);
        bool isGdiOrGdiPlusFunction = DrawableObject::IsGdiOrGdiPlusFunction(function);

        drawableObject.Set(DrawableObjectAttributeFontFamily, isGdiOrGdiPlusFunction ? fontFamilyNameProperties.gdiFamilyName.c_str() : fontFamilyNameProperties.dwriteFamilyName.c_str());
        drawableObject.Set(DrawableObjectAttributeWeight, isGdiOrGdiPlusFunction ? fontFamilyNameProperties.gdiFontWeight : fontFamilyNameProperties.dwriteFontWeight);
        drawableObject.Set(DrawableObjectAttributeSlope, isGdiOrGdiPlusFunction ? fontFamilyNameProperties.gdiFontSlope : fontFamilyNameProperties.dwriteFontSlope);
        drawableObject.Set(DrawableObjectAttributeStretch, isGdiOrGdiPlusFunction ? fontFamilyNameProperties.gdiFontStretch : fontFamilyNameProperties.dwriteFontStretch);
        drawableObject.Set(DrawableObjectAttributeFontFilePath, fontFamilyNameProperties.filePath.c_str());

        if (fontFamilyNameProperties.fontSize > 0)
            drawableObject.Set(DrawableObjectAttributeFontSize, ToChar16(fontSizeString.data()));
    }

    DrawableObjectAndValues::Update(drawableObjects_, selectedDrawableObjectIndices);

    DeferUpdateUi(
        NeededUiUpdateDrawableObjectsListView |
        NeededUiUpdateAttributesListView |
        NeededUiUpdateAttributeValuesListView |
        NeededUiUpdateAttributeValuesEdit |
        NeededUiUpdateAttributeValuesSlider |
        NeededUiUpdateDrawableObjectsCanvas
        );

    return S_OK;
}


MainWindow* g_chooseFontMainWindow = nullptr;


UINT_PTR CALLBACK ChooseFontHookProc(
    HWND dialogHandle,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    // Save callback data, which is our window class.
    if (message == WM_INITDIALOG)
    {
        CHOOSEFONT* chooseFont = reinterpret_cast<CHOOSEFONT*>(lParam);
        g_chooseFontMainWindow = reinterpret_cast<MainWindow*>(chooseFont->lCustData);
        return true;
    }

    if (message == WM_COMMAND && HIWORD(wParam) == BN_CLICKED)
    {
        // Check for Ok or Apply.
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == WM_USER + 2 /*after WM_CHOOSEFONT_GETLOGFONT*/)
        {
            LOGFONT logFont = {};
            MainWindow::FontFamilyNameProperties fontFamilyNameProperties = {};

            SendMessage(dialogHandle, WM_CHOOSEFONT_GETLOGFONT, 0, OUT (LPARAM)&logFont);
            if (g_chooseFontMainWindow != nullptr)
            {
                SetFontFamilyNameProperties(logFont, OUT fontFamilyNameProperties);
                g_chooseFontMainWindow->UpdateDrawableObjectsFromFontFamilyNameProperties(fontFamilyNameProperties);
            }
        }
    }

    return 0;
}


HRESULT MainWindow::SelectFontFamily()
{
    LOGFONT logFont = {};
    GetLogFontFromDrawableObjects(OUT logFont);

    //////////////////////////////
    // Initialize CHOOSEFONT for the dialog.

    #ifndef CF_INACTIVEFONTS
    #define CF_INACTIVEFONTS 0x02000000L
    #endif
    g_chooseFontMainWindow  = nullptr;
    CHOOSEFONT chooseFont   = {};
    chooseFont.lpfnHook     = &ChooseFontHookProc;
    chooseFont.lCustData    = reinterpret_cast<LPARAM>(this);
    chooseFont.lStructSize  = sizeof(chooseFont);
    chooseFont.hwndOwner    = hwnd_;
    chooseFont.lpLogFont    = &logFont;
    chooseFont.iPointSize   = INT(-logFont.lfHeight * 10 + .5f);
    chooseFont.rgbColors    = 0;
    chooseFont.Flags        = CF_SCREENFONTS
                            | CF_PRINTERFONTS
                            | CF_INACTIVEFONTS // Don't hide fonts!
                            | CF_NOVERTFONTS // Verticalness is handled via the reading direction attribute instead.
                            | CF_NOSCRIPTSEL
                            | CF_INITTOLOGFONTSTRUCT
                            | CF_ENABLEHOOK
                            | CF_APPLY // Show the Apply button so you can change the font before closing the dialog.
                            ;

    // Show the common font dialog box.
    if (!::ChooseFont(IN OUT &chooseFont))
    {
        // Check whether error or the user canceled.
        auto errorCode = CommDlgExtendedError();
        return errorCode == 0 ? S_FALSE : HRESULT_FROM_WIN32(errorCode);
    }

    // No need to update anything here because the ChooseFontHookProc callback already did.
    return S_OK;
}


#if 0
STDMETHODIMP MainWindow::ChooseColor(IN OUT uint32_t& color)
{
    static COLORREF customColors[16] = {};
    CHOOSECOLOR colorOptions = {};
    colorOptions.lStructSize = sizeof(colorOptions);
    colorOptions.hwndOwner = hwnd_;
    colorOptions.rgbResult = color & 0x00FFFFFF;
    colorOptions.lpCustColors = customColors;
    colorOptions.Flags = CC_ANYCOLOR|CC_FULLOPEN|CC_RGBINIT;

    if (!::ChooseColor(&colorOptions))
    {
        // Check whether error or the user canceled.
        return CommDlgExtendedError() == 0 ? S_FALSE : E_FAIL;
    }

    color = 0xFF000000 | colorOptions.rgbResult;
    return S_OK;
}
#endif


HRESULT MainWindow::SelectFontFile()
{
    std::u16string filePath;
    auto const* filters = u"Fonts (*.ttf *.otf)\0" u"*.ttf;*.otf;*.cff;*.ttc;*.otc;*.tte\0"
                          u"All files (*)\0" u"*\0";

    if (!GetOpenFileName(hwnd_, filters, OUT filePath, u"Open font file"))
        return S_FALSE;

    return LoadFontFileIntoDrawableObjects(filePath.c_str());
}


HRESULT MainWindow::LoadFontFileIntoDrawableObjects(_In_z_ char16_t const* filePath)
{
    // Update the path for every selected drawable object.
    std::vector<uint32_t> const drawableObjectIndices = GetSelectedDrawableObjectIndices();
    DrawableObjectAndValues::Set(drawableObjects_, drawableObjectIndices, DrawableObjectAttributeFontFilePath, filePath);

    ComPtr<IDWriteFactory> dwriteFactory;
    ComPtr<IDWriteGdiInterop> gdiInterop;
    ComPtr<IDWriteFontCollection> fontCollection;

    IFR(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(OUT &dwriteFactory)));
    IFR(dwriteFactory->GetGdiInterop(OUT &gdiInterop));

    // Get the name of the first font in the collection.

    IFR(CreateFontCollection(dwriteFactory, DWRITE_FONT_FAMILY_MODEL_WEIGHT_STRETCH_STYLE, ToWChar(filePath), IntLen(filePath), OUT &fontCollection));

    std::u16string familyName, faceName, win32FamilyName, win32FaceName, preferredFamilyName, preferredFaceName, fullName;
    std::vector<DWRITE_FONT_AXIS_VALUE> fontAxisValues;

    // Print all the found faces.
    for (uint32_t familyIndex = 0, familyCount = fontCollection->GetFontFamilyCount(); familyIndex < familyCount; ++familyIndex)
    {
        ComPtr<IDWriteFontFamily> fontFamily;
        if (FAILED(fontCollection->GetFontFamily(familyIndex, OUT &fontFamily)))
            continue;

        GetFontFamilyName(fontFamily.Get(), nullptr, OUT familyName);

        for (uint32_t faceIndex = 0, faceCount = fontFamily->GetFontCount(); faceIndex < faceCount; ++faceIndex)
        {
            ComPtr<IDWriteFont> font;
            if (FAILED(fontFamily->GetFont(faceIndex, OUT &font))/* || innerFont->GetSimulations() != DWRITE_FONT_SIMULATIONS_NONE*/)
                continue;

            GetFontFaceName(font, nullptr, OUT faceName);
            GetInformationalString(font, DWRITE_INFORMATIONAL_STRING_WIN32_FAMILY_NAMES, nullptr, OUT win32FamilyName);
            GetInformationalString(font, DWRITE_INFORMATIONAL_STRING_WIN32_SUBFAMILY_NAMES, nullptr, OUT win32FaceName);
            GetInformationalString(font, DWRITE_INFORMATIONAL_STRING_PREFERRED_FAMILY_NAMES, nullptr, OUT preferredFamilyName);
            GetInformationalString(font, DWRITE_INFORMATIONAL_STRING_PREFERRED_SUBFAMILY_NAMES, nullptr, OUT preferredFaceName);
            GetInformationalString(font, DWRITE_INFORMATIONAL_STRING_FULL_NAME, nullptr, OUT fullName);
            DWRITE_FONT_SIMULATIONS fontSimulations = font->GetSimulations();
            GetFontAxisValues(font, OUT fontAxisValues);

            DWRITE_FONT_STYLE fontStyle = font->GetStyle();
            AppendLog(u"%d:%d = wws:'%s'|'%s',  pref:'%s'|'%s',  win32:'%s'|'%s',  full:'%s'%s wght=%d wdth=%d ital=%d slnt=%d\r\n",
                familyIndex,
                faceIndex,
                familyName.c_str(),
                faceName.c_str(),
                preferredFamilyName.c_str(),
                preferredFaceName.c_str(),
                win32FamilyName.c_str(),
                win32FaceName.c_str(),
                fullName.c_str(),
                fontSimulations != DWRITE_FONT_SIMULATIONS_NONE ? u",  simulated" : u"",
                font->GetWeight(),
                font->GetStretch(),
                (fontStyle == DWRITE_FONT_STYLE_ITALIC) ? 1 : 0,
                (fontStyle == DWRITE_FONT_STYLE_OBLIQUE) ? -20 : 0
            );
        }
    }

    ComPtr<IDWriteFontFamily> firstFontFamily;
    ComPtr<IDWriteFont> firstFont;
    IFR(fontCollection->GetFontFamily(/*index*/0, OUT &firstFontFamily));
    IFR(firstFontFamily->GetFirstMatchingFont(DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STRETCH_NORMAL, DWRITE_FONT_STYLE_NORMAL, OUT &firstFont));

    MainWindow::FontFamilyNameProperties fontFamilyNameProperties = {};
    SetFontFamilyNameProperties(firstFont, filePath, OUT fontFamilyNameProperties);
    return UpdateDrawableObjectsFromFontFamilyNameProperties(fontFamilyNameProperties);
}


HRESULT MainWindow::LoadDrawableObjectsSettings(bool clearExistingItems, bool merge)
{
    std::u16string filePath;

    if (!GetOpenFileName(hwnd_, g_openSaveFiltersList, OUT filePath, u"Open settings or text file"))
        return S_OK;

    // todo::: consolidate this extension checking logic somewhere.
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
    auto* filenameExtension = FindFileNameExtension(filePath);

    if (_wcsicmp(ToWChar(filenameExtension), L"TextLayoutSamplerSettings") == 0)
    {
        hr = LoadDrawableObjectsSettings(filePath.c_str(), clearExistingItems, merge);
    }
    else if (_wcsicmp(ToWChar(filenameExtension), L"txt") == 0)
    {
        hr = LoadTextFileIntoDrawableObjects(filePath.c_str());
    }

    if (hr == HRESULT_FROM_WIN32(ERROR_BAD_FORMAT))
        ShowMessageAndAppendLog(u"Unknown file format '%s', 0x%08X", filePath.c_str(), hr);
    else if (FAILED(hr))
        ShowMessageAndAppendLog(u"Failed to load from file '%s', 0x%08X", filePath.c_str(), hr);

    return hr;
}


HRESULT MainWindow::LoadDrawableObjectsSettings(_In_z_ char16_t const* filePath, bool clearExistingItems, bool merge)
{
    TextTree data;
    std::u16string inputText;

    AppendLog(u"Reading settings file '%s'\r\n", filePath);

    // todo: Store the base path so that relative resources like font files can be found regardless of the current path.
    previousSettingsFilePath_ = filePath;

    DeferUpdateUi(
        NeededUiUpdateDrawableObjectsListView |
        NeededUiUpdateAttributesListView |
        NeededUiUpdateAttributeValuesListView |
        NeededUiUpdateAttributeValuesEdit |
        NeededUiUpdateAttributeValuesSlider |
        NeededUiUpdateDrawableObjectsCanvas |
        NeededUiUpdateTextEdit
        );

    // Read file and parse.
    IFR(ReadTextFile(filePath, OUT inputText));

    JsonexParser parser(inputText, JsonexParser::OptionsDefault);
    parser.ReadNodes(IN OUT data);

    if (clearExistingItems)
    {
        drawableObjects_.clear();
    }

    TextTree::NodePointer subroot = data.BeginFirstChild();

    Attribute::PredefinedValue recognizedSettings[] = {
        {1,u"content"},
        {2,u"objects"},
    };

    for (TextTree::NodePointer node = subroot.begin(), nodeEnd = subroot.end(); node != nodeEnd; ++node)
    {
        std::u16string text = node.GetText();
        std::u16string value = node.GetSubvalue();

        uint32_t settingEnumValue;
        if (FAILED(Attribute::PredefinedValue::MapNameToValue({recognizedSettings, countof(recognizedSettings)}, text.c_str(), OUT settingEnumValue)))
            continue;

        switch (settingEnumValue)
        {
        case 1: // content
            if (value.compare(u"TextLayoutSamplerSettings") != 0)
            {
                AppendLog(u"File did not contain expected content. '%s' != TextLayoutSamplerSettings\r\n", value.c_str());
                return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
            }
            break;
        case 2: // objects
            if (merge)
            {
                std::vector<DrawableObjectAndValues> drawableObjects;
                DrawableObjectAndValues::Load(node, OUT drawableObjects);
                if (!drawableObjects.empty())
                {
                    DrawableObjectAndValues::Merge(*drawableObjects.data(), IN OUT drawableObjects_);
                }
            }
            else
            {
                DrawableObjectAndValues::Load(node, IN OUT drawableObjects_);
            }
            break;
        }
    }

    return S_OK;
}


HRESULT MainWindow::LoadTextFileIntoDrawableObjects(_In_z_ char16_t const* filePath)
{
    std::u16string inputText;

    AppendLog(u"Reading text file '%s'\r\n", filePath);

    DeferUpdateUi(
        NeededUiUpdateDrawableObjectsListView |
        NeededUiUpdateAttributesListView |
        NeededUiUpdateAttributeValuesListView |
        NeededUiUpdateAttributeValuesEdit |
        NeededUiUpdateAttributeValuesSlider |
        NeededUiUpdateDrawableObjectsCanvas |
        NeededUiUpdateTextEdit
        );

    // Read file and parse.
    IFR(ReadTextFile(filePath, OUT inputText));

    if (drawableObjects_.empty())
    {
        // todo::: Initialize defaults somewhere shared.
        drawableObjects_.resize(1);
        auto& drawableObject = drawableObjects_[0];
        drawableObject.Set(DrawableObjectAttributeFunction, u"IDWriteBitmapRenderTarget IDWriteTextLayout");
        drawableObject.Set(DrawableObjectAttributeFontFamily, u"Segoe UI");
        drawableObject.Set(DrawableObjectAttributeFontSize, u"18");
    }

    for (auto& drawableObject : drawableObjects_)
    {
        drawableObject.Set(DrawableObjectAttributeText, inputText.c_str());
        drawableObject.Update();
    }

    return S_OK;
}


HRESULT MainWindow::StoreTextFileFromDrawableObjects(_In_z_ char16_t const* filePath)
{
    std::u16string outputText;

    AppendLog(u"Writing text file '%s'\r\n", filePath);

    uint32_t selectedDrawableObjectIndex;
    if (SUCCEEDED(MainWindow::GetSelectedDrawableObject(OUT selectedDrawableObjectIndex)))
    {
        auto& drawableObject = drawableObjects_[selectedDrawableObjectIndex];
        auto text = drawableObject.GetString(DrawableObjectAttributeText);
        IFR(WriteTextFile(filePath, text.data(), static_cast<uint32_t>(text.size()) ));
    }

    return S_OK;
}


HRESULT MainWindow::GetSelectedDrawableObject(_Out_ uint32_t& selectedDrawableObjectIndex)
{
    // Return the first selected index, or the first drawable object if none are selected.
    selectedDrawableObjectIndex = ~0u;
    uint32_t index = 0;

    // Get the current selected drawable item (if exactly one).
    std::vector<uint32_t> selectedDrawableObjectIndices = GetSelectedDrawableObjectIndices();
    if (!selectedDrawableObjectIndices.empty())
    {
        index = selectedDrawableObjectIndices.front();
    }

    if (index >= drawableObjects_.size())
    {
        ShowMessageAndAppendLog(u"Select one drawable object first.");
        return E_BOUNDS;
    }

    selectedDrawableObjectIndex = index;
    return S_OK;
}


HRESULT MainWindow::GetFileOrFamilyName(
    uint32_t selectedDrawableObjectIndex,
    _Out_ std::u16string& fileOrFamilyName
    )
{
    if (selectedDrawableObjectIndex >= drawableObjects_.size())
        return E_BOUNDS;

    auto& drawableObject = drawableObjects_[selectedDrawableObjectIndex];
    auto originalFontFilePath = drawableObject.GetString(DrawableObjectAttributeFontFilePath);
    if (!originalFontFilePath.empty())
    {
        fileOrFamilyName.assign(originalFontFilePath.data(), originalFontFilePath.size());
    }
    else
    {
        auto familyName = drawableObject.GetString(DrawableObjectAttributeFontFamily);
        fileOrFamilyName.assign(familyName.data(), familyName.size());
    }

    return S_OK;
}


HRESULT MainWindow::SaveSelectedFontFile()
{
    std::u16string filePath;
    auto const* filters = u"Font files\0" u"*.ttf;*.otf;*.ttc;*.cff\0"
                          u"All files (*)\0" u"*\0";

    // Get the first selected drawable item to use either
    // the existing file path or family name.
    uint32_t selectedDrawableObjectIndex;
    IFR(GetSelectedDrawableObject(OUT selectedDrawableObjectIndex));
    IFR(GetFileOrFamilyName(selectedDrawableObjectIndex, OUT filePath));

    if (!GetSaveFileName(hwnd_, filePath.c_str(), filters, u"ttf", OUT filePath))
        return S_OK;

    DrawingCanvasControl& drawingCanvas = *DrawingCanvasControl::GetClass(GetWindowFromId(hwnd_, IdcDrawingCanvas));
    auto& drawableObject = drawableObjects_[selectedDrawableObjectIndex];

    // Save the font data finally.

    HRESULT hr = DrawableObject::SaveFontFile(drawableObject, drawingCanvas, filePath.c_str());
    if (SUCCEEDED(hr))
    {
        AppendLog(u"Wrote font to file '%s'.\r\n", filePath.c_str());
    }
    else if (hr == DWRITE_E_NOFONT || hr == DWRITE_E_FILENOTFOUND)
    {
        auto familyName = drawableObject.GetString(DrawableObjectAttributeFontFamily);
        ShowMessageAndAppendLog(u"Could not write font named '%s' to file '%s' because font could not be created. Error = 0x%08X", familyName.data(), filePath.c_str(), hr);
    }
    else
    {
        ShowMessageAndAppendLog(u"Could not write font to file '%s'. Error = 0x%08X", filePath.c_str(), hr);
    }

    return hr;
}


HRESULT MainWindow::GetAllFontCharacters(bool copyToClipboardInstead, bool getOnlyColorFontCharacters)
{
    uint32_t selectedDrawableObjectIndex;
    IFR(GetSelectedDrawableObject(OUT selectedDrawableObjectIndex));

    DrawingCanvasControl& drawingCanvas = *DrawingCanvasControl::GetClass(GetWindowFromId(hwnd_, IdcDrawingCanvas));
    auto& drawableObject = drawableObjects_[selectedDrawableObjectIndex];

    std::u16string characters;
    IFR(ShowMessageIfError(
        u"Could not get font characters.\r\nError = 0x%08X",
        DrawableObject::GetFontCharacters(drawableObject, drawingCanvas, getOnlyColorFontCharacters, OUT characters)
    ));

    if (copyToClipboardInstead)
    {
        IFR(CopyTextToClipboard(hwnd_, characters));
    }
    else
    {
        std::vector<uint32_t> drawableObjectIndices = GetSelectedDrawableObjectIndices();
        DrawableObjectAndValues::Set(drawableObjects_, drawableObjectIndices, DrawableObjectAttributeText, characters);
        DrawableObjectAndValues::Update(drawableObjects_, drawableObjectIndices);
    }

    DeferUpdateUi(
        NeededUiUpdateDrawableObjectsListView |
        NeededUiUpdateAttributesListView |
        NeededUiUpdateAttributeValuesListView |
        NeededUiUpdateAttributeValuesEdit |
        NeededUiUpdateAttributeValuesSlider |
        NeededUiUpdateDrawableObjectsCanvas |
        NeededUiUpdateTextEdit
        );

    return S_OK;
}


HRESULT MainWindow::ExportFontGlyphData()
{
    std::u16string filePath;
    auto const* filters = u"Font glyph files\0" u"*.svg;*.png;*.tiff;*.tif;*.jpeg;*.jpg\0"
                          u"All files (*)\0" u"*\0";

    // Get the first selected drawable item.
    uint32_t selectedDrawableObjectIndex;
    IFR(GetSelectedDrawableObject(OUT selectedDrawableObjectIndex));
    IFR(GetFileOrFamilyName(selectedDrawableObjectIndex, OUT filePath));

    // Remove any extension, appending underscore to separate.
    filePath.erase(FindFileNameExtension(filePath) - filePath.data());
    if (!filePath.empty() && filePath.back() == '.')
        filePath.pop_back();
    filePath += u"_";

    if (!GetSaveFileName(hwnd_, filePath.c_str(), filters, nullptr, OUT filePath))
        return S_OK;

    DrawingCanvasControl& drawingCanvas = *DrawingCanvasControl::GetClass(GetWindowFromId(hwnd_, IdcDrawingCanvas));
    auto& drawableObject = drawableObjects_[selectedDrawableObjectIndex];

    // Save the font data finally.

    HRESULT hr = DrawableObject::ExportFontGlyphData(drawableObject, drawingCanvas, filePath);
    if (hr == S_OK)
    {
        AppendLog(u"Exported glyph data to file path '%s'.\r\n", filePath.c_str());
    }
    else if (SUCCEEDED(hr))
    {
        ShowMessageAndAppendLog(u"The font had no SVG/PNG/TIFF/JPEG glyph image data to export (just TrueType/CFF outlines).\r\n");
    }
    else
    {
        ShowMessageAndAppendLog(u"Could not write glyph data to files '%s'. Error = 0x%08X", filePath.c_str(), hr);
    }

    return hr;
}


HRESULT MainWindow::AutofitDrawableObjects(bool useMaximumWidth, bool useMaximumHeight)
{
    // Autofit all the selected objects to their actual contents, looping through and asking each
    // for its bounds, the updating its properties.

    std::vector<uint32_t> drawableObjectIndices = GetSelectedDrawableObjectIndices();
    auto const drawableObjectsTotal = drawableObjects_.size();
    DrawingCanvasControl& drawingCanvas = *DrawingCanvasControl::GetClass(GetWindowFromId(hwnd_, IdcDrawingCanvas));

    std::vector<D2D_SIZE_F> sizes(drawableObjects_.size());
    float maximumWidth = 0, maximumHeight = 0;

    // The first pass gets the sizes of all the objects.
    for (auto drawableObjectIndex : drawableObjectIndices)
    {
        if (drawableObjectIndex >= drawableObjectsTotal)
            continue;

        auto& drawableObject = drawableObjects_[drawableObjectIndex];

        D2D_RECT_F layoutBounds, contentBounds;
        if (SUCCEEDED(drawableObject.drawableObject_->GetBounds(drawableObject, drawingCanvas, OUT layoutBounds, OUT contentBounds)))
        {
            PixelAlignRect(IN OUT contentBounds);
            auto& size    = sizes[drawableObjectIndex];
            size.width    = contentBounds.right - contentBounds.left;
            size.height   = contentBounds.bottom - contentBounds.top;
            maximumWidth  = std::max(maximumWidth, size.width);
            maximumHeight = std::max(maximumHeight, size.height);
        }
    }

    // The second pass updates them, considering whether to maximize them to the largest object found.
    for (auto drawableObjectIndex : drawableObjectIndices)
    {
        if (drawableObjectIndex >= drawableObjectsTotal)
            continue;

        auto& drawableObject = drawableObjects_[drawableObjectIndex];
        D2D_SIZE_F& size = sizes[drawableObjectIndex];
        if (size.width > 0 && size.height > 0)
        {
            drawableObject.Set(DrawableObjectAttributeWidth,  uint32_t(useMaximumWidth  ? std::max(maximumWidth,  size.width)  : size.width));
            drawableObject.Set(DrawableObjectAttributeHeight, uint32_t(useMaximumHeight ? std::max(maximumHeight, size.height) : size.height));
            drawableObject.Update();
        }
    }

    DeferUpdateUi(
        NeededUiUpdateDrawableObjectsListView |
        NeededUiUpdateAttributesListView |
        NeededUiUpdateAttributeValuesListView |
        NeededUiUpdateAttributeValuesEdit |
        NeededUiUpdateAttributeValuesSlider |
        NeededUiUpdateDrawableObjectsCanvas |
        NeededUiUpdateTextEdit
    );

    return S_OK;
}


HRESULT MainWindow::StoreDrawableObjectsSettings()
{
    std::u16string filePath;

    if (!GetSaveFileName(hwnd_, previousSettingsFilePath_.c_str(), g_openSaveFiltersList, u"TextLayoutSamplerSettings", OUT filePath))
        return S_OK;

    HRESULT hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
    auto* filenameExtension = FindFileNameExtension(filePath);

    if (_wcsicmp(ToWChar(filenameExtension), L"TextLayoutSamplerSettings") == 0)
    {
        hr = StoreDrawableObjectsSettings(filePath.c_str());
    }
    else if (_wcsicmp(ToWChar(filenameExtension), L"txt") == 0)
    {
        hr = StoreTextFileFromDrawableObjects(filePath.c_str());
    }

    if (hr == S_OK)
    {
        AppendLog(u"Wrote successfully.\r\n", filePath.c_str());
    }
    if (hr == HRESULT_FROM_WIN32(ERROR_BAD_FORMAT)) // Consolidate this repeated error message.
    {
        ShowMessageAndAppendLog(u"Unknown file format '%s', 0x%08X", filePath.c_str(), hr);
    }
    else if (FAILED(hr))
    {
        ShowMessageAndAppendLog(u"Failed to save to file '%s', 0x%08X", filePath.c_str(), hr);
    }

    return hr;
}


HRESULT MainWindow::StoreDrawableObjectsSettings(_In_z_ char16_t const* filePath)
{
    TextTree data;
    AppendLog(u"Writing settings file '%s'\r\n", filePath);

    previousSettingsFilePath_ = filePath;

    // Create the text tree root node and outermost object.
    data.Append(TextTree::Node::TypeRoot, 1, u"", 0);
    TextTree::NodePointer root = data.begin();
    TextTree::NodePointer subroot = root.AppendChild(TextTree::Node::TypeObject, u"", 0);

    // Write all the objects.
    subroot.SetKeyValue(u"content", u"TextLayoutSamplerSettings", uint32_t(countof(u"TextLayoutSamplerSettings"))-1);
    auto objectsNode = subroot.AppendChild(TextTree::Node::TypeArray, u"objects", uint32_t(countof(u"objects")-1));
    DrawableObjectAndValues::Store(drawableObjects_, objectsNode);

    // Serialize and write the file.
    JsonexWriter writer(JsonexWriter::OptionsDefault);
    writer.WriteNodes(data);
    array_ref<char16_t const> outputJson = writer.GetText();
    IFR(WriteTextFile(filePath, outputJson.data(), static_cast<uint32_t>(outputJson.size())));

    return S_OK;
}


void MainWindow::ReadAttributeFilterEdit()
{
    GetWindowText(GetWindowFromId(hwnd_, IdcAttributesFilterEdit), OUT attributeFilter_);
}


void MainWindow::ReadAttributeValueEdit()
{
    GetWindowText(GetWindowFromId(hwnd_, IdcAttributeValuesEdit), OUT selectedAttributeValue_);

    UpdateDrawableObjectValuesUsing(selectedAttributeValue_);
}


void MainWindow::UpdateDrawableObjectValuesUsing(std::u16string const& newStringValue)
{
    if (drawableObjects_.empty())
        return;

    std::vector<uint32_t> attributeIndices = GetSelectedAttributeIndices();
    std::vector<uint32_t> drawableObjectIndices = GetSelectedDrawableObjectIndices();

    if (drawableObjectIndices.empty() || attributeIndices.empty())
        return;

    // Check each attribute in the dialog.
    for (auto attributeIndex : attributeIndices)
    {
        if (attributeIndex >= DrawableObjectAttributeTotal)
            continue;

        // Copy the current text value of the attribute from the dialog control to all selected attributes.
        DrawableObjectAndValues::Set(
            drawableObjects_,
            drawableObjectIndices,
            attributeIndex,
            newStringValue
            );
    }

    DrawableObjectAndValues::Update(
        drawableObjects_,
        drawableObjectIndices
        );
}


void MainWindow::ChangeSettingsVisibility(SettingsVisibility settingsVisibility)
{
    settingsVisibility_ = settingsVisibility;
    bool isFullyVisible = (settingsVisibility == SettingsVisibilityFull);
    int fullShowMode = isFullyVisible ? SW_SHOW : SW_HIDE;
    int lightShowMode = (settingsVisibility == SettingsVisibilityLight) ? SW_SHOW : SW_HIDE;

    ShowWindow(GetWindowFromId(hwnd_, IdcEditText), lightShowMode);
    ShowWindow(GetWindowFromId(hwnd_, IdcDrawableObjectsList), fullShowMode);
    ShowWindow(GetWindowFromId(hwnd_, IdcAttributesList), fullShowMode);
    ShowWindow(GetWindowFromId(hwnd_, IdcAttributesFilterEdit), fullShowMode);
    ShowWindow(GetWindowFromId(hwnd_, IdcAttributeValuesEdit), fullShowMode);
    ShowWindow(GetWindowFromId(hwnd_, IdcAttributeValuesList), fullShowMode);
    ShowWindow(GetWindowFromId(hwnd_, IdcAttributeValuesList), fullShowMode);
    EnableWindow(GetWindowFromId(hwnd_, IdcDrawableObjectCreate), isFullyVisible);
    EnableWindow(GetWindowFromId(hwnd_, IdcDrawableObjectDelete), isFullyVisible);
    EnableWindow(GetWindowFromId(hwnd_, IdcDrawableObjectMoveUp), isFullyVisible);
    EnableWindow(GetWindowFromId(hwnd_, IdcDrawableObjectMoveDown), isFullyVisible);

    DeferUpdateUi(
        NeededUiFillAttributesListView |
        NeededUiUpdateAttributesListView |
        NeededUiUpdateDrawableObjectsListView |
        NeededUiUpdateAttributeValuesListView |
        NeededUiUpdateAttributeValuesSlider |
        NeededUiUpdateAttributeValuesEdit |
        NeededUiUpdateTextEdit
        );
    UpdateUi();
    Resize(IddMainWindow);
}


MainWindow::DialogProcResult CALLBACK MainWindow::OnCommand(HWND hwnd, int id, HWND hwndControl, UINT codeNotify)
{
    #if DEBUG_SHOW_WINDOWS_MESSAGES
    if (id != IdcLog)
        AppendLog(L"Message=%d, id=%d, ctl=%08X, notify=%d\r\n", WM_COMMAND, id, hwndControl, codeNotify);
    #endif

    // Parse the menu selections:
    switch (id)
    {
    case IDCANCEL:
    case IDCLOSE:
        drawableObjects_.clear(); // Clear these before the canvas in case of any dependencies.
        DestroyWindow(hwnd);
        PostQuitMessage(0);
        delete this;
        break; // do NOT reference class after this

    case IDOK:
        {
            // Switch focus to another control when pressing Enter.
            auto controlId = GetDlgCtrlID(GetFocus());
            switch (controlId)
            {
            case IdcAttributesFilterEdit:
                {
                    auto attributeList = GetWindowFromId(hwnd, IdcAttributesList);
                    SetFocus(attributeList);
                    ListView_SetItemState(attributeList, -1, 0, LVIS_SELECTED);
                    ListView_SetItemState(attributeList, 0, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
                }
                break;
            default:
                return false;
            }
        }
        break;

    case IdmCopyImage:
        {
            HWND canvasHwnd = GetWindowFromId(hwnd_, IdcDrawingCanvas);
            if (canvasHwnd != nullptr)
                DrawingCanvasControl::GetClass(canvasHwnd)->CopyToClipboard();
        }
        break;

    case IdcDrawableObjectSelectAll:
        ListView_SetItemState(GetWindowFromId(hwnd_, IdcDrawableObjectsList), -1, LVIS_SELECTED, LVIS_SELECTED);
        break;

    case IdcEditText:
        switch (codeNotify)
        {
        case EN_CHANGE:
            {
                std::u16string text;
                std::vector<uint32_t> drawableObjectIndices = GetSelectedDrawableObjectIndices();
                GetWindowText(GetWindowFromId(hwnd_, IdcEditText), OUT text);
                DrawableObjectAndValues::Set(drawableObjects_, drawableObjectIndices, DrawableObjectAttributeText, text.c_str());
                DrawableObjectAndValues::Update(drawableObjects_, drawableObjectIndices);

                DeferUpdateUi(NeededUiUpdateDrawableObjectsCanvas);
            }
            break;

        default:
            return false;
        }
        break;

    case IdcAttributeValuesEdit:
        switch (codeNotify)
        {
        case EN_CHANGE:
            {
                // Reflect the user-typed value into the other list views.
                ReadAttributeValueEdit();

                // Update the values list if single line edit to reprioritize values.
                // Avoid doing so for multi-line text controls which are long and won't
                // match choices in the list anyway usually.
                bool isMultiline = !!(GetWindowStyle(hwndControl) & ES_MULTILINE);
                if (!isMultiline)
                    isTypingAttributeValueToFilter_ = true;

                DeferUpdateUi(
                    NeededUiUpdateDrawableObjectsListView |
                    NeededUiUpdateDrawableObjectsCanvas |
                    NeededUiUpdateAttributesListView |
                    NeededUiUpdateAttributeValuesSlider |
                    (isMultiline ? NeededUiUpdateNone : NeededUiUpdateAttributeValuesListView)
                    // Not NeededUiUpdateAttributeValuesEdit
                    );
            }
            break;
        default:
            return false;
        }
        break;

    case IdcAttributesFilterEdit:
        switch (codeNotify)
        {
        case EN_CHANGE:
            {
                // Reflect the user-typed filter.
                ReadAttributeFilterEdit();
                DeferUpdateUi(NeededUiFillAttributesListView | NeededUiUpdateAttributesListView);
            }
            break;
        default:
            return false;
        }
        break;

    case IdcSettingsHidden:
    case IdcSettingsLight:
    case IdcSettingsFull:
        switch (codeNotify)
        {
        case BN_CLICKED:
            ChangeSettingsVisibility(SettingsVisibility(id - IdcSettingsFirst));
            break;
        default:
            return false;
        }
        break;

    case IdcDrawableObjectListLoad:
        if (codeNotify != BN_CLICKED)
            return false;

        LoadDrawableObjectsSettings();
        break;

    case IdcDrawableObjectListStore:
        if (codeNotify != BN_CLICKED)
            return false;

        StoreDrawableObjectsSettings();
        break;

    case IdcDrawableObjectDelete:
        if (codeNotify != BN_CLICKED)
            return false;

        DeleteDrawableObjectsListViewSelected();
        break;

    case IdcDrawableObjectCreate:
        if (codeNotify != BN_CLICKED)
            return false;

        CreateDrawableObjectsListViewSelected();
        break;

    case IdcDrawableObjectMoveUp:
    case IdcDrawableObjectMoveDown:
        ShiftSelectedDrawableObjects((id == IdcDrawableObjectMoveUp) ? -1 : 1);
        break;

    case IdcSelectFontFamily:
        if (codeNotify != BN_CLICKED)
            return false;
            
        SelectFontFamily();
        break;

    case IdcSelectFontFile:
        if (codeNotify != BN_CLICKED)
            return false;

        SelectFontFile();
        break;

    case IdcAssortedActions:
        OnAssortedActions(hwndControl);
        break;

    default:
        return false; // unhandled
    }

    return true; // handled
}


MainWindow::DialogProcResult CALLBACK MainWindow::OnVerticalScroll(HWND hwnd, HWND controlHandle, UINT code, int position)
{
    OnHorizontalOrVerticalScroll(hwnd, SB_VERT, code, g_smallMouseWheelStep);
    return true;
}


void MainWindow::OnHorizontalOrVerticalScroll(HWND hwnd, int barType, UINT code, int smallStep)
{
    // Update the scroll bar.
    int delta;
    int newPosition = MapScrollBarCodeToPosition(hwnd, barType, code,smallStep, OUT delta);
    if (delta == 0)
        return;

    SetScrollPos(hwnd, barType, newPosition, TRUE);

    // Scroll the pixels and child controls.
    int dx = 0, dy = -delta;
    if (barType == SB_HORZ)
        std::swap(dx, dy);

    ScrollWindowEx(hwnd, dx, dy, nullptr, nullptr, nullptr, nullptr, SW_SCROLLCHILDREN|SW_ERASE|SW_INVALIDATE);
}


MainWindow::DialogProcResult CALLBACK MainWindow::OnNotification(HWND hwnd, int controlId, NMHDR* notifyMessageHeader)
{
    NMHDR& nmh = *notifyMessageHeader;

    #if DEBUG_SHOW_WINDOWS_MESSAGES
    AppendLog(L"time=%d notify hwnd=%08X, controlId=%08X, code=%08X\r\n", GetTickCount(), hwnd, controlId, notifyMessageHeader->code);
    #endif

    switch (controlId)
    {
    case IdcDrawableObjectsList:
        switch (nmh.code)
        {
        case LVN_ITEMCHANGED:
            {
                // If selection changed, update lists and values.
                NMLISTVIEW const& nm = reinterpret_cast<NMLISTVIEW&>(nmh);
                int selectionFlag = (nm.uNewState ^ nm.uOldState) & LVIS_SELECTED;
                if (selectionFlag)
                {
                    DeferUpdateUi(NeededUiUpdateAttributesListView | NeededUiUpdateAttributeValuesListView | NeededUiUpdateAttributeValuesSlider | NeededUiUpdateAttributeValuesEdit);
                }
            }
            break;

        case NM_RETURN:
            SetFocus(GetWindowFromId(hwnd, IdcAttributesList));
            break;

        //case NM_KEYDOWN: // ListView controls apparently don't send a NM_KEYDOWN!? Even though the structure is basically identical. :/
        case LVN_KEYDOWN:
            {
                NMLVKEYDOWN const& keyDown = *reinterpret_cast<NMLVKEYDOWN*>(notifyMessageHeader);
                if (keyDown.wVKey == VK_DELETE)
                {
                    isRecursing_ = true;
                    DeleteDrawableObjectsListViewSelected();
                    isRecursing_ = false;
                }
                else
                {
                    return false;
                }
            }
            break;

        //case NM_DBLCLK: Ignore clicks.
        case LVN_ITEMACTIVATE: // Double click
            {
                // Determine which column was clicked to select the attribute in the list.
                NMITEMACTIVATE const& itemActivate = reinterpret_cast<NMITEMACTIVATE&>(nmh);
                LVHITTESTINFO hitTestInfo = {};
                hitTestInfo.pt = itemActivate.ptAction;
                int rowIndex = ListView_SubItemHitTest(nmh.hwndFrom, OUT &hitTestInfo);
                if (rowIndex < 0)
                    break;

                // Map column to attribute
                auto attributeListViewHwnd = GetWindowFromId(hwnd_, IdcAttributesList);
                rowIndex = ListViewRemapLparamToIndex(attributeListViewHwnd, hitTestInfo.iSubItem);
                ListView_SelectSingleVisibleItem(attributeListViewHwnd, rowIndex);
                selectedAttributeIndex_ = DrawableObjectAttribute(hitTestInfo.iSubItem);

                UpdateUi(); // Update everything now rather than deferred, so that setting focus works well.
                HWND editHwnd = GetWindowFromId(hwnd_, IdcAttributeValuesEdit);
                SetFocus(editHwnd);
                Edit_SetSel(editHwnd, 0, -1);
            }
            break;

        case LVN_GETEMPTYTEXT:
            HandleSysListViewEmptyText((LPARAM)notifyMessageHeader, u"No drawable objects. Click '+' to create one.");
            return DialogProcResult(true, 1);

        default:
            return false;
        } // nmh.code
        break;

    case IdcAttributesList:
        switch (nmh.code)
        {
        case LVN_ITEMCHANGED:
            {
                NMLISTVIEW const& nm = reinterpret_cast<NMLISTVIEW&>(nmh);
                int selectionFlag = (nm.uNewState ^ nm.uOldState) & LVIS_SELECTED;
                if (!selectionFlag)
                    break; // No change.

                // Get the selected attributes.
                std::vector<uint32_t> selectedAttributeIndices = GetListViewMatchingIndices(nmh.hwndFrom, LVNI_SELECTED, false);
                ListViewRemapIndicesToLparam(nmh.hwndFrom, IN OUT selectedAttributeIndices);

                // Determine the primary selected attribute, which may be none, a single selection,
                // or the first item of a multiple selection (if they share the same value list,
                // otherwise none if they vary).
                selectedAttributeIndex_ = DrawableObjectAttributeTotal; // Reset for now.
                Attribute::PredefinedValue const* previousPredefinedValues = nullptr;
                bool isFirstIndex = true;

                // Check whether the selected attributes vary in their predefined value list.
                // Some attributes share the same list, such as colors, in which case it's fine to update them all.
                for (auto index : selectedAttributeIndices)
                {
                    ThrowIf(index >= countof(DrawableObject::attributeList), "Index is beyond DrawableObject::attributeList size!");

                    auto const& attribute = DrawableObject::attributeList[index];
                    if (isFirstIndex)
                    {
                        isFirstIndex = false;
                        previousPredefinedValues = attribute.predefinedValues.data();
                        selectedAttributeIndex_ = static_cast<DrawableObjectAttribute>(index);
                    }
                    else if (attribute.predefinedValues.data() != previousPredefinedValues) // They vary.
                    {
                        selectedAttributeIndex_ = DrawableObjectAttributeTotal;
                        break;
                    }
                }

                DeferUpdateUi(NeededUiUpdateAttributeValuesListView | NeededUiUpdateAttributeValuesSlider | NeededUiUpdateAttributeValuesEdit);
            }
            break;

        //case LVN_ITEMACTIVATE: // causes double draw :/
        case NM_RETURN:
        case NM_DBLCLK:
            {
                auto editHwnd = GetWindowFromId(hwnd_, IdcAttributeValuesEdit);
                SetFocus(editHwnd);
                Edit_SetSel(editHwnd, 0, -1);
            }
            break;

        case LVN_KEYDOWN:
            {
                NMLVKEYDOWN const& keyDown = *reinterpret_cast<NMLVKEYDOWN*>(notifyMessageHeader);
                if (keyDown.wVKey == VK_DELETE)
                {
                    isRecursing_ = true;
                    selectedAttributeValue_.clear();
                    UpdateDrawableObjectValuesUsing(selectedAttributeValue_);
                    isRecursing_ = false;
                    DeferUpdateUi(
                        NeededUiUpdateDrawableObjectsListView |
                        NeededUiUpdateDrawableObjectsCanvas |
                        NeededUiUpdateAttributesListView |
                        NeededUiUpdateAttributeValuesListView |
                        NeededUiUpdateAttributeValuesSlider |
                        NeededUiUpdateAttributeValuesEdit
                        );
                }
                else
                {
                    return false;
                }
            }
            break;

        default:
            return false;
        } // nmh.code
        break;

    case IdcAttributeValuesList:
        switch (nmh.code)
        {
        case LVN_ITEMCHANGED:
            {
                // Udpate the edit string with the selected list view item.
                NMLISTVIEW const& nm = *reinterpret_cast<NMLISTVIEW*>(notifyMessageHeader);
                int selectionFlag = (nm.uNewState ^ nm.uOldState) & nm.uNewState & LVIS_FOCUSED;
                if (!selectionFlag || selectedAttributeIndex_ >= countof(DrawableObject::attributeList))
                    break;

                // Get the selected item, mapping listview index to attribute index.
                uint32_t attributeValueIndex = ListView_GetNextItem(nm.hdr.hwndFrom, -1, LVNI_FOCUSED | LVNI_SELECTED);
                if (int32_t(attributeValueIndex) < 0)
                    break;

                ListViewRemapIndicesToLparam(nm.hdr.hwndFrom, IN OUT wrap_single_array_ref(attributeValueIndex));

                auto const& attribute = DrawableObject::attributeList[selectedAttributeIndex_];
                if (attributeValueIndex >= attribute.predefinedValues.size())
                    break;

                // Update the edit control.
                std::u16string stringValueBuffer;
                auto* stringValue = attribute.GetPredefinedValue(
                    attributeValueIndex,
                    attribute.GetPredefinedValueFlagsString | attribute.GetPredefinedValueFlagsName | attribute.GetPredefinedValueFlagsInteger,
                    OUT stringValueBuffer
                    );
                HWND editHwnd = GetWindowFromId(hwnd_, IdcAttributeValuesEdit);
                selectedAttributeValue_ = stringValue;

                // Update EDIT control. Prevent a recursion issue where SetWindowText
                // unfortunately sends a WM_COMMAND EN_CHANGE for single line EDIT
                // controls even though it's being set programmatically, not by the
                // user (it inconsistently does not send the message for multi-line
                // EDIT's though).

                isRecursing_ = true;
                SetWindowText(editHwnd, ToWChar(stringValue));
                Edit_SetSel(editHwnd, 0, -1);
                isRecursing_ = false;

                UpdateDrawableObjectValuesUsing(selectedAttributeValue_);
                DeferUpdateUi(NeededUiUpdateDrawableObjectsListView | NeededUiUpdateDrawableObjectsCanvas | NeededUiUpdateAttributesListView);
            }
            break;

        //case LVN_ITEMACTIVATE: // causes double draw :/
        case NM_RETURN:
        case NM_DBLCLK:
            SetFocus(GetWindowFromId(hwnd_, IdcAttributesList));
            break;

        case LVN_GETEMPTYTEXT:
            HandleSysListViewEmptyText((LPARAM)notifyMessageHeader, u"No predefined values for this attribute.");
            return DialogProcResult(true, 1);

        default:
            return false;
        } // nmh.code
        break;

    case IdcDrawingCanvas:
        switch (nmh.code)
        {
        case NM_CLICK:
            if (settingsVisibility_ == SettingsVisibilityFull)
            {
                NMCLICK const& click = reinterpret_cast<NMCLICK&>(nmh);
                DrawingCanvasControl& drawingCanvas = *DrawingCanvasControl::GetClass(GetWindowFromId(hwnd_, IdcDrawingCanvas));
                D2D_POINT_2F point = drawingCanvas.RemapPoint({float(click.pt.x), float(click.pt.y)}, /*fromCanvasToWorld*/true);
                for (size_t drawableObjectIndex = 0; drawableObjectIndex < drawableObjects_.size(); ++drawableObjectIndex)
                {
                    auto const& drawableObject = drawableObjects_[drawableObjectIndex];
                    if (drawableObject.IsPointInside(point.x, point.y))
                    {
                        ListView_SelectSingleVisibleItem(GetWindowFromId(hwnd_, IdcDrawableObjectsList), int(drawableObjectIndex));
                        break;
                    }
                }
            }
            break;
        case NM_CUSTOMDRAW:
            {
                NMCUSTOMDRAW const& customDraw = reinterpret_cast<NMCUSTOMDRAW&>(nmh);
                switch (customDraw.dwDrawStage)
                {
                case CDDS_PREERASE:
                    {
                        DrawingCanvas& drawingCanvas = *DrawingCanvasControl::GetClass(GetWindowFromId(hwnd_, IdcDrawingCanvas));
                        drawingCanvas.ClearBackground(DrawableObject::defaultCanvasColor);
                    }
                    //return {true, CDRF_DOERASE};
                    return {true, CDRF_SKIPDEFAULT};

                case CDDS_POSTERASE:
                    {
                        DX_MATRIX_3X2F matrix;
                        DrawingCanvasControl& drawingCanvas = *DrawingCanvasControl::GetClass(GetWindowFromId(hwnd_, IdcDrawingCanvas));
                        drawingCanvas.CalculateViewMatrix(OUT matrix);
                        DrawableObjectAndValues::Arrange(drawableObjects_, drawingCanvas);
                        DrawableObjectAndValues::Draw(drawableObjects_, drawingCanvas, matrix);
                        drawingCanvas.RetireStaleSharedResources();
                    }
                    return {true, CDRF_DODEFAULT};

                default:
                    return false;
                }
            }
            break;

        default:
            return false;
        }
        break;

    case IdcDrawableObjectListLoad:
        switch (nmh.code)
        {
        case BCN_DROPDOWN:
            {
                NMBCDROPDOWN const& dropDown = reinterpret_cast<NMBCDROPDOWN&>(nmh);
                TrackPopupMenu_Item items[] = {
                    {IdcDrawableObjectListLoad,   u"Load..."},
                    {IdcDrawableObjectListAppend, u"Append..."},
                    {IdcDrawableObjectListMerge,  u"Merge..."},
                };

                int menuId = TrackPopupMenu(
                    make_array_ref(items, countof(items)),
                    dropDown.hdr.hwndFrom,
                    hwnd_
                    );

                switch (menuId)
                {
                case IdcDrawableObjectListLoad:   LoadDrawableObjectsSettings(); break;
                case IdcDrawableObjectListAppend: LoadDrawableObjectsSettings(false); break;
                case IdcDrawableObjectListMerge:  LoadDrawableObjectsSettings(false, true); break;
                }
            }
            break;
        }
        break;

    case IdcAssortedActions:
        switch (nmh.code)
        {
        // todo: Also drop down list when the regular button part is clicked.
        case BCN_DROPDOWN:
            {
                NMBCDROPDOWN const& dropDown = reinterpret_cast<NMBCDROPDOWN&>(nmh);
                OnAssortedActions(dropDown.hdr.hwndFrom);
            }
            break;

        default:
            return false;
        }
        break;

    default:
        return false;

    } // switch controlId

    return true;
}


void MainWindow::Resize(int id)
{
    HWND hwnd;
    long const spacing = 4;
    RECT clientRect;

    switch (id)
    {
    case IddMainWindow:
        {
            hwnd = hwnd_;
            GetClientRect(hwnd, &clientRect);
            if (IsRectEmpty(&clientRect))
                return; // Avoid unnecessary resizing logic if minimized.

            InflateRect(&clientRect, -spacing, -spacing);

            WindowPosition windowPositions[] = {
                /* 00 */ WindowPosition(GetWindowFromId(hwnd, IdcDrawableObjectListLoad), PositionOptionsAlignTop),
                /* 01 */ WindowPosition(GetWindowFromId(hwnd, IdcDrawableObjectListStore), PositionOptionsAlignTop),
                /* 02 */ WindowPosition(GetWindowFromId(hwnd, IdcDrawableObjectCreate), PositionOptionsAlignTop),
                /* 03 */ WindowPosition(GetWindowFromId(hwnd, IdcDrawableObjectDelete), PositionOptionsAlignTop),
                /* 04 */ WindowPosition(GetWindowFromId(hwnd, IdcDrawableObjectMoveUp), PositionOptionsAlignTop),
                /* 05 */ WindowPosition(GetWindowFromId(hwnd, IdcDrawableObjectMoveDown), PositionOptionsAlignTop),
                /* 06 */ WindowPosition(GetWindowFromId(hwnd, IdcSelectFontFamily), PositionOptionsAlignTop),
                /* 07 */ WindowPosition(GetWindowFromId(hwnd, IdcSelectFontFile), PositionOptionsAlignTop),
                /* 08 */ WindowPosition(GetWindowFromId(hwnd, IdcSettingsLabel), PositionOptionsAlignTop),
                /* 09 */ WindowPosition(GetWindowFromId(hwnd, IdcSettingsHidden), PositionOptionsAlignTop),
                /* 10 */ WindowPosition(GetWindowFromId(hwnd, IdcSettingsLight), PositionOptionsAlignTop),
                /* 11 */ WindowPosition(GetWindowFromId(hwnd, IdcSettingsFull), PositionOptionsAlignTop),
                /* 12 */ WindowPosition(GetWindowFromId(hwnd, IdcAssortedActions), PositionOptionsAlignTop),
                /* 13 */ WindowPosition(GetWindowFromId(hwnd, IdcEditText), PositionOptionsFillWidth | PositionOptionsFillHeight | PositionOptionsAlignTop | PositionOptionsPreNewLine),
                /* 14 */ WindowPosition(GetWindowFromId(hwnd, IdcCanvasLabelsVisible), PositionOptionsAlignTop),
                /* 15 */ WindowPosition(GetWindowFromId(hwnd, IdcCanvasLabelsAdjacent), PositionOptionsAlignTop | PositionOptionsPostNewLine),
                /* 16 */ WindowPosition(GetWindowFromId(hwnd, IdcDrawableObjectsList), PositionOptionsFillWidth | PositionOptionsFillHeight | PositionOptionsPreNewLine),
                /* 17 */ WindowPosition(GetWindowFromId(hwnd, IdcAttributesFilterEdit), PositionOptionsFillWidth | PositionOptionsIgnored),
                /* 18 */ WindowPosition(GetWindowFromId(hwnd, IdcAttributesList), PositionOptionsFillWidth | PositionOptionsFillHeight),
                /* 19 */ WindowPosition(GetWindowFromId(hwnd, IdcAttributeValuesEdit), PositionOptionsFillWidth | PositionOptionsFillHeight | PositionOptionsIgnored),
                /* 20 */ WindowPosition(GetWindowFromId(hwnd, IdcAttributeValueSlider), PositionOptionsFillWidth | PositionOptionsIgnored),
                /* 21 */ WindowPosition(GetWindowFromId(hwnd, IdcAttributeValuesList), PositionOptionsFillWidth | PositionOptionsFillHeight),
                /* 22 */ WindowPosition(GetWindowFromId(hwnd, IdcDrawingCanvas), PositionOptionsFillWidth | PositionOptionsFillHeight | PositionOptionsAlignLeft | PositionOptionsPreNewLine),
                /* 23 */ WindowPosition(GetWindowFromId(hwnd, IdcLog), PositionOptionsFillWidth | PositionOptionsAlignTop | PositionOptionsPreNewLine),
            };
            WindowPosition& windowPositionEditText              = windowPositions[13];
            WindowPosition& windowPositionDrawableObjectsList   = windowPositions[16];
            WindowPosition& windowPositionAttributesFilterEdit  = windowPositions[17];
            WindowPosition& windowPositionAttributesList        = windowPositions[18];
            WindowPosition& windowPositionAttributeValuesEdit   = windowPositions[19];
            WindowPosition& windowPositionAttributeValuesSlider = windowPositions[20];
            WindowPosition& windowPositionAttributeValuesList   = windowPositions[21];

            // Apply initial overall resizing.
            WindowPosition::ReflowGrid(windowPositions, uint32_t(countof(windowPositions)), clientRect, spacing, 0, PositionOptionsNone);
            windowPositionDrawableObjectsList.ClampRect({0x7FFF,340});
            windowPositionAttributesList.ClampRect({340,340});
            windowPositionAttributeValuesList.ClampRect({340,340});
            windowPositionEditText.ClampRect({0x7FFF,160});
            WindowPosition::ReflowGrid(windowPositions, uint32_t(countof(windowPositions)), clientRect, spacing, 0, PositionOptionsNone);

            // Resize the objects edit and list controls.
            RECT attributesRect = windowPositionAttributesList.rect;
            windowPositionAttributesFilterEdit.options &= ~PositionOptionsIgnored;
            windowPositionAttributesList.SetOptions(PositionOptionsFillHeight, /*clear*/PositionOptionsAlignVMask | PositionOptionsUseSlackHeight);
            WindowPosition::ReflowGrid(&windowPositionAttributesFilterEdit, 2, attributesRect, /*spacing*/0, 0, PositionOptionsFlowVertical | PositionOptionsUnwrapped);

            // Resize the attribute values edit, slider, and list controls.
            RECT attributeValuesRect = windowPositionAttributeValuesList.rect;
            windowPositionAttributeValuesEdit.options &= ~PositionOptionsIgnored;
            #if 0 // todo::: enable slider
            windowPositionAttributeValuesSlider.options &= ~PositionOptionsIgnored;
            #endif
            windowPositionAttributeValuesList.SetOptions(PositionOptionsFillHeight, PositionOptionsAlignVMask | PositionOptionsUseSlackHeight);
            WindowPosition::ReflowGrid(&windowPositionAttributeValuesEdit, 3, attributeValuesRect, /*spacing*/0, 0, PositionOptionsFlowVertical | PositionOptionsUnwrapped);
            windowPositionAttributeValuesEdit.ClampRect({0x7FFF, (GetWindowStyle(windowPositionAttributeValuesEdit.hwnd) & ES_MULTILINE) ? 48*3/2 : 12*3/2});
            WindowPosition::ReflowGrid(&windowPositionAttributeValuesEdit, 3, attributeValuesRect, /*spacing*/0, 0, PositionOptionsFlowVertical | PositionOptionsUnwrapped);

            WindowPosition::Update(windowPositions, uint32_t(countof(windowPositions)));
        }
        break;

    case IdcDrawingCanvas:
        {
            hwnd = GetWindowFromId(hwnd_, IdcDrawingCanvas);
            GetClientRect(hwnd, &clientRect);
            if (IsRectEmpty(&clientRect))
                return; // Avoid unnecessary resizing logic if minimized.

            SCROLLINFO si = {};
            si.cbSize = sizeof(SCROLLINFO);

            const int barType[] = { SB_HORZ, SB_VERT };
            const int pageSize[] = { clientRect.right - clientRect.left, clientRect.bottom - clientRect.top };

            for (size_t i = 0; i < countof(barType); ++i)
            {
                si.fMask = SIF_PAGE;
                si.nPage = pageSize[i];
                SetScrollInfo(hwnd, barType[i], &si, TRUE);

                si.fMask = SIF_RANGE | SIF_POS;
                GetScrollInfo(hwnd, barType[i], &si);
            }
        }
        break;
    }
}


void MainWindow::OnAssortedActions(HWND anchorControl)
{
    TrackPopupMenu_Item constexpr static items[] = {
        {IdcSaveSelectedFontFile,  u"Save font file..."},
        {IdcExportGlyphImageData, u"Export all glyph image data..." },
        { 0, u"-" },
        {IdcGetAllFontCharacters, u"Get all font characters"},
        {IdcGetAllColorFontCharacters, u"Get all color font characters"},
        {IdcCopyAllFontCharacters, u"Copy all font characters to clipboard"},
        {0, u"-"},
        { IdcAutofitDrawableObjects, u"Autofit drawable objects" },
        {IdcAutofitDrawableObjectsUniformly, u"Autofit drawable objects uniformly" },
    };

    int menuId = TrackPopupMenu(
        make_array_ref(items, countof(items)),
        anchorControl,
        hwnd_
        );

    switch (menuId)
    {
    case IdcSaveSelectedFontFile: SaveSelectedFontFile(); break;
    case IdcGetAllFontCharacters: GetAllFontCharacters(/*copyToClipboardInstead*/false, /*getOnlyColorFontCharacters*/ false); break;
    case IdcGetAllColorFontCharacters: GetAllFontCharacters(/*copyToClipboardInstead*/false, /*getOnlyColorFontCharacters*/ true); break;
    case IdcCopyAllFontCharacters: GetAllFontCharacters(/*copyToClipboardInstead*/true, /*getOnlyColorFontCharacters*/ false); break;
    case IdcExportGlyphImageData: ExportFontGlyphData(); break;
    case IdcAutofitDrawableObjects: AutofitDrawableObjects(/*useMaximumWidth*/false, /*useMaximumHeight*/false); break;
    case IdcAutofitDrawableObjectsUniformly: AutofitDrawableObjects(/*useMaximumWidth*/true, /*useMaximumHeight*/true); break;
    }
}


void MainWindow::AppendLogCached(const char16_t* logMessage, ...)
{
    va_list argList;
    va_start(argList, logMessage);

    char16_t buffer[1000];
    buffer[0] = 0;
    StringCchVPrintf(OUT ToWChar(buffer), countof(buffer), ToWChar(logMessage), argList);

    cachedLog_ += buffer;
}


void MainWindow::AppendLogDirect(const char16_t* logMessage)
{
    HWND hwndLog = GetWindowFromId(hwnd_, IdcLog);

    uint32_t textLength = Edit_GetTextLength(hwndLog);
    if (!cachedLog_.empty())
    {
        Edit_SetSel(hwndLog, textLength, textLength);
        Edit_ReplaceSel(hwndLog, cachedLog_.c_str());
        cachedLog_.clear();
        textLength = Edit_GetTextLength(hwndLog);
    }
    Edit_SetSel(hwndLog, textLength, textLength);
    Edit_ReplaceSel(hwndLog, logMessage);
}


void MainWindow::AppendLog(const char16_t* logMessage, ...)
{
    va_list argList;
    va_start(argList, logMessage);
    
    char16_t buffer[1000];
    buffer[0] = 0;
    StringCchVPrintf(OUT ToWChar(buffer), countof(buffer), ToWChar(logMessage), argList);

    AppendLogDirect(buffer);
}


HRESULT MainWindow::ShowMessageIfError(const char16_t* logMessage, HRESULT hr)
{
    if (FAILED(hr))
    {
        MainWindow::ShowMessageAndAppendLog(logMessage, hr);
    }
    return hr;
}


void MainWindow::ShowMessageAndAppendLog(const char16_t* logMessage, ...)
{
    va_list argList;
    va_start(argList, logMessage);

    char16_t buffer[1000];
    buffer[0] = 0;
    StringCchVPrintf(OUT ToWChar(buffer), countof(buffer), ToWChar(logMessage), argList);

    HWND__* h = reinterpret_cast<HWND__*>(hwnd_); // hack:::
    MessageBoxShaded::Show(h, buffer, u"", MB_OK|MB_ICONWARNING);

    AppendLogDirect(buffer);
    if (buffer[wcslen(ToWChar(buffer))] != '\n')
        AppendLogDirect(u"\r\n");
}


void MainWindow::ClearLog()
{
    HWND hwndLog = GetWindowFromId(hwnd_, IdcLog);
    Edit_SetText(hwndLog, L"");

    std::u16string().swap(cachedLog_);
}
