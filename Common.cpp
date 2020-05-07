//----------------------------------------------------------------------------
//  History:    2015-06-19 Dwayne Robinson - Created
//----------------------------------------------------------------------------
#include "precomp.h"
#include <string>
#include <stdint.h>
#include "Common.h"

#if USE_CPP_MODULES
import Common.String;
#else
#include "Common.ArrayRef.h"
#include "Common.String.h"
#endif


////////////////////////////////////////

bool ThrowIf(bool value, _In_opt_z_ char const* message)
{
    if (value)
        throw std::runtime_error(message != nullptr ? message : "Unexpected failure");

    return value;
}


bool TestBit(void const* memoryBase, uint32_t bitIndex) noexcept
{
    return _bittest( reinterpret_cast<long const*>(memoryBase), bitIndex) != 0;
}


bool ClearBit(void* memoryBase, uint32_t bitIndex) noexcept
{
    return _bittestandreset( reinterpret_cast<long*>(memoryBase), bitIndex) != 0;
}


bool SetBit(void* memoryBase, uint32_t bitIndex) noexcept
{
    return _bittestandset( reinterpret_cast<long*>(memoryBase), bitIndex) != 0;
}


void GetCommandLineArguments(_Inout_ std::u16string& commandLine)
{
    // Get the original command line argument string as a single string,
    // not the preparsed argv or CommandLineToArgvW, which fragments everything
    // into separate words which aren't really useable when you have your own
    // syntax to parse. Skip the module filename because we're only interested
    // in the parameters following it.

    // Skip the module name.
    const wchar_t* initialCommandLine = GetCommandLine();
    wchar_t ch = initialCommandLine[0];
    if (ch == '"')
    {
        do
        {
            ch = *(++initialCommandLine);
        } while (ch != '\0' && ch != '"');
        if (ch == '"')
            ++initialCommandLine;
    }
    else
    {
        while (ch != '\0' && ch != ' ')
        {
            ch = *(++initialCommandLine);
        }
    }

    // Assign it and strip any leading or trailing spaces.
    auto commandLineLength = wcslen(initialCommandLine);
    commandLine.assign(initialCommandLine, &initialCommandLine[commandLineLength]);
    TrimSpaces(IN OUT commandLine);
}


static constexpr uint32_t g_endiannessSignature = 0x01020304;
static constexpr uint8_t g_lowByteOfEndiannessSignature = (const uint8_t&)(g_endiannessSignature);
static_assert(g_lowByteOfEndiannessSignature & 4, "Must be logical endian");
