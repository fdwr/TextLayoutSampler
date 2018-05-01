//----------------------------------------------------------------------------
//  Author:     Dwayne Robinson
//  History:    2018-04-30 Created
//----------------------------------------------------------------------------
#include "precomp.h"

#if USE_MODULES
import Common.ArrayRef;
#else
#include "Common.ArrayRef.h"
#include "Common.FastVector.h"
#include <assert.h>
#endif

////////////////////////////////////////

#ifdef _DEBUG

void fast_vector_test_as_parameter(fast_vector<int>& ints)
{
    ints.resize(20);
}

class SimpleStruct
{
    char c;
};
class ComplexStruct
{
public:
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


void fast_vector_test()
{
    fast_vector<int, 20, false> ints;
    fast_vector<int, 30, true> ints2;

    assert(ints.size() == 0);
    assert(ints.capacity() == 20);
    assert(ints2.size() == 0);
    assert(ints2.capacity() == 30);

    ints2.resize(20);

    // Explicitly verify that no data initialization happens,
    // staying within the size limits of the 
    int* originalDataPointer = ints.data();
    *originalDataPointer = 13;
    ints.resize(20);
    assert(ints.size() == 20);
    assert(ints.capacity() == 20);
    assert(ints.front() == 13);
    assert(ints.data() == originalDataPointer); // Should still point to internal array.

    // The other array should be explicitly initialized though to zeroes.
    assert(std::all_of(ints2.begin(), ints2.end(), [](auto v) { return v == 0; }));

    // Resize smaller. Capacity should remain large.
    ints.resize(10);
    assert(ints.size() == 10);
    assert(ints.capacity() == 20);

    // Resize larger than the fixed size buffer to allocate memory.
    ints.resize(30);
    int* secondDataPointer = ints.data();
    assert(ints.size() == 30);
    assert(ints.capacity() == 30);
    assert(secondDataPointer != originalDataPointer); // Allocation should have happened.
    assert(ints.front() == 13); // Value should have been copied over to memory block.

    // Resize smaller again, using memory block this time. Capacity should remain large.
    ints.resize(10);
    assert(ints.size() == 10);
    assert(ints.capacity() == 30);
    ints.resize(0);
    assert(ints.size() == 0);
    assert(ints.capacity() == 30);

    // Grow even larger, enough to reallocate the memory block.
    ints.reserve(40);
    assert(ints.capacity() >= 40);
    assert(ints.front() == 13); // Value should still exist, even after reallocations.

    // Shrink the memory block to actual size.
    assert(ints.capacity() == 40);
    ints.resize(30);
    ints.shrink_to_fit();
    assert(ints.capacity() == 30);
    assert(ints.size() == 30);
    assert(ints.front() == 13); // Value should still exist, even after reallocations.

    ints.clear();
    ints.shrink_to_fit();
    assert(ints.capacity() == 0);
    assert(ints.size() == 0);

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
}


struct fast_vector_test_class
{
    fast_vector_test_class() { fast_vector_test(); }
};
fast_vector_test_class fast_vector_test_class_instance;

#endif // _DEBUG
