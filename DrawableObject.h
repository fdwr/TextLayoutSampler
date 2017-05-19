//----------------------------------------------------------------------------
//  Author:         Dwayne Robinson
//  History:        2015-06-19 Created
//  Description:    Individual drawable objects
//----------------------------------------------------------------------------
#pragma once


////////////////////////////////////////////////////////////////////////////////
// Generic attributes and interfaces.

// All supported attributes a drawable object can ask for.
enum DrawableObjectAttribute : uint32_t
{
    DrawableObjectAttributeFunction,
    DrawableObjectAttributeVisibility,
    DrawableObjectAttributeLabel,
    DrawableObjectAttributeText,
    DrawableObjectAttributeGlyphs,
    DrawableObjectAttributeAdvances,
    DrawableObjectAttributeOffsets,
    DrawableObjectAttributeFontSize,
    DrawableObjectAttributeFontFamily,
    DrawableObjectAttributeWeight,
    DrawableObjectAttributeStretch,
    DrawableObjectAttributeSlope,
    DrawableObjectAttributeFontFilePath,
    DrawableObjectAttributeFontFaceIndex,
    DrawableObjectAttributeFontSimulations,
    DrawableObjectAttributeDWriteFontFaceType,
    DrawableObjectAttributePadding,
    DrawableObjectAttributeWidth,
    DrawableObjectAttributeHeight,
    DrawableObjectAttributePosition,
    DrawableObjectAttributeTransform,
    DrawableObjectAttributePixelZoom,
    DrawableObjectAttributeReadingDirection,
    DrawableObjectAttributeColumnAlignment,
    DrawableObjectAttributeRowAlignment,
    DrawableObjectAttributeJustification,
    DrawableObjectAttributeTypographicFeatures,
    DrawableObjectAttributeLineWrappingMode,
    DrawableObjectAttributeDWriteRenderingMode,
    DrawableObjectAttributeGdiRenderingMode,
    DrawableObjectAttributeGdiPlusRenderingMode,
    DrawableObjectAttributeDWriteMeasuringMode,
    DrawableObjectAttributeDWriteVerticalGlyphOrientation,
    DrawableObjectAttributeLanguageList,
    DrawableObjectAttributeTextColor,
    DrawableObjectAttributeBackColor,
    DrawableObjectAttributeLayoutColor,
    DrawableObjectAttributeColorPaletteIndex,
    DrawableObjectAttributeColorFont,
    DrawableObjectAttributePixelSnapping,
    DrawableObjectAttributeClipping,
    DrawableObjectAttributeUnderline,
    DrawableObjectAttributeStrikethrough,
    DrawableObjectAttributeFontFallback,
    DrawableObjectAttributeTabWidth,
    DrawableObjectAttributeHotkeyMode,
    DrawableObjectAttributeTrimmingGranularity,
    DrawableObjectAttributeTrimmingSign,
    DrawableObjectAttributeTrimmingDelimiter,
    DrawableObjectAttributeUser32DrawTextAsEditControl,
    DrawableObjectAttributeTotal,
};


enum DrawableObjectFunction : uint32_t
{
    DrawableObjectFunctionNop,
    DrawableObjectFunctionDWriteBitmapRenderTargetLayoutDraw,
    DrawableObjectFunctionDWriteBitmapRenderTargetDrawGlyphRun,
    DrawableObjectFunctionDirect2DDrawTextLayout,
    DrawableObjectFunctionDirect2DDrawText,
    DrawableObjectFunctionDirect2DDrawGlyphRun,
    DrawableObjectFunctionGdiTextOut,
    DrawableObjectFunctionUser32DrawText,
    DrawableObjectFunctionGdiPlusDrawString,
    DrawableObjectFunctionGdiPlusDrawDriverString,
    DrawableObjectFunctionDrawColorBitmapGlyphRun,
    DrawableObjectFunctionDrawSvgGlyphRun,
    DrawableObjectFunctionTotal,
#if 0
    GDI GetCharacterPlacement
    GDI GetGlyphIndices
    Uniscribe ScriptStringOut
    User32 EDIT
    RichEdit
    Trident
    XAML
#endif
};


enum DrawableObjectLineWrappingMode
{
    LineWrappingModeNone = 0,    // No wrapping
    LineWrappingModeWordCharacter = 1,    // Whole words, resorting to characters if word is too long
    LineWrappingModeWord = 2,    // Whole words
    LineWrappingModeCharacter = 3,    // Individual characters
    // These actually parallel the DWrite modes, like DWRITE_WORD_WRAPPING_CHARACTER, but one off, without deprecated _WRAP.
};

enum DrawableObjectAlignmentMode
{
    DrawableObjectAlignmentModeLeading = 0,
    DrawableObjectAlignmentModeTrailing = 1,
    DrawableObjectAlignmentModeCenter = 2,
};

enum DrawableObjectJustificationMode
{
    DrawableObjectJustificationModeUnjustified = 0,
    DrawableObjectJustificationModeJustified = 1,
};

static_assert(DrawableObjectAlignmentModeLeading == DWRITE_TEXT_ALIGNMENT_LEADING, "");
static_assert(DrawableObjectAlignmentModeTrailing == DWRITE_TEXT_ALIGNMENT_TRAILING, "");
static_assert(DrawableObjectAlignmentModeCenter == DWRITE_TEXT_ALIGNMENT_CENTER, "");
static_assert(DrawableObjectAlignmentModeLeading == DWRITE_PARAGRAPH_ALIGNMENT_NEAR, "");
static_assert(DrawableObjectAlignmentModeTrailing == DWRITE_PARAGRAPH_ALIGNMENT_FAR, "");
static_assert(DrawableObjectAlignmentModeCenter == DWRITE_PARAGRAPH_ALIGNMENT_CENTER, "");

enum DrawableObjectHotkeyMode
{
    DrawableObjectHotkeyModeNone = 0,
    DrawableObjectHotkeyModeShow = 1,
    DrawableObjectHotkeyModeHide = 2,
};

enum DrawableObjectTrimmingGranularity
{
    DrawableObjectTrimmingGranularityNone = 0,
    DrawableObjectTrimmingGranularityCharacter = 1,
    DrawableObjectTrimmingGranularityWord = 2,
    DrawableObjectTrimmingGranularityPartialGlyph = 3,
};


interface AttributeSource;


class DrawableObject : public ComObject
{
public:
    // Called when changes occur to the attributes, so the drawable object can
    // free any cached data or check for changes.
    virtual HRESULT Update(IAttributeSource& attributeSource);

    // Returns the bounds of where the object should be drawn and the
    // size/location of the content to draw. The content can exceed the layout
    // bounds, but it is usually smaller. For efficiency, the content bounds
    // may be an estimate rather than pixel perfect.
    virtual HRESULT GetBounds(
        IAttributeSource& attributeSource,
        DrawingCanvas& drawingCanvas,
        _Out_ D2D_RECT_F& layoutBounds,
        _Out_ D2D_RECT_F& contentBounds
        );

    // Draw the object onto the canvas at the given location.
    virtual HRESULT Draw(
        IAttributeSource& attributeSource,
        DrawingCanvas& drawingCanvas,
        float x,
        float y,
        DX_MATRIX_3X2F const& transform
        );

    ////////////////////
    // Helpers
    static DrawableObject* Create(DrawableObjectFunction functionId);
    static void GenerateLabel(IAttributeSource& attributeSource, _Inout_ std::u16string& label);
    static HRESULT GetDWriteFontFace(IAttributeSource& attributeSource, DrawingCanvas& drawingCanvas, _COM_Outptr_ IDWriteFontFace** fontFace);
    static HRESULT SaveFontFile(IAttributeSource& attributeSource, DrawingCanvas& drawingCanvas, char16_t const* filePath);
    static HRESULT ExportFontGlyphData(IAttributeSource& attributeSource, DrawingCanvas& drawingCanvas, array_ref<char16_t const> filePath);
    static HRESULT GetFontCharacters(IAttributeSource& attributeSource, DrawingCanvas& drawingCanvas, bool getOnlyColorFontCharacters, _Out_ std::u16string& characters);
    static bool IsGdiOrGdiPlusFunction(DrawableObjectFunction functionType) throw();

    static const Attribute attributeList[DrawableObjectAttributeTotal];
    static const Attribute::PredefinedValue functions[12];
    static const Attribute::PredefinedValue visibilities[2];
    static const Attribute::PredefinedValue enabledValues[2];
    static const Attribute::PredefinedValue textDefaults[68];
    static const Attribute::PredefinedValue readingDirections[8];
    static const Attribute::PredefinedValue glyphDefaults[3];
    static const Attribute::PredefinedValue fontSizes[47];
    static const Attribute::PredefinedValue layoutSizes[13];
    static const Attribute::PredefinedValue typographicFeatures[5];
    static const Attribute::PredefinedValue languages[14];
    static const Attribute::PredefinedValue fontSimulations[4];
    static const Attribute::PredefinedValue textColors[152];
    static const Attribute::PredefinedValue colorPaletteIndices[2];
    static const Attribute::PredefinedValue weights[11];
    static const Attribute::PredefinedValue stretches[9];
    static const Attribute::PredefinedValue slopes[3];
    static const Attribute::PredefinedValue alignments[3];
    static const Attribute::PredefinedValue justifications[2];
    static const Attribute::PredefinedValue wrappingModes[4];
    static const Attribute::PredefinedValue dwriteMeasuringModes[3];
    static const Attribute::PredefinedValue dwriteRenderingModes[7];
    static const Attribute::PredefinedValue dwriteVerticalGlyphOrientation[2];
    static const Attribute::PredefinedValue gdiRenderingModes[7];
    static const Attribute::PredefinedValue gdiPlusRenderingModes[6];
    static const Attribute::PredefinedValue dwriteFontFaceTypes[8];
    static const Attribute::PredefinedValue transforms[18];
    static const Attribute::PredefinedValue pixelZooms[5];
    static const Attribute::PredefinedValue hotkeyDisplays[3];
    static const Attribute::PredefinedValue trimmingGranularities[4];
    static const Attribute::PredefinedValue trimmingDelimiters[2];

    static const D2D_RECT_F emptyRect;
    static const DX_MATRIX_3X2F identityTransform;
    static const float defaultWidth;
    static const float defaultHeight;
    static const float defaultFontSize;
    static const float defaultTabWidth;
    static const uint32_t defaultFontColor;
    static const uint32_t defaultBackColor;
    static const uint32_t defaultLayoutColor;
    static const uint32_t defaultCanvasColor;

    enum Category
    {
        CategoryLight = 0x00000001, // This attribute should be visible even in the light settings mode (very common attribute).
    };
};


////////////////////////////////////////////////////////////////////////////////
// Cached common data structures shared by some of the drawable objects.
//
// Some of these have an Update() method that explicitly compares which
// attributes changed since the last cached data, to reduce unnecessary work
// because object creation is more expensive.
//
// Others only ensure that a useable object is cached and do not compare
// current attributes, since object creation is fairly light. So the caller
// must call Invalidate from its Update, then may lazily call EnsureCached
// before measuring or drawing.

struct CachedDWriteFontFace
{
    ComPtr<IDWriteFontFace> fontFace;
    uint32_t cookieFontFilePath = ~0u;
    uint32_t cookieFamilyName = ~0u;
    uint32_t cookieWeight = ~0u;
    uint32_t cookieStretch = ~0u;
    uint32_t cookieSlope = ~0u;
    uint32_t cookieFontSimulations = ~0u;
    uint32_t cookieFontFaceIndex = ~0u;
    uint32_t cookieDWriteFontFaceType = ~0u;

    HRESULT Update(IAttributeSource& attributeSource, DrawingCanvas& drawingCanvas);
    void Invalidate() { fontFace.clear(); }
};


struct CachedDWriteRenderingParams
{
    ComPtr<IDWriteRenderingParams> renderingParams;
    uint32_t cookieRenderingMode = ~0;

    HRESULT Update(IAttributeSource& attributeSource, DrawingCanvas& drawingCanvas);
    void Invalidate() { renderingParams.clear(); }
};


struct CachedDWriteTextFormat
{
    ComPtr<IDWriteTextFormat> textFormat;

    HRESULT EnsureCached(IAttributeSource& attributeSource, DrawingCanvas& drawingCanvas);
    void Invalidate() { textFormat.clear(); }
};


struct CachedDWriteTextLayout
{
    ComPtr<IDWriteTextLayout> textLayout;

    HRESULT EnsureCached(IAttributeSource& attributeSource, DrawingCanvas& drawingCanvas, _In_ IDWriteTextFormat* textFormat);
    void Invalidate() { textLayout.clear(); }
};


struct CachedGdiFont
{
    GdiFontHandle font;

    HRESULT EnsureCached(IAttributeSource& attributeSource, DrawingCanvas& drawingCanvas);
    void Invalidate() { font.clear(); }
};


struct CachedTransform
{
    DX_MATRIX_3X2F transform;
    operator DX_MATRIX_3X2F&() { return transform; }

    HRESULT Update(IAttributeSource& attributeSource);
};


struct CachedGdiPlusStringFormat
{
    optional_value<Gdiplus::StringFormat> stringFormat;
    operator Gdiplus::StringFormat&() { return stringFormat.value(); } // Assumes !stringFormat.empty()

    HRESULT EnsureCached(IAttributeSource& attributeSource, DrawingCanvas& drawingCanvas);
    void Invalidate() { stringFormat.clear(); }
};


struct CachedGdiPlusFont
{
    // Wrap the font in an optional to get around the lack of a default constructor for Font.
    optional_value<Gdiplus::FontFamily> fontFamily;
    optional_value<Gdiplus::Font> font;

    operator Gdiplus::Font&() { return font.value(); } // Assumes !stringFormat.empty()

    HRESULT EnsureCached(IAttributeSource& attributeSource, DrawingCanvas& drawingCanvas, bool isDriverString);
    void Invalidate() { fontFamily.clear(); font.clear(); }
};


struct CachedGdiPlusStartup
{
    GdiPlusStartupAutoResource gdiplusToken;

    HRESULT EnsureCached();
    void Invalidate();
};


////////////////////////////////////////////////////////////////////////////////
// Specific implementations of various drawable objects.


class DrawableObjectGdiTextOut : public DrawableObject
{
public:
    virtual HRESULT GetBounds(
        IAttributeSource& attributeSource,
        DrawingCanvas& drawingCanvas,
        _Out_ D2D_RECT_F& layoutBounds,
        _Out_ D2D_RECT_F& contentBounds
        ) override;

    virtual HRESULT Draw(
        IAttributeSource& attributeSource,
        DrawingCanvas& drawingCanvas,
        float x,
        float y,
        DX_MATRIX_3X2F const& transform
        ) override;

protected:
    CachedGdiFont font_;
};


class DrawableObjectUser32DrawText : public DrawableObject
{
public:
    virtual HRESULT GetBounds(
        IAttributeSource& attributeSource,
        DrawingCanvas& drawingCanvas,
        _Out_ D2D_RECT_F& layoutBounds,
        _Out_ D2D_RECT_F& contentBounds
        ) override;

    virtual HRESULT Draw(
        IAttributeSource& attributeSource,
        DrawingCanvas& drawingCanvas,
        float x,
        float y,
        DX_MATRIX_3X2F const& transform
        ) override;

protected:
    HRESULT DrawInternal(
        IAttributeSource& attributeSource,
        DrawingCanvas& drawingCanvas,
        float x,
        float y,
        DX_MATRIX_3X2F const& transform,
        _Out_opt_ D2D_RECT_F* layoutBounds,
        _Out_opt_ D2D_RECT_F* contentBounds
        );

    CachedGdiFont font_;
};


// Base class for DWRITE_GLYPH_RUN users - does not draw anything.
class DrawableObjectDWriteGlyphRun : public DrawableObject
{
public:
    virtual HRESULT GetBounds(
        IAttributeSource& attributeSource,
        DrawingCanvas& drawingCanvas,
        _Out_ D2D_RECT_F& layoutBounds,
        _Out_ D2D_RECT_F& contentBounds
        ) override;

protected:
    CachedDWriteFontFace fontFace_;
    CachedDWriteRenderingParams renderingParams_;
};


class DrawableObjectDWriteBitmapRenderTargetDrawGlyphRun : public DrawableObjectDWriteGlyphRun
{
public:
    virtual HRESULT Draw(
        IAttributeSource& attributeSource,
        DrawingCanvas& drawingCanvas,
        float x,
        float y,
        DX_MATRIX_3X2F const& transform
        ) override;
};


class DrawableObjectDirect2DDrawGlyphRun : public DrawableObjectDWriteGlyphRun
{
public:
    virtual HRESULT Draw(
        IAttributeSource& attributeSource,
        DrawingCanvas& drawingCanvas,
        float x,
        float y,
        DX_MATRIX_3X2F const& transform
        ) override;
};


class DrawableObjectDirect2DDrawColorBitmapGlyphRun : public DrawableObjectDWriteGlyphRun
{
public:
    virtual HRESULT Draw(
        IAttributeSource& attributeSource,
        DrawingCanvas& drawingCanvas,
        float x,
        float y,
        DX_MATRIX_3X2F const& transform
        ) override;
};


class DrawableObjectDirect2DDrawSvgGlyphRun : public DrawableObjectDWriteGlyphRun
{
public:
    virtual HRESULT Draw(
        IAttributeSource& attributeSource,
        DrawingCanvas& drawingCanvas,
        float x,
        float y,
        DX_MATRIX_3X2F const& transform
        ) override;
};


// Base class for IDWriteTextLayout users - does not draw anything.
class DrawableObjectDWriteTextLayout : public DrawableObject
{
public:
    virtual HRESULT Update(IAttributeSource& attributeSource) override;

    virtual HRESULT GetBounds(
        IAttributeSource& attributeSource,
        DrawingCanvas& drawingCanvas,
        _Out_ D2D_RECT_F& layoutBounds,
        _Out_ D2D_RECT_F& contentBounds
        ) override;

protected:
    CachedDWriteTextFormat textFormat_;
    CachedDWriteTextLayout textLayout_;
    CachedDWriteRenderingParams renderingParams_;
};


class DrawableObjectDWriteBitmapRenderTargetLayoutDraw : public DrawableObjectDWriteTextLayout
{
public:
    virtual HRESULT Draw(
        IAttributeSource& attributeSource,
        DrawingCanvas& drawingCanvas,
        float x,
        float y,
        DX_MATRIX_3X2F const& transform
        ) override;
};


class DrawableObjectDirect2DDrawText : public DrawableObjectDWriteTextLayout
{
public:
    DrawableObjectDirect2DDrawText(bool shouldCreateTextLayout = false);

    virtual HRESULT Draw(
        IAttributeSource& attributeSource,
        DrawingCanvas& drawingCanvas,
        float x,
        float y,
        DX_MATRIX_3X2F const& transform
        ) override;

protected:
    bool shouldCreateTextLayout_ = false;
};


class DrawableObjectGdiPlusDrawString : public DrawableObject
{
public:
    virtual HRESULT Update(IAttributeSource& attributeSource) override;

    virtual HRESULT GetBounds(
        IAttributeSource& attributeSource,
        DrawingCanvas& drawingCanvas,
        _Out_ D2D_RECT_F& layoutBounds,
        _Out_ D2D_RECT_F& contentBounds
        ) override;

    virtual HRESULT Draw(
        IAttributeSource& attributeSource,
        DrawingCanvas& drawingCanvas,
        float x,
        float y,
        DX_MATRIX_3X2F const& transform
        ) override;

    CachedGdiPlusStartup cachedStartup_;
    CachedGdiPlusStringFormat cachedStringFormat_;
    CachedGdiPlusFont cachedFont_;
};


class DrawableObjectGdiPlusDrawDriverString : public DrawableObject
{
public:
    virtual HRESULT Update(IAttributeSource& attributeSource) override;

    virtual HRESULT GetBounds(
        IAttributeSource& attributeSource,
        DrawingCanvas& drawingCanvas,
        _Out_ D2D_RECT_F& layoutBounds,
        _Out_ D2D_RECT_F& contentBounds
        ) override;

    virtual HRESULT Draw(
        IAttributeSource& attributeSource,
        DrawingCanvas& drawingCanvas,
        float x,
        float y,
        DX_MATRIX_3X2F const& transform
        ) override;

    CachedGdiPlusStartup cachedStartup_;
    CachedGdiPlusFont cachedFont_;
};
