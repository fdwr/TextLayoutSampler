//+---------------------------------------------------------------------------
//  DWrite helper functions for commonly needed tasks.
//
//  History:    2009-09-09  Dwayne Robinson - Created
//----------------------------------------------------------------------------
#pragma once

#if USE_CPP_MODULES
import Common.ArrayRef;
import Common.String;
import FileHelpers;
import Common.AutoResource.Windows;
#else
#include "Common.ArrayRef.h"
#include "Common.String.h"
#include "FileHelpers.h"
#include "Common.AutoResource.Windows.h"
#endif


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
    ) noexcept;

HRESULT CreateFontFaceFromFile(
    IDWriteFactory* factory,
    _In_z_ const wchar_t* fontFilePath,
    uint32_t fontFaceIndex,
    DWRITE_FONT_FACE_TYPE fontFaceType, // pass DWRITE_FONT_FACE_TYPE_UNKNOWN to analyze
    DWRITE_FONT_SIMULATIONS fontSimulations,  // usually just DWRITE_FONT_SIMULATIONS_NONE
    array_ref<DWRITE_FONT_AXIS_VALUE> fontAxisValues,
    _COM_Outptr_ IDWriteFontFace** fontFace
    ) noexcept;

HRESULT CreateFontFaceFromTextFormat(
    IDWriteTextFormat* textFormat,
    _COM_Outptr_ IDWriteFontFace** fontFace
    ) noexcept;

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
    array_ref<const DWRITE_FONT_AXIS_VALUE> fontAxisValues,
    _COM_Outptr_ IDWriteFontFace** newFontFace
    );

HRESULT CreateFontCollection(
    _In_ IDWriteFactory* factory,
    DWRITE_FONT_FAMILY_MODEL fontFamilyModel,
    _In_bytecount_(fontFileNamesSize) const wchar_t* fontFileNames,
    _In_ uint32_t fontFileNamesSize, // Number of wchar_t's, not number file name count
    _COM_Outptr_ IDWriteFontCollection** fontCollection
    ) noexcept;

HRESULT CreateFontCollection(
    _In_ IDWriteFactory* factory,
    DWRITE_FONT_FAMILY_MODEL fontFamilyModel,
    _In_reads_(fontFilesCount) IDWriteFontFile* const* fontFiles,
    uint32_t fontFilesCount,
    _COM_Outptr_ IDWriteFontCollection** fontCollection
    ) noexcept;

HRESULT GetFilePath(
    IDWriteFontFile* fontFile,
    OUT std::u16string& filePath
    ) noexcept;

HRESULT GetFilePath(
    IDWriteFontFace* fontFace,
    OUT std::u16string& filePath
    ) noexcept;

HRESULT GetFontFile(
    IDWriteFontFace* fontFace,
    OUT IDWriteFontFile** fontFile
    );

HRESULT GetFileModifiedDate(
    IDWriteFontFace* fontFace,
    _Out_ FILETIME& fileTime
    ) noexcept;

HRESULT SaveDWriteFontFile(
    IDWriteFontFileStream* fontFileStream,
    _In_z_ char16_t const* filePath
    );

HRESULT SaveDWriteFontFile(
    IDWriteFontFace* fontFace,
    _In_z_ char16_t const* filePath
    );

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
    ) noexcept;

HRESULT GetFontFaceMetrics(
    IDWriteFontFace* fontFace,
    float fontEmSize,
    DWRITE_MEASURING_MODE measuringMode,
    _Out_ DWRITE_FONT_METRICS* fontMetrics
    ) noexcept;

HRESULT GetFontFaceAdvances(
    IDWriteFontFace* fontFace,
    float fontEmSize,
    array_ref<uint16_t const> glyphIds,
    DWRITE_MEASURING_MODE measuringMode,
    bool isSideways,
    _Out_ array_ref<float> glyphAdvances
    ) noexcept;

// The glyph advances remain in font design units and are not scaled by the size.
// The size is merely for the sake of grid-fitting in non-ideal measuring modes.
HRESULT GetFontFaceDesignAdvances(
    IDWriteFontFace* fontFace,
    float fontEmSize,
    array_ref<uint16_t const> glyphIds,
    DWRITE_MEASURING_MODE measuringMode,
    bool isSideways,
    _Out_ array_ref<int32_t> glyphAdvances
    ) noexcept;

HRESULT GetLocalizedStringLanguage(
    IDWriteLocalizedStrings* strings,
    uint32_t stringIndex,
    OUT std::u16string& value
    ) noexcept;

// Try to get the string in the preferred language, else English, else the first index.
// The function does not return an error if the string does exist, just empty string.
HRESULT GetLocalizedString(
    IDWriteLocalizedStrings* strings,
    _In_opt_z_ const wchar_t* preferredLanguage,
    OUT std::u16string& value
    ) noexcept;

// Get the string from the given index.
// The function does not return an error if the string does exist, just empty string.
HRESULT GetLocalizedString(
    IDWriteLocalizedStrings* strings,
    uint32_t stringIndex,
    OUT std::u16string& value
    ) noexcept;

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

HRESULT GetFontAxisValues(
    IDWriteFontFaceReference* fontFaceReference,
    _Out_ std::vector<DWRITE_FONT_AXIS_VALUE>& fontAxisValues
    );

HRESULT GetFontAxisValues(
    IDWriteFontFace* fontFace,
    _Out_ std::vector<DWRITE_FONT_AXIS_VALUE>& fontAxisValues
    );

HRESULT GetFontAxisValues(
    IDWriteFont* font,
    _Out_ std::vector<DWRITE_FONT_AXIS_VALUE>& fontAxisValues
    );

float GetFontAxisValue(
    array_ref<DWRITE_FONT_AXIS_VALUE const> fontAxisValues,
    DWRITE_FONT_AXIS_TAG axisTag,
    float defaultValue
    );

bool IsKnownFontFileExtension(_In_z_ const wchar_t* fileExtension) noexcept;

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
    ) noexcept;

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
    ) noexcept;

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
    ) noexcept;

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
    ) noexcept;

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
    ) noexcept;

D2D1_RECT_F GetBlackBox(
    const DWRITE_OVERHANG_METRICS& overhangMetrics,
    const DWRITE_TEXT_METRICS& textMetrics
    );

// Union of D2D and DWrite's matrix to facilitate
// usage between them, while not breaking existing
// applications that use one or the other.
union DX_MATRIX_3X2F
{
    // Explicity named fields for clarity.
    struct { // Win8
        FLOAT xx; // x affects x (horizontal scaling / cosine of rotation)
        FLOAT xy; // x affects y (vertical shear     / sine of rotation)
        FLOAT yx; // y affects x (horizontal shear   / negative sine of rotation)
        FLOAT yy; // y affects y (vertical scaling   / cosine of rotation)
        FLOAT dx; // displacement of x, always orthogonal regardless of rotation
        FLOAT dy; // displacement of y, always orthogonal regardless of rotation
    };
    struct { // D2D Win7
        FLOAT _11;
        FLOAT _12;
        FLOAT _21;
        FLOAT _22;
        FLOAT _31;
        FLOAT _32;
    };
    struct { // DWrite Win7
        FLOAT m11;
        FLOAT m12;
        FLOAT m21;
        FLOAT m22;
        FLOAT m31;
        FLOAT m32;
    };
    float m[6]; // Would [3][2] be better, more useful?

    DWRITE_MATRIX dwrite;
    D2D1_MATRIX_3X2_F d2d;
    XFORM gdi;
};

void CombineMatrix(
    _In_  DX_MATRIX_3X2F const& a,
    _In_  DX_MATRIX_3X2F const& b,
    _Out_ DX_MATRIX_3X2F& result
    );

DX_MATRIX_3X2F CombineMatrix(
    _In_  DX_MATRIX_3X2F const& a,
    _In_  DX_MATRIX_3X2F const& b
    );
