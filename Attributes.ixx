//----------------------------------------------------------------------------
//  History:    2015-06-19 Dwayne Robinson - Created
//----------------------------------------------------------------------------
#include "precomp.h"

#include "Attributes.h"

////////////////////////////////////////

char16_t const* Attribute::PredefinedValue::GetName() const
{
    return name != nullptr ? name : u"";
}


char16_t const* Attribute::GetPredefinedValueName(uint32_t valueIndex) const
{
    if (valueIndex >= this->predefinedValues.size())
        return u"";

    return this->predefinedValues[valueIndex].GetName();
}


char16_t const* Attribute::GetPredefinedValue(uint32_t valueIndex, GetPredefinedValueFlags flags, OUT std::u16string& buffer) const
{
    if (valueIndex >= this->predefinedValues.size())
        return u"";

    auto const& predefinedValue = this->predefinedValues[valueIndex];

    if ((flags & GetPredefinedValueFlagsString) && predefinedValue.stringValue != nullptr)
        return predefinedValue.stringValue;

    if ((flags & GetPredefinedValueFlagsName) && predefinedValue.name != nullptr)
        return predefinedValue.name;

    if (flags & GetPredefinedValueFlagsInteger)
    {
        if (semantic == SemanticColor)
        {
            StringifyColor(predefinedValue.integerValue, OUT buffer);
        }
        else
        {
            auto& newBuffer = std::to_wstring(predefinedValue.integerValue);
            auto& recastBuffer = reinterpret_cast<std::u16string&>(newBuffer);
            buffer = recastBuffer;
        }
        return buffer.c_str();
    }

    return u"";
}


HRESULT Attribute::ParseString(
    _In_z_ char16_t const* stringValue,
    _Inout_ std::vector<uint8_t>& data
    ) const
{
    data.clear();

    if (IsTypeParsable(this->type))
    {
        // Read each element from string, parsing into data types.

        Attribute::Variant buffer;
        size_t typeSize = GetTypeSizeof(this->type);

        while (stringValue[0] != '\0')
        {
            size_t dataOldSize = data.size();
            IFR(ParseString(stringValue, OUT &stringValue, IN OUT buffer));
            data.resize(dataOldSize + typeSize);
            memcpy(&data[dataOldSize], buffer.buffer, typeSize);
        }
    }
    else
    {
        // Copy the characters directly rather than parsing as elements.

        size_t stringLength = wcslen(ToWChar(stringValue));

        static_assert(Attribute::TypeTotal == 13, "Update this table in case a new string type was added.");
        switch (GetBaseType(this->type))
        {
        case TypeCharacter16:
            {
                // Copy UTF-16 characters directly.
                auto byteLength = stringLength * sizeof(char16_t);
                data.resize(byteLength); // Pad with nul.
                memcpy(data.data(), stringValue, byteLength);
            }
            break;

        case TypeCharacter32: // Full UTF-32 character codes
            {
                // Promote UTF-16 to UTF-32 in a single call.
                size_t maxByteLength = stringLength * sizeof(char32_t);
                data.resize(maxByteLength);
                char32_t* utf32text = reinterpret_cast<char32_t*>(data.data());

                size_t convertedLength = static_cast<uint32_t>(ConvertTextUtf16ToUtf32NoReplacement(
                    {stringValue, stringValue + stringLength},
                    { utf32text, utf32text + stringLength},
                    nullptr // sourceCount
                    ));
                data.resize(convertedLength * sizeof(char32_t));
            }
            break;

        default:
            assert(false);
        case TypeCharacter8:  // UTF-8 variable size characters
            return HRESULT_FROM_WIN32(ERROR_UNMAPPED_SUBSTITUTION_STRING);
        }
    }
    return S_OK;
}


HRESULT Attribute::ParseString(
    _In_z_ char16_t const* stringValue,
    _Out_ char16_t const** nextStringValue,
    _Inout_ Variant& data
    ) const
{
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_UNMAPPED_SUBSTITUTION_STRING);

    if (stringValue == '\0')
        return hr;

    static_assert(Attribute::TypeTotal == 13, "Update this switch statement.");
    switch (GetBaseType(this->type))
    {
    case TypeBool8:
        stringValue = SkipSpaces(stringValue);
        hr = ParseBool(stringValue, OUT data.b);
        stringValue = SkipToEnd(stringValue);
        break;

    case TypeInteger8:
    case TypeUInteger8:
    case TypeInteger16:
    case TypeUInteger16:
    case TypeInteger32:
    case TypeUInteger32:
        stringValue = SkipSpaces(stringValue);
        switch (this->semantic)
        {
        case SemanticEnum:
        case SemanticEnumExclusive:
            hr = ParseEnum(stringValue, OUT data.ui32);
            stringValue = SkipToEnd(stringValue);
            break;

        case SemanticColor:
            hr = ParseColor(stringValue, OUT data.ui32);
            stringValue = SkipToEnd(stringValue);
            break;

        case SemanticCharacterTags:
            hr = ParseIntegerTag(stringValue, OUT data.ui32);
            stringValue = SkipToNextWord(stringValue);
            break;

        default:
            hr = ParseInteger(stringValue, OUT data.ui32);
            stringValue = SkipToNextWord(stringValue);
            break;
        }
        break;

    case TypeFloat32:
        stringValue = SkipSpaces(stringValue);
        hr = ParseFloat(stringValue, OUT data.f32);
        stringValue = SkipToNextWord(stringValue);
        break;

    case TypeCharacter8:
        hr = S_OK;
        data.ch8 = static_cast<char8_t>(*stringValue); // Read a single character, truncated to ASCII.
        ++stringValue;
        break;

    case TypeCharacter16:
        hr = S_OK;
        data.ch16 = *stringValue; // Read a single character.
        ++stringValue;
        break;

    case TypeCharacter32:
        {
            hr = S_OK;
            size_t sourceCount = 0;
            UnicodeCharacterReader reader(stringValue, stringValue + 2);
            data.ch32 = reader.ReadNext();
            if (reader.front() != '\0')
                stringValue = reader.data();
        }
        break;

    default:
    // case TypeNone:
    // case TypePointer:
        hr = HRESULT_FROM_WIN32(ERROR_UNMAPPED_SUBSTITUTION_STRING);
        stringValue = SkipToEnd(stringValue);
        break;
    }

    // Return the updated string pointer after being moved forward.
    *nextStringValue = stringValue;

    return hr;
}


HRESULT Attribute::MapValueToName(_Out_ uint32_t enumValue, _Out_ std::u16string& stringValue) const
{
    stringValue.clear();

    // Look for enum value.
    for (uint32_t i = 0, ci = uint32_t(this->predefinedValues.size()); i < ci; ++i)
    {
        if (this->predefinedValues[i].integerValue == enumValue)
        {
            stringValue = this->predefinedValues[i].name;
            return S_OK;
        }
    }

    // Otherwise look for a numeric value, confirming that numeric value is
    // actually in the enumeration set.
    auto& newString = std::to_wstring(enumValue);
    auto& recastString = reinterpret_cast<std::u16string&>(newString);
    std::swap(stringValue, recastString);

    return S_OK;
}


HRESULT Attribute::PredefinedValue::MapNameToValue(
    array_ref<Attribute::PredefinedValue const> values,
    _In_z_ char16_t const* stringValue,
    _Out_ uint32_t& value
    )
{
    value = 0;

    // Look for matching name string, such as "DWRITE_WORD_WRAPPING_NO_WRAP".
    for (auto const& predefinedValue : values)
    {
        if (_wcsicmp(ToWChar(predefinedValue.name), ToWChar(stringValue)) == 0)
        {
            value = predefinedValue.integerValue;
            return S_OK;
        }
    }

    // There were no matches for this string.
    return HRESULT_FROM_WIN32(ERROR_UNMAPPED_SUBSTITUTION_STRING);
}


HRESULT Attribute::MapNameToValue(_In_z_ char16_t const* stringValue, _Out_ uint32_t& value) const
{
    return PredefinedValue::MapNameToValue(this->predefinedValues, stringValue, OUT value);
}


namespace
{
    HRESULT ValidateStringValueParameter(_In_z_ char16_t const* stringValue)
    {
        if (stringValue == nullptr)
            return E_INVALIDARG;

        if (stringValue[0] == '\0')
            return HRESULT_FROM_WIN32(ERROR_UNMAPPED_SUBSTITUTION_STRING);

        return S_OK;
    }
}


HRESULT Attribute::ParseEnum(_In_z_ char16_t const* stringValue, _Out_ uint32_t& enumValue) const
{
    enumValue = 0;
    ISR(MapNameToValue(stringValue, OUT enumValue));

    // No string matched. So try parsing the string as raw integer.
    IFR(ParseInteger(stringValue, OUT enumValue));

    // Check whether there is a match in the value list.
    if (semantic == SemanticEnumExclusive)
    {
        for (auto& predefinedValue : this->predefinedValues)
        {
            if (predefinedValue.integerValue == enumValue)
            {
                return S_OK;
            }
        }
        return HRESULT_FROM_WIN32(ERROR_UNMAPPED_SUBSTITUTION_STRING);
    }

    return S_OK;
}


HRESULT Attribute::ParseBool(_In_z_ char16_t const* stringValue, _Out_ bool& boolValue) const
{
    IFR(ValidateStringValueParameter(stringValue));

    uint32_t enumValue = 0;
    if (SUCCEEDED(MapNameToValue(stringValue, OUT enumValue)))
    {
        boolValue = !!enumValue;
        return S_OK;
    }

    // No string matched, so check a few known ones.
    if (_wcsicmp(ToWChar(stringValue), L"true") == 0
    ||  _wcsicmp(ToWChar(stringValue), L"yes") == 0
    ||  _wcsicmp(ToWChar(stringValue), L"on") == 0)
    {
        boolValue = true;
        return S_OK;
    }

    if (_wcsicmp(ToWChar(stringValue), L"false") == 0
    ||  _wcsicmp(ToWChar(stringValue), L"no") == 0
    ||  _wcsicmp(ToWChar(stringValue), L"off") == 0)
    {
        boolValue = false;
        return S_OK;
    }

    // No string matched. So try parsing the string as raw integer.
    if (SUCCEEDED(ParseInteger(stringValue, OUT enumValue)))
    {
        boolValue = !!enumValue;
        return S_OK;
    }

    return HRESULT_FROM_WIN32(ERROR_UNMAPPED_SUBSTITUTION_STRING);
}


void Attribute::StringifyColor(uint32_t colorValue, _Out_ std::u16string& buffer)
{
    buffer = u"#";
    buffer.resize(1 + 8);
    for (uint32_t i = 8; i > 0; --i)
    {
        const static char* indexToChar = "0123456789ABCDEF";
        buffer[i] = indexToChar[colorValue & 0xF];
        colorValue >>= 4;
    }
}


HRESULT Attribute::ParseColor(_In_z_ char16_t const* stringValue, _Out_ uint32_t& colorValue) const
{
    colorValue = 0xFF000000;
    IFR(ValidateStringValueParameter(stringValue));

    ISR(MapNameToValue(stringValue, OUT colorValue));

    char16_t* stringValueEnd = const_cast<char16_t*>(stringValue);

    if (stringValue[0] == '#')
    {
        // Read single hex value in ARGB order (e.g. #FF80E0DC).
        ++stringValue;
        colorValue = std::wcstoul(ToWChar(stringValue), OUT ToWChar(&stringValueEnd), 16);
        if (stringValueEnd == stringValue)
            return HRESULT_FROM_WIN32(ERROR_UNMAPPED_SUBSTITUTION_STRING);

        for (uint32_t i = 0; i < 7; ++i)
        {
            if (stringValue[i] == '\0')
                colorValue |= 0xFF000000; // Add alpha if not a full eight characters
        }
    }
    else
    {
        uint32_t partialColorValue = 0;

        // Read triplet of decimal values with optional alpha in ARGB order (e.g. 255 128 192 224).
        for (uint32_t i = 0; i < 4; ++i)
        {
            uint32_t integerValue = std::wcstoul(ToWChar(stringValue), OUT ToWChar(&stringValueEnd), 10);
            if (stringValueEnd == stringValue)
                return HRESULT_FROM_WIN32(ERROR_UNMAPPED_SUBSTITUTION_STRING);

            partialColorValue <<= 8;
            partialColorValue |= std::min(integerValue, 255ui32);
            stringValue = SkipToNextWord(stringValue);

            if (stringValue[0] == '\0')
            {
                if (i < 3)
                    partialColorValue |= 0xFF000000; // Add full alpha if not given.
                break;
            }
        }
        colorValue = partialColorValue;
    }

    return S_OK;
}


HRESULT Attribute::ParseInteger(_In_z_ char16_t const* stringValue, _Out_ uint32_t& integerValue)
{
    integerValue = 0;
    IFR(ValidateStringValueParameter(stringValue));
    char16_t* stringValueEnd = const_cast<char16_t*>(stringValue);

    integerValue = std::wcstoul(ToWChar(stringValue), OUT ToWChar(&stringValueEnd), 10);

    return (stringValueEnd > stringValue) ? S_OK : HRESULT_FROM_WIN32(ERROR_UNMAPPED_SUBSTITUTION_STRING);
}


HRESULT Attribute::ParseIntegerHex(_In_z_ char16_t const* stringValue, _Out_ uint32_t& integerValue)
{
    integerValue = 0;
    IFR(ValidateStringValueParameter(stringValue));
    char16_t* stringValueEnd = const_cast<char16_t*>(stringValue);

    integerValue = std::wcstoul(ToWChar(stringValue), OUT ToWChar(&stringValueEnd), 16);
    return (stringValueEnd > stringValue) ? S_OK : HRESULT_FROM_WIN32(ERROR_UNMAPPED_SUBSTITUTION_STRING);
}


HRESULT Attribute::ParseIntegerTag(_In_z_ char16_t const* stringValue, _Out_ uint32_t& integerValue)
{
    integerValue = 0;
    IFR(ValidateStringValueParameter(stringValue));

    // Copy the individual characters into the bytes of the integer.
    // Note it is endianness agnostic, always writing the bytes in memory order.
    uint8_t* tagArray = reinterpret_cast<uint8_t*>(&integerValue);
    for (uint32_t i = 0; i < 4 && stringValue[i] != '\0'; ++i)
    {
        tagArray[i] = static_cast<uint8_t>(stringValue[i]);
    }

    return S_OK;
}


HRESULT Attribute::ParseFloat(_In_z_ char16_t const* stringValue, _Out_ float& floatValue)
{
    floatValue = 0.0f;
    IFR(ValidateStringValueParameter(stringValue));
    char16_t* stringValueEnd = const_cast<char16_t*>(stringValue);

    floatValue = std::wcstof(ToWChar(stringValue), OUT ToWChar(&stringValueEnd));
    return (stringValueEnd > stringValue) ? S_OK : HRESULT_FROM_WIN32(ERROR_UNMAPPED_SUBSTITUTION_STRING);
}


Attribute::Type Attribute::GetBaseType(Attribute::Type type)
{
    return Type(type & TypeBaseMask);
}


bool Attribute::AreCompatibleTypes(Attribute::Type type1, Attribute::Type type2)
{
    static_assert(Attribute::TypeTotal == 13, "Update this table");
    constexpr static uint8_t category[Attribute::TypeTotal] = {
        0, // TypeNone
        1, // TypeBool8
        2, // TypeCharacter8
        3, // TypeCharacter16
        4, // TypeCharacter32
        2, // TypeInteger8
        2, // TypeUInteger8
        3, // TypeInteger16
        3, // TypeUInteger16
        4, // TypeInteger32
        4, // TypeUInteger32
        5, // TypeFloat32
        6, // TypePointer
    };
    if (type1 == type2)
        return true;

    type1 = GetBaseType(type1);
    type2 = GetBaseType(type2);

    if (type1 >= TypeTotal || type2 >= TypeTotal)
        return false;
    
    return category[type1] == category[type2];
}


size_t Attribute::GetTypeSizeof(Attribute::Type type)
{
    static_assert(Attribute::TypeTotal == 13, "Update this table");
    constexpr static uint8_t typeSizeof[Attribute::TypeTotal] = {
        0, // TypeNone
        1, // TypeBool8
        1, // TypeCharacter8
        2, // TypeCharacter16
        4, // TypeCharacter32
        1, // TypeInteger8
        1, // TypeUInteger8
        2, // TypeInteger16
        2, // TypeUInteger16
        4, // TypeInteger32
        4, // TypeUInteger32
        4, // TypeFloat32
        sizeof(void*), // TypePointer
    };
    type = GetBaseType(type);
    if (type >= TypeTotal)
        return false;

    return typeSizeof[type];
}


bool Attribute::IsTypeArray(Type type)
{
    return !!(type & TypeArray);
}


bool Attribute::IsTypeNumeric(Attribute::Type type)
{
    // Consider numeric. Note boolean is considered numeric, from the point of view that false=0 and true=1.
    static_assert(Attribute::TypeTotal == 13, "Update this table");
    constexpr static bool isNumeric[Attribute::TypeTotal] = {
        false, // TypeNone
        true,  // TypeBool8
        false, // TypeCharacter8
        false, // TypeCharacter16
        false, // TypeCharacter32
        true,  // TypeInteger8
        true,  // TypeUInteger8
        true,  // TypeInteger16
        true,  // TypeUInteger16
        true,  // TypeInteger32
        true,  // TypeUInteger32
        true,  // TypeFloat32
        true,  // TypePointer
    };
    type = GetBaseType(type);
    if (type >= TypeTotal)
        return false;

    return isNumeric[type];
}


bool Attribute::IsTypeParsable(Attribute::Type type)
{
    // Requires special parsing
    static_assert(Attribute::TypeTotal == 13, "Update this table");
    constexpr static bool isTypeParseable[Attribute::TypeTotal] = {
        false, // TypeNone
        true,  // TypeBool8
        false, // TypeCharacter8
        false, // TypeCharacter16
        false, // TypeCharacter32
        true,  // TypeInteger8
        true,  // TypeUInteger8
        true,  // TypeInteger16
        true,  // TypeUInteger16
        true,  // TypeInteger32
        true,  // TypeUInteger32
        true,  // TypeFloat32
        true,  // TypePointer
    };
    type = GetBaseType(type);
    if (type >= TypeTotal)
        return false;

    return isTypeParseable[type];
}


HRESULT Attribute::GetPredefinedValueIndices(
    _In_z_ char16_t const* stringValue,
    _Out_ array_ref<uint32_t> orderedIndices
    ) const
{
    if (stringValue == nullptr)
        return E_INVALIDARG;

    auto predefinedValuesCount = static_cast<uint32_t>(predefinedValues.size());
    if (static_cast<uint32_t>(orderedIndices.size()) < predefinedValuesCount)
        return E_NOT_SUFFICIENT_BUFFER;

    if (predefinedValuesCount == 0)
        return S_OK;

    ListSubstringPrioritizer substringPrioritizer(ToChar16ArrayRef(stringValue), predefinedValuesCount);

    for (uint32_t i = 0; i < predefinedValuesCount; ++i)
    {
        auto& valueMapping = this->predefinedValues[i];
        auto* name = valueMapping.GetName();
        auto weight = substringPrioritizer.GetStringWeight(ToChar16ArrayRef(name));
        substringPrioritizer.SetItemWeight(i, weight);
    }

    substringPrioritizer.GetItemIndices(OUT orderedIndices, /*excludeMismatches*/false);

    return S_OK;
}


HRESULT AttributeValue::Set(Attribute const& attribute, _In_z_ char16_t const* newStringValue)
{
    // Increment the cookie value so that callers can know when the value is
    // different lest they cached results.
    ++this->cookieValue;

    size_t stringLength = wcslen(ToWChar(newStringValue));

    // Handle any special cases here.
    switch (attribute.semantic)
    {
    case attribute.SemanticFilePath:
        // Strip any quotes if present, which happens when pasted from copy-path using Explorer.
        if (newStringValue[0] == '\"')
        {
            ++newStringValue;
            --stringLength;
        }
        if (newStringValue[stringLength - 1] == '\"')
        {
            --stringLength;
        }
        break;
    }

    this->stringValue.assign(newStringValue, stringLength);

    HRESULT hr;
    if (attribute.IsTypeArray())
    {
        // If array type, initialize the variable length data.
        hr = attribute.ParseString(this->stringValue.c_str(), OUT this->dataArray);
    }
    else
    {
        // Otherwise just copy a single value out to the single unit variant.
        char16_t const* stringEnd = nullptr;
        hr = attribute.ParseString(this->stringValue.c_str(), OUT &stringEnd, OUT this->data);
    }

    // If successfully parsed, update the type. Otherwise leave as empty.
    this->data.type = SUCCEEDED(hr) ? attribute.type : Attribute::TypeNone;

    return hr;
}


array_ref<uint8_t> AttributeValue::Get()
{
    if (Attribute::IsTypeArray(data.type))
    {
        // Return the array data directly.
        return dataArray;
    }
    else
    {
        // Return the single instance field (the data array is not allocated).
        auto byteSize = Attribute::GetTypeSizeof(data.type);
        return byte_array_ref(&data.buffer[0], &data.buffer[byteSize]);
    }
}


// Return a wstring copy of the string as a wstring rather than
// a lightweight view into the text.
HRESULT IAttributeSource::GetString(uint32_t id, _Out_ std::u16string& s)
{
    array_ref<char16_t> value;
    IFR(GetString(id, OUT value));
    s.assign(value.data(), value.size());
    return S_OK;
}


array_ref<char16_t> IAttributeSource::GetString(uint32_t id)
{
    array_ref<char16_t> value;
    GetString(id, OUT value); // Ignore errors, returning empty value.
    return value;
}
