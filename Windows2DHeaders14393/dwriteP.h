//+--------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//  Abstract:
//      DirectX Typography Services private API definitions. Requires DWrite.h to be included.
//      Some of these methods will eventually be public but are not yet public in M1.
//      Nevertheless we expose them internally because D2D will need them.
//
//----------------------------------------------------------------------------

#ifndef DWRITEP_H_INCLUDED
#define DWRITEP_H_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif

#include <DWrite_3.h>


interface DWRITE_DECLARE_INTERFACE("28D5197F-3940-4535-B315-E327E13D845B") IDWriteTextFormatDebug : public IUnknown
{
    STDMETHOD(SetForceSlowPath)(BOOL force = true) PURE;
    STDMETHOD_(BOOL, GetForceSlowPath)() PURE;
    STDMETHOD(SetForceReshape)(BOOL force = true) PURE;
    STDMETHOD_(BOOL, GetForceReshape)() PURE;
    STDMETHOD(SetDisableFontFallback)(BOOL disableFontFallback = true) PURE;
    STDMETHOD_(BOOL, GetDisableFontFallback)() PURE;

    /// <summary>
    /// Gets the length in characters (not including the null terminator) of the locale name list.
    /// </summary>
    /// <returns>
    /// Number of total characters in the list.
    /// </returns>
    STDMETHOD_(UINT32, GetLocaleNameListLength)() PURE;

    /// <summary>
    /// Get list of semicolon delimited locale names (e.g. "en-us;zh-Hans;ja;").
    /// </summary>
    /// <param name="localeName">Character array that receives the locale name.</param>
    /// <param name="localeNameSize">Size of the array in characters. The size must include space for the terminating
    ///     null character.</param>
    /// <returns>
    /// Standard HRESULT error code. E_NOT_SUFFICIENT_BUFFER is returned if the buffer is too small.
    /// </returns>
    STDMETHOD(GetLocaleNameList)(
        _Out_writes_z_(localeNameListLength) WCHAR* localeNameList,
        UINT32 localeNameListLength
        ) PURE;

    /// <summary>
    /// Sets a list of semicolon delimited locale names (e.g. "en-us;zh-Hans;ja;").
    /// </summary>
    /// <param name="localeName">Locale name</param>
    /// <returns>
    /// Standard HRESULT error code.
    /// </returns>
    STDMETHOD(SetLocaleNameList)(
        _In_z_ WCHAR const* localeNameList
        ) PURE;
};

/// <summary>
/// Tolerance value to use when analyzing transform values.
/// This tolerance means the floating point will round to zero when converted to 16.16 fixed point.
/// </summary>
#define DWRITE_TRANSFORM_TOLERANCE (1.0f / (1 << 16))

/// <summary>
/// The DWRITE_RASTER_TYPE enumeration identifies a format of a glyph bitmap.
/// </summary>
enum DWRITE_RASTER_TYPE
{
    DWRITE_RASTER_TYPE_INVALID = -1,

    /// <summary>
    /// Specifies that the rasterized glyphrun has no overscaling.
    /// </summary>
    DWRITE_RASTER_1x1,

    /// <summary>
    /// Specifies that the rasterized glyphrun is overscaled by
    /// 6x horizontally.
    /// </summary>
    DWRITE_RASTER_6x1,

    /// <summary>
    /// Specifies that the rasterized glyphrun is overscaled by
    /// 6x horizontally and 5x vertically.
    /// </summary>
    DWRITE_RASTER_6x5,

    /// <summary>
    /// Specifies that the rasterized glyphrun is overscaled by
    /// 8x horizontally and 1x vertically.
    /// </summary>
    DWRITE_RASTER_8x1,

    /// <summary>
    /// Specifies that the glyphs are rasterized with 4x4 overscaling.
    /// However, the pixel format of the glyph run bitmap is 16x1,
    /// as the 4 rows of each 4x4 tile are flattened and interleaved.
    ///
    ///  Pure overscale bitmap     4x4 subtile interleaved format
    ///
    ///  Column 0123  4567
    ///  Row 0 [AAAA][BBBB]       
    ///  Row 1 [CCCC][DDDD]
    ///  Row 2 [EEEE][FFFF]      Column   0123012301230123  4567456745674567
    ///  Row 3 [GGGG][HHHH]      Row0..3 [AAAACCCCEEEEGGGG][BBBBDDDDFFFFHHHH]
    ///         ----  ----   -->          ----------------  ----------------
    ///  Row 4 [IIII][JJJJ]      Row4..7 [IIIIKKKKMMMMOOOO][JJJJLLLLNNNNPPPP]
    ///  Row 5 [KKKK][LLLL]
    ///  Row 6 [MMMM][NNNN]
    ///  Row 7 [OOOO][PPPP]
    ///      
    /// </summary>
    DWRITE_RASTER_4x4_AS_16x1,

    /// <summary>
    /// Specifies that the rasterized glyphrun is overscaled by
    /// 2x horizontally and 2x vertically.
    /// </summary>
    DWRITE_RASTER_2x2,

    /// <summary>
    /// Total number of raster types.
    /// </summary>
    DWRITE_RASTER_TOTAL
};

/// <summary>
/// Specifies options for obtaining glyphs from the cache
/// </summary>
enum DWRITE_GLYPH_OPTIONS
{
    /// <summary>
    /// No options
    /// </summary>
    DWRITE_GLYPH_OPTIONS_NONE = 0,

    /// <summary>
    /// This serves as a hint for how long the DWrite glyph lookup cache
    /// should cache the glyph. Most glyphs are short lived as they are
    /// assumed to be cached by the client. However, for glyphs not cached
    /// by the client, DWrite should hold on to them for longer.
    /// </summary>
    DWRITE_GLYPH_OPTIONS_LONG_LIVED = 1,

    /// <summary>
    /// Impacts which dithering pattern is used for 2x2 bitmaps.
    /// </summary>
    DWRITE_GLYPH_OPTIONS_ODD_DITHERING = 2,
};

#ifdef DEFINE_ENUM_FLAG_OPERATORS
DEFINE_ENUM_FLAG_OPERATORS(DWRITE_GLYPH_OPTIONS);
#endif

// Opaque type that represents an entry in a glyph lookup cache. Each lookup cache entry corresponds to
// a unique combination of font and rasterization parameters (e.g., size, transform, rendering mode, etc.).
#ifdef  __cplusplus_cli
// Managed C++ produces compile errors when this is an undefined struct. Typedef to void to work around this.
typedef void DWRITE_LOOKUP_CACHE_ENTRY;
#else
struct DWRITE_LOOKUP_CACHE_ENTRY;
#endif


struct DWRITE_GLYPH_BITMAP_INFO
{
    UINT16  width;
    UINT16  height;
    INT16   horizontalOriginX;
    INT16   horizontalOriginY;
    BYTE    flags;

    enum Flags
    {
        FlagAliased         = 0x01,
        FlagPixelAligned    = 0x02,
        FlagGrayscaleFilter = 0x04,
        FlagsMask           = 0x07
    };
};

struct DWRITE_MERGE_RANGE
{
    UINT32 startIndex;  // array index of first glyph in range
    UINT32 endIndex;    // index one past the last glyph in the range
};

enum DWRITE_FONT_AXIS_TAG : UINT32
{
    DWRITE_FONT_AXIS_TAG_WEIGHT       = DWRITE_MAKE_OPENTYPE_TAG('w','g','h','t'),
    DWRITE_FONT_AXIS_TAG_WIDTH        = DWRITE_MAKE_OPENTYPE_TAG('w','d','t','h'), 
    DWRITE_FONT_AXIS_TAG_SLANT        = DWRITE_MAKE_OPENTYPE_TAG('s','l','n','t'),
    DWRITE_FONT_AXIS_TAG_OPTICAL_SIZE = DWRITE_MAKE_OPENTYPE_TAG('o','p','s','z'),
    DWRITE_FONT_AXIS_TAG_ITALIC       = DWRITE_MAKE_OPENTYPE_TAG('i','t','a','l'),
};

struct DWRITE_FONT_AXIS_VALUE
{
    DWRITE_FONT_AXIS_TAG axisTag;
    FLOAT value;
};

/// <summary>
/// Old version of the glyph lookup cache used by the CTEE tests
/// </summary>
interface DWRITE_DECLARE_INTERFACE("C20525BF-8D72-47E2-9625-A0C2DCDEA5C7") IDWriteOldGlyphLookupCache : public IUnknown
{
    // renderingMode and gridFitMode cannot be *DEFAULT.
    virtual HRESULT STDMETHODCALLTYPE GetLookupCacheEntry(
        IDWriteFontFace* fontFace,
        FLOAT fontEmSize,
        BOOL isSideways,
        _In_opt_ DWRITE_MATRIX const* transform,
        DWRITE_RENDERING_MODE1 renderingMode,
        DWRITE_GRID_FIT_MODE gridFitMode,
        DWRITE_TEXT_ANTIALIAS_MODE antialiasMode,
        __out DWRITE_LOOKUP_CACHE_ENTRY** cacheEntry,
        __out DWRITE_RASTER_TYPE* overscaleRasterType
        ) PURE;

    virtual HRESULT STDMETHODCALLTYPE GetGlyphBitmapInfo(
        IDWriteFontFace* fontFace,
        DWRITE_LOOKUP_CACHE_ENTRY* cacheEntry,
        UINT32 glyphCount,
        __in_ecount(glyphCount) UINT16 const* glyphIndices,
        __out_ecount(glyphCount) DWRITE_GLYPH_BITMAP_INFO* glyphInfo
        ) PURE;
};

// Represents an entry in a glyph lookup cache. Each lookup cache entry corresponds to
// a unique combination of font and rasterization parameters (e.g., size, transform, rendering mode, etc.).
// Note: this is not a COM interface and is not reference counted.
interface IDWriteLookupCacheEntry
{
    // Gets a pointer to a client-defined object associated with this object.
    virtual void* STDMETHODCALLTYPE GetItemData() PURE;

    // Saves a pointer to a client-defined object associated with a lookup cache entry.
    // Call SetItemData(NULL) when the client-defined object is deleted.
    virtual void STDMETHODCALLTYPE SetItemData(void* itemData) PURE;
};

interface IDWriteLookupCacheBitmapEntry : public IDWriteLookupCacheEntry
{
};

interface IDWriteLookupCacheOutlineEntry : public IDWriteLookupCacheEntry
{
};


/// <summary>
/// IDWriteGlyphLookupCache speeds lookups of glyph bitmaps in the font cache. Optionally, a client can
/// facilitate its own caching of per-glyph data by associating item data with glyph lookup cache entries.
/// </summary>
interface DWRITE_DECLARE_INTERFACE("D4E275F2-7B28-4D55-A1E0-54610B55E83D") IDWriteGlyphLookupCache : public IUnknown
{
    // renderingMode and gridFitMode cannot be *DEFAULT.
    virtual HRESULT STDMETHODCALLTYPE GetLookupCacheBitmapEntry(
        IDWriteFontFace* fontFace,
        FLOAT fontEmSize,
        BOOL isSideways,
        _In_opt_ DWRITE_MATRIX const* transform,
        DWRITE_RENDERING_MODE1 renderingMode,
        DWRITE_GRID_FIT_MODE gridFitMode,
        DWRITE_TEXT_ANTIALIAS_MODE antialiasMode,
        __out IDWriteLookupCacheBitmapEntry** cacheEntry,
        __out DWRITE_RASTER_TYPE* overscaleRasterType,
        __out void** itemData
        ) PURE;

    // Note: isSideways is only sometimes incorporated into the key
    virtual HRESULT STDMETHODCALLTYPE GetLookupCacheOutlineEntry(
        IDWriteFontFace* fontFace,
        BOOL isSideways,
        _Out_ IDWriteLookupCacheOutlineEntry** cacheEntry,
        _Out_ void** itemData
        ) PURE;

    virtual HRESULT STDMETHODCALLTYPE GetGlyphBitmapInfo(
        IDWriteFontFace* fontFace,
        IDWriteLookupCacheBitmapEntry* cacheEntry,
        UINT32 glyphCount,
        __in_ecount(glyphCount) UINT16 const* glyphIndices,
        __out_ecount(glyphCount) DWRITE_GLYPH_BITMAP_INFO* glyphInfo
        ) PURE;

    // Copies a glyph to the specified overscale coordinate in the specified 1bpp bitmap.
    virtual HRESULT STDMETHODCALLTYPE FillGlyphBitmap(
        IDWriteFontFace* fontFace,
        IDWriteLookupCacheBitmapEntry* cacheEntry,
        UINT16 glyphIndex,
        DWRITE_GLYPH_OPTIONS options,
        int overscaleLeft,
        int overscaleTop,
        UINT32 targetHeight,
        UINT32 targetByteStride,
        __out_bcount(targetHeight * targetByteStride) void* targetBuffer
        ) PURE;

    // Returns an outline of the given glyph
    virtual HRESULT STDMETHODCALLTYPE GetGlyphOutline(
        IDWriteFontFace* fontFace,
        IDWriteLookupCacheOutlineEntry* cacheEntry,
        FLOAT emSize,
        UINT16 glyphIndex,
        BOOL isSideways,
        DWRITE_GLYPH_OPTIONS options,
        _In_ IDWriteGeometrySink* geometrySink
        ) PURE;

    // Should be called if an outline is not cached by the client so that the
    // outline lives for a longer amount of time in the DWrite cache.
    virtual HRESULT STDMETHODCALLTYPE EnsureGlyphOutlineLongLived(
        IDWriteFontFace* fontFace,
        IDWriteLookupCacheOutlineEntry* cacheEntry,
        UINT16 glyphIndex
        ) PURE;

    // Returns a pointer to the specified cached bitmap if the glyph is cached in uncompressed
    // form. The pointer reimains valid until then ext call to Cleanpu. If the cached glyph is 
    // compressed, the function succeeds but the output pointer is set to NULL.
    // The isCached parameter specifies whether the caller will cache the result. This
    // serves as a hint for how long the glyph lookup cache itself should cache the glyph.
    virtual HRESULT STDMETHODCALLTYPE TryGetUncompressedBitmap(
        IDWriteFontFace* fontFace,
        IDWriteLookupCacheBitmapEntry* cacheEntry,
        UINT16 glyphIndex,
        DWRITE_GLYPH_OPTIONS options,
        __out UINT32* bitmapHeight,
        __out UINT32* byteStride,
        __out void const**  bitmapPixels
        ) PURE;

    // To be called periodically by a client (e.g., after rendering a frame) to free resources 
    // used by the glyph lookup cache.
    virtual void STDMETHODCALLTYPE Cleanup() PURE;
};

interface IDWriteGlyphBitmapArray;

/// <summary>
/// Private glyph run analysis interface.
/// </summary>
interface DWRITE_DECLARE_INTERFACE("9908D45A-0E1D-4B25-9A84-9801D932887B") IDWritePrivateGlyphRunAnalysis : public IDWriteGlyphRunAnalysis
{
    /// <summary>
    /// Gets the union of the bounding rectangles for rasterized glyphs of the specified type. 
    /// The returned coordinates are in device units.
    /// </summary>
    STDMETHOD(GetRunBitmapBounds)(
        DWRITE_RASTER_TYPE rasterType,
        __out RECT* bitmapBounds
        ) PURE;

    /// <summary>
    /// Draws gyphs to a 1bpp bitmap in the raster type's format.
    /// The stride does not have to be a multiple of 4, but must be at least as great as the width rounded
    /// up to a four-byte boundary.
    /// </summary>
    STDMETHOD(FillRunBitmap)(
        DWRITE_RASTER_TYPE rasterType,
        __in RECT const* bitmapBounds,
        UINT32 targetHeight,
        UINT32 targetByteStride,
        __out_bcount(targetHeight * targetByteStride) void* targetBuffer
        ) PURE;

    /// <summary>
    /// Gets properties required for ClearType blending. Differs from GetAlphaBlendParams in that it
    /// dials down the enhanced contrast when the foreground color is light enough.
    /// </summary>
    /// <param name="renderingParams">Rendering parameters object. In most cases, the values returned in the output
    /// parameters are based on the properties of this object. The exception is if a GDI-compatible rendering mode
    /// is specified.</param>
    /// <param name="colorR">Red channel of the foreground color. Pass 0 to bypass the light-on-dark adjustment</param>
    /// <param name="colorG">Green channel of the foreground color. Pass 0 to bypass the light-on-dark adjustment</param>
    /// <param name="colorB">Blue channel of the foreground color. Pass 0 to bypass the light-on-dark adjustment</param>
    /// <param name="blendGamma">Receives the gamma value to use for gamma correction.</param>
    /// <param name="blendEnhancedContrast">Receives the enhanced contrast value.</param>
    /// <param name="blendClearTypeLevel">Receives the ClearType level.</param>
    STDMETHOD(GetAlphaBlendParamsWithLightOnDarkAdjustment)(
        IDWriteRenderingParams* renderingParams,
        FLOAT foregroundColorRed,
        FLOAT foregroundColorGreen,
        FLOAT foregroundColorBlue,
        __out FLOAT* blendGamma,
        __out FLOAT* blendEnhancedContrast,
        __out FLOAT* blendClearTypeLevel
        ) PURE;
};

struct D2D_POINT_2F;

// Private factory used by CTEE test code and by Splash
// These methods are exposed via a separate interface so that these components do not have to
// take a dependency on IDWritePrivateFontFace, which tends to change over time. In
// particular, IDWritePrivateFontFace derives from the most recent public font face
// interface which means the vtable offsets of private methods change whenever we
// change the public API.
interface DWRITE_DECLARE_INTERFACE("1C895B12-D420-45C7-9986-2A2FFF4391AD") IDWriteOldPrivateFactory : public IUnknown
{
    // Same as IDWriteFactory3
    STDMETHOD(CreateGlyphRunAnalysis)(
        _In_ DWRITE_GLYPH_RUN const* glyphRun,
        _In_opt_ DWRITE_MATRIX const* transform,
        DWRITE_RENDERING_MODE1 renderingMode,
        DWRITE_MEASURING_MODE measuringMode,
        DWRITE_GRID_FIT_MODE gridFitMode,
        DWRITE_TEXT_ANTIALIAS_MODE antialiasMode,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        _COM_Outptr_ IDWriteGlyphRunAnalysis** glyphRunAnalysis
        ) PURE;

    // Same as IDWriteFactory3
    STDMETHOD(CreateCustomRenderingParams)(
        FLOAT gamma,
        FLOAT enhancedContrast,
        FLOAT grayscaleEnhancedContrast,
        FLOAT clearTypeLevel,
        DWRITE_PIXEL_GEOMETRY pixelGeometry,
        DWRITE_RENDERING_MODE1 renderingMode,
        DWRITE_GRID_FIT_MODE gridFitMode,
        _COM_Outptr_ IDWriteRenderingParams3** renderingParams
        ) PURE;

    // Same as IDWritePrivateFactory
    virtual void STDMETHODCALLTYPE GetAlphaBlendParamsWithLightOnDarkAdjustment(
        IDWriteRenderingParams* renderingParams,
        IDWriteFontFace* fontFace,
        DWRITE_RENDERING_MODE1 renderingMode,
        DWRITE_TEXT_ANTIALIAS_MODE antialiasMode,
        bool useGrayFilter, // true if any bitmap in the run specifies FlagGrayscaleFilter
        FLOAT foregroundColorRed,
        FLOAT foregroundColorGreen,
        FLOAT foregroundColorBlue,
        __out FLOAT* blendGamma,
        __out FLOAT* blendEnhancedContrast,
        __out FLOAT* blendClearTypeLevel
        ) PURE;

    // Same as IDWritePrivateFactory
    STDMETHOD(ComputeGlyphOriginsInDIPs)(
        DWRITE_GLYPH_RUN const* glyphRun,
        DWRITE_MEASURING_MODE measuringMode,
        D2D_POINT_2F baselineOrigin,
        FLOAT pixelsPerDip,
        _In_opt_ DWRITE_MATRIX const* transform,
        _Out_writes_(glyphRun->glyphCount) D2D_POINT_2F* glyphOrigins
        ) PURE;

    // Same as IDWritePrivateFactory
    STDMETHOD(GetMergeRanges)(
        DWRITE_GLYPH_RUN const* glyphRun,
        _Out_writes_to_(glyphRun->glyphCount / 2, *rangeCount) DWRITE_MERGE_RANGE* mergeRanges,
        _Out_range_(<=, glyphRun->glyphCount / 2) UINT32* rangeCount
        ) PURE;

    STDMETHOD(CreateGlyphLookupCacheSingleThreaded)(
        _Out_ IDWriteOldGlyphLookupCache** glyphLookupCache
        ) PURE;
    
    /// <summary>
    /// Returns the size of the buffer needed for CreateGlyphRunAnalysisInBuffer.
    /// </summary>
    STDMETHOD(GetGlyphRunAnalysisSize)( 
        UINT32 glyphCount, 
        _Out_ UINT32 *bufferSize
        ) PURE;

    /// <summary>
    /// Creates the object in the memory passed in by the caller. The memory must
    /// be kept alive for the lifetime of the object.
    /// </summary>
    STDMETHOD(CreateGlyphRunAnalysisInBuffer)(
        _In_ DWRITE_GLYPH_RUN const* glyphRun,
        _In_opt_ DWRITE_MATRIX const* transform,
        DWRITE_RENDERING_MODE1 renderingMode,
        DWRITE_MEASURING_MODE measuringMode,
        DWRITE_GRID_FIT_MODE gridFitMode,
        DWRITE_TEXT_ANTIALIAS_MODE antialiasMode,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        __in_ecount(bufferSize) BYTE *buffer,
        UINT32 bufferSize,
        _Out_ IDWriteGlyphRunAnalysis** glyphRunAnalysis
        ) PURE;
};

// Private factory used by Win8.1 Splash.
// This interface should be removed once Splash moves to IDWriteOldPrivateFactory.
interface DWRITE_DECLARE_INTERFACE("C0E8D1E0-1284-4337-BA66-1455E62C3A6E") IDWritePrivateFactory : public IDWriteFactory2
{
    // Win8.1 private method
    STDMETHOD(CreateGlyphRunAnalysis)(
        _In_ DWRITE_GLYPH_RUN const* glyphRun,
        _In_opt_ DWRITE_MATRIX const* transform,
        DWRITE_RENDERING_MODE renderingMode,
        DWRITE_MEASURING_MODE measuringMode,
        BOOL rasterizeClearTypeAsGrayscale,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        _Out_ IDWriteGlyphRunAnalysis** glyphRunAnalysis
        ) PURE;

    // Win8.1 private method
    STDMETHOD(GetGlyphRunAnalysisSize)( 
        UINT32 glyphCount, 
        _Out_ UINT32 *bufferSize
        ) PURE;

    // Win8.1 private method
    STDMETHOD(CreateGlyphRunAnalysisInBuffer)(
        _In_ DWRITE_GLYPH_RUN const* glyphRun,
        _In_opt_ DWRITE_MATRIX const* transform,
        DWRITE_RENDERING_MODE renderingMode,
        DWRITE_MEASURING_MODE measuringMode,
        BOOL rasterizeClearTypeAsGrayscale,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        __in_ecount(bufferSize) BYTE *buffer,
        UINT32 bufferSize,
        _Out_ IDWriteGlyphRunAnalysis** glyphRunAnalysis
        ) PURE;
};

// Private factory used by D2D.
interface DWRITE_DECLARE_INTERFACE("BD6F5199-F548-493F-A88E-F9419845919B") IDWritePrivateFactory1 : public IUnknown
{
    STDMETHOD(CreateGlyphLookupCacheSingleThreaded)(
        _Out_ IDWriteGlyphLookupCache** glyphLookupCache
        ) PURE;

    virtual void STDMETHODCALLTYPE GetAlphaBlendParamsWithLightOnDarkAdjustment(
        IDWriteRenderingParams* renderingParams,
        IDWriteFontFace* fontFace,
        DWRITE_RENDERING_MODE1 renderingMode,
        DWRITE_TEXT_ANTIALIAS_MODE antialiasMode,
        bool useGrayFilter, // true if any bitmap in the run specifies FlagGrayscaleFilter
        FLOAT foregroundColorRed,
        FLOAT foregroundColorGreen,
        FLOAT foregroundColorBlue,
        __out FLOAT* blendGamma,
        __out FLOAT* blendEnhancedContrast,
        __out FLOAT* blendClearTypeLevel
        ) PURE;

    STDMETHOD(ComputeGlyphOriginsInDIPs)(
        DWRITE_GLYPH_RUN const* glyphRun,
        DWRITE_MEASURING_MODE measuringMode,
        D2D_POINT_2F baselineOrigin,
        FLOAT pixelsPerDip,
        _In_opt_ DWRITE_MATRIX const* transform,
        _Out_writes_(glyphRun->glyphCount) D2D_POINT_2F* glyphOrigins
        ) PURE;

    STDMETHOD(GetMergeRanges)(
        DWRITE_GLYPH_RUN const* glyphRun,
        _Out_writes_to_(glyphRun->glyphCount / 2, *rangeCount) DWRITE_MERGE_RANGE* mergeRanges,
        _Out_range_(<=, glyphRun->glyphCount / 2) UINT32* rangeCount
        ) PURE;

    STDMETHOD_(void, OverrideSystemRenderingParams)();

    STDMETHOD_(FLOAT, GetCompatibleGammaOverride)();

    STDMETHOD_(void, SetCompatibleGammaOverride)(
        FLOAT gamma
        );

    /// <summary>
    /// Creates a reference to a remote font given a provider key.
    /// </summary>
    /// <param name="fontProviderIdentifier">Unique identifier of the font data provider.</param>
    /// <param name="fontFileReferenceKey">Font file reference key that uniquely identifies the font file resource
    /// within the scope of the font provider being used.</param>
    /// <param name="fontFileReferenceKeySize">Size of font file reference key in bytes.</param>
    /// <param name="faceIndex">The zero based index of a font face in cases when the font files contain a collection of font faces.
    ///     If the font files contain a single face, this value should be zero.</param>
    /// <param name="fontFaceSimulationFlags">Font face simulation flags for algorithmic emboldening and italicization.</param>
    /// <param name="fontFaceReference">Contains newly created font face reference object, or nullptr in case of failure.</param>
    /// <returns>
    /// Standard HRESULT error code.
    /// </returns>
    STDMETHOD(CreateFontFaceReference)(
        _In_ REFIID fontProviderIdentifier,
        _In_reads_bytes_(fontProviderFileReferenceKeySize) void const* fontProviderFileReferenceKey,
        UINT32 fontProviderFileReferenceKeySize,
        UINT32 faceIndex,
        DWRITE_FONT_SIMULATIONS fontSimulations,
        _COM_Outptr_ IDWriteFontFaceReference** fontFaceReference
        ) PURE;
};

// Private factory used only by GDI
interface DWRITE_DECLARE_INTERFACE("AD247592-E5B0-458E-BA14-3FB9C25FAA0D") IDWritePrivateFactoryForGdi : public IUnknown
{
    // Notifies the font cache that fonts have been added to or removed from the
    // current GDI session.
    STDMETHOD_(void, UpdateSessionFonts)() PURE;
};

// Private factory used for testing purposes.
interface DWRITE_DECLARE_INTERFACE("DF66EA23-699C-4A3C-906F-A840AE5FEDA8") IDWriteDebugFactory : public IUnknown
{
    // Cleans up the font cache by deleting cached font files.
    // Pass 0 for the byte size cap to use the default policy.
    // To trim everything for testing purposes, you may just pass 1 byte.
    // The function waits until clean up is done and all deletable files are deleted,
    // but times out after five seconds which should be plenty of time. Note that
    // some files may not actually be deletable yet if another process has them open.
    STDMETHOD(CleanupFontDownloadCache)(UINT32 byteSizeCap) PURE;
};

// Private interface implemented by the DWrite factory and used for CSS interop.
interface DWRITE_DECLARE_INTERFACE("ECEFF950-8FEA-4CD8-9D66-45A77D0A795C") IDWriteCssInterop : public IUnknown
{
    /// <summary>
    /// The AddFontsFromStylesheet methods parses a string containing one or more CSS
    /// @font-face rules, and adds the resulting font face references and properties
    /// to the specified font set builder.
    /// </summary>
    /// <param name="styleSheet">Specifies the stylesheet text. The text must conform
    /// to CSS syntax and should contain one or more @font-face rules. The method skips
    /// any content outside of @font-face rules.</param>
    /// <param name="styleSheetLength">Length of the stylesheet, in characters.</param>
    /// <param name="localFontSet">Optional font set used to resolve local font face
    /// references in the stylesheet. If this parameter is NULL then local font face
    /// references are ignored.</param>
    /// <param name="inMemoryLoader">Optional in-memory loader used to resolve data URLs
    /// in the stylesheet. If this parameter is NULL then data URLs are ignored. The
    /// caller can determine whether any in-memory font file references were created by
    /// calling IDWriteInMemoryFontFileLoader::GetFileCount. If so, the caller is 
    /// responsible for ensuring that the loader exists and is registered for as long as 
    /// the font set (and any fonts created from it) are in use.</param>
    /// <param name="remoteLoader">Optional remote file loader used to resolve HTTP and
    /// HTTPS references in the stylesheet. If this parameter is NULL then such URLs are
    /// ignored.</param>
    /// <param name="baseUrl">Optional base URL used to convert relative URLs in the
    /// stylesheet to absolute URLs. If this parameter is NULL then relative URLs are
    /// ignored.</param>
    /// <param name="fontSetBuilder">Font set builder to which font file references
    /// and associated properties are added.</param>
    /// <returns>
    /// Standard HRESULT error code.
    /// </returns>
    STDMETHOD(AddFontsFromStylesheet)(
        _In_reads_(styleSheetLength) WCHAR const* styleSheet,
        UINT32 styleSheetLength,
        _In_opt_ IDWriteFontSet* localFontSet,
        _In_opt_ IDWriteInMemoryFontFileLoader* inMemoryLoader,
        _In_opt_ IDWriteRemoteFontFileLoader* remoteLoader,
        _In_opt_z_ WCHAR const* baseUrl,
        _In_ IDWriteFontSetBuilder* fontSetBuilder
        ) PURE;
};


// Interface exposing private font face methods used by GDI+.
// These methods are exposed via a separate interface so that GDI+ does not have to
// take a dependency on IDWritePrivateFontFace, which tends to change over time. In
// particular, IDWritePrivateFontFace derives from the most recent public font face
// interface which means the vtable offsets of private methods change whenever we
// change the public API.
interface DWRITE_DECLARE_INTERFACE("10BF8AB9-3452-42DD-8A00-4B0D9E7DA570") IDWriteGdiPlusFontFace : public IUnknown
{
    /// <summary>
    /// Returns the number of embedded bitmaps at a certain ppem.
    /// Used by GDI+ to determine whether to use embedded bitmaps.
    /// </summary>
    /// <param name="pixelsPerEm">The size to query embedded bitmaps for</param>
    STDMETHOD_(UINT32, GetEmbeddedBitmapCount)(UINT32 pixelsPerEm) PURE;

    /// <summary>
    /// Creates an array of glyph bitmaps that can be queried for bounds
    /// and bitmaps. It is analogous to IDWriteFontFace::CreateGlyphRunAnalysis.
    /// Note that unlike the glyph run analysis, all glyphs will be the same
    /// raster type, with no mixed content. Embedded bitmaps will only be
    /// returned when the raster type is DWRITE_RASTER_1x1 and hinting is
    /// enabled (such as GDI+ TextRenderingHint::SingleBitPerPixel). For any
    /// other mode, or for 1x1 without hinting (such as GdiPlusSingleBitPerPixel),
    /// they are not returned.
    /// </summary>
    /// <param name="rasterType">Glyph bitmap format.</param>
    /// <param name="gridFitMode">The type of grid fitting. This must be an
    ///     explicit mode, not DWRITE_GRID_FIT_DEFAULT.</param>
    /// <param name="fontEmSize">Em size of the font.</param>
    /// <param name="dpiX">Dots per inch, where 96 equals 1:1.</param>
    /// <param name="dpiY">Dots per inch, where 96 equals 1:1.</param>
    /// <param name="transform">Transform that applies to each glyph. Note
    ///     that unlike the glyph run, where advances are specified, the caller
    ///     of this function explicitly knows the position of each glyph in
    ///     device subpixel coordinates. No additional translation or rotation
    ///     modifies the positions.</param>
    /// <param name="isSideways">Whether the glyph is in a sideways run. Note
    ///     that since the caller explicitly supplies the transform per glyph,
    ///     and since there is no concept of a baseline, sideways here is for
    ///     the purpose of correct italic simulation. It does not apply any
    ///     additional 90 degree rotation (the caller has already done this),
    ///     and it does not change the interpretation of the glyph origin,
    ///     which remains at 0,0 in the font. Similarly, right-to-left text
    ///     also has the glyph origin at 0,0.</param>
    /// <param name="glyphCount">How many glyphs.</param>
    /// <param name="glyphIndices">Array of glyph id's to get bitmaps for.</param>
    /// <param name="glyphBitmapArray">The created bitmap glyph array.</param>
    STDMETHOD(CreateGlyphBitmapArray)(
        DWRITE_RASTER_TYPE rasterType,
        DWRITE_GRID_FIT_MODE gridFitMode,
        FLOAT fontEmSize,
        FLOAT dpiX,
        FLOAT dpiY,
        __in_opt DWRITE_MATRIX const* transform,
        BOOL isSideways,
        UINT32 glyphCount,
        __in_ecount(glyphCount) UINT16 const* glyphIndices,
        __out IDWriteGlyphBitmapArray** glyphBitmapArray
        ) PURE;

    /// <summary>
    /// Returns whether or not the font specifies to use grayscale at the given
    /// ppem size. Used by GDI+.
    /// </summary>
    /// <param name="pixelsPerEm">The size to query embedded bitmaps for</param>
    STDMETHOD_(BOOL, IsGrayscaleFontSize)(UINT32 pixelsPerEm) PURE;

    /// <summary>
    /// Returns whether or not any of the code pages in the font are East Asian.
    /// Used by GDI+ for backwards compatibility.
    /// </summary>
    STDMETHOD_(BOOL, HasFullWidthCodePage)() PURE;
};

// Old private factory used by CTEE test code
// These methods are exposed via a separate interface so that these components do not have to
// take a dependency on IDWritePrivateFontFace, which tends to change over time. In
// particular, IDWritePrivateFontFace derives from the most recent public font face
// interface which means the vtable offsets of private methods change whenever we
// change the public API.
interface DWRITE_DECLARE_INTERFACE("1E21C0EF-74B8-423D-95CB-41C9297E095A") IDWriteOldPrivateFontFace : public IUnknown
{
    // Same as IDWriteFontFace3
    STDMETHOD(GetRecommendedRenderingMode)(
        FLOAT fontEmSize,
        FLOAT dpiX,
        FLOAT dpiY,
        _In_opt_ DWRITE_MATRIX const* transform,
        BOOL isSideways,
        DWRITE_OUTLINE_THRESHOLD outlineThreshold,
        DWRITE_MEASURING_MODE measuringMode,
        _In_opt_ IDWriteRenderingParams* renderingParams,
        _Out_ DWRITE_RENDERING_MODE1* renderingMode,
        _Out_ DWRITE_GRID_FIT_MODE* gridFitMode
        ) PURE;

    // The following method is from IDWriteGdiPlusFontFace but is also declared
    // here for convenience -- e.g., so test code can call these methods without
    // having to query for another interface.
    STDMETHOD_(UINT32, GetEmbeddedBitmapCount)(UINT32 pixelsPerEm) PURE;
};

// New private interface obtainable through IDWriteFontFace::QueryInterface
interface DWRITE_DECLARE_INTERFACE("F5ADA939-A4E1-4C50-921B-0E448CB7A8D4") IDWritePrivateFontFace : public IDWriteFontFace4
{
    /// <summary>
    /// Returns TRUE if any of the eight required OpenType tables are missing from
    /// this font. The return value is always true if the font is a raw CFF font.
    /// </summary>
    STDMETHOD_(BOOL, IsRequiredTableMissing)() PURE;

    /// <summary>
    /// Returns whether or not the font has any OpenType lookup with a space glyph
    /// within the lookup's glyph coverage.
    /// </summary>
    STDMETHOD_(BOOL, CanShapeAcrossSpace)() PURE;
};

// Analogous to IDWriteGlyphRunAnalaysis, but used by GDI+.
interface DWRITE_DECLARE_INTERFACE("3A168521-8E30-4064-B3BB-0AC876AA884A") IDWriteGlyphBitmapArray : public IUnknown
{
    /// <summary>
    /// Returns the number of glyphs in this array.
    /// </summary>
    STDMETHOD_(UINT32, GetGlyphCount)() PURE;

    /// <summary>
    /// Returns the bounds of a given glyph. The bounding rectangle is in device
    /// coordinates (i.e., whole pixels), not overscale coordinates. The glyph
    /// offset allows glyphs to be positioned at subpixels. This method is
    /// analogous to IDWriteGlyphRunAnalysis::GetRunBitmapBounds.
    /// </summary>
    /// <remarks>
    /// The returned bitmap bounds can be empty for certain characters, most
    /// typically the space. For these, you can either call FillGlyphBitmap
    /// and receive an empty bitmap, or just bypass the call entirely.
    /// </remarks>
    STDMETHOD(GetGlyphBitmapBounds)(
        UINT32 arrayIndex,
        FLOAT glyphOffsetX,
        FLOAT glyphOffsetY,
        __out RECT* bitmapBounds
        ) PURE;

    /// <summary>
    // Fills the buffer with the overscale data for the given glyph bitmap.
    // This method is analogous to IDWriteGlyphRunAnalaysis::FillRunBitmap
    /// </summary>
    /// <remarks>
    /// The glyph offset X and Y should be the same you passed earlier to
    /// GetGlyphBitmapBounds.
    /// </remarks>
    STDMETHOD(FillGlyphBitmap)(
        UINT32 arrayIndex,
        FLOAT glyphOffsetX,
        FLOAT glyphOffsetY,
        __in RECT const* bitmapBounds,
        UINT32 targetHeight,
        UINT32 targetByteStride,
        __out_bcount(targetHeight * targetByteStride) void* targetBuffer
        ) PURE;
};


interface DWRITE_DECLARE_INTERFACE("B7E6163E-7F46-43B4-84B3-E4E6249C365E") IDWritePrivateTextAnalyzer : public IUnknown
{
    /// <summary>
    /// Determines the reading direction of the content based on the
    /// first strong character.
    /// </summary>
    /// <param name="analysisSource">Source object to analyze.</param>
    /// <param name="textPosition">Starting position to analyze from.</param>
    /// <param name="textLength">Length of text to analyze.</param>
    /// <param name="readingDirection">The reading direction it determined.</param>
    /// <param name="isAmbiguousReadingDirection">The text contains no strong
    ///     characters, meaning the caller should use their own policy such as
    ///     the user locale reading direction or IME direction.</param>
    STDMETHOD(GetContentReadingDirection)(
        _In_ IDWriteTextAnalysisSource* analysisSource,
        UINT32 textPosition,
        UINT32 textLength,
        _Out_ DWRITE_READING_DIRECTION* readingDirection,
        _Out_ BOOL* isAmbiguousReadingDirection
        ) PURE;
};


interface DWRITE_DECLARE_INTERFACE("4A1B37B6-E40B-494D-9551-F5B1BAE95377") IDWritePrivateTextAnalysisSource : public IUnknown
{
    /// <summary>
    /// Get list of semicolon delimited locale names on the range (e.g. "en-us;zh-Hans;ja;")
    /// </summary>
    /// <param name="textPosition">Position to get the locale name of.</param>
    /// <param name="textLength">Receives the length from the given position up to the
    ///     next differing locale.</param>
    /// <param name="localeNameList">Address that receives a pointer to the list of
    ///     locale names at the specified text position.</param>
    /// <remarks>
    /// The localeNameList pointer must remain valid until the next call or until
    /// the analysis returns.
    /// </remarks>
    STDMETHOD(GetLocaleNameList)(
        UINT32 textPosition,
        _Out_ UINT32* textLength,
        _Outptr_result_z_ WCHAR const** localeNameList
        ) PURE;
};

#if NTDDI_VERSION >= NTDDI_WIN10_RS2

interface DWRITE_DECLARE_INTERFACE("77F8C547-3EDE-4378-B494-20613254396D") IDWriteRemoteFontSetLoader : public IUnknown
{
    STDMETHOD(BeginAddFontsFromUrl)(
        _In_ IDWriteFactory5* factory,
        _In_opt_ IDWriteInMemoryFontFileLoader* memoryLoader,
        _In_z_ wchar_t const* fontSetUrl,
        _In_ IDWriteFontSetBuilder* fontSetBuilder,
        _COM_Outptr_ IDWriteAsyncResult** asyncResult
        ) PURE;
};

#endif // NTDDI_VERSION >= NTDDI_WIN10_RS2

#endif /* DWRITEP_H_INCLUDED */
