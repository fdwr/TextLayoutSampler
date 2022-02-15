//----------------------------------------------------------------------------
//  History:    2018-11-29 Dwayne Robinson - Created
//----------------------------------------------------------------------------

#if USE_CPP_MODULES
    module;
#endif

#include "precomp.h"
#include <assert.h>
#include <variant>

#if USE_CPP_MODULES
    export module Common.Variant;
    export
    {
        #include "Common.Variant.h"
    }
#else
    #include "Common.Variant.h"
#endif

#ifdef _DEBUG

// Standard variant.
void TestVariantStd()
{
    std::variant<float, char, int> v = 1234;

    v = 3.14f;   float f = std::get<float>(v); assert(f == 3.14f);
    v = 3;       int   i = std::get<int>(v);   assert(i == 3);
    v = char(3); int   c = std::get<char>(v);  assert(c == 3);

    bool isFloat = std::holds_alternative<float>(v);
    bool isInt = std::holds_alternative<int>(v);
    bool isChar = std::holds_alternative<char>(v);
    assert(!isFloat);
    assert(!isInt);
    assert(isChar);

    size_t currentIndex = v.index();
    size_t charIndex = variant_index<decltype(v), char>();
    assert(currentIndex == 1);
    assert(charIndex == 1);

    v = 3.0f;
    auto callback = [](auto& v)->void {v = v + 2; };
    std::visit(callback, v);
    float f2 = std::get<float>(v);
    assert(f2 == 5.0f);

    switch (v.index())
    {
    case variant_index<decltype(v), float>(): assert(true);  break;
    case variant_index<decltype(v), int>():   assert(false); break;
    case variant_index<decltype(v), char>():  assert(false); break;
    }
}

// Variant with discoverable methods.
void TestVariantIntuitive()
{
    variantex<float, char, int> v = 1234;

    v = 3.14f;   float f = v.get<float>(); assert(f == 3.14f);
    v = 3;       int   i = v.get<int>();   assert(i == 3);
    v = char(3); int   c = v.get<char>();  assert(c == 3);

    bool isFloat = v.is_type<float>();
    bool isInt = v.is_type<int>();
    bool isChar = v.is_type<char>();
    assert(!isFloat);
    assert(!isInt);
    assert(isChar);

    size_t currentIndex = v.index();
    size_t charIndex = v.index_of_type<char>();
    assert(currentIndex == 1);
    assert(charIndex == 1);

    v = 3.0f;
    auto callback = [](auto& v)->void {v = v + 2; };
    v.call(callback);
    float f2 = std::get<float>(v);
    assert(f2 == 5.0f);

    switch (v.index())
    {
    case v.index_of_type<float>(): assert(true);  break;
    case v.index_of_type<int>():   assert(false); break;
    case v.index_of_type<char>():  assert(false); break;
    }
}

struct variantex_test_class
{
    variantex_test_class()
    {
        TestVariantStd();
        TestVariantIntuitive();
    }
};
variantex_test_class variantex_test_class_instance;

#endif // _DEBUG
