//+---------------------------------------------------------------------------
//
//  Contents:   Helper for enumerating files.
//
//  History:    2013-10-04   dwayner    Created
//
//----------------------------------------------------------------------------
#include "precomp.h"

#include <windows.h>
#include <string>
#include <Shlwapi.h>
#include "FileHelpers.h"
#include "Common.AutoResource.h"
#include "Common.ArrayRef.h"
#include "Common.String.h"


#pragma comment(lib, "Shlwapi.lib")


HRESULT ReadTextFile(const char16_t* filename, OUT std::u16string& text) throw()
{
    unsigned long bytesRead;
    std::vector<char> fileData;

    ////////////////////
    // Open the file handle, and read the contents.

    HANDLE file = CreateFile(
                    ToWChar(filename),
                    GENERIC_READ,
                    FILE_SHARE_READ,
                    nullptr,
                    OPEN_EXISTING,
                    FILE_FLAG_SEQUENTIAL_SCAN,
                    nullptr
                    );
    FileHandle scopedHandle(file);

    if (file == INVALID_HANDLE_VALUE)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    const unsigned long fileSize = GetFileSize(file, nullptr);
    try
    {
        fileData.resize(fileSize);
        text.resize(fileSize);
    }
    catch (...)
    {
        return E_OUTOFMEMORY;
    }

    if (!ReadFile(file, OUT fileData.data(), fileSize, OUT &bytesRead, nullptr))
    {
        text.clear();
        return HRESULT_FROM_WIN32(GetLastError());
    }

    scopedHandle.Clear();

    ////////////////////
    // Convert UTF-8 to UTF-16. Note 'text' already has capacity at least
    // equal to fileData. So no out-of-memory exceptions will occur.

    assert(text.size() >= fileData.size());
    ConvertTextUtf8ToUtf16(fileData, OUT text);

    return S_OK;
}


HRESULT WriteTextFile(const char16_t* filename, const std::u16string& text) throw()
{
    return WriteTextFile(filename, text.c_str(), static_cast<uint32_t>(text.size()));
}


HRESULT WriteTextFile(
    const char16_t* filename,
    array_ref<char16_t const> text
    ) throw()
{
    return WriteTextFile(filename, text.data(), static_cast<uint32_t>(text.size()));
}


HRESULT WriteTextFile(
    char16_t const* filename,
    __in_ecount(textLength) char16_t const* text,
    uint32_t textLength
    ) throw()
{
    std::string fileData;

    ////////////////////
    // Convert UTF-16 to UTF-8.

    try
    {
        ConvertTextUtf16ToUtf8(make_array_ref(text, textLength), OUT fileData);
    }
    catch (...)
    {
        return E_OUTOFMEMORY;
    }

    ////////////////////
    // Open the file, and write the contents.

    HANDLE file = CreateFile(
                    ToWChar(filename),
                    GENERIC_WRITE,
                    0, // No FILE_SHARE_READ
                    nullptr,
                    CREATE_ALWAYS,
                    FILE_FLAG_SEQUENTIAL_SCAN,
                    nullptr
                    );
    FileHandle scopedHandle(file);

    if (file == INVALID_HANDLE_VALUE)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    unsigned long bytesWritten;
    if (!WriteFile(file, fileData.data(), static_cast<unsigned long>(fileData.size()), OUT &bytesWritten, nullptr))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;
}


HRESULT ReadBinaryFile(const char16_t* filename, IN OUT std::vector<uint8_t>& fileData)
{
    fileData.clear();

    HANDLE file = CreateFile(
                    ToWChar(filename),
                    GENERIC_READ,
                    FILE_SHARE_DELETE | FILE_SHARE_READ,
                    nullptr,
                    OPEN_EXISTING,
                    FILE_FLAG_SEQUENTIAL_SCAN,
                    nullptr
                    );
    FileHandle scopedHandle(file);

    if (file == INVALID_HANDLE_VALUE)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    const unsigned long fileSize = GetFileSize(file, nullptr);
    try
    {
        fileData.resize(fileSize);
    }
    catch (...)
    {
        return E_OUTOFMEMORY;
    }

    unsigned long bytesRead;
    if (!ReadFile(file, fileData.data(), fileSize, OUT &bytesRead, nullptr))
    {
        fileData.clear();
        return HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;
}


HRESULT WriteBinaryFile(
    _In_z_ const char16_t* filename,
    _In_reads_bytes_(fileDataSize) const void* fileData,
    uint32_t fileDataSize
    )
{
    unsigned long bytesWritten;

    HANDLE file = CreateFile(
                    ToWChar(filename),
                    GENERIC_WRITE,
                    FILE_SHARE_DELETE | FILE_SHARE_READ,
                    nullptr,
                    CREATE_ALWAYS,
                    FILE_FLAG_SEQUENTIAL_SCAN,
                    nullptr
                    );
    FileHandle scopedHandle(file);

    if (file == INVALID_HANDLE_VALUE)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (!WriteFile(file, fileData, fileDataSize, &bytesWritten, nullptr))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;
}


HRESULT WriteBinaryFile(_In_z_ char16_t const* filename, const std::vector<uint8_t>& fileData)
{
    return WriteBinaryFile(filename, fileData.data(), static_cast<uint32_t>(fileData.size()));
}


HRESULT WriteBinaryFile(_In_z_ char16_t const* filename, array_ref<uint8_t const> fileData)
{
    return WriteBinaryFile(filename, fileData.data(), static_cast<uint32_t>(fileData.size()));
}


std::u16string GetFullFileName(array_ref<char16_t const> fileName)
{
    std::u16string fullFileName;
    fullFileName.resize(std::max(fileName.size(), size_t(MAX_PATH)));

    char16_t* filePart;
    auto newSize = GetFullPathName(ToWChar(fileName.data()), static_cast<DWORD>(fullFileName.size() + 1), OUT ToWChar(&fullFileName[0]), OUT ToWChar(&filePart));
    if (newSize == 0)
    {
        // Something went wrong with the function. So, at least return the file name that came in.
        fullFileName.assign(fileName.data(), fileName.size());
    }
    else
    {
        fullFileName.resize(newSize);
    }

    return fullFileName;
}


std::u16string GetActualFileName(array_ref<char16_t const> fileName)
{
    std::u16string actualFileName;
    actualFileName.assign(fileName.begin(), fileName.end());

    WIN32_FIND_DATA findData;
    HANDLE findHandle = FindFirstFile(ToWChar(fileName.data()), OUT &findData);
    if (findHandle != INVALID_HANDLE_VALUE)
    {
        // Write over the filename with the one FindFirstFile returned.
        char16_t* filePart = const_cast<char16_t*>(FindFileNameStart(actualFileName));
        size_t fileNameSize = actualFileName.data() + actualFileName.size() - filePart;
        memcpy(filePart, findData.cFileName, std::min(sizeof(findData.cFileName), fileNameSize) * sizeof(char16_t));
    }

    return actualFileName;
}


const char16_t* FindFileNameStart(array_ref<char16_t const> fileName)
{
    const char16_t* p = std::find(fileName.begin(), fileName.end(), '\0');
    while (p != fileName.begin())
    {
        --p;
        char16_t ch = *p;
        if (ch == '\\' || ch == '/' || ch == ':')
        {
            ++p;
            break;
        }
    }

    return p;
}


const char16_t* FindFileNameExtension(array_ref<char16_t const> fileName)
{
    const char16_t* p = std::find(fileName.begin(), fileName.end(), '\0');
    const char16_t* extension = p;

    while (p != fileName.begin())
    {
        --p;
        char16_t ch = *p;
        if (ch == '\\' || ch == '/' || ch == ':')
        {
            ++p;
            break;
        }
        else if (ch == '.')
        {
            extension = p + 1;
            break;
        }
    }

    return extension;
}


bool FileContainsWildcard(array_ref<char16_t const> fileName)
{
    for (char16_t ch : fileName)
    {
        if (ch == 0)
        {
            break;
        }
        if (ch == '*' || ch == '?')
        {
            return true;
        }
    }

    return false;
}


enum PathPartType
{
    PathPartTypeInvalid             = 0x80000000,
    PathPartTypeFileName            = 0x00000000,   // "arial.ttf" (default enum value)
    PathPartTypeMask                = 0x00000001,   // "*.ttf"
    PathPartTypeMultipleMasks       = 0x00000002,   // "*.ttf;*.ttc;*.otf"
    PathPartTypeDirectory           = 0x00000004,   // windows in "c:\windows"
    PathPartTypeDirectoryRecursion  = 0x00000008,   // ** in "c:\users\**\*.otf"
};


PathPartType GetNextPathPart(
    const char16_t* filePath,
    size_t filePathStart,
    OUT size_t& filePathPartBegin,
    OUT size_t& filePathPartEnd
    )
{
    // Retrieve the next part of the path from the given start, returning
    // the beginning, ending, and type of the path part.
    //
    // Exactly one file with full path.
    //      "c:\windows\fonts\arial.ttf"
    //
    //      "c:"            - Directory
    //      "windows"       - Directory
    //      "fonts"         - Directory
    //      "arial.ttf"     - Filename
    //
    // All font files (TrueType or OpenType) starting with 'a' in all
    // subfolders.
    //
    //      "d:\fonts\micro*\**\a*.ttf;a*.otf"
    //
    //      "d:"            - Directory
    //      "fonts"         - Directory
    //      "micro*"        - Directory Mask
    //      "**"            - Directory Recursion
    //      "a*.ttf;a*.otf" - FileName Mask
    //
    filePathPartBegin = filePathStart;
    filePathPartEnd   = filePathStart;

    if (filePath[filePathStart] == '\0')
        return PathPartTypeInvalid;

    // Return type as filename until reaching a path separator or mask.
    PathPartType type = PathPartTypeFileName;
    size_t offset = filePathStart;

    // Skip any leading slashes.
    char16_t ch;
    while (ch = filePath[offset], ch != '\0')
    {
        if (ch != '\\' && ch != '/')
            break;

        ++offset;
    }
    size_t offsetStart = offset;
    filePathPartBegin = offset;

    // Read up to a slash or end of the string.
    while (ch = filePath[offset], ch != '\0')
    {
        if (ch == '\\' || ch == '/')
        {
            type = PathPartType(type | PathPartTypeDirectory);
            break;
        }
        else if (ch == '*' || ch == '?')
        {
            type = PathPartType(type | PathPartTypeMask);
        }
        else if (ch == ';')
        {
            type = PathPartType(type | PathPartTypeMultipleMasks);
        }

        ++offset;
    }

    // Look for the special "**" and mark it as recursive.
    if (type & PathPartTypeMask)
    {
        if (type & PathPartTypeDirectory)
        {
            if (offset - offsetStart == 2
            &&  filePath[offsetStart] == '*'
            &&  filePath[offsetStart + 1] == '*')
            {
                type = PathPartType(type | PathPartTypeDirectoryRecursion);
            }
        }
    }

    filePathPartBegin = offsetStart;
    filePathPartEnd   = offset;

    return type;
}


// Single entry for the enumeration queue.
struct EnumerateMatchingFilesEntry
{
    size_t filePathSize;        // Size in code units of the file path being enumerated.
    size_t fileMaskOffset;      // Offset into the filemask.
    size_t queueNamesOffset;    // Current offset into the queue names (incremented each time).
    size_t queueNamesStart;     // Beginning offset into the queue names (to free the buffer later).
};


void EnumerateAndAddToQueue(
    PathPartType type,
    size_t filePathFileNameBegin,
    size_t fileMaskOffset,
    IN OUT std::u16string& filePath,      // full path plus mask on input, but arbitrarily modified on output
    IN OUT std::u16string& fileNames,     // accumulated list of nul-terminated filenames
    IN OUT std::u16string& queueNames,    // accumulated list of nul-terminated subdirectories remaining to process
    IN OUT std::vector<EnumerateMatchingFilesEntry>& queue // List of queued directory entries to add to
    )
{
    // Read all the files in from the given filePath, and appending to the
    // fileNames and queueNames.
    //
    // Exactly one file with full path.
    //
    //      "c:\windows\fonts\arial.ttf"
    //
    // All font files (TrueType or OpenType) starting with 'a' in all
    // subfolders.
    //
    //      "d:\fonts\micro*\**\a*.ttf;a*.otf"

    //-wprintf(L"enumerating %ls, '%ls' ----------------\r\n", (type & PathPartTypeDirectory) ? L"dirs" : L"files", filePath.c_str());

    std::u16string mask;
    if (type & PathPartTypeMultipleMasks)
    {
        // If the string contains multiple wildcards separated by
        // semicolons ("*.ttf;*.otf"), which FindFirstFile doesn't
        // understand, then set the FindFirstFile file mask to a
        // wildcard, and explicitly match each filename.
        mask = filePath.substr(filePathFileNameBegin);
        filePath.resize(filePathFileNameBegin);
        filePath.push_back('*');
    }

    WIN32_FIND_DATA findData;
    HANDLE findHandle = FindFirstFile(ToWChar(filePath.c_str()), OUT &findData);
    if (findHandle != INVALID_HANDLE_VALUE)
    {
        bool isEntryAlreadyStoredInQueue = false;
        do
        {
            if (type & PathPartTypeMultipleMasks)
            {
                // FindFirstFile returns all filenames, so match explicitly.
                HRESULT hr = PathMatchSpecEx(findData.cFileName, ToWChar(mask.c_str()), PMSF_MULTIPLE);
                if (hr != S_OK)
                {
                    continue; // Skip this one, error or S_FALSE
                }
            }

            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (type & PathPartTypeDirectory)
                {
                    // Skip the unnecessary self-referential entries.
                    if (findData.cFileName[0] == '.'
                    && (findData.cFileName[1] == '.' || findData.cFileName[1] == '\0'))
                    {
                        continue;
                    }

                    // Add an entry to the queue.
                    if (!isEntryAlreadyStoredInQueue)
                    {
                        // Store the full directory path into the queue for
                        // the following subdirectory names.
                        filePath.resize(filePathFileNameBegin);
                        auto queueNamesOffset = queueNames.size();
                        queueNames.push_back('\\');
                        queueNames.append(filePath);
                        queueNames.push_back('\0');

                        // EnumerateMatchingFilesEntry
                        queue.push_back({ filePathFileNameBegin, fileMaskOffset, queueNamesOffset, queueNamesOffset });

                        isEntryAlreadyStoredInQueue = true;
                    }

                    // Append subdirectory name to queue.
                    queueNames.append(ToChar16(findData.cFileName));
                    queueNames.push_back('\0');
                    //-wprintf(L"push directory name=%ls\r\n", findData.cFileName);
                }
            }
            else
            {
                if (!(type & PathPartTypeDirectory))
                {
                    // Record filename.
                    filePath.resize(filePathFileNameBegin);
                    filePath.append(ToChar16(findData.cFileName));
                    fileNames.append(filePath);
                    fileNames.push_back('\0');
                    //-wprintf(L"push file name=%ls\r\n", filePath.c_str());
                }
            }
        } while (FindNextFile(findHandle, OUT &findData));

        FindClose(findHandle);
    }
}


HRESULT EnumerateMatchingFiles(
    __in_z_opt const char16_t* fileDirectory,
    __in_z_opt char16_t const* originalFileMask,
    IN OUT std::u16string& fileNames // Append list of nul-delimited fileNames.
    )
{
    if (fileDirectory == nullptr)
        fileDirectory = u"";

    if (originalFileMask == nullptr)
        originalFileMask = u"*";

    std::u16string fileMask;      // input file mask, combined with the file directory
    std::u16string filePath;      // current file path being enumerated
    std::u16string queueNames;// list of nul-terminated filenames
    std::vector<EnumerateMatchingFilesEntry> queue;

    size_t fileMaskOffset = 0;

    // Combine the mask with file directory.
    fileMask.resize(MAX_PATH);
    PathCombine(OUT ToWChar(&fileMask[0]), ToWChar(fileDirectory), ToWChar(originalFileMask));
    fileMask.resize(wcslen(ToWChar(fileMask.data())));

    // Pop an entry from the queue and read all the matching files/directories,
    // pushing any directories onto the queue.
    //
    // The queue stores a series of nul-terminated strings consisting of
    // file paths and subsequent subdirectory names. A file path starts
    // with a '\' and stores a 16-bit index value of where in the mask it
    // should continue matching from.
    //
    for (;;)
    {
        size_t fileMaskPartBegin = 0;
        size_t fileMaskPartEnd = 0;

        PathPartType type = GetNextPathPart(fileMask.data(), fileMaskOffset, OUT fileMaskPartBegin, OUT fileMaskPartEnd);
        while (type == PathPartTypeDirectory)
        {
            filePath.append(&fileMask[fileMaskPartBegin], fileMaskPartEnd - fileMaskPartBegin);
            filePath.push_back('\\');
            fileMaskOffset = fileMaskPartEnd;
            type = GetNextPathPart(fileMask.data(), fileMaskOffset, OUT fileMaskPartBegin, OUT fileMaskPartEnd);
        }

        const size_t filePathFileNameBegin = filePath.size();

        if (type & PathPartTypeDirectoryRecursion)
        {
            // Read all subdirectories in the current path.
            filePath.push_back(L'*');
            EnumerateAndAddToQueue(
                type,
                filePathFileNameBegin,
                fileMaskPartBegin,
                IN OUT filePath,
                IN OUT fileNames,
                IN OUT queueNames,
                IN OUT queue
                );
            filePath.resize(filePathFileNameBegin);

            // Exhaust any additional recursive segments in case the caller passed c:\fog\**\**\bat.ext.
            while (type & PathPartTypeDirectoryRecursion)
            {
                fileMaskOffset = fileMaskPartEnd;
                type = GetNextPathPart(fileMask.data(), fileMaskOffset, OUT fileMaskPartBegin, OUT fileMaskPartEnd);
            }
        }

        // Append the file or folder mask to complete the search path.
        filePath.append(&fileMask[fileMaskPartBegin], fileMaskPartEnd - fileMaskPartBegin);

        EnumerateAndAddToQueue(
            type,
            filePathFileNameBegin,
            fileMaskPartEnd,
            IN OUT filePath,
            IN OUT fileNames,
            IN OUT queueNames,
            IN OUT queue
            );

        if (queue.empty())
            break;

        // Read the next name.
        auto& entry = queue.back();
        char16_t* entryName = &queueNames[entry.queueNamesOffset];
        size_t entryLength = wcslen(ToWChar(entryName));

        // If the name is a full path, set the current file path.
        if (entryName[0] == '\\')
        {
            ++entryName; // Skip the slash.
            filePath.assign(entryName);
            assert(entry.filePathSize == filePath.size());
            entry.queueNamesOffset += entry.filePathSize + 1 + 1; // Skip null and slash.
            entryName += entry.filePathSize + 1;
            entryLength = wcslen(ToWChar(entryName));
        }

        // Append the next subdirectory.
        entry.queueNamesOffset += entryLength + 1;
        filePath.resize(entry.filePathSize);
        filePath.append(entryName, entryLength);
        filePath.push_back('\\');
        fileMaskOffset = entry.fileMaskOffset;

        // Pop the entry and all names if it was the last one.
        if (entry.queueNamesOffset >= queueNames.size())
        {
            queueNames.resize(entry.queueNamesStart);
            queue.pop_back();
        }
    }

    return S_OK;
}
