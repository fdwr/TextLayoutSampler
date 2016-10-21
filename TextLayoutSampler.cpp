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

#if 0
#include <stdint.h>

struct BackwardsEndianUint32
{
    constexpr BackwardsEndianUint32(uint32_t n)
        :   value_{((n>>24)&255),((n>>16)&255),((n>>8)&255),((n>>0)&255)}
    {
    }

    // Explicit getter returning native type. Converts from file representation,
    // which is little-endian.
    uint32_t Get() const throw()
    {
        return 
            (static_cast<uint32_t>(value_[3]) << 24) | 
            (static_cast<uint32_t>(value_[2]) << 16) | 
            (static_cast<uint32_t>(value_[1]) << 8) | 
            value_[0];
    }

    // Implicit conversion to native type.
    operator uint32_t() const throw()
    {
        return Get();
    }

    void Set(uint32_t v) throw()
    {
        value_[0] = uint8_t(v);
        value_[1] = uint8_t(v >> 8);
        value_[2] = uint8_t(v >> 16);
        value_[3] = uint8_t(v >> 24);
    }

    BackwardsEndianUint32& operator =(uint32_t v) throw()
    {
        Set(v);
        return *this;
    }

    uint8_t value_[4];
};


static const BackwardsEndianUint32 g_n[] = {23};
#endif

#if 0 // todo:delete
void ExportSvgs(char16_t const* fileInputPath, char16_t const* fileOutputPattern)
{
    std::vector<uint8_t> fileBytes;
    ReadBinaryFile(fileInputPath, OUT fileBytes);
    auto fileBytesArrayRef = make_array_ref(fileBytes);
    auto recastFileBytes = fileBytesArrayRef.reinterpret_as<char>();
    std::string fileChars(recastFileBytes.data(), recastFileBytes.size());
    std::string xmlHeader("<?xml");
    std::u16string filePath;
    int i = 0;
    for (size_t begin = 0; begin < fileChars.size(); )
    {
        auto end = fileChars.find(xmlHeader, begin+1);
        if (end == std::string::npos)
        {
            end = fileChars.size();
        }
        GetFormattedString(OUT filePath, fileOutputPattern, i);
        WriteBinaryFile(filePath.data(), fileChars.data() + begin, end - begin);
        begin = end;
        ++i;
    }
}

//ExportSvgs(u"D:/fonts/color/svg/Emoji One/EmojiOne-Regular_10014h_BC6DF4h.svg", u"D:/fonts/color/svg/Emoji One/EmojiOne-Regular_%003d.svg");
//ExportSvgs(u"D:/fonts/color/svg/Segoe UI Emoji/seguiemj_SVG_EC878h_2CE4FEh.svg", u"D:/fonts/color/svg/Segoe UI Emoji/seguiemj_SVG_%003d.svg");
//ExportSvgs(u"D:/fonts/color/svg/Source Code Pro/SourceCodePro-Regular_2CF70h_1BB7h.svg", u"D:/fonts/color/svg/Source Code Pro/SourceCodePro-Regular_%003d.svg");
//ExportSvgs(u"D:/fonts/color/svg/Trajan Color/TrajanColor-SharedSVG_A90h_8352h.otf.svg", u"D:/fonts/color/svg/Trajan Color/TrajanColor-SharedSVG_%003d.svg");


#endif

#if 0 // todo:delete
void ExportPngs(char16_t const* fileInputPath, char16_t const* fileOutputPattern)
{
    std::vector<uint8_t> fileBytes;
    ReadBinaryFile(fileInputPath, OUT fileBytes);
    auto fileBytesArrayRef = make_array_ref(fileBytes);
    auto recastFileBytes = fileBytesArrayRef.reinterpret_as<char>();
    std::string fileChars(recastFileBytes.data(), recastFileBytes.size());
    std::string pngHeader("/x0089PNG");
    std::u16string filePath;
    int i = 0;
    size_t begin = fileChars.find(pngHeader, 0);
    while (begin < fileChars.size())
    {
        auto end = fileChars.find(pngHeader, begin+1);
        if (end == std::string::npos)
        {
            end = fileChars.size();
        }
        GetFormattedString(OUT filePath, fileOutputPattern, i);
        WriteBinaryFile(filePath.data(), fileChars.data() + begin, end - begin);
        begin = end;
        ++i;
    }
}

//ExportPngs(u"d:/fonts/color/sbix/Apple Color Emoji 2015-08-28.ttf", u"d:/fonts/color/sbix/Apple Color Emoji Pngs/Apple Color Emoji 2015-08-28_%003d.png");
//ExportPngs(u"d:/fonts/color/sbix/Ringo-Blingo-sbixOFL-2.003.ttf", u"d:/fonts/color/sbix/Ringo-Blingo Pngs/Ringo-Blingo-sbixOFL-2.003_%003d.png");
//ExportPngs(u"C:/Windows/Resources/Themes/aero/aero.msstyles", u"c:/temp/aero/aero_%003d.png");



#endif



#if 0

__interface DECLSPEC_UUID("100cad4e-d6af-4c9e-8a08-d695b11caa49") IFoo
{
    virtual int f1() abstract;
    virtual int f2() abstract;
};

__interface DECLSPEC_UUID("200cad4e-d6af-4c9e-8a08-d695b11caa49") IFooA : IFoo
{
    virtual int f3() abstract;
    virtual int f4() abstract;
};

__interface DECLSPEC_UUID("300cad4e-d6af-4c9e-8a08-d695b11caa49") IFooA2 : IFooA
{
    virtual int f5() abstract;
    virtual int f6() abstract;
};

__interface DECLSPEC_UUID("400cad4e-d6af-4c9e-8a08-d695b11caa49") IFooB : IFoo
{
    virtual int f7() abstract;
    virtual int f8() abstract;
};

class Foo : public IFooA2, public IFooB
{
    virtual int f1() { return 1; }
    virtual int f2() { return 2; }
    virtual int f3() { return 3; }
    virtual int f4() { return 4; }
    virtual int f5() { return 5; }
    virtual int f6() { return 6; }
    virtual int f7() { return 7; }
    virtual int f8() { return 8; }
};


#if 0
template <typename T0, typename T1>
class TypeListNode
{
public:
    typedef T0 Head;
    typedef T1 Tail;
};

template <typename... Ts>
struct TypeList
{};

template <typename T, typename... Ts>
struct TypeList<T, Ts...> : TypeList<Ts...>
{
    //TypeList(T t, Ts... ts) : TypeList<Ts...>(ts...), tail(t) {}
public:
    using Head = T0;
    using Tail = T1;
};
#endif

struct GuidAndPointerAdjustment
{
    GUID guid;
    ptrdiff_t pointerAdjustment;

    constexpr void* AdjustPointer(void* p) const throw()
    {
        return reinterpret_cast<uint8_t*>(p) + pointerAdjustment;
    }
};

template <typename BaseType, typename SupportedType>
ptrdiff_t GetPointerAdjustment()
{
    return reinterpret_cast<uint8_t*>(reinterpret_cast<BaseType*>(42)) // Use any non-zero constant
         - reinterpret_cast<uint8_t*>(static_cast<SupportedType*>(reinterpret_cast<BaseType*>(42)));
}

template <typename... Ts> struct MyTypeList {};

// Variable size array of GUID and pointer adjustments.
template <typename BaseType, typename SupportedType, typename... Ts>
struct MyTypeList<BaseType, SupportedType, Ts...> : MyTypeList<BaseType, Ts...>
{
    constexpr MyTypeList()
    :   MyTypeList<BaseType, Ts...>(),
        entry{ __uuidof(SupportedType), GetPointerAdjustment<BaseType, SupportedType>()}
    {}

    GuidAndPointerAdjustment entry;
};

static_assert(sizeof(GuidAndPointerAdjustment) == sizeof(MyTypeList<IUnknown>), "Must be same size to treat as if array.");

size_t sg = sizeof();
size_t smt = sizeof();

HRESULT QueryInterfaceImpl(
    IID const& iid,
    array_ref<GuidAndPointerAdjustment const> interfaces,
    void* originalObjectPointer,
    _Out_ void** newObjectPointer
    )
{
    for (auto& i : interfaces)
    {
        if (i.guid == iid)
        {
            *newObjectPointer = i.AdjustPointer(originalObjectPointer);
            return S_OK;
        }
    }
    *newObjectPointer = nullptr;
    return E_NOTIMPL;
}


using MyInterfaceList = MyTypeList<Foo, IFooA, IFooA2, IFooB>;
static MyInterfaceList g_myInterfaceList;


class IThing
{
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(IID const& iid, _Out_ void** object) throw()
    {
        auto interfaceArrayRef = make_array_ref(&g_myInterfaceList.entry, sizeof(g_myInterfaceList) / sizeof(g_myInterfaceList.entry));
        return QueryInterfaceImpl(iid, interfaceArrayRef, this, object);
    }
};


//
//template <typename ObjectType>
//void MyQueryInterface(ObjectType* object, void** newObject)
//{
//    *newObject = nullptr;
//}


void foo(IUnknown* i)
{
    ;
}


void foo()
{
    IDWriteTextLayout2* tl = nullptr;
    foo(tl);

    Foo f;

    size_t s = sizeof(MyInterfaceList);
    size_t sg = sizeof(GuidAndPointerAdjustment);
    size_t smt = sizeof(MyTypeList<IUnknown>);
    size_t si = sizeof(g_myInterfaceList);
    s; sg; smt; si;

    Foo*    foo    = static_cast<Foo*>(&f);
    IFoo*   ifoo   = static_cast<IFoo*>(static_cast<IFooA*>(&f));
    IFooA*  ifooa  = static_cast<IFooA*>(&f);
    IFooA2* ifooa2 = static_cast<IFooA2*>(&f);
    IFooB*  ifoob  = static_cast<IFooB*>(&f);

    foo    ;
    ifoo   ;
    ifooa  ;
    ifooa2 ;
    ifoob  ;

    Foo* bogusfoo = reinterpret_cast<Foo*>(65536);
    Foo*    nullptrfoo    = static_cast<Foo*>(static_cast<Foo*>(bogusfoo));
    IFoo*   nullptrifoo   = static_cast<IFoo*>(static_cast<IFooA*>(static_cast<Foo*>(bogusfoo)));
    IFooA*  nullptrifooa  = static_cast<IFooA*>(static_cast<Foo*>(bogusfoo));
    IFooA2* nullptrifooa2 = static_cast<IFooA2*>(static_cast<Foo*>(bogusfoo));
    IFooB*  nullptrifoob  = static_cast<IFooB*>(static_cast<Foo*>(bogusfoo));

    nullptrfoo    ;
    nullptrifoo   ;
    nullptrifooa  ;
    nullptrifooa2 ;
    nullptrifoob  ;
}



#endif

#if 0
DEFINE_ENUM_FLAG_OPERATORS(Gdiplus::StringFormatFlags);
#endif

#if 0

template<typename T>
class arf
{
public:
    // All array_ref's are friends of each other to enable constructors to
    // read other variants, mainly for copy constructors from a non-const to
    // const array_ref but also for reinterpret_reset.
    template <typename U>
    friend class arf;

    using TwithoutConst = std::remove_const<const int>::type;
    using TwithConst = T const;

    // construct/copy
    arf() = default;
    arf(int a) : x(a) {}
    arf(arf<typename T> const& other) = default;

    //template<
    //    //typename T2,
    //    //std::is_same<TwithoutConst, TwithConst>::value 
    //    typename = std::enable_if<false>::type
    //>
    //arf(arf<TwithoutConst> const& other) { return static_cast<arf<T> const&>(other);}
    //arf(arf<int const> const& other) { return static_cast<arf<T> const&>(other);}
    //template<typename InType>
    ////arf(typename std::enable_if< true, T >::type other)
    ////arf(typename std::enable_if< !(std::is_same<InType, arf<T> >::value), arf<T> >::type const& a)
    //arf(arf<int> const& a)
    ////arf(typename std::enable_if< std::is_same<TwithoutConst, TwithConst>::value, T >::type other)
    //{
    //    x = a.x;
    //    //
    //}

    template<
        typename InType,
        typename = std::enable_if_t<!std::is_same< arf<T>, InType>::value > // || !std::is_same<T, decltype(std::data(ContiguousContainer))> >
    >
    arf(InType const& a)
    {
        x = a.x;
        //
    }

    template<
        typename InType,
        typename = std::enable_if_t<!std::is_same< arf<T>, InType>::value > // || !std::is_same<T, decltype(std::data(ContiguousContainer))> >
    >
    arf(InType const& a)
    {
        x = a.x;
        //
    }
    //arf<TwithoutConst> GetThing() { return arf<TwithoutConst>();};
    //operator arf<TwithoutConst>() { return arf<TwithoutConst>();};
    //arf(arf<int const> const& other) = default;
#if 0
    //arf(pointer array, size_t elementCount) : begin_(array), end_(array + elementCount) {}
    //arf(pointer begin, pointer end) : begin_(begin), end_(end) {}

    //  >

    template<
        typename ContiguousContainer,
        typename = std::enable_if_t<!std::is_base_of<arf<T>, ContiguousContainer>::value > // || !std::is_same<T, decltype(std::data(ContiguousContainer))> >
        > arf(ContiguousContainer& container)
    {
        x = 2;
    }

    // todo::: std::enable_if<std::is_same<T1, T2>::value>::type
    //typename std::enable_if<
    //    std::is_base_of<
    //    implementation,
    //    typename std::remove_reference<T>::type
    //    >::value,
    //    void
    //>::type* dummy = 0
#endif

    int x = 0;
};

//void foo(arf<int> a)
//{
//    arf<int const> b;
//    arf<int> b;
//    arf<int> c = b;
//    arf<int const> d = b;
//}
#endif

#if 0
struct Cat
{
    int y;

    Cat(int x)
    {
        y = x;
    }

    Cat()
    :   Cat(13)
    {}

};
#endif


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
