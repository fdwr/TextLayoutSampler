//----------------------------------------------------------------------------
//  History:    2015-06-19 Dwayne Robinson - Created
//----------------------------------------------------------------------------
#pragma once


struct WindowDpiScaler
{
    int32_t dpiX = 96;
    int32_t dpiY = 96;

    void UpdateDpi(HWND hwnd)
    {
        // Ideally we'd support multiple monitors of varying DPI's...
        //
        //      HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
        //      HRESULT GetDpiForMonitor(honitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
        //
        // But GetDpiForMonitor is unavailable on Windows 7.
        // So hopefully an HDC suitable for the current window is good enough:

        HDC hdc = GetDC(hwnd);
        dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
        dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
        ReleaseDC(hwnd, hdc);
    }

    int32_t ScaleSizeX(int value) {return value * dpiX / 96;};
    int32_t ScaleSizeY(int value) {return value * dpiY / 96;};
};

class MainWindow
{
public:
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

////////////////////////////////////////
// Windowing related.

public:
    MainWindow(HWND hwnd)
        :   hwnd_(hwnd)
    { }

    static HWND Create();

    static INT_PTR CALLBACK StaticDialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    HWND GetHwnd() const;
    static MainWindow* GetClass(HWND hwnd);

    enum NeededUiUpdate : uint32_t
    {
        NeededUiUpdateNone = 0,
        NeededUiUpdateDrawableObjectsListView = 1,
        NeededUiUpdateDrawableObjectsCanvas = 2,
        NeededUiFillAttributesListView = 4,
        NeededUiUpdateAttributesListView = 8,
        NeededUiUpdateAttributeValuesListView = 16,
        NeededUiUpdateAttributeValuesSlider = 32,
        NeededUiUpdateAttributeValuesEdit = 64,
        NeededUiUpdateTextEdit = 128,
    };

    enum SettingsVisibility
    {
        SettingsVisibilityHidden,
        SettingsVisibilityLight,
        SettingsVisibilityFull,
        SettingsVisibilityTotal
    };

    enum TextEscapeMode
    {
        TextEscapeModeNone,     // Raw verbatim UTF-16
        TextEscapeModeCppUcn,   // C++ Universal character names
        TextEscapeModeHtmlNcr,  // HTML Numeric character references
        TextEscapeModeHtmlTotal
    };

    struct FontFamilyNameProperties;

    void InitializeDefaultDrawableObjects();
    HRESULT LoadTextFileIntoDrawableObjects(_In_z_ char16_t const* filePath);
    HRESULT StoreTextFileFromDrawableObjects(_In_z_ char16_t const* filePath);
    HRESULT LoadFontFileIntoDrawableObjects(_In_z_ char16_t const* filePath);
    HRESULT LoadDrawableObjectsSettings(_In_z_ char16_t const* filePath, bool clearExistingItems = true, bool merge = false);
    HRESULT StoreDrawableObjectsSettings(_In_z_ char16_t const* filePath);
    HRESULT SaveSelectedFontFile();
    HRESULT SaveUnpackedWoffFontFile();
    HRESULT ExportFontGlyphData();
    void    SetWindowTranslucency(uint32_t alpha);
    HRESULT AutofitDrawableObjects(bool useMaximumWidth, bool useMaximumHeight);
    HRESULT SetNoLineWrapOnDrawableObjects();
    HRESULT GetSelectedDrawableObject(_Out_ uint32_t& selectedDrawableObject); // S_FALSE and ~0 if none.
    HRESULT GetAllFontCharacters(bool copyToClipboardInstead, bool getOnlyColorFontCharacters);
    HRESULT GetLogFontFromDrawableObjects(_Out_ LOGFONT& logFont);
    HRESULT UpdateDrawableObjectsFromFontFamilyNameProperties(FontFamilyNameProperties const& fontFamilyNameProperties);

protected:
    MainWindow::DialogProcResult CALLBACK DialogProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    static void RegisterCustomClasses();

    INT_PTR InitializeMainDialog();
    void InitializeDrawableObjectsListView();
    void InitializeAttributesListView();
    void FillAttributesListView();
    void InitializeAttributeValuesListView();
    void UpdateUi();
    void DeferUpdateUi(NeededUiUpdate neededUiUpdate = NeededUiUpdateNone, uint32_t timeOut = 50);
    void UpdateDrawableObjectsListView();
    void DeleteDrawableObjectsListViewSelected();
    void CreateDrawableObjectsListViewSelected();
    void EnsureAtLeastOneDrawableObject();
    void ShiftSelectedDrawableObjects(int32_t shiftDirection /* down = positive, up = negative */);
    HRESULT LoadDrawableObjectsSettings(bool clearExistingItems = true, bool merge = false);
    HRESULT StoreDrawableObjectsSettings();
    void UpdateAttributesListView();
    void UpdateAttributeValuesListView();
    void UpdateAttributeValuesEdit();
    void UpdateAttributeValuesSlider();
    void UpdateTextEdit();
    void ReadAttributeFilterEdit();
    void ReadAttributeValueEdit();
    void ChangeSettingsVisibility(SettingsVisibility settingsVisibility);
    void UpdateDrawableObjectValuesUsing(std::u16string const& newValueString);
    void RepaintDrawableObjects();
    void Resize(int id);
    HRESULT SelectFontFile();
    HRESULT SelectFontFamily();
    std::vector<uint32_t> GetSelectedDrawableObjectIndices(bool returnAllIfEmpty = true);
    std::vector<uint32_t> GetSelectedAttributeIndices();
    HRESULT GetFileOrFamilyName(uint32_t selectedDrawableObjectIndex, _Out_ std::u16string& fileOrFamilyName);

////////////////////////////////////////
// Miscellaneous public.

public:
    void AppendLogCached(char16_t const* logMessage, ...);
    void AppendLog(char16_t const* logMessage, ...);
    void AppendLogUnformatted(char16_t const* logMessage);
    void ShowMessageAndAppendLog(char16_t const* logMessage, ...);
    void ShowMessageAndAppendLogUnformatted(char16_t const* logMessage);
    HRESULT ShowMessageIfError(char16_t const* logMessage, HRESULT hr, ...);

    void ClearLog();

////////////////////////////////////////
// Internal functions.

protected:
    MainWindow::DialogProcResult CALLBACK OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
    MainWindow::DialogProcResult CALLBACK OnNotification(HWND hwnd, int controlId, NMHDR* notifyMessageHeader);
    MainWindow::DialogProcResult CALLBACK OnVerticalScroll(HWND hwnd, HWND controlHandle, UINT code, int position);
    MainWindow::DialogProcResult CALLBACK OnDragAndDrop(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void OnHorizontalOrVerticalScroll(HWND hwnd, int barType, UINT code, int smallStep);
    void OnAssortedActions(HWND anchorControl);
    void OnTextEscapeMode(HWND anchorControl);
    void SetTextEscapeMode(TextEscapeMode textEscapeMode);
    void UnescapeText(IN OUT std::u16string& text);
    void EscapeText(IN OUT std::u16string& text);

    HWND GetSubdialogItem(int dialogId, int childId);

    ////////////////////////////////////////
protected:
    HWND hwnd_;
    static HACCEL g_accelTable;
    std::u16string cachedLog_;
    bool isRecursing_ = false;
    bool isTypingAttributeValueToFilter_ = false; // Was recently typing a character into the value edit field.
    NeededUiUpdate neededUiUpdate_ = NeededUiUpdateNone;
    DrawableObjectAttribute selectedAttributeIndex_ = DrawableObjectAttributeTotal;
    SettingsVisibility settingsVisibility_ = SettingsVisibilityLight;
    std::u16string attributeFilter_;
    std::u16string selectedAttributeValue_;
    std::u16string previousSettingsFilePath_;
    TextEscapeMode textEscapeMode_ = TextEscapeModeNone;
    WindowDpiScaler dpiScaler_;

    std::vector<DrawableObjectAndValues> drawableObjects_;
};

DEFINE_ENUM_FLAG_OPERATORS(MainWindow::NeededUiUpdate);
