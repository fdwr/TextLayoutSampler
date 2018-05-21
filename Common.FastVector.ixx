//----------------------------------------------------------------------------
//  History:    2018-04-30 Dwayne Robinson - Created
//----------------------------------------------------------------------------


#if USE_CPP_MODULES
import Common.ArrayRef;
import Common.FastVector;
#else
#define _SCL_SECURE_NO_WARNINGS // I hate doing this, but Visual Studio offers no substitute for std::uninitialized_copy.
#include "Common.ArrayRef.h"
#include "Common.FastVector.h"
#include <algorithm>
#include <assert.h>
#endif


////////////////////////////////////////

#ifdef _DEBUG

#define INCLUDE_EXCEPTION_TESTS 0 // Enable for bounds checking via at().

void fast_vector_test_as_parameter(fast_vector<int>& ints)
{
    ints.resize(20);
}

struct SimpleStruct
{
    char c;
};

struct ComplexStruct
{
    ComplexStruct()
    {
        s[0] += rand() % 26;
    }

    std::string s = "A";

    friend bool operator==(const ComplexStruct& lhs, const ComplexStruct& rhs);
};

bool operator==(const ComplexStruct& lhs, const ComplexStruct& rhs)
{
    return lhs.s == rhs.s;
};

struct SseSizedType
{
    alignas(16) uint8_t bytes[16];
};


void fast_vector_test()
{
    fast_vector<int, 20, false> ints;
    fast_vector<int, 30, true> ints2;

    assert(ints.size() == 0);
    assert(ints.capacity() == 20);
    assert(ints2.size() == 0);
    assert(ints2.capacity() == 30);
    ints.resize(1);
    ints.shrink_to_fit();
    assert(ints.capacity() == 20); // Capacity of fixed size vector does not change.

    // Explicitly verify that no data initialization happens when ShouldInitializeElements = false.
    int* originalDataPointer = ints.data();
    *originalDataPointer = 13;
    ints.resize(20);
    assert(ints.size() == 20);
    assert(ints.capacity() == 20);
    assert(ints.front() == 13);
    assert(ints.data() == originalDataPointer); // Should still point to internal array.

    // All elements should be initialized when ShouldInitializeElements = true.
    ints2.resize(20);
    assert(std::all_of(ints2.begin(), ints2.end(), [](auto v) { return v == 0; }));

    // Resize smaller. Capacity should remain the same.
    ints.resize(10);
    assert(ints.size() == 10);
    assert(ints.capacity() == 20);

    // Resize larger than the fixed size buffer to allocate memory.
    ints.resize(30);
    int* secondDataPointer = ints.data();
    assert(ints.size() == 30);
    assert(ints.capacity() == 30);
    assert(secondDataPointer != originalDataPointer); // Allocation should have happened.
    assert(ints.front() == 13); // Previous value should have been preserved.

    // Resize smaller again, using memory block this time. Capacity should remain large.
    ints.resize(10);
    assert(ints.size() == 10);
    assert(ints.capacity() == 30);
    ints.resize(0);
    assert(ints.size() == 0);
    assert(ints.capacity() == 30);

    // Grow even larger, enough to reallocate the memory block.
    // Exact capacity doesn't matter, as it is amortized growth.
    ints.reserve(40);
    assert(ints.capacity() == 40);
    assert(ints.front() == 13); // Value should still exist, even after reallocations.

    // Shrink the memory block to actual size.
    assert(ints.capacity() == 40);
    ints.resize(30);
    ints.shrink_to_fit();
    assert(ints.capacity() == 30);
    assert(ints.size() == 30);
    assert(ints.front() == 13); // Value should still exist, even after reallocations.

    // Verify incremental allocation upon resize.
    size_t originalCapacity = ints.capacity();
    ints.resize(31);
    size_t newCapacity = ints.capacity();
    assert(newCapacity > originalCapacity);
    ints.resize(32);
    assert(ints.capacity() == newCapacity); // Should not have reallocated again.

    // Clear it completely to verify capacity is reduced and memory freed.
    ints.clear();
    ints.shrink_to_fit();
    assert(ints.capacity() == 0);
    assert(ints.size() == 0);
    assert(ints.data() == nullptr); // If sized to zero, memory should be deallocated.
    ints.resize(30);
    ints.free();
    assert(ints.capacity() == 0);
    assert(ints.size() == 0);
    assert(ints.data() == nullptr); // If sized to zero, memory should be deallocated.

    // Append items to list.
    ints.resize(30);
    ints[0] = 13;
    ints.push_back(31);
    ints.push_back(32);
    assert(ints.front() == 13);
    assert(ints.back() == 32);
    assert(ints.size() == 30+2);

    // Assign new values.
    int xs[] = {1,2,3};
    ints.assign(xs);
    assert(ints.size() == 3);
    assert(ints.front() == 1);
    assert(ints.back() == 3);
    assert(*ints.data() == 1);
    assert(ints[0] == 1);
    assert(ints[2] == 3);

    // Create another vector from the old one.
    ints2[0] = 1;
    ints2[2] = 3;
    fast_vector<int, 20> ints3(ints2);
    ints.clear();
    assert(ints.size() == 0);
    assert(ints3.size() == 20);
    assert(ints3[0] == 1);
    assert(ints3[2] == 3);

    // Check manipulation through a function parameter.
    fast_vector_test_as_parameter(ints2);
    fast_vector_test_as_parameter(ints3);
    assert(ints2.size() == 20);
    assert(ints3.size() == 20);

    // Test more complex structures.
    fast_vector<SimpleStruct, 20, false> simpleStructs;
    fast_vector<ComplexStruct, 20, true> complexStructs;
    fast_vector<ComplexStruct, 20, true> complexStructs2;
    fast_vector<ComplexStruct, 21, true> complexStructs3;
    complexStructs.resize(10);

    // Check assignment with the exact same array count
    // and a differing array count.
    complexStructs2 = complexStructs;
    complexStructs3 = complexStructs;
    assert(complexStructs[0] == complexStructs2[0]);
    assert(complexStructs[0] == complexStructs3[0]);
    assert(complexStructs2.size() == complexStructs.size());
    assert(complexStructs2.capacity() >= complexStructs.capacity());

    // Verify that memory can be reassigned.
    fast_vector<uint32_t, 0, true> detachableMemory(20);
    fast_vector<uint8_t, 0, false> reattachedMemory;
    detachableMemory[0] = 42;
    detachableMemory[19] = 42;

    // Reattach memory and reinterpret it as bytes.
    // Then move it back and verify again.
    reattachedMemory.attach_memory(detachableMemory.detach_memory());
    assert(reattachedMemory[0 * 4] == 42);
    assert(reattachedMemory[19 * 4] == 42);
    detachableMemory.attach_memory(reattachedMemory.detach_memory());
    assert(detachableMemory[0] == 42);
    assert(detachableMemory[19] == 42);

    // Try the empty case.
    detachableMemory.clear();
    reattachedMemory.attach_memory(detachableMemory.detach_memory());
    assert(reattachedMemory.size() == 0);

    // Verify that memory can be reassigned.
    uint32_t memoryBuffer[20] = {10, 11, 12};
    fast_vector<uint32_t, 0, true> explicitMemory(fast_vector_use_memory_buffer, memoryBuffer);
    assert(memoryBuffer[0] == 10); // Can access local buffer via vector.
    explicitMemory.resize(2);
    assert(memoryBuffer[0] == 0); // First entry should have been wiped.
    assert(memoryBuffer[2] == 12); // Third entry should be intact.

    // Verify vector can be transferred.
    fast_vector<ComplexStruct, 4> complexStruct2(20);
    std::string originalValue = complexStruct2.front().s;
    fast_vector<ComplexStruct, 4> complexStruct3;
    complexStruct3.transfer_from(complexStruct2);
    assert(complexStruct3[0].s == originalValue);
    complexStruct2 = std::move(complexStruct3); // Now move back
    assert(complexStruct2[0].s == originalValue);
    fast_vector<ComplexStruct> complexStruct4(std::move(complexStruct2));
    assert(complexStruct4.size() == 20);
    assert(complexStruct2.size() == 0);
    fast_vector<ComplexStruct, 30> complexStruct5(std::move(complexStruct4));
    assert(complexStruct5.size() == 20);
    assert(complexStruct4.size() == 0);
    assert(complexStruct5[0].s == originalValue);

    // Try a type which has greater alignment needs than malloc provides.
    fast_vector<SseSizedType, 4, false> sseSizeType(10);
    fast_vector<SseSizedType, 4, false> sseSizeType2;
    assert(sseSizeType.size() == 10);
    sseSizeType.resize(20);
    sseSizeType.clear();
    sseSizeType.shrink_to_fit();
    sseSizeType.free();
    sseSizeType.shrink_to_fit();
    sseSizeType.resize(20);
    sseSizeType2.attach_memory(sseSizeType.detach_memory());

    // Verify insertion and erasure works correctly.
    int values[] = {1,2,3,4,5};
    int appendedValues[] = {42,43};
    int prependedValues[] = {-2,-1};
    int insertedValues[] = {13,14};
    int expectedValues[] = {-2,-1,13,14,3,4,5,42,43};
    ints.assign(values);
    assert(ints.size() == 5);
    ints.append(appendedValues);
    ints.insert(2, insertedValues);
    ints.insert(size_t(0), prependedValues);
    ints.erase(ints.begin() + 2, ints.begin() + 4);
    assert(make_array_ref(ints) == make_array_ref(expectedValues));

    // Ensure construction from pointer+count and iterator pairs work.
    fast_vector<int, 4> intsUsingPointerCount(values, std::size(values));
    fast_vector<int, 4> intsUsingIterators(std::begin(values), std::end(values));
    fast_vector<int64_t, 4> ints64UsingIterators(std::begin(values), std::end(values));
    assert(intsUsingPointerCount[0] == 1);
    assert(intsUsingPointerCount[4] == 5);
    assert(intsUsingPointerCount.size() == 5);
    assert(intsUsingIterators[0] == 1);
    assert(intsUsingIterators[4] == 5);
    assert(intsUsingIterators.size() == 5);
    assert(ints64UsingIterators[0] == 1);
    assert(ints64UsingIterators[4] == 5);
    assert(ints64UsingIterators.size() == 5);
    ints64UsingIterators.assign(std::begin(appendedValues), std::end(appendedValues));
    assert(ints64UsingIterators[0] == 42);
    assert(ints64UsingIterators[1] == 43);
    assert(ints64UsingIterators.size() == 2);

    fast_vector<int, 4> initializerListInts({1,3,4});

    #if INCLUDE_EXCEPTION_TESTS // Noisy.
    fast_vector<uint32_t> shouldThrow;
    bool didThrow = false;
    try
    {
        shouldThrow.at(0);
    }
    catch (std::out_of_range const&)
    {
        didThrow = true;
    }
    assert(didThrow);
    #endif // INCLUDE_EXCEPTION_TESTS
}


struct fast_vector_test_class
{
    fast_vector_test_class() { fast_vector_test(); }
};
fast_vector_test_class fast_vector_test_class_instance;

#endif // _DEBUG
