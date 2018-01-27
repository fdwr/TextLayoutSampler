//----------------------------------------------------------------------------
//  Author:     Dwayne Robinson
//  History:    2015-06-19 Created
//----------------------------------------------------------------------------
#pragma once


inline bool IsSurrogate(char32_t ch) throw()
{
    // 0xD800 <= ch <= 0xDFFF
    return (ch & 0xF800) == 0xD800;
}

inline bool IsLeadingSurrogate(char32_t ch) throw()
{
    // 0xD800 <= ch <= 0xDBFF
    return (ch & 0xFC00) == 0xD800;
}

inline bool IsTrailingSurrogate(char32_t ch) throw()
{
    // 0xDC00 <= ch <= 0xDFFF
    return (ch & 0xFC00) == 0xDC00;
}

inline bool IsCharacterBeyondBmp(char32_t ch) throw()
{
    return ch >= 0x10000;
}

inline char32_t MakeUnicodeCodePoint(char32_t high, char32_t low) throw()
{
    return ((high & 0x03FF) << 10 | (low & 0x03FF)) + 0x10000;
}

// Split into leading and trailing surrogatse.
// From http://unicode.org/faq/utf_bom.html#35
inline char16_t GetLeadingSurrogate(char32_t ch)
{
    return char16_t(0xD800 + (ch >> 10)  - (0x10000 >> 10));
}

inline char16_t GetTrailingSurrogate(char32_t ch)
{
    return char16_t(0xDC00 + (ch & 0x3FF));
}

inline bool IsHexDigit(char32_t ch) throw()
{
    return (ch >= '0' && ch <= '9') || (ch &= ~32, ch >= 'A' && ch <= 'F');
}

enum UnicodeCodePoint
{
    UnicodeSpace                    = 0x000020,
    UnicodeNbsp                     = 0x0000A0,
    UnicodeSoftHyphen               = 0x0000AD,
    UnicodeEnQuadSpace              = 0x002000,
    UnicodeZeroWidthSpace           = 0x00200B,
    UnicodeDottedCircle             = 0x0025CC,
    UnicodeIdeographicSpace         = 0x003000,
    UnicodeInlineObject             = 0x00FFFC,   // for embedded objects
    UnicodeReplacementCharacter     = 0x00FFFD,   // for invalid sequences
    UnicodeMax                      = 0x10FFFF,
    UnicodeTotal                    = 0x110000,
};

char16_t const* SkipSpaces(_In_z_ char16_t const* stringValue) throw();
char16_t const* SkipToNextWord(_In_z_ char16_t const* stringValue) throw();
char16_t const* SkipToEnd(_In_z_ char16_t const* stringValue) throw();

// Replaces unpaired surrogates with U+FFFD.
_Out_range_(0, utf32text.end_ - utf32text.begin_)
size_t ConvertTextUtf16ToUtf32(
    array_ref<char16_t const> utf16text,
    OUT array_ref<char32_t> utf32text,
    _Out_opt_ size_t* sourceCount
    ) throw();

// Carries unpaired surrogates through, for testing of API behavior.
_Out_range_(0, utf32text.end_ - utf32text.begin_)
size_t ConvertTextUtf16ToUtf32NoReplacement(
    array_ref<char16_t const> utf16text,
    OUT array_ref<char32_t> utf32text,
    _Out_opt_ size_t* sourceCount
    ) throw();

_Out_range_(0, return)
size_t ConvertUtf32ToUtf16(
    array_ref<char32_t const> utf32text,
    OUT array_ref<char16_t> utf16text
    ) throw();

// Consumes the byte order mark.
void ConvertTextUtf8ToUtf16(
    array_ref<char const> utf8text,
    OUT std::u16string& utf16text
    );

// Prepends a byte order mark.
void ConvertTextUtf16ToUtf8(
    array_ref<char16_t const> utf16text,
    OUT std::string& utf8text
    );

void GetFormattedString(_Inout_ std::u16string& returnString, bool shouldConcatenate, _In_z_ const char16_t* formatString, va_list vargs);
void GetFormattedString(_Out_ std::u16string& returnString, _In_z_ const char16_t* formatString, ...);
void AppendFormattedString(_Inout_ std::u16string& returnString, _In_z_ const char16_t* formatString, ...);
void TrimSpaces(_Inout_ std::u16string& text);
void UnquoteString(_Inout_ std::u16string& path);
void ToUpperCase(_Inout_ array_ref<char16_t> s);
void UnescapeCppUniversalCharacterNames(array_ref<char16_t const> escapedText, OUT std::u16string& expandedText);
void UnescapeHtmlNamedCharacterReferences(array_ref<char16_t const> escapedText, OUT std::u16string& expandedText);
void EscapeCppUniversalCharacterNames(array_ref<char16_t const> text, OUT std::u16string& escapedText);
void EscapeHtmlNamedCharacterReferences(array_ref<char16_t const> text, OUT std::u16string& escapedText);
void RemoveTrailingZeroes(_Inout_ std::u16string& text) throw();
void WriteZeroPaddedHexNum(uint32_t value, OUT array_ref<char16_t> buffer);
uint32_t ReadUnsignedNumericValue(_Inout_ array_ref<char16_t const>& text, uint32_t base); // Unlike wcstoul, respects length limit, and doesn't throw exception!
array_ref<wchar_t> ToWString(int32_t value, OUT array_ref<wchar_t> s);

static_assert(sizeof(wchar_t) == sizeof(char16_t), "These casts only work on platforms where wchar_t is 16 bits.");
inline wchar_t* ToWChar(char16_t* p) { return reinterpret_cast<wchar_t*>(p); }
inline wchar_t** ToWChar(char16_t** p) { return reinterpret_cast<wchar_t**>(p); }
inline wchar_t const* ToWChar(char16_t const* p) { return reinterpret_cast<wchar_t const*>(p); }
inline wchar_t const** ToWChar(char16_t const** p) { return reinterpret_cast<wchar_t const**>(p); }
inline char16_t* ToChar16(wchar_t* p) { return reinterpret_cast<char16_t*>(p); }
inline char16_t const* ToChar16(wchar_t const* p) { return reinterpret_cast<char16_t const*>(p); }
inline char16_t const** ToChar16(wchar_t const** p) { return reinterpret_cast<char16_t const**>(p); }

uint32_t IntLen(_In_z_ char16_t const* text);

array_ref<char16_t const> ToChar16ArrayRef(_In_z_ char16_t const* text);

struct UnicodeCharacterReader : public array_ref<char16_t const>
{
    UnicodeCharacterReader() = default;

    UnicodeCharacterReader(array_ref::pointer begin, array_ref::pointer end) : array_ref(begin, end)
    {}

    // todo: Inheriting constructors causes Visual Studio 15.4.1 compiler to crash when calling base class size().
    //using array_ref::array_ref;

    bool IsAtEnd()
    {
        return begin_ >= end_;
    }

    char32_t ReadNext()
    {
        if (begin_ >= end_)
            return 0;

        char32_t ch = *begin_;
        ++begin_;

        // Just use the character if not a surrogate code point.
        // For unpaired surrogates, pass the isolated surrogate
        // through (rather than remap to U+FFFD).
        if (IsLeadingSurrogate(ch) && begin_ < end_)
        {
            char32_t leading = ch;
            char32_t trailing = *begin_;
            if (IsTrailingSurrogate(trailing))
            {
                ch = MakeUnicodeCodePoint(leading, trailing);
                ++begin_;
            }
        }

        return ch;
    }
};
