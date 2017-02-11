//+---------------------------------------------------------------------------
//
//  Copyright (c) Microsoft, 2008. All rights reserved.
//
//  Contents:   Text Layout test app.
//
//  Author:     Dwayne Robinson (dwayner@microsoft.com)
//
//  History:    2008-02-11   dwayner    Created
//
//----------------------------------------------------------------------------
#include "precomp.h"
#include "Common.String.h"


array_ref<char16_t const> ToChar16ArrayRef(_In_z_ char16_t const* text)
{
    return make_array_ref(text, wcslen(ToWChar(text)));
}


uint32_t IntLen(_In_z_ char16_t const* text)
{
    return static_cast<uint32_t>(wcslen(ToWChar(text)));
}


char16_t const* SkipSpaces(_In_z_ char16_t const* stringValue) throw()
{
    for (char16_t ch; ch = *stringValue, ch != '\0' && ch == ' '; ++stringValue)
    { }
    return stringValue;
}


char16_t const* SkipToNextWord(_In_z_ char16_t const* stringValue) throw()
{
    for (char16_t ch; ch = *stringValue, ch != '\0' && (ch != ' ' && ch != ','); ++stringValue)
    { }
    stringValue = SkipSpaces(stringValue);
    if (*stringValue == ',')
    {
        ++stringValue;
        stringValue = SkipSpaces(stringValue);
    }
    return stringValue;
}


char16_t const* SkipToEnd(_In_z_ char16_t const* stringValue) throw()
{
    for (char16_t ch; (ch = *stringValue, ch != '\0'); ++stringValue)
    { }
    return stringValue;
}


void RemoveTrailingZeroes(std::u16string& text) throw()
{
    while (!text.empty() && text.back() == '0')
    {
        text.pop_back();
    }
    while (!text.empty() && text.back() == '.')
    {
        text.pop_back();
    }
}


array_ref<wchar_t> to_wstring(int32_t value, OUT array_ref<wchar_t> s)
{
    auto charactersWritten = swprintf_s(s.data(), s.size(), L"%d", value);
    return make_array_ref(s.data(), std::max(charactersWritten, 0));
}


// Internal version. Should call the other two overloads publicly.
void GetFormattedString(_Inout_ std::u16string& returnString, bool shouldConcatenate, _In_z_ const char16_t* formatString, va_list vargs) 
{
    if (formatString != nullptr)
    {
        const size_t increasedLen = _vscwprintf(ToWChar(formatString), vargs) + 1; // Get string length, plus one for NUL
        const size_t oldLen = shouldConcatenate ? returnString.size() : 0;
        const size_t newLen = oldLen + increasedLen;
        returnString.resize(newLen);
        _vsnwprintf_s(ToWChar(&returnString[oldLen]), increasedLen, increasedLen, ToWChar(formatString), vargs);
        returnString.resize(newLen - 1); // trim off the NUL
    }
}


void GetFormattedString(_Out_ std::u16string& returnString, _In_z_ const char16_t* formatString, ...) 
{
    va_list vargs = nullptr;
    va_start(vargs, formatString); // initialize variable arguments
    GetFormattedString(OUT returnString, /*shouldConcatenate*/false, formatString, vargs);
    va_end(vargs); // Reset variable arguments
}


void AppendFormattedString(_Inout_ std::u16string& returnString, _In_z_ const char16_t* formatString, ...) 
{
    va_list vargs = nullptr;
    va_start(vargs, formatString); // initialize variable arguments
    GetFormattedString(OUT returnString, /*shouldConcatenate*/true, formatString, vargs);
    va_end(vargs); // Reset variable arguments
}


void TrimSpaces(_Inout_ std::u16string& text)
{
    // Trim space (U+0020) and tab. It does not trim all whitespace, like U+200X
    // or the new line controls.

    // Trim trailing spaces
    size_t lastPos = text.find_last_not_of(u" \t");
    if (lastPos != std::string::npos)
    {
        text.erase(lastPos+1);
    }

    // Trim leading spaces
    size_t firstPos = text.find_first_not_of(u" \t");
    if (firstPos != 0)
    {
        if (firstPos == std::string::npos)
            firstPos = text.size();
        text.erase(0, firstPos);
    }
}


void UnquoteString(_Inout_ std::u16string& path)
{
    if (path.back() == '\"')
    {
        path.pop_back();
    }
    if (path.front() == '\"')
    {
        path.erase(0, 1);
    }
}


void ToUpperCase(_Inout_ array_ref<char16_t> s)
{
    for (char16_t& ch : s)
    {
        ch = toupper(ch);
    }
}


void UnescapeString(
    _In_z_ const char16_t* escapedText,
    OUT std::u16string& expandedText
    )
{
    expandedText.clear();

    const char16_t* p = escapedText;

    for (;;)
    {
        char32_t ch = *p;
        if (ch == '\0')
            break;

        // Check escape codes.
        if (ch == '\\')
        {
            const char16_t* escapeStart = p;
            char16_t* escapeEnd = const_cast<char16_t*>(escapeStart + 1);
            char32_t replacement = L'?';
            ++p;

            switch (*escapeEnd)
            {
            case L'r': // return
                replacement = L'\r';
                ++escapeEnd;
                break;

            case L'n': // new line
                replacement = L'\n';
                ++escapeEnd;
                break;

            case L't': // tab
                replacement = L'\t';
                ++escapeEnd;
                break;

            case L'q': // quote
                replacement = L'\"';
                ++escapeEnd;
                break;

            case L'b': // backspace
                replacement = 0x0008;
                ++escapeEnd;
                break;

            case L'f': // form feed
                replacement = 0x000C;
                ++escapeEnd;
                break;

            case L'x':
            case L'u':
            case L'U':
                replacement = (char32_t) wcstoul(ToWChar(escapeStart + 2), OUT ToWChar(&escapeEnd), 16);
                break;

            case L'0': case L'1': case L'2': case L'3': case L'4':
            case L'5': case L'6': case L'7': case L'8': case L'9':
                // Support decimal here (octal is not supported)
                replacement = (char32_t) wcstoul(ToWChar(escapeStart + 1), OUT ToWChar(&escapeEnd), 10);
                break;

            case L'\\':
                replacement = L'\\';
                ++escapeEnd;
                break;

                // Anything else is a question mark.
            }

            if (IsCharacterBeyondBmp(replacement))
            {
                expandedText.push_back(GetLeadingSurrogate(replacement));
                expandedText.push_back(GetTrailingSurrogate(replacement));
            }
            else
            {
                expandedText.push_back(char16_t(replacement));
            }
            p = escapeEnd;
        }
        else // Just push ordinary code unit
        {
            expandedText.push_back(char16_t(ch));
            p++;
        }
    }
}


_Out_range_(0, utf32text.end_ - utf32text.begin_)
size_t ConvertTextUtf16ToUtf32(
    array_ref<char16_t const> utf16text,
    OUT array_ref<char32_t> utf32text,
    _Out_opt_ size_t* sourceCount
    ) throw()
{
    // Can have more UTF16 characters than UTF32,
    // but never the other way around.

    size_t const utf16count = utf16text.size();
    size_t const utf32count = utf32text.size();

    size_t utf16index = 0, utf32index = 0;
    while (utf32index < utf32count && utf16index < utf16count)
    {
        char32_t ch2, ch = utf16text[utf16index++];

        // If surrogate code point, combine this code unit with the next.
        // Otherwise just return the current character.
        if (IsSurrogate(ch))
        {
            if (utf16index < utf16count && (ch2 = utf16text[utf16index], IsTrailingSurrogate(ch2)))
            {
                ++utf16index;
                ch = MakeUnicodeCodePoint(ch, ch2);
            }
            else
            {
                // Illegal unpaired surrogate. Substitute with replacement char.
                ch = UnicodeReplacementCharacter;
            }
        }
        utf32text[utf32index++] = ch;
    }

    if (sourceCount != nullptr)
        *sourceCount = utf16index;

    return utf32index;
}


_Out_range_(0, utf32text.end_ - utf32text.begin_)
size_t ConvertTextUtf16ToUtf32NoReplacement(
    array_ref<char16_t const> utf16text,
    OUT array_ref<char32_t> utf32text,
    _Out_opt_ size_t* sourceCount
    ) throw()
{
    // Can have more UTF16 characters than UTF32,
    // but never the other way around.

    size_t const utf16count = utf16text.size();
    size_t const utf32count = utf32text.size();

    UnicodeCharacterReader reader = { utf16text.data(), utf16text.data() + utf16count };
    size_t utf32index = 0;
    for (; !reader.IsAtEnd() && utf32index < utf32count; ++utf32index)
    {
        utf32text[utf32index] = reader.ReadNext();
    }

    if (sourceCount != nullptr)
        *sourceCount = reader.end - reader.current;

    return utf32index;
}


_Out_range_(0, destMax)
size_t ConvertUtf32ToUtf16(
    array_ref<char32_t const> utf32text,
    OUT array_ref<char16_t> utf16text
    ) throw()
{
    size_t si = 0, di = 0;
    size_t sc = utf32text.size(), dc = utf16text.size();

    if (dc <= 0)
        return 0;

    for ( ; si < sc; ++si)
    {
        if (di >= dc)
            break;

        char32_t ch = utf32text[si];

        if (ch > 0xFFFF && dc - di >= 2)
        {
            // Split into leading and trailing surrogatse.
            // From http://unicode.org/faq/utf_bom.html#35
            utf16text[di + 0] = GetLeadingSurrogate(ch);
            utf16text[di + 1] = GetTrailingSurrogate(ch);
            ++di;
        }
        else
        {
            // A BMP character (or isolated surrogate)
            utf16text[di + 0] = wchar_t(ch);
        }
        ++di;
    }

    return di;
}


namespace
{
    const static char utf8bom[] = {char(0xEF),char(0xBB),char(0xBF)};
}


void ConvertTextUtf8ToUtf16(
    array_ref<char const> utf8text,
    OUT std::u16string& utf16text
    )
{
    // This function can only throw if out-of-memory when resizing utf16text.
    // If utf16text is already reserve()'d, no exception will happen.

    // Skip byte-order-mark.
    int32_t startingOffset = 0;
    if (utf8text.size() >= countof(utf8bom)
    &&  memcmp(utf8text.data(), utf8bom, countof(utf8bom)) == 0)
    {
        startingOffset = ARRAYSIZE(utf8bom);
    }

    utf16text.resize(utf8text.size());

    // Convert UTF-8 to UTF-16.
    int32_t charsConverted =
        MultiByteToWideChar(
        CP_UTF8,
        0, // no flags for UTF8 (we allow invalid characters for testing)
        (LPCSTR)&utf8text[startingOffset],
        int32_t(utf8text.size() - startingOffset),
        OUT ToWChar(const_cast<char16_t*>(utf16text.data())), // workaround issue http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-active.html#2391
        int32_t(utf16text.size())
        );

    // Shrink to actual size.
    utf16text.resize(charsConverted);
}


void ConvertTextUtf16ToUtf8(
    array_ref<char16_t const> utf16text,
    OUT std::string& utf8text
    )
{
    utf8text.clear();

    // Convert UTF-8 to UTF-16.
    int32_t charsConverted =
        WideCharToMultiByte(
        CP_UTF8,
        0, // no flags for UTF8 (we allow invalid characters for testing)
        ToWChar(utf16text.data()),
        int32_t(utf16text.size()),
        nullptr, // null destination buffer the first time to get estimate.
        0,
        nullptr, // defaultChar
        nullptr // usedDefaultChar
        );

    // If no characters were converted (or if overflow), return empty string.
    if (charsConverted <= 0)
        return;
    auto const bomCount = countof(utf8bom);
    auto totalLength = charsConverted + bomCount;
    if (uint32_t(charsConverted) <= bomCount)
        return;

    utf8text.reserve(totalLength);
    utf8text.append(&utf8bom[0], bomCount);
    utf8text.resize(totalLength);

    // Convert UTF-8 to UTF-16.
    WideCharToMultiByte(
        CP_UTF8,
        0, // no flags for UTF8 (we allow invalid characters for testing)
        ToWChar(utf16text.data()),
        int32_t(utf16text.size()),
        OUT &utf8text[bomCount],
        int32_t(utf8text.size() - bomCount),
        nullptr, // defaultChar
        nullptr // usedDefaultChar
        );
}
