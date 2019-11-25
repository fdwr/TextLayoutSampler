//----------------------------------------------------------------------------
//  History:    2018-04-30 Dwayne Robinson - Created
//----------------------------------------------------------------------------
#pragma once


#include <iterator>
#include <memory> // For uninitialized_move/copy and std::unique_ptr.
#include <assert.h>
#include <algorithm>

#if USE_CPP_MODULES
//import std.core;
import Common.ArrayRef;
#else
#ifdef USE_GSL_SPAN_INSTEAD_OF_ARRAY_REF
#include <gsl/span>
#define array_ref gsl::span
#else
#include "Common.ArrayRef.h" // gsl::span may be mostly substituted instead of array_ref, just missing intersects().
#endif
#endif

MODULE(Common.FastVector);
EXPORT_BEGIN


#pragma warning(push)
#pragma warning(disable:4127) // Conditional expression is constant. VS can't tell that certain compound conditionals of template parameters aren't always constant when the tempalate parameter is true.

#if defined(_ITERATOR_DEBUG_LEVEL) && _ITERATOR_DEBUG_LEVEL > 0
// For std::uninitialized_copy and std::uninitialized_move.
#define FASTVECTOR_MAKE_UNCHECKED stdext::make_unchecked_array_iterator
#else
#define FASTVECTOR_MAKE_UNCHECKED
#endif

// fast_vector is a dynamic array that can substitute for std::vector, where it:
// (1) avoids heap allocations when the element count fits within the fixed-size capacity.
// (2) avoids unnecessarily initializing elements if ShouldInitializeElements == false.
//     This is useful for large buffers which will just be overwritten soon anyway.
// (3) supports most vector methods except insert/erase/emplace.
//
// Template parameters:
// - DefaultArraySize - passing 0 means it is solely heap allocated. Passing > 0
//   reserves that much element capacity before allocating heap memory.
//
// - ShouldInitializeElements - ensures elements are constructed when resizing, and
//   it must be true for objects with non-trivial constructors, but it can be set
//   false for large buffers to avoid unnecessarily initializing memory which will
//   just be overwritten soon later anyway.
//
// Examples:
//  fast_vector<int, 20> axes        - up to 20 integers before heap allocation.
//  fast_vector<int, 0, false> axes  - always heap allocated but never initialized.
//  fast_vector<int> axes            - basically std::vector.

template<typename T, size_t DefaultArraySize = 0, bool ShouldInitializeElements = true>
class fast_vector;

enum fast_vector_use_memory_buffer_enum
{
    fast_vector_use_memory_buffer // To avoid ambiguous constructor overload resolution.
};

// The base class is separated out for the customization of passing specific
// memory, and to avoid bloating additional template permutations solely due to
// differing array sizes.
template<typename T, bool ShouldInitializeElements>
class fast_vector<T, 0, ShouldInitializeElements>
{
    static_assert(ShouldInitializeElements || std::is_trivial<T>::value);
    using self = fast_vector<T, 0, ShouldInitializeElements>;

public:
    // Standard container type definitions.
    using value_type = T;
    using pointer = T*;
    using reference = T&;
    using iterator = pointer;
    using const_reference = T const&;
    using const_iterator = T const*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using mutable_value_type = typename std::remove_const<T>::type;
    using mutable_iterator = mutable_value_type*;

    // If true, the data types needs additional alignment like SSE and AVX types.
    constexpr static bool NeedsTypeAlignmentBeyondMaxAlignT = alignof(T) > alignof(std::max_align_t);
    constexpr static size_t MinimumMemoryBlockSize = NeedsTypeAlignmentBeyondMaxAlignT ? sizeof(void*) : 0;

public:
    constexpr fast_vector() noexcept
    {
    }

    fast_vector(size_t initialSize)
    {
        resize(initialSize);
    }

    fast_vector(array_ref<const T> initialValues)
    {
        assign(initialValues);
    }

    fast_vector(const fast_vector& otherVector)
    {
        assign(otherVector);
    }

    fast_vector(fast_vector&& otherVector) // Throws std::bad_alloc if low on memory.
    {
        transfer_from(otherVector);
    }

    // Construct using explicit fixed size memory buffer.
    // - Used by the derived template specialization which includes a fixed size buffer.
    // - May also be used by the caller to pass an explicit buffer such as a local array,
    //   where the buffer is guaranteed to live for the lifetime of the fast_vector.
    constexpr fast_vector(fast_vector_use_memory_buffer_enum, array_ref<T> initialBackingArray)
    :   data_(initialBackingArray.data()),
        capacity_(initialBackingArray.size())
    {
    }

    fast_vector(fast_vector_use_memory_buffer_enum, array_ref<T> initialBackingArray, array_ref<const T> initialValues)
    :   data_(initialBackingArray.data()),
        capacity_(initialBackingArray.size())
    {
        assign(initialValues);
    }

    fast_vector(fast_vector_use_memory_buffer_enum, array_ref<T> initialBackingArray, size_t initialSize)
    :   data_(initialBackingArray.data()),
        capacity_(initialBackingArray.size())
    {
        resize(initialSize);
    }

    fast_vector(fast_vector_use_memory_buffer_enum, array_ref<T> initialBackingArray, pointer p, size_t elementCount)
    :   data_(initialBackingArray.data()),
        capacity_(initialBackingArray.size())
    {
        assign({p, elementCount});
    }

    template <typename IteratorType>
    fast_vector(fast_vector_use_memory_buffer_enum, array_ref<T> initialBackingArray, IteratorType begin, IteratorType end)
    :   data_(initialBackingArray.data()),
        capacity_(initialBackingArray.size())
    {
        assign(begin, end);
    }
 
    ~fast_vector() noexcept(std::is_nothrow_destructible<T>::value)
    {
        free();
    }

    // Clear the vector and free all memory. Calling clear() then shrink_to_fit()
    // is a less efficient way to accomplish it too.
    void free()
    {
        clear(); // Destruct objects and zero the size.

        if (dataIsAllocatedMemory_)
        {
            FreeMemoryBlock(data_);
            data_ = nullptr;
            capacity_ = 0;
        }
    }

    // Iterators
    iterator begin() const noexcept                 { return data_; }
    iterator end() const noexcept                   { return data_ + size_; }
    const_iterator cbegin() const noexcept          { return begin(); }
    const_iterator cend() const noexcept            { return end(); };
    reverse_iterator rbegin() const noexcept        { return reverse_iterator(begin()); }
    reverse_iterator rend() const noexcept          { return reverse_iterator(end()); }
    const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(begin()); }
    const_reverse_iterator crend() const noexcept   { return const_reverse_iterator(end()); }

    // Capacity
    size_type size() const noexcept                 { return size_; }
    size_type size_in_bytes() const noexcept        { return size_ * sizeof(T); }
    size_type capacity() const noexcept             { return capacity_; }
    static constexpr size_type max_size() noexcept  { return SIZE_MAX / sizeof(T); }
    bool empty() const noexcept                     { return size_ == 0; }

    // Element access
    T& operator[](size_t i) const noexcept          { return data_[i]; }
    T& front() const noexcept                       { return data_[0]; }
    T& back() const noexcept                        { return data_[size_ - 1]; }
    T* data() const noexcept                        { return data_; }
    T* data_end() const noexcept                    { return data_ + size_; }

    array_ref<T> data_span() noexcept               { return {data_, size_}; }
    array_ref<T const> data_span() const noexcept   { return {data_, size_}; }

    T& at(size_t i)                                 { return checked_read(i); }
    T& at(size_t i) const                           { return checked_read(i); }

    T& checked_read(size_t i)
    {
        if (i >= size_)
        {
            throw std::out_of_range("fast_vector array index out of range.");
        }

        return data_[i];
    }

    const T& checked_read(size_t i) const
    {
        return const_cast<self&>(*this).checked_read(i);
    }

    fast_vector& operator=(const fast_vector& otherVector)
    {
        assign(otherVector);
        return *this;
    }

    fast_vector& operator=(fast_vector&& otherVector) // Throws std::bad_alloc if low on memory.
    {
        transfer_from(otherVector);
        return *this;
    }

    // Clear any existing elements and copy the new elements from span.
    void assign(array_ref<const T> span)
    {
        #ifndef USE_GSL_SPAN_INSTEAD_OF_ARRAY_REF
        assert(!span.intersects(data_span())); // No self intersection.
        #endif

        clear();

        size_t newSize = span.size();
        reserve(newSize);
        std::uninitialized_copy(FASTVECTOR_MAKE_UNCHECKED(span.data()), FASTVECTOR_MAKE_UNCHECKED(span.data()) + newSize, /*out*/ FASTVECTOR_MAKE_UNCHECKED(data_));
        size_ = newSize;
    }

    // Clear any existing elements and copy the new elements from the iterable range.
    template <typename IteratorType>
    void assign(IteratorType begin, IteratorType end)
    {
        clear();

        size_t newSize = std::distance(begin, end);
        reserve(newSize);
        std::uninitialized_copy(FASTVECTOR_MAKE_UNCHECKED(begin), FASTVECTOR_MAKE_UNCHECKED(end), /*out*/ FASTVECTOR_MAKE_UNCHECKED(data_));
        size_ = newSize;
    }

    // Tranfer all elements from the other vector to this one.
    // This only offers the weak exception guarantee, that no leaks will happen
    // if type T throws in the middle of a copy.
    void transfer_from(array_ref<const T> span)
    {
        #ifndef USE_GSL_SPAN_INSTEAD_OF_ARRAY_REF
        assert(!span.intersects(data_span())); // No self intersection.
        #endif

        clear();

        size_t newSize = span.size();
        reserve(newSize);
        std::uninitialized_move(FASTVECTOR_MAKE_UNCHECKED(span.data()), FASTVECTOR_MAKE_UNCHECKED(span.data()) + newSize, /*out*/ FASTVECTOR_MAKE_UNCHECKED(data_));
        size_ = newSize;
    }

    // Tranfer all elements from the other vector to this one.
    // This only offers the weak exception guarantee, that no leaks will happen
    // if type T throws in the middle of a copy.
    void transfer_from(fast_vector<T, 0, ShouldInitializeElements>& other)
    {
        if (&other == this)
        {
            return; // Nop for self assignment.
        }

        if (other.dataIsAllocatedMemory_)
        {
            free();

            // Take ownership of new data directly.
            data_ = other.data_;
            other.data_ = nullptr;
            size_ = other.size_;
            other.size_ = 0;
            capacity_ = other.capacity_;
            other.capacity_ = 0;
            dataIsAllocatedMemory_ = other.dataIsAllocatedMemory_;
            other.dataIsAllocatedMemory_ = false;
        }
        else
        {
            // Copying from a fixed size buffer; so it's unsafe to simply
            // steal the pointers as that may leave a dangling pointer
            // when the other fast vector disappears.
            transfer_from(make_array_ref(other));
        }
    }

    void clear() noexcept(std::is_nothrow_destructible<T>::value)
    {
        if (ShouldInitializeElements)
        {
            std::destroy(data_, data_ + size_);
        }
        size_ = 0;
        // But do not free heap memory.
    }

    void resize(size_t newSize)
    {
        if (newSize > size_)
        {
            if (newSize > capacity_)
            {
                reserve_at_least(newSize);
            }

            // Grow data to the new size, calling the default constructor on each new item.
            if (ShouldInitializeElements)
            {
                std::uninitialized_value_construct<iterator>(data_ + size_, data_ + newSize);
            }

            size_ = newSize;
        }
        else if (newSize < size_)
        {
            // Shrink the data to the new size, calling the destructor on each item. Capacity remains intact.
            if (ShouldInitializeElements)
            {
                std::destroy(data_ + newSize, data_ + size_);
            }

            size_ = newSize;
        }
    }

    void reserve(size_t newCapacity)
    {
        if (newCapacity <= capacity_)
        {
            return; // Nothing to do.
        }
    
        if (newCapacity > max_size())
        {
            throw std::bad_alloc(); // Too many elements.
        }

        size_t newByteSize = newCapacity * sizeof(T);
        ReallocateMemory(newByteSize);

        capacity_ = newCapacity;
    }

    void reserve_at_least(size_t newCapacity)
    {
        // Grow with 1.5x factor to avoid frequent reallocations.
        newCapacity = std::max((size_ * 3 / 2), newCapacity);
        reserve(newCapacity);
    }

    void shrink_to_fit()
    {
        if (!dataIsAllocatedMemory_ || capacity_ == size_)
        {
            return; // Nothing to do.
        }

        size_t newByteSize = size_ * sizeof(T);
        ReallocateMemory(newByteSize);

        capacity_ = size_;
    }


    void push_back(const T& newValue)
    {
        reserve(size_ + 1);// 
        new(static_cast<void*>(data_ + size_)) T(newValue);
        ++size_;
    }

    void push_back(T&& newValue)
    {
        reserve(size_ + 1);
        new(static_cast<void*>(data_ + size_)) T(std::move(newValue));
        ++size_;
    }

    void append(array_ref<const T> span)
    {
        insert(size_, span);
    }

    void insert(size_t insertionOffset, array_ref<const T> span)
    {
        insert(insertionOffset, span.begin(), span.end());
    }

    void insert(size_t insertionOffset, const_iterator begin, const_iterator end)
    {
        assert(insertionOffset <= size_);

        // Grow array size.
        size_t newSize = std::distance(begin, end) + size_;
        if (newSize < size_)
        {
            throw std::bad_alloc();
        }
        reserve_at_least(newSize);

        // Add empty elements to the end.
        size_t oldSize = size_;
        if (ShouldInitializeElements)
        {
            std::uninitialized_value_construct<iterator>(data_ + size_, data_ + newSize);
        }
        size_ = newSize;

        // Shift elements to the end to make more room.
        std::move_backward(data_ + insertionOffset, data_ + oldSize, data_ + newSize);

        std::copy(begin, end, /*out*/ data_ + insertionOffset);
    }

    void insert(const_iterator position, array_ref<const T> span)
    {
        insert(position - begin(), span.begin(), span.end());
    }

    void erase(const_iterator begin, const_iterator end)
    {
        assert(end >= begin);
        assert(begin >= data_);
        assert(end <= data_ + size_);

        // Shift elements to front.
        size_t oldSize = size_;
        std::copy(iterator(end), data_ + size_, iterator(begin));
        size_ -= std::distance(begin, end);

        // Destroy elements at the end.
        if (ShouldInitializeElements)
        {
            std::destroy(data_ + size_, data_ + oldSize);
        }
    }

    void erase(const_iterator position)
    {
        erase(position, position + 1);
    }

    // Returns underlying malloc()'d memory block.
    // - May be transferred to another fast_vector via attach_memory.
    // - Caller must free() the memory if not transferred.
    // - This is more safely used when T is a POD type, as the caller would also
    //   need to appropriately call any destructors if complex objects.
    // - If the vector is using fixed size memory (no allocations have happened),
    //   the returned span is empty and points to null.
    // - If the vector is using allocated memory, the memory block will be not
    //   empty and non-null, and the vector is then empty.
    // - No object destructors are called.
    // - If NeedsTypeAlignmentBeyondMaxAlignT is true, the data array might start
    //   after the memory block for alignment.
    //
    // fast_vector<int> vector(20);
    // ...
    // auto memory = vector.detach_memory();
    // std::unique_ptr<char, decltype(&std::free)> p(memory.data(), &std::free);
    // ...
    // otherVector.attach_memory(memory);
    // p.release(); // unique_ptr does not own it anymore.
    //
    array_ref<uint8_t> detach_memory() noexcept
    {
        array_ref<uint8_t> data;

        // Only heap allocated memory can be returned, not the fixed size buffer.
        if (dataIsAllocatedMemory_)
        {
            // Return the raw memory block, from the actual beginning of memory to the true end.
            // The data might start beyond the memory block for alignment purposes.
            uint8_t* bytes = reinterpret_cast<uint8_t*>(data_);
            size_t byteCount = size_ * sizeof(T);
            data = {reinterpret_cast<uint8_t*>(GetMemoryBlock(data_)), bytes + byteCount};

            data_ = nullptr;
            size_ = 0;
            capacity_ = 0;
        }

        return data;
    }

    // Take ownership of the memory, which came from malloc or another fast_vector
    // via detach_memory.
    // - If ShouldInitializeElements == true, then the memory is presumed to contain
    //   valid objects, which will be destructed when the fast_vector dies.
    // - Because the memory is raw, it's possible to reuse a different data type.
    // - The memory must be aligned to alignof(std::max_align_t).
    // - If NeedsTypeAlignmentBeyondMaxAlignT is true, the memory must be at least
    //   sizeof(void*) bytes.
    //
    void attach_memory(array_ref<uint8_t> memory) noexcept(std::is_nothrow_destructible<T>::value)
    {
        #ifndef USE_GSL_SPAN_INSTEAD_OF_ARRAY_REF
        assert(!memory.intersects(data_span())); // No self intersection.
        #endif

        assert((reinterpret_cast<size_t>(memory.data()) & (alignof(std::max_align_t) - 1)) == 0); // malloc/realloc should return aligned pointers up to max_align_t.
        assert(memory.data() == nullptr || memory.size() >= MinimumMemoryBlockSize);

        free();

        T* data = AlignMemoryBlock(memory.data());

        // Take ownership of new data.
        data_ = data;
        uint8_t* dataEnd = memory.data() + memory.size();
        size_ = (dataEnd - reinterpret_cast<uint8_t*>(data_)) / sizeof(T);
        capacity_ = size_;
        dataIsAllocatedMemory_ = true;
    }

protected:
    void ReallocateMemory(size_t newByteSize) // Throws std::bad_alloc if low on memory, or if T's move constructor fails.
    {
        assert(newByteSize >= size_ * sizeof(T)); // Shouldn't have been called otherwise, because it's wrong for size_ to be less than actual memory.

        // Handle alignment needs when the type is beyond what malloc/realloc guarantee.
        // This needed for SSE and AVX types (which need 16 and 32 bytes). Any other standard
        // type should have sufficient alignment by default.
        newByteSize = InflateMemorySizeForAlignment(newByteSize);

        // Try to reallocate the memory block directly rather allocate a new block and manually
        // copying. The memory manager can sometimes just extend the existing data block in-place
        // if there is space behind the block. Only trivially moveable types comply though, whereas
        // std::string would not comply due to the small string optimization.
        
        if (std::is_trivially_move_constructible<T>::value && dataIsAllocatedMemory_)
        {
            // Try to just reallocate the existing memory block.
            
            void* memory = realloc(GetMemoryBlock(data_), newByteSize);
            if (memory == nullptr && newByteSize > 0)
            {
                throw std::bad_alloc();
            }
            data_ = AlignMemoryBlock(/*modified*/ memory);
        }
        else
        {
            // Allocate a new memory buffer if one isn't allocated yet,
            // or if the data type is complex enough that it's not trivially
            // moveable (e.g. std::string, which contains pointers that point
            // into the class address itself for the small string optimization).

            void* memory = std::malloc(newByteSize);
            std::unique_ptr<void, decltype(std::free)*> newDataHolder(memory, &std::free);

            // Copy an any existing elements from the fixed size buffer.
            T* newData = AlignMemoryBlock(memory);
            std::uninitialized_move(FASTVECTOR_MAKE_UNCHECKED(data_), FASTVECTOR_MAKE_UNCHECKED(data_) + size_, /*out*/ FASTVECTOR_MAKE_UNCHECKED(newData));

            // Release the existing block, and assign the new one.
            if (dataIsAllocatedMemory_)
            {
                FreeMemoryBlock(data_);
            }
            newDataHolder.release();
            data_ = newData;
            dataIsAllocatedMemory_ = true;

            // Caller updates capacity_ and size_. This simply reallocates the memory block.
        }
    }

    // No object destruction, just a direct free.
    static void FreeMemoryBlock(T* data)
    {
        std::free(GetMemoryBlock(data));
    }

    // Get the actual memory block associated with the data.
    static void* GetMemoryBlock(T* data)
    {
        mutable_value_type* mutableData = const_cast<mutable_value_type*>(data);

        if (NeedsTypeAlignmentBeyondMaxAlignT && data != nullptr)
        {
            // Read the pointer to the actual block which is set immediately before the data.
            // [ptr to memory block][data ...]
            return reinterpret_cast<void**>(mutableData)[-1];
        }
        else
        {
            return mutableData;
        }
    }

    // Return aligned data for the given type, storing the raw memory block from malloc.
    // Write the memory block associated with the data, and return the data.
    // The caller has already ensure enough space exists in the block for alignment.
    static T* AlignMemoryBlock(void* memory)
    {
        if (NeedsTypeAlignmentBeyondMaxAlignT && memory != nullptr)
        {
            // Return a pointer to the aligned data, which starts after the pointer to the original
            // memory.
            //
            // [ptr to memory block][data ...]

            // Return  the pointer to the actual block which is set immediately before the data.
            // [ptr to memory block][data ...]
            constexpr size_t alignmentMask = alignof(T) - 1;
            static_assert((alignof(T) & alignmentMask) == 0, "Alignment must be a power of 2.");
            static_assert(NeedsTypeAlignmentBeyondMaxAlignT == false || alignof(T) >= alignof(void*), "Alignment must be at least the size of a pointer for NeedsTypeAlignmentBeyondMaxAlignT to be true.");

            // Align memory. Note calling std::align is awkward given it expects sizes, which are
            // already correct earlier and do not need to be passed to this function.
            size_t memoryAddress = reinterpret_cast<size_t>(memory);
            memoryAddress = (memoryAddress + sizeof(void*) + alignmentMask) & ~alignmentMask;
            mutable_value_type* data = reinterpret_cast<mutable_value_type*>(memoryAddress);

            // Store the pointer to the original memory block.
            reinterpret_cast<void**>(data)[-1] = memory;
            return data;
        }
        else
        {
            assert((reinterpret_cast<size_t>(memory) & (alignof(std::max_align_t) - 1)) == 0); // malloc/realloc should return aligned pointers up to max_align_t.
            return reinterpret_cast<T*>(memory);
        }
    }

    static size_t InflateMemorySizeForAlignment(size_t newByteSize)
    {
        if (NeedsTypeAlignmentBeyondMaxAlignT && newByteSize != 0)
        {
            // Need to increase the allocation size to include enough for full type alignment
            // and the size of a single pointer preceding the data.
            size_t alignedByteSize = newByteSize + alignof(T) - alignof(std::max_align_t) + MinimumMemoryBlockSize;
            if (alignedByteSize < newByteSize)
            {
                throw std::bad_alloc(); // Too much memory requested and overflowed.
            }
            newByteSize = alignedByteSize;
        }
        // else malloc/realloc default alignment will suffice.

        return newByteSize;
    }

protected:
    T* data_ = nullptr;     // May point to fixed size array or allocated memory, depending on dataIsAllocatedMemory_.
    size_t size_ = 0;       // Count in elements.
    size_t capacity_ = 0;   // Count in elements.
    bool dataIsAllocatedMemory_ = false;
};


// Derived specialization which includes a fixed size buffer.
template <typename T, size_t DefaultArraySize, bool ShouldInitializeElements>
class fast_vector : public fast_vector<T, 0, ShouldInitializeElements>
{
public:
    using BaseClass = fast_vector<T, 0, ShouldInitializeElements>;

    constexpr fast_vector() noexcept
    :   BaseClass(fast_vector_use_memory_buffer, GetFixedSizeArrayData())
    {
    }

    fast_vector(array_ref<const T> initialValues)
    :   BaseClass(fast_vector_use_memory_buffer, GetFixedSizeArrayData(), initialValues)
    {
    }

    fast_vector(std::initializer_list<const T> initialValues)
    :   BaseClass(fast_vector_use_memory_buffer, GetFixedSizeArrayData(), make_array_ref(initialValues))
    {
    }

    fast_vector(const fast_vector& otherVector)
    :   BaseClass(otherVector)
    {
    }

    fast_vector(typename BaseClass::pointer p, size_t elementCount)
    :   BaseClass(fast_vector_use_memory_buffer, GetFixedSizeArrayData(), p, elementCount)
    {
    }

    template <typename IteratorType>
    fast_vector(IteratorType begin, IteratorType end)
    :   BaseClass(fast_vector_use_memory_buffer, GetFixedSizeArrayData(), begin, end)
    {
    }

    fast_vector(size_t initialSize)
    :   BaseClass(fast_vector_use_memory_buffer, GetFixedSizeArrayData(), initialSize)
    {
    }

    // Move constructor. Will not throw unless type T throws, since the
    // target is guaranteed to have enough space.
    fast_vector(fast_vector&& otherVector) noexcept(std::is_nothrow_move_assignable<T>::value)
    :   BaseClass(std::move(otherVector))
    {
    }

    // Explicit overload for base class, to avoid the compiler from undesireably
    // choosing the constructor overload which takes a span of initialValues or
    // the copy constructor.
    fast_vector(BaseClass&& otherVector) // Throws std::bad_alloc if low on memory.
    :   BaseClass(std::move(otherVector))
    {
    }

    // Move assignment. Will not throw unless type T throws, since the
    // target vector is guaranteed to have a large enough fixed-size array.
    fast_vector& operator=(fast_vector&& otherVector) noexcept(std::is_nothrow_move_assignable<T>::value && std::is_nothrow_destructible<T>::value)
    {
        BaseClass::transfer_from(otherVector);
        return *this;
    }

    fast_vector& operator=(BaseClass&& otherVector) // Throws std::bad_alloc if low on memory.
    {
        BaseClass::transfer_from(otherVector);
        return *this;
    }

    fast_vector& operator=(const fast_vector& otherVector)
    {
        BaseClass::assign(otherVector);
        return *this;
    }

    fast_vector& operator=(const BaseClass& otherVector)
    {
        BaseClass::assign(otherVector);
        return *this;
    }

private:
    constexpr array_ref<T> GetFixedSizeArrayData() noexcept
    {
        return array_ref<T>(reinterpret_cast<T*>(std::data(fixedSizedArrayData_)), std::size(fixedSizedArrayData_));
    }

private:
    // Uninitialized data to be used by the base class.
    // It's declared as raw bytes rather than an std::array<T> to avoid any initialization cost
    // up front, only initializing the fields which actually exist when resized later.
    //
    // Note Visual Studio 2017 15.8 requires you to declare _ENABLE_EXTENDED_ALIGNED_STORAGE
    // if your type T is > max_align_t.
    //
    std::aligned_storage_t<sizeof(T), alignof(T)> fixedSizedArrayData_[DefaultArraySize];
};

#ifdef USE_GSL_SPAN_INSTEAD_OF_ARRAY_REF
#undef array_ref
#endif

#undef FASTVECTOR_MAKE_UNCHECKED

#pragma warning(pop)

EXPORT_END
