//----------------------------------------------------------------------------
//  History:        2015-06-19 Dwayne Robinson - Created
//  Description:    Generic drawable object instance with attribute values.
//----------------------------------------------------------------------------
#pragma once


#if USE_MODULES
import Common.ListSubstringPrioritizer;
import Common.AutoResource;
import Common.AutoResource.Windows;
import Common.ArrayRef;
import Common.String;
import Attributes;
import DrawingCanvas;
import DrawableObject;
import TextTreeParser;
#else
#include "Common.ListSubstringPrioritizer.h"
#include "Common.AutoResource.h"
#include "Common.AutoResource.Windows.h"
#include "Common.ArrayRef.h"
#include "Common.String.h"
#include "Attributes.h"
#include "DrawingCanvas.h"
#include "DrawableObject.h"
#include "TextTreeParser.h"
#endif


//interface AttributeSource;
//class DrawableObject; // causes - fatal error C1001: An internal error has occurred in the compiler.

// Combination of the drawable object and its associated attribute values.
struct DrawableObjectAndValues : public IAttributeSource
{
public:
    ComPtr<DrawableObject> drawableObject_;
    AttributeValue values_[DrawableObjectAttributeTotal];
    std::u16string label_;
    RECT labelRect_;            // Label rectangle in post-transform canvas coordinates.
    D2D_RECT_F objectRect_;     // Object rectangle in post-transform canvas coordinates. Best rounded to whole pixel.
    D2D_RECT_F layoutBounds_;   // Extents of layout boundary, in pre-transform world coordinates.
    D2D_RECT_F contentBounds_;  // Actual content boundary, in pre-transform world coordinates.
    CachedTransform transform_; // Transform from world coordinates to 
    D2D_POINT_2F origin_;       // Offset from <0,0>. May be non-zero if rotation exists or content is larger than layout.

public:
    // IAttributeSource implementation.
    virtual HRESULT GetString(uint32_t id, _Out_ array_ref<char16_t>& value) override;
    using IAttributeSource::GetString;

    virtual HRESULT GetValueData(uint32_t id, _Out_ Attribute::Type& type, _Out_ array_ref<uint8_t>& value) override;

    virtual HRESULT GetCookie(uint32_t id, _Inout_ uint32_t& cookieValue);

public:
    //////////
    bool IsVisible() const;

    HRESULT Set(DrawableObjectAttribute attributeIndex, _In_z_ char16_t const* stringValue);

    HRESULT Set(DrawableObjectAttribute attributeIndex, _In_z_ uint32_t value);

    // Call after setting string values (not every single set call, but before Draw).
    // This creates the drawableObject if not already created and forwards the call
    // to the internal drawableObject::Update.
    void Update();

    // Call when copying from an existing one.
    void Invalidate();

    bool IsPointInside(float x, float y) const;

    //////////
    // Static helpers

    // Arrange all the objects onto where they belong on the canvas.
    // This should be called at least once before Draw and then after
    // attribute changes, which will affect where the objects fit.
    // Hidden objects will be skipped, where DrawableObjectAttributeVisibility == false.
    // Objects with fixed positions will be drawn at their locations. Others
    // will use the padding and be laid out sequentially based on size.
    static void Arrange(
        array_ref<DrawableObjectAndValues> drawableObjects,
        DrawingCanvas& drawingCanvas
        );

    // Set the string value for the given attribute across the drawable objects.
    static void Set(
        array_ref<DrawableObjectAndValues> drawableObjects,
        array_ref<uint32_t const> drawableObjectsIndices,
        uint32_t attributeIndex,
        std::u16string const& newText
        );

    // Draw all drawable objects.
    // Arrange should have already been called. Otherwise objects will be drawn
    // at the default position <0,0> and overlap each other.
    // Hidden objects will be skipped, where DrawableObjectAttributeVisibility == false.
    static void Draw(
        array_ref<DrawableObjectAndValues> drawableObjects,
        DrawingCanvas& drawingCanvas,
        DX_MATRIX_3X2F const& canvasTransform
        );

    // Update the given drawable objects. This is usually called after a series
    // of SetStringValue calls.
    static void Update(
        array_ref<DrawableObjectAndValues> drawableObjects,
        array_ref<uint32_t const> drawableObjectsIndices
        );

    // Get the appropriate string value for the given attribute index across
    // multiple objects. If the values are consistent across all interested
    // objects, return that value. Otherwise return the default string.
    static char16_t const* GetStringValue(
        array_ref<DrawableObjectAndValues> drawableObjects,
        array_ref<uint32_t> drawableObjectIndices,
        uint32_t attributeIndex,
        _In_z_ char16_t const* defaultStringIfMixedValues // What to return if values are mixed between the objects.
        );

    static void Load(
        TextTree::NodePointer node,
        _Inout_ std::vector<DrawableObjectAndValues>& drawableObjects
        );

    static void Merge(
        DrawableObjectAndValues const& overridingDrawableObject,
        _Inout_ array_ref<DrawableObjectAndValues> drawableObjects
        );

    static void Store(
        array_ref<DrawableObjectAndValues> drawableObjects,
        TextTree::NodePointer objectsNode
        );
};
