//+---------------------------------------------------------------------------
//
//  Contents:   DWrite helper functions for commonly needed tasks.
//
//  History:    2009-09-09  dwayner
//
//----------------------------------------------------------------------------
#pragma once

interface IDWriteFactory;
interface ID2D1RenderTarget;
interface ID2D1Brush;


// A much more useful metrics structure for display.
// Side-bearings are awfully confusing with unclear positive/negative sign depending on whether inside or outside the
// bounding box, plus an inverted 2D coordinate system compared to the rest of DirectWrite and Direct2D. So this
// structure simply stores the two bounding boxes clearly.
struct DWritExGlyphMetrics
{
    int32_t verticalOriginX;
    int32_t verticalOriginY;
    int32_t advanceWidth;
    int32_t advanceHeight;
    int32_t top;
    int32_t left;
    int32_t right;
    int32_t bottom;

    void Set(DWRITE_GLYPH_METRICS const& glyphMetrics);
};

HRESULT LoadDWrite(
    _In_z_ const wchar_t* dllPath,
    DWRITE_FACTORY_TYPE factoryType, // DWRITE_FACTORY_TYPE_SHARED
    _COM_Outptr_ IDWriteFactory** factory,
    _Out_ HMODULE& moduleHandle
    ) throw();

HRESULT CreateFontFaceFromFile(
    IDWriteFactory* factory,
    _In_z_ const wchar_t* fontFilePath,
    uint32_t fontFaceIndex,
    DWRITE_FONT_FACE_TYPE fontFaceType, // pass DWRITE_FONT_FACE_TYPE_UNKNOWN to analyze
    DWRITE_FONT_SIMULATIONS fontSimulations,  // usually just DWRITE_FONT_SIMULATIONS_NONE
    _COM_Outptr_ IDWriteFontFace** fontFace
    ) throw();

HRESULT CreateFontFaceFromTextFormat(
    IDWriteTextFormat* textFormat,
    _COM_Outptr_ IDWriteFontFace** fontFace
    ) throw();

HRESULT CreateFontFace(
    IDWriteFontCollection* fontCollection,
    _In_z_ const wchar_t* fontFamilyName,
    DWRITE_FONT_WEIGHT fontWeight,
    DWRITE_FONT_STRETCH fontStretch,
    DWRITE_FONT_STYLE fontSlope,
    _COM_Outptr_ IDWriteFontFace** fontFace
    );

HRESULT RecreateFontFace(
    IDWriteFactory* factory,
    IDWriteFontFace* originalFontFace,
    DWRITE_FONT_SIMULATIONS fontSimulations,
    _COM_Outptr_ IDWriteFontFace** newFontFace
    );

HRESULT CreateFontCollection(
    _In_ IDWriteFactory* factory,
    _In_bytecount_(fontFileNamesSize) const wchar_t* fontFileNames,
    _In_ uint32_t fontFileNamesSize, // Number of wchar_t's, not number file name count
    _COM_Outptr_ IDWriteFontCollection** fontCollection
    ) throw();

HRESULT CreateFontCollection(
    _In_ IDWriteFactory* factory,
    _In_reads_(fontFilesCount) IDWriteFontFile* const* fontFiles,
    uint32_t fontFilesCount,
    _COM_Outptr_ IDWriteFontCollection** fontCollection
    ) throw();

HRESULT GetFilePath(
    IDWriteFontFile* fontFile,
    OUT std::u16string& filePath
    ) throw();

HRESULT GetFilePath(
    IDWriteFontFace* fontFace,
    OUT std::u16string& filePath
    ) throw();

HRESULT GetFontFile(
    IDWriteFontFace* fontFace,
    OUT IDWriteFontFile** fontFile
    );

HRESULT GetFileModifiedDate(
    IDWriteFontFace* fontFace,
    _Out_ FILETIME& fileTime
    ) throw();

// Overload that takes a DWRITE_MEASURING_MODE, since the measuring mode is often used.
HRESULT CreateTextLayout(
    IDWriteFactory* factory,
    __in_ecount(textLength) wchar_t const* text,
    uint32_t textLength,
    IDWriteTextFormat* textFormat,
    float maxWidth,
    float maxHeight,
    DWRITE_MEASURING_MODE measuringMode,
    _Out_ IDWriteTextLayout** textLayout
    ) throw();

HRESULT GetFontFaceMetrics(
    IDWriteFontFace* fontFace,
    float fontEmSize,
    DWRITE_MEASURING_MODE measuringMode,
    _Out_ DWRITE_FONT_METRICS* fontMetrics
    ) throw();

HRESULT GetFontFaceAdvances(
    IDWriteFontFace* fontFace,
    float fontEmSize,
    array_ref<uint16_t const> glyphIds,
    DWRITE_MEASURING_MODE measuringMode,
    bool isSideways,
    _Out_ array_ref<float> glyphAdvances
    ) throw();

// The glyph advances remain in font design units and are not scaled by the size.
// The size is merely for the sake of grid-fitting in non-ideal measuring modes.
HRESULT GetFontFaceDesignAdvances(
    IDWriteFontFace* fontFace,
    float fontEmSize,
    array_ref<uint16_t const> glyphIds,
    DWRITE_MEASURING_MODE measuringMode,
    bool isSideways,
    _Out_ array_ref<int32_t> glyphAdvances
    ) throw();

HRESULT GetLocalizedStringLanguage(
    IDWriteLocalizedStrings* strings,
    uint32_t stringIndex,
    OUT std::u16string& value
    ) throw();

// Try to get the string in the preferred language, else English, else the first index.
// The function does not return an error if the string does exist, just empty string.
HRESULT GetLocalizedString(
    IDWriteLocalizedStrings* strings,
    _In_opt_z_ const wchar_t* preferredLanguage,
    OUT std::u16string& value
    ) throw();

// Get the string from the given index.
// The function does not return an error if the string does exist, just empty string.
HRESULT GetLocalizedString(
    IDWriteLocalizedStrings* strings,
    uint32_t stringIndex,
    OUT std::u16string& value
    ) throw();

HRESULT GetFontFaceName(
    IDWriteFont* font,
    _In_opt_z_ wchar_t const* languageName,
    OUT std::u16string& value
    );

// Returns the family name of the font (in the language requested, if available, else the default English name).
HRESULT GetFontFamilyName(
    IDWriteFont* font,
    _In_opt_z_ wchar_t const* languageName,
    OUT std::u16string& value
    );

HRESULT GetFontFamilyName(
    IDWriteFontFamily* fontFamily,
    _In_opt_z_ wchar_t const* languageName,
    OUT std::u16string& stringValue
    );

HRESULT GetInformationalString(
    IDWriteFont* font,
    DWRITE_INFORMATIONAL_STRING_ID informationalStringID,
    _In_opt_z_ wchar_t const* languageName,
    _Out_ std::u16string& stringValue
    );

HRESULT GetInformationalString(
    IDWriteFontFace* fontFace,
    DWRITE_INFORMATIONAL_STRING_ID informationalStringID,
    _In_opt_z_ wchar_t const* languageName,
    _Out_ std::u16string& value
    );

bool IsKnownFontFileExtension(_In_z_ const wchar_t* fileExtension) throw();

// Draw a text layout to a bitmap render target.
HRESULT DrawTextLayout(
    IDWriteFactory* dwriteFactory, // Needed for TranslateColorGlyphRun.
    IDWriteBitmapRenderTarget* renderTarget,
    IDWriteRenderingParams* renderingParams,
    IDWriteTextLayout* textLayout,
    float x,
    float y,
    COLORREF textColor = 0,
    uint32_t colorPaletteIndex = 0, // Use 0xFFFFFFFF if no palette (monochrome)
    bool enablePixelSnapping = true
    );

HRESULT DrawColorGlyphRun(
    IDWriteFactory* dwriteFactory,
    IDWriteBitmapRenderTarget* renderTarget,
    DWRITE_GLYPH_RUN const& glyphRun,
    DWRITE_MATRIX const& transform,
    DWRITE_MEASURING_MODE measuringMode,
    float baselineOriginX,
    float baselineOriginY,
    IDWriteRenderingParams* renderingParams,
    COLORREF textColor = 0x00000000,
    uint32_t paletteIndex = 0 // Use 0xFFFFFFFF if no palette (monochrome)
    );

HRESULT DrawColorGlyphRun(
    IDWriteFactory* dwriteFactory,
    ID2D1RenderTarget* renderTarget,
    DWRITE_GLYPH_RUN const& glyphRun,
    DWRITE_MATRIX const& transform,
    DWRITE_MEASURING_MODE measuringMode,
    float baselineOriginX,
    float baselineOriginY,
    ID2D1Brush* brush,
    uint32_t colorPalette // 0xFFFFFFFF if none
    );

HRESULT GetFontCharacterCoverageCounts(
    array_ref<IDWriteFontFace* const> fontFaces,
    array_ref<char32_t const> unicodeCharactersIn,
    bool getOnlyColorFontCharacters,
    std::function<void(uint32_t i, uint32_t total)> progress,
    _Out_ std::vector<uint16_t>& coverageCounts
    );

HRESULT GetStringFromCoverageCount(
    array_ref<uint16_t const> characterCounts,
    uint32_t lowCount,
    uint32_t highCount,
    _Out_ std::u16string& text
    );

HRESULT PlacementsToAbsolutePoints(
    IDWriteFontFace* fontFace,
    float fontEmSize,
    float baselineOriginX,
    float baselineOriginY,
    bool isSideways,
    bool isRtl,
    uint32_t glyphCount,
    _In_reads_(glyphCount) const uint16_t* glyphIndices,
    _In_reads_(glyphCount) const float* glyphAdvances,
    _In_reads_opt_(glyphCount) const DWRITE_GLYPH_OFFSET* glyphOffsets,
    _Out_writes_(glyphCount) D2D_POINT_2F* absoluteGlyphOffsets
    );

HRESULT PlacementsToAbsoluteOffsets(
    IDWriteFontFace* fontFace,
    float fontEmSize,
    float baselineOriginX,
    float baselineOriginY,
    bool isSideways,
    bool isRtl,
    bool invertYCoordinate,
    uint32_t glyphCount,
    _In_reads_(glyphCount) const uint16_t* glyphIndices,
    _In_reads_(glyphCount) const float* glyphAdvances,
    _In_reads_opt_(glyphCount) const DWRITE_GLYPH_OFFSET* glyphOffsets,
    _Out_writes_(glyphCount) DWRITE_GLYPH_OFFSET* absoluteGlyphOffsets
    );

D2D1_RECT_F GetBlackBox(
    const DWRITE_OVERHANG_METRICS& overhangMetrics,
    const DWRITE_TEXT_METRICS& textMetrics
    );
