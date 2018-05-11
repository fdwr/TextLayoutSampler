//----------------------------------------------------------------------------
//  History:    2015-06-19 Dwayne Robinson - Created
//----------------------------------------------------------------------------
#pragma once


#if USE_CPP_MODULES
import Common.String;
import Common.ArrayRef;
import Common.OptionalValue;
import Common.ListSubstringPrioritizer;
#else
#include "Common.String.h"
#include "Common.ArrayRef.h"
#include "Common.OptionalValue.h"
#include "Common.ListSubstringPrioritizer.h"
#endif


// Definition of attribute, including the type, name, and default values.
// The current value is stored separately.
struct Attribute
{
    // Basic data types (no complex classes with state cleanup).
    enum Type : uint8_t
    {
        TypeNone,
        TypeBool8,          // true/false
        TypeCharacter8,     // UTF-8 variable size characters
        TypeCharacter16,    // UTF-16 variable size characters
        TypeCharacter32,    // Full character codes
        TypeInteger8,
        TypeUInteger8,
        TypeInteger16,
        TypeUInteger16,
        TypeInteger32,
        TypeUInteger32,
        TypeFloat32,
        TypePointer,
        TypeTotal,
        TypeBaseMask = 127,
        TypeLargestSizeof = 8, // Byte count of largest type.

        // Array types for the most common ones.
        TypeArray = 128, // Can be ORed with earlier types.
        TypeString8 = TypeCharacter8 | TypeArray,
        TypeString16 = TypeCharacter16 | TypeArray,
        TypeArrayUInteger8 = TypeUInteger8 | TypeArray,
        TypeArrayUInteger16 = TypeUInteger16 | TypeArray,
        TypeArrayUInteger32 = TypeUInteger32 | TypeArray,
        TypeArrayFloat32 = TypeFloat32 | TypeArray,
    };

    template <typename T> struct TypeMap { static const Type type = TypeNone;};
    template<> struct TypeMap<bool> { static const Type type = TypeBool8; };
    template<> struct TypeMap<char> { static const Type type = TypeBool8; };
    template<> struct TypeMap<wchar_t> { static const Type type = TypeCharacter16; };
    template<> struct TypeMap<char16_t> { static const Type type = TypeCharacter16; };
    template<> struct TypeMap<char32_t> { static const Type type = TypeCharacter32; };
    template<> struct TypeMap<uint8_t> { static const Type type = TypeInteger8; };
    template<> struct TypeMap<int8_t> { static const Type type = TypeUInteger8; };
    template<> struct TypeMap<int16_t> { static const Type type = TypeInteger16; };
    template<> struct TypeMap<uint16_t> { static const Type type = TypeUInteger16; };
    template<> struct TypeMap<int32_t> { static const Type type = TypeInteger32; };
    template<> struct TypeMap<uint32_t> { static const Type type = TypeUInteger32; };
    template<> struct TypeMap<float> { static const Type type = TypeFloat32; };
    template<> struct TypeMap<DWORD> { static const Type type = TypeUInteger32; }; // Mainly for COLORREF
    template<typename T> struct TypeMap<T*> { static const Type type = TypePointer; };

    // Special semantics for parsing the string or presenting user
    // interface options. For example, a color value informs the
    // application an appropriate picker to the user.
    enum Semantic : uint8_t
    {
        SemanticNone,
        SemanticEnum,           // Usually used with Type*Integer32, or UInteger8 for named booleans.
        SemanticEnumExclusive,  // Only valid predefined values may be used.
        SemanticColor,          // Used with TypeUInteger32, TypeArrayUInteger8, or TypeArrayFloat32.
        SemanticDelta,          // Indicates this value is an offset rather than absolute value.
        SemanticLongText,       // Longer multi-line text.
        SemanticCharacterTags,  // Intended to be used with TypeUInteger32. e.g. vert kern liga.
        SemanticFilePath,       // Used with TypeString*.
        SemanticPassword,       // Used with TypeString*.
    };

    // Predefined value for an attribute to select from a list.
    // In the case of enumerations, the predefined values should include all
    // possible values. For strings and numbers, they should include a few
    // representative values for that attribute. The value can be either a
    // string or a number, depending on the type, in which case the other
    // field is optional/ignored.
    struct PredefinedValue
    {
        uint32_t integerValue;      // Numeric form of the value (optional if not numeric Attribute::type)
        char16_t const* name;       // Optional if the value has no name.
        char16_t const* stringValue;// String identifier (optional if numeric Attribute::type)

        char16_t const* GetName() const;

        static HRESULT MapNameToValue(
            array_ref<Attribute::PredefinedValue const> values,
            _In_z_ char16_t const* stringValue,
            _Out_ uint32_t& value
            );
    };

    using char8_t = unsigned char; // Code points should never be signed.

    // A live value for an attribute.
    union Variant
    {
        bool b;
        char8_t ch8;
        char16_t ch16;
        char32_t ch32;
        int8_t i8;
        uint8_t ui8;
        int16_t i16;
        uint16_t ui16;
        int32_t i32;
        uint32_t ui32;
        float f32;
        void* p;
        struct
        {
            uint8_t buffer[TypeLargestSizeof];
            Type type; // Matches corresponding Attribute.type (or TypeNone if not initialized yet).
        };
    };

    //////////
    Type type;                          // Data type of attribute.
    Semantic semantic;                  // Specific semantics of the data type.
    uint16_t category;                  // Arbitrary flags for the caller to utilize when filtering.

    uint32_t id;                        // Numeric identifier defined by users of this structure.
    char16_t const* name;               // Name of the attribute. e.g. "layout_width"
    char16_t const* display;            // Display string. e.g. "Text layout width"

    // List of possible values, particularly useful for enums.
    _Maybenull_ char16_t const* defaultValue;
    array_ref<Attribute::PredefinedValue const> predefinedValues;

    char16_t const* description;        // Describe the attribute to the user.

    ////////////////////////////////////////

    static bool IsTypeArray(Type type);
    static bool AreCompatibleTypes(Attribute::Type type1, Attribute::Type type2); // Compatible if merely different in sign.
    static size_t GetTypeSizeof(Attribute::Type type);
    static bool IsTypeNumeric(Attribute::Type type);
    static bool IsTypeParsable(Attribute::Type type); // Requires parsing, not just a string of characters.
    static Type GetBaseType(Attribute::Type type);

    bool IsTypeArray() const noexcept { return IsTypeArray(this->type); }
    bool AreCompatibleTypes(Attribute::Type type2) const noexcept { return AreCompatibleTypes(this->type, type2); }
    size_t GetTypeSizeof() const noexcept { return GetTypeSizeof(this->type); }

    char16_t const* GetPredefinedValueName(uint32_t valueIndex) const;

    enum GetPredefinedValueFlags : uint32_t
    {
        GetPredefinedValueFlagsNone = 0,
        GetPredefinedValueFlagsString = 1,    // Return string form if present.
        GetPredefinedValueFlagsName = 2,      // Return the name as the value.
        GetPredefinedValueFlagsInteger = 4,   // Return string representation of integer.
    };
    char16_t const* GetPredefinedValue(uint32_t valueIndex, GetPredefinedValueFlags getValueFlags, OUT std::u16string& buffer) const;

    ////////////////////////////////////////
    // Mapping functions from strings to types. They return an error if the
    // stringValue is empty or does not map to a valid type
    // HRESULT_FROM_WIN32(ERROR_UNMAPPED_SUBSTITUTION_STRING).

    // Low level function to read a single value.
    HRESULT ParseString(_In_z_ char16_t const* stringValue, _Out_ char16_t const** nextStringValue, _Inout_ Variant& data) const;

    // Parse the string into an array of the primitive data type written to a byte region.
    // Empty string is okay for this function because it returns an empty vector with S_OK.
    HRESULT ParseString(_In_z_ char16_t const* stringValue, _Inout_ std::vector<uint8_t>& data) const;

    // Map a named identifier to a value or vice versa.
    HRESULT MapNameToValue(_In_z_ char16_t const* stringValue, _Out_ uint32_t& enumValue) const;
    HRESULT MapValueToName(_Out_ uint32_t enumValue, _Out_ std::u16string& stringValue) const;
    HRESULT ParseEnum(_In_z_ char16_t const* stringValue, _Out_ uint32_t& enumValue) const;
    HRESULT ParseBool(_In_z_ char16_t const* stringValue, _Out_ bool& boolValue) const;
    HRESULT ParseColor(_In_z_ char16_t const* stringValue, _Out_ uint32_t& integerValue) const;
    static HRESULT ParseInteger(_In_z_ char16_t const* stringValue, _Out_ uint32_t& integerValue);
    static HRESULT ParseIntegerHex(_In_z_ char16_t const* stringValue, _Out_ uint32_t& integerValue);
    static HRESULT ParseIntegerTag(_In_z_ char16_t const* stringValue, _Out_ uint32_t& integerValue);
    static HRESULT ParseFloat(_In_z_ char16_t const* stringValue, _Out_ float& floatValue);
    static void StringifyColor(uint32_t colorValue, _Out_ std::u16string& buffer);

    // Get list of predefined values in priority order based on the string.
    // This is useful for UI where you want the best match to be at the top.
    HRESULT GetPredefinedValueIndices(
        _In_z_ char16_t const* stringValue,
        _Out_ array_ref<uint32_t> orderedIndices
        ) const;
};

DEFINE_ENUM_FLAG_OPERATORS(Attribute::GetPredefinedValueFlags);


// Each attribute value has a string representation and a cached binary form.
// Note the dataArray is empty if the data has a single element, small
// enough to just fit in the variant.
struct AttributeValue
{
    Attribute::Variant data; // Room for one element, which is the common case.
    std::vector<uint8_t> dataArray; // Variable length data in case the fixed size variant is too small.
    std::u16string stringValue; // string representation, typed by user or read from data file.
    uint32_t cookieValue = 0; // useful to compare for value changes, incremented each time.

    array_ref<uint8_t> Get(); // Get the data.
    HRESULT Set(Attribute const& attribute, _In_z_ char16_t const* newStringValue);

    AttributeValue()
    {
        data.type = Attribute::TypeNone;
    }
};


// Source interface for reading attribute values.
// - Each indexed attribute has both a string form (GetString) and a cached
//   binary form (GetValueData). In cases where the data type actually is a
//   string, the string and data array may have identical content (barring
//   any escape sequences).
// - The implementation may store the backing values however it wants, but
//   utilizing AttributeValue is convenient.
interface IAttributeSource
{
public:
    ////////////////////////////////////////
    // String form getters.
    // - The string is both length delimited and nul-terminated.
    // - The returned reference must remain valid for the lifetime of the
    //   function that was passed the IAttributeSource, and it should be
    //   the same pointer each call.

    // Get a lightweight reference to the indexed string.
    // On error, it returns empty string.
    array_ref<char16_t> GetString(uint32_t id);

    // Return copy of the string rather than view.
    HRESULT GetString(uint32_t id, _Out_ std::u16string& s);

    // Client implemented. The other two forms are conveniences that just
    // forward to this implementation.
    virtual HRESULT GetString(uint32_t id, _Out_ array_ref<char16_t>& value) = 0;

    ////////////////////////////////////////
    // Value getters.
    // - Any data value pointers returned must stay valid for the lifetime of
    //   the function that was passed the IAttributeSource. The caller should
    //   make a copy if needed longer than that.
    // - These functions are called frequently, so overhead should be light.

    // Get single value of specific type (not binary data).
    // If the actual type is incompatible with the desired type or the value
    // is empty, it returns the default value and HRESULT for
    // ERROR_UNMAPPED_SUBSTITUTION_STRING.
    template <typename T>
    T GetValue(uint32_t id, T defaultValue)
    {
        Attribute::Type actualType;
        Attribute::Type desiredType = Attribute::TypeMap<T>::type;
        array_ref<uint8_t> byteValues;

        if (FAILED(GetValueData(id, OUT actualType, OUT byteValues)) || byteValues.size() < sizeof(T))
        // todo::: restore once you figure out enums! || !Attribute::AreCompatibleTypes(desiredType, actualType))
        {
            // Return default value on error ERROR_UNMAPPED_SUBSTITUTION_STRING.
            return defaultValue;
        }
        return *reinterpret_cast<T*>(byteValues.data());
    }

    // Returns optional value. The value will be initialized if present, else empty.
    template <typename T>
    optional_value<T> GetOptionalValue(uint32_t id)
    {
        Attribute::Type actualType;
        array_ref<uint8_t> byteValues;
        optional_value<T> value;
        if (FAILED(GetValueData(id, OUT actualType, OUT byteValues)) || byteValues.size() < sizeof(T))
        {
            return value;
        }
        T& t = *reinterpret_cast<T*>(byteValues.data());
        value.emplace(t);
        return value;
    }

    // Return whether the desired type exists.
    template <typename T>
    bool HasValue(uint32_t id)
    {
        Attribute::Type actualType;
        array_ref<uint8_t> byteValues;
        if (FAILED(GetValueData(id, OUT actualType, OUT byteValues)) || byteValues.size() < sizeof(T))
        {
            return false;
        }
        return true;
    }

    // Get an array of specific data types.
    // If the actual type is incompatible with the desired type, it returns
    // an empty array and HRESULT for ERROR_UNMAPPED_SUBSTITUTION_STRING.
    template <typename T>
    HRESULT GetValues(uint32_t id, _Out_ array_ref<T>& values)
    {
        Attribute::Type actualType;
        Attribute::Type desiredType = Attribute::TypeMap<T>::type;
        array_ref<uint8_t> byteValues;

        IFR(GetValueData(id, OUT actualType, OUT byteValues));
        // // todo::: restore once you figure out enums! || !Attribute::AreCompatibleTypes(desiredType, actualType))
        //if (!Attribute::AreCompatibleTypes(desiredType, actualType))
        //{
        //    return HRESULT_FROM_WIN32(ERROR_UNMAPPED_SUBSTITUTION_STRING);
        //}
        values.reset(byteValues.reinterpret_as<T>());
        return S_OK;
    }

    // Get an array of specific data types.
    // If the actual type is incompatible with the desired type, it returns
    // an empty array and HRESULT for ERROR_UNMAPPED_SUBSTITUTION_STRING.
    template <typename T>
    array_ref<T> GetValues(uint32_t id)
    {
        Attribute::Type actualType;
        Attribute::Type desiredType = Attribute::TypeMap<T>::type;
        array_ref<uint8_t> byteValues;
        array_ref<T> values;

        if (SUCCEEDED(GetValueData(id, OUT actualType, OUT byteValues)))
        {
            values.reset(byteValues.reinterpret_as<T>());
        }
        // // todo::: restore once you figure out enums! || !Attribute::AreCompatibleTypes(desiredType, actualType))
        //if (!Attribute::AreCompatibleTypes(desiredType, actualType))
        //{
        //    return HRESULT_FROM_WIN32(ERROR_UNMAPPED_SUBSTITUTION_STRING);
        //}
        return values;
    }

    // Client implemented. Callers generally use the more convenient ones above,
    // since the actual implementation just returns raw byte data.
    virtual HRESULT GetValueData(uint32_t id, _Out_ Attribute::Type& type, _Out_ array_ref<uint8_t>& value) = 0;

    ////////////////////////////////////////
    // Cookie functions for value modification awareness.

    // Returns true if the setting has been modified, updating the caller's
    // cookie value to the current one.
    bool IsCookieSame(uint32_t id, _Inout_ uint32_t& previousCookieValue)
    {
        uint32_t cookieValue = 0;
        GetCookie(id, OUT cookieValue);
        if (cookieValue == previousCookieValue)
            return true;

        previousCookieValue = cookieValue;
        return false;
    }

    // Retrieve the last update for this attribute, enabling a caller to
    // cache results for expensive operations. It's okay to update the
    // cookie value even if the same value was set for an attribute (it just
    // means the client will lose caching), but the cookieValue must be
    // updated each time that attribute is changed (incrementing is
    // simple enough).
    virtual HRESULT GetCookie(uint32_t id, _Out_ uint32_t& cookieValue) = 0;
};
