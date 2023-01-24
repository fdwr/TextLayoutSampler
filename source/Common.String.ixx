//+---------------------------------------------------------------------------
//  String helper functions.
//
//  History:    2008-02-11   Dwayne Robinson - Created
//----------------------------------------------------------------------------

#if USE_CPP_MODULES
    module;
#endif

#include "precomp.h"
//#include <sal.h>
//#include <string>
//#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
//#define NOMINMAX
//#define NOSERVICE
//#define NOMCX
//#define NOIME
//#include <windows.h> // TODO: Remove. Only needed for MultiByteToWideChar.

#if USE_CPP_MODULES
    export module Common.String;
    import Common.ArrayRef;
    export
    {
        #include "Common.String.h"
    }
#else
    #include "Common.ArrayRef.h"
    #include "Common.String.h"
#endif

////////////////////////////////////////

array_ref<char16_t const> ToChar16ArrayRef(_In_z_ char16_t const* text)
{
    return make_array_ref(text, wcslen(ToWChar(text)));
}


uint32_t IntLen(_In_z_ char16_t const* text)
{
    return static_cast<uint32_t>(wcslen(ToWChar(text)));
}


char16_t const* SkipSpaces(_In_z_ char16_t const* stringValue) noexcept
{
    for (char16_t ch; ch = *stringValue, ch != '\0' && ch == ' '; ++stringValue)
    { }
    return stringValue;
}


char16_t const* SkipToNextWord(_In_z_ char16_t const* stringValue) noexcept
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


char16_t const* SkipToEnd(_In_z_ char16_t const* stringValue) noexcept
{
    for (char16_t ch; (ch = *stringValue, ch != '\0'); ++stringValue)
    { }
    return stringValue;
}


void RemoveTrailingZeroes(std::u16string& text) noexcept
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


array_ref<wchar_t> ToWString(int32_t value, /*out*/ array_ref<wchar_t> s)
{
    auto charactersWritten = swprintf_s(s.data(), s.size(), L"%d", value);
    return make_array_ref(s.data(), std::max(charactersWritten, 0));
}


// Fills the entire buffer up to fixed size, including leading zeroes.
void WriteZeroPaddedHexNum(uint32_t value, /*out*/ array_ref<char16_t> text)
{
    // Convert character to digits.
    while (!text.empty())
    {
        char16_t digit = value & 0xF;
        digit += (digit >= 10) ? 'A' - 10 : '0';
        text.back() = digit;
        text.pop_back();
        value >>= 4;
    }
}


// 'text' is updated to point to first character after all characters consumed.
uint32_t ReadUnsignedNumericValue(_Inout_ array_ref<char16_t const>& text, _In_range_(2, 36) uint32_t base)
{
    // Sadly, both wcstoul and std::stoul are useless functions because:
    // (1) wcstoul doesn't respect any boundaries and tries to parse beyond the code sequence
    //     (e.g. \x12345 should be treated as {0x1234, '5'}, not as {0x12345})
    // (2) std::stoul throws an exception on parse error, which is overkill for the user
    //     interactively typing in a number.
    // (3) std::stoul requries a std::string as input, which gimps its utility.
    // Additionally, some uses such as escapement conversion don't want whitespace skipped.

    // - 'text' is updated upon returning to point after the consumed part.
    // - Any character outside the radix stops the read. So 123A4G would stop at 'A' for decimal,
    //   but it would continue until 'G' for hexademical.
    // - An empty string returns 0.
    // - The caller doesn't receive a flag, but it can easily detect missing strings or whether
    //   the entire number was read by checking the return array_ref.

    uint32_t value = 0;
    array_ref<char16_t const> input = text;

    while (!input.empty())
    {
        uint32_t digit = input.front();

        if (digit < '0')
            break;

        digit -= '0'; // Handle 0..9.

        if (digit >= 10) // Handle A..Z.
        {
            digit &= ~32; // Make upper case.
            if (digit < 'A' - '0')
                break;

            digit -= 'A' - '0' - 10;
        }

        if (digit >= base)
        {
            break;
        }

        value = value * base + digit;
        input.pop_front();
    }

    text = input;

    return value;
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
    GetFormattedString(/*out*/ returnString, /*shouldConcatenate*/false, formatString, vargs);
    va_end(vargs); // Reset variable arguments
}


void AppendFormattedString(_Inout_ std::u16string& returnString, _In_z_ const char16_t* formatString, ...) 
{
    va_list vargs = nullptr;
    va_start(vargs, formatString); // initialize variable arguments
    GetFormattedString(/*out*/ returnString, /*shouldConcatenate*/true, formatString, vargs);
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
    if (path.empty())
        return;

    if (path.back() == '\"')
    {
        path.pop_back();
    }

    if (path.empty())
        return;

    if (path.front() == '\"')
    {
        path.erase(0, 1);
    }
}


void ToUpperCase(_Inout_ array_ref<char16_t> s)
{
#ifdef WINVER
    CharUpperBuff(ToWChar(s.data()), static_cast<uint32_t>(s.size()));
#else
    for (char16_t& ch : s)
    {
        ch = toupper(ch);
    }
#endif
}


void UnescapeCppUniversalCharacterNames(
    array_ref<char16_t const> escapedText,
    /*out*/ std::u16string& expandedText
    )
{
    expandedText.clear();
    expandedText.reserve(escapedText.size());

    while (!escapedText.empty())
    {
        char16_t ch = escapedText.front();
        escapedText.pop_front();

        // Check escape codes.
        if (ch == '\\' && !escapedText.empty())
        {
            char32_t replacement = L'\\';
            char16_t code = escapedText.front();

            switch (code)
            {
            case 'a':  replacement = 0x0007; escapedText.pop_front(); break; // Alert (Beep, Bell)
            case 'b':  replacement = 0x0008; escapedText.pop_front(); break; // Backspace
            case 'f':  replacement = 0x000C; escapedText.pop_front(); break; // Formfeed
            case 'n':  replacement = 0x000A; escapedText.pop_front(); break; // Newline (Line Feed)
            case 'r':  replacement = 0x000D; escapedText.pop_front(); break; // Carriage Return
            case 't':  replacement = 0x0009; escapedText.pop_front(); break; // Horizontal Tab
            case 'v':  replacement = 0x000B; escapedText.pop_front(); break; // Vertical Tab
            case '\\': replacement = 0x005C; escapedText.pop_front(); break; // Backslash
            case '\'': replacement = 0x0027; escapedText.pop_front(); break; // Single quotation mark
            case '\"': replacement = 0x0022; escapedText.pop_front(); break; // Double quotation mark
            case '?':  replacement = 0x003F; escapedText.pop_front(); break; // Question mark
            case L'x':
            case L'u':
            case L'U':
                {
                    size_t expectedHexSequenceLength = (code == 'U') ? 8 : 4;
                    char16_t const* escapeStart = escapedText.data() + 1; // Skip the 'x' 'u' 'U'
                    char16_t const* escapeEnd = std::min(escapeStart + expectedHexSequenceLength, escapedText.data_end());
                    array_ref<char16_t const> digitSpan = {escapeStart, escapeEnd};

                    // Parse the number.
                    if (digitSpan.size() >= expectedHexSequenceLength)
                    {
                        char32_t hexValue = ReadUnsignedNumericValue(/*inout*/ digitSpan, 16);
                        if (digitSpan.empty()) // Completely read the sequence.
                        {
                            replacement = hexValue;
                            escapedText.reset(digitSpan.begin(), escapedText.end());
                        }
                    }
                    // Else parse error. So keep '\' to preserve original text.
                }
                break;

            // Anything else yields a '\', preserving the original text.
            // Silly octal is not supported.
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
        }
        else // Just append ordinary code unit.
        {
            expandedText.push_back(ch);
        }
    }
}


void UnescapeHtmlNamedCharacterReferences(array_ref<char16_t const> escapedText, /*out*/ std::u16string& expandedText)
{
    expandedText.clear();
    expandedText.reserve(escapedText.size());

    while (!escapedText.empty())
    {
        char16_t ch = escapedText.front();
        escapedText.pop_front();

        // Check escape codes.
        if (ch == '&' && !escapedText.empty())
        {
            char32_t replacement = L'&';
            char16_t const* escapeStart = escapedText.data();
            char16_t const* escapeEnd = escapeStart;

            // Only numeric escapes are supported: &#1234;&#x1A2B;
            // Not named ones: &amp;
            if (*escapeStart == '#')
            {
                uint32_t radix = 10; // Assume decimal, unless 'x' follows.
                ++escapeStart;
                if (escapeStart < escapedText.data_end() && *escapeStart == 'x')
                {
                    radix = 16; // Hexadecimal.
                    ++escapeStart;
                }

                // Parse the number, and replacing on error with just a '\' to preserve original text.
                array_ref<char16_t const> digitSpan = {escapeStart, escapedText.end()};
                replacement = ReadUnsignedNumericValue(/*inout*/ digitSpan, radix);

                // Successful if the digits were not empty and a semicolon was present.
                if (digitSpan.begin() > escapedText.begin() && !digitSpan.empty() && digitSpan.front() == ';')
                {
                    escapedText.reset(digitSpan.begin() + 1, escapedText.end()); // After the semicolon.
                }
                else // Parse error. So restore '\' to preserve original text.
                {
                    replacement = L'\\';
                }
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
        }
        else // Just append ordinary code unit.
        {
            expandedText.push_back(ch);
        }
    }
}


void EscapeCppUniversalCharacterNames(
    array_ref<char16_t const> text,
    /*out*/ std::u16string& escapedText
    )
{
    constexpr size_t escapePrefixLength = 2; // \u or \U
    constexpr size_t shortEscapeDigitLength = 4;
    constexpr size_t longEscapeDigitLength = 8;
    char16_t shortEscapedSequence[6] = { '\\','u','0','0','0','0' };
    char16_t longEscapedSequence[10] = { '\\','U','0','0','0','0','0','0','0','0' };

    escapedText.clear();
    escapedText.reserve(text.size() * std::size(shortEscapedSequence));
    array_ref<char16_t> shortDigitRange(&shortEscapedSequence[escapePrefixLength], &shortEscapedSequence[escapePrefixLength + shortEscapeDigitLength]);
    array_ref<char16_t> longDigitRange(&longEscapedSequence[escapePrefixLength], &longEscapedSequence[escapePrefixLength + longEscapeDigitLength]);

    for (Utf16CharacterReader reader(text.data(), text.data_end()); !reader.IsAtEnd(); )
    {
        char32_t ch = reader.ReadNext();

        if (IsCharacterBeyondBmp(ch))
        {
            // Write surrogate pair.
            WriteZeroPaddedHexNum(ch, /*out*/ longDigitRange);
            escapedText.insert(escapedText.size(), longEscapedSequence, std::size(longEscapedSequence));
        }
        else // Single UTF-16 code unit.
        {
            WriteZeroPaddedHexNum(ch, /*out*/ shortDigitRange);
            escapedText.insert(escapedText.size(), shortEscapedSequence, std::size(shortEscapedSequence));
        }
    }
}


void EscapeHtmlNamedCharacterReferences(
    array_ref<char16_t const> text,
    /*out*/ std::u16string& escapedText
    )
{
    constexpr size_t escapePrefixLength = 3; // &#x
    constexpr size_t shortEscapeDigitLength = 4;
    constexpr size_t longEscapeDigitLength = 8;
    constexpr size_t escapeSuffixLength = 1; // ;
    char16_t shortEscapedSequence[8] = { '&','#','x','0','0','0','0',';' };
    char16_t longEscapedSequence[12] = { '&','#','x','0','0','0','0','0','0','0','0',';' };

    escapedText.clear();
    escapedText.reserve(text.size() * std::size(shortEscapedSequence));
    array_ref<char16_t> shortDigitRange(shortEscapedSequence + escapePrefixLength, shortEscapedSequence + escapePrefixLength + shortEscapeDigitLength);
    array_ref<char16_t> longDigitRange(longEscapedSequence + escapePrefixLength, longEscapedSequence + escapePrefixLength + longEscapeDigitLength);

    for (Utf16CharacterReader reader(text.data(), text.data_end()); !reader.IsAtEnd(); )
    {
        char32_t ch = reader.ReadNext();

        if (IsCharacterBeyondBmp(ch))
        {
            // Write surrogate pair.
            WriteZeroPaddedHexNum(ch, /*out*/ longDigitRange);
            escapedText.insert(escapedText.size(), longEscapedSequence, std::size(longEscapedSequence));
        }
        else // Single UTF-16 code unit.
        {
            WriteZeroPaddedHexNum(ch, /*out*/ shortDigitRange);
            escapedText.insert(escapedText.size(), shortEscapedSequence, std::size(shortEscapedSequence));
        }
    }
}


_Out_range_(0, utf32text.end_ - utf32text.begin_)
size_t ConvertTextUtf16ToUtf32(
    array_ref<char16_t const> utf16text,
    /*out*/ array_ref<char32_t> utf32text,
    _Out_opt_ size_t* sourceCount
    ) noexcept
{
    // Convert all code points, substituting the replacement character for unpaired surrogates.

    Utf16CharacterReader reader(utf16text.data(), utf16text.data_end());
    size_t utf32count = utf32text.size();
    size_t utf32index = 0;

    for (; !reader.IsAtEnd() && utf32index < utf32count; ++utf32index)
    {
        utf32text[utf32index] = reader.ReadNext();
    }

    // Return how many UTF-16 code units and UTF-32 units were read/written.
    // Might have more UTF16 code units than UTF32, but never the other way around.

    if (sourceCount != nullptr)
        *sourceCount = reader.size();

    return utf32index;
}


_Out_range_(0, utf32text.end_ - utf32text.begin_)
size_t ConvertTextUtf16ToUtf32NoReplacement(
    array_ref<char16_t const> utf16text,
    /*out*/ array_ref<char32_t> utf32text,
    _Out_opt_ size_t* sourceCount
    ) noexcept
{
    Utf16CharacterReader reader(utf16text.data(), utf16text.data_end());
    size_t const utf32count = utf32text.size();
    size_t utf32index = 0;

    for (; !reader.IsAtEnd() && utf32index < utf32count; ++utf32index)
    {
        utf32text[utf32index] = reader.ReadNextRaw();
    }

    if (sourceCount != nullptr)
        *sourceCount = reader.size();

    return utf32index;
}


_Out_range_(0, destMax)
size_t ConvertUtf32ToUtf16(
    array_ref<char32_t const> utf32text,
    /*out*/ array_ref<char16_t> utf16text,
    _Out_opt_ size_t* sourceCount
) noexcept
{
    size_t sourceIndex = 0, destinationIndex = 0;
    size_t sc = utf32text.size(), destinationCount = utf16text.size();

    if (destinationCount <= 0)
        return 0;

    for (; sourceIndex < sc; ++sourceIndex)
    {
        if (destinationIndex >= destinationCount)
            break;

        char32_t ch = utf32text[sourceIndex];

        if (IsCharacterBeyondBmp(ch) && destinationCount - destinationIndex >= 2)
        {
            // Split into leading and trailing surrogatse.
            // From http://unicode.org/faq/utf_bom.html#35
            utf16text[destinationIndex + 0] = GetLeadingSurrogate(ch);
            utf16text[destinationIndex + 1] = GetTrailingSurrogate(ch);
            ++destinationIndex;
        }
        else
        {
            // A BMP character (or isolated surrogate)
            utf16text[destinationIndex + 0] = wchar_t(ch);
        }
        ++destinationIndex;
    }

    if (sourceCount != nullptr)
        *sourceCount = sourceIndex;

    return destinationIndex;
}


namespace
{
    const static char utf8bom[] = {char(0xEF),char(0xBB),char(0xBF)};
}


void ConvertTextUtf8ToUtf16(
    array_ref<char const> utf8text,
    /*out*/ std::u16string& utf16text
    )
{
    // This function can only throw if out-of-memory when resizing utf16text.
    // If utf16text is already reserve()'d, no exception will happen.

    // Skip byte-order-mark.
    int32_t startingOffset = 0;
    if (utf8text.size() >= std::size(utf8bom)
    &&  memcmp(utf8text.data(), utf8bom, std::size(utf8bom)) == 0)
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
         /*out*/ ToWChar(const_cast<char16_t*>(utf16text.data())), // workaround issue http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-active.html#2391
        int32_t(utf16text.size())
        );

    // Shrink to actual size.
    utf16text.resize(charsConverted);
}


void ConvertTextUtf16ToUtf8(
    array_ref<char16_t const> utf16text,
    /*out*/ std::string& utf8text
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
    auto const bomCount = sizeof(utf8bom);
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
        /*out*/ &utf8text[bomCount],
        int32_t(utf8text.size() - bomCount),
        nullptr, // defaultChar
        nullptr // usedDefaultChar
        );
}
