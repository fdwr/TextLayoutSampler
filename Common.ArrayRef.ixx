//----------------------------------------------------------------------------
//  History:    2015-06-19 Dwayne Robinson - Created
//              2015-10-20 Forked
//----------------------------------------------------------------------------
#include "precomp.h"

#include "Common.ArrayRef.h"

////////////////////////////////////////

#ifdef _DEBUG

template <typename ResourceType = void*>
class WrapperType
{
public:
    WrapperType(ResourceType resource) : resource_(resource) {}
    WrapperType(WrapperType<ResourceType> const& other) : resource_(other.resource_) {}
    inline WrapperType() { resource_ = 0; }
    inline operator ResourceType() const noexcept { return resource_; }
    inline operator ResourceType&() noexcept { return resource_; }
    inline ResourceType* operator&() noexcept { return &resource_; }
    inline ResourceType operator->() const noexcept { return resource_; }
    inline WrapperType& operator=(ResourceType resource) { resource_ = resource; return *this; }

    ResourceType resource_;
};


void array_ref_test()
{
    // array_ref works with std::initializer_list, but note you cannot pass an
    // initializer literal directly to the constructor as it is ambiguous to
    // the compiler whether you are passing an initializer list or calling
    // a constructor in-place. e.g.
    //
    //      array_ref<int32_t const> fromInitializerList({4,5,6});
    //
    // But you can wrap it with make_array_ref first, or assign it to a
    // transient local via auto.
    std::initializer_list<int32_t> integersInitializerList = { 1,2,3 };
    auto integersInitializerList2 = { 4,5,6 };
    array_ref<int32_t const> fromInitializerList(integersInitializerList);
    auto autoFromInitializerList(make_array_ref(integersInitializerList2));
    auto fromLiteralInitializerList2(make_array_ref({4,5,6}));

    uint32_t integersArray[] = { 1,2,3 };
    array_ref<uint32_t> fromArray(integersArray);
    array_ref<uint32_t const> fromArrayConst(integersArray);
    array_ref<uint32_t> fromArrayAssignment = integersArray;
    array_ref<uint32_t const> fromArrayConstAssignment = integersArray;
    array_ref<uint32_t const> fromArrayViaMakeArrayRef(make_array_ref(integersArray));
    fromArray[0] = 13;

    // Should be able to assign non-const array ref to const.
    array_ref<uint32_t> fromArrayRef = fromArray; // Calls default copy constructor.
    array_ref<uint32_t const> fromArrayRefConst = fromArrayConst; // Calls default copy constructor.
    array_ref<uint32_t const> fromArrayRefConst2 = fromArray; // Calls the templatized copy constructor.

    std::array<uint32_t, 3> integersStdArray = { 1,2,3 };
    std::array<uint32_t, 3> const integersStdArrayConst = { 1,2,3 };
    std::array<uint32_t const, 3> integersStdArrayConstInt = { 1,2,3 };
    array_ref<uint32_t> fromStdArray(integersStdArray);
    array_ref<uint32_t const> fromStdArrayConst(integersStdArrayConst);
    array_ref<uint32_t const> fromStdArrayConstInt(integersStdArrayConstInt);
    fromStdArrayConst = fromStdArrayConstInt;
    integersStdArray[0] = 13;

    std::u16string charactersString(u"Hello world");
    std::u16string const charactersStringConst(u"Hello world");
    array_ref<char16_t> fromWstring(charactersString);
    array_ref<char16_t const> fromWstringConst(charactersStringConst);
    fromWstring[0] = 'A';
    fromWstringConst.reset(fromWstringConst); // Reset to self.
    fromWstringConst.reset(fromWstring); // Reset from non-const.
    fromWstringConst.reset(fromWstring.data(), fromWstring.data_end());
    fromWstringConst.reset(fromWstring.data(), fromWstring.size());

    std::vector<int32_t> integersVector = { 1,2,3 };
    std::vector<int32_t> const integersVectorConst = { 1,2,3 };
    array_ref<int32_t> fromVector(integersVector);
    array_ref<int32_t const> fromVectorConst(integersVectorConst);
    fromVectorConst = integersVectorConst;
    integersVector[0] = 13;

    std::vector<WrapperType<int> > wrappedIntVector;
    array_ref<WrapperType<int> > fromWrappedIntVector(wrappedIntVector);
}


struct array_ref_test_class
{
    array_ref_test_class() { array_ref_test(); }
};
array_ref_test_class array_ref_test_class_instance;

#endif // _DEBUG
