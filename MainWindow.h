//----------------------------------------------------------------------------
//  Author:     Dwayne Robinson
//  History:    2015-06-19 Created
//----------------------------------------------------------------------------
#pragma once


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

    struct FontFamilyNameProperties;

    HRESULT LoadTextFileIntoDrawableObjects(_In_z_ char16_t const* filePath);
    HRESULT StoreTextFileFromDrawableObjects(_In_z_ char16_t const* filePath);
    HRESULT LoadFontFileIntoDrawableObjects(_In_z_ char16_t const* filePath);
    HRESULT LoadDrawableObjectsSettings(_In_z_ char16_t const* filePath, bool clearExistingItems = true, bool merge = false);
    HRESULT StoreDrawableObjectsSettings(_In_z_ char16_t const* filePath);
    HRESULT SaveSelectedFontFile();
    HRESULT ExportFontGlyphData();
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
    void DeferUpdateUi(NeededUiUpdate neededUiUpdate = NeededUiUpdateNone);
    void UpdateDrawableObjectsListView();
    void DeleteDrawableObjectsListViewSelected();
    void CreateDrawableObjectsListViewSelected();
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
    std::vector<uint32_t> GetSelectedDrawableObjectIndices();
    std::vector<uint32_t> GetSelectedAttributeIndices();
    HRESULT GetFileOrFamilyName(uint32_t selectedDrawableObjectIndex, _Out_ std::u16string& fileOrFamilyName);

////////////////////////////////////////
// Miscellaneous public.

public:
    void AppendLogCached(const char16_t* logMessage, ...);
    void AppendLog(const char16_t* logMessage, ...);
    void AppendLogDirect(const char16_t* logMessage);
    void ShowMessageAndAppendLog(const char16_t* logMessage, ...);
    HRESULT ShowMessageIfError(const char16_t* logMessage, HRESULT hr);
    void ClearLog();

////////////////////////////////////////
// Internal functions.

protected:
    MainWindow::DialogProcResult CALLBACK OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
    MainWindow::DialogProcResult CALLBACK OnNotification(HWND hwnd, int controlId, NMHDR* notifyMessageHeader);
    MainWindow::DialogProcResult CALLBACK OnVerticalScroll(HWND hwnd, HWND controlHandle, UINT code, int position);
    MainWindow::DialogProcResult CALLBACK ProcessDragAndDrop(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void OnHorizontalOrVerticalScroll(HWND hwnd, int barType, UINT code, int smallStep);
    void OnAssortedActions(HWND anchorControl);

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

    std::vector<DrawableObjectAndValues> drawableObjects_;

};

DEFINE_ENUM_FLAG_OPERATORS(MainWindow::NeededUiUpdate);
