//----------------------------------------------------------------------------
//  History:    2018-04-30 Dwayne Robinson - Created
//----------------------------------------------------------------------------
#pragma once


#if USE_CPP_MODULES
    import std.core;
    import Common.ArrayRef;
#else

    #ifdef USE_GSL_SPAN_INSTEAD_OF_ARRAY_REF
        #include <gsl/span>
        #define array_ref gsl::span
    #else
        #include "Common.ArrayRef.h" // gsl::span may be mostly substituted instead of array_ref, just missing intersects().
    #endif

    #include <memory> // For uninitialized_move/copy and std::unique_ptr.
    #include <assert.h>
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
//  fast_vector<int> axes            - basically std vector.

template<typename T, size_t DefaultArraySize = 0, bool ShouldInitializeElements = true>
class fast_vector;

enum fast_vector_use_memory_buffer_enum
{
    fast_vector_use_memory_buffer
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
    // Types
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
 
    ~fast_vector() noexcept(std::is_nothrow_destructible<T>::value)
    {
        free();
    }

    // Clear the vector and free all memory. Calling clear() then shrink_to_fit()
    // a less efficient way to accomplish it too.
    void free()
    {
        clear(); // Destruct objects and zero the size.
        capacity_ = 0;

        if (dataIsAllocatedMemory_)
        {
            std::free(data_);
            data_ = nullptr;
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
        std::uninitialized_copy(span.data(), span.data() + newSize, /*out*/ data_);
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
        std::uninitialized_move(span.data(), span.data() + newSize, /*out*/data_);
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
            clear(); // Destroy any existing elements.

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
            transfer_from(other);
        }
    }

    void clear() noexcept(std::is_nothrow_destructible<T>::value)
    {
        std::destroy(data_, data_ + size_);
        size_ = 0;
        // But do not free heap memory.
    }

    void resize(size_t newSize)
    {
        if (newSize > size_)
        {
            if (newSize > capacity_)
            {
                // Grow with 1.5x factor to avoid frequent reallocations.
                size_t newCapacity = std::max((size_ * 3 / 2), newSize);
                reserve(newCapacity);
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
        reserve(size_ + 1);
        new(&data_[size_]) T(newValue);
        ++size_;
    }

    void push_back(T&& newValue)
    {
        reserve(size_ + 1);
        new(&data_[size_]) T(std::move(newValue));
        ++size_;
    }

    // Returns underlying malloc()'d memory block.
    // - May be transferred to another fast_vector via attach_memory.
    // - Caller must free the memory if not transferred.
    // - This is more safely used when T is a POD type, as the caller would also
    //   need to appropriately call any destructors if complex objects.
    // - If the vector is using fixed size memory (no allocations have happened),
    //   the returned span is empty and points to null.
    // - If the vector is using allocated memory, the memory block will be not
    //   empty and non-null, and the vector is then empty.
    // - No object destructors are called.
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

        if (dataIsAllocatedMemory_)
        {
            // Only heap allocated memory can be returned, not the fixed size buffer.
            data = {reinterpret_cast<uint8_t*>(data_), size_ * sizeof(T)};
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
    //
    void attach_memory(array_ref<uint8_t> data) noexcept(std::is_nothrow_destructible<T>::value)
    {
        assert(reinterpret_cast<T*>(data.data()) != data_);

        clear(); // Destroy any existing elements.

        // Take ownership of new data.
        data_ = reinterpret_cast<T*>(data.data());
        size_ = data.size() / sizeof(T);
        capacity_ = size_;
        dataIsAllocatedMemory_ = true;
    }

protected:
    void ReallocateMemory(size_t newByteSize) // Throws std::bad_alloc if low on memory, or if T's move constructor fails.
    {
        assert(newByteSize >= size_ * sizeof(T)); // Shouldn't have been called otherwise, because it's wrong for size_ to be less than actual memory.

        if (dataIsAllocatedMemory_ && std::is_trivially_move_constructible<T>::value)
        {
            // Try to just reallocate the existing memory block.
            // This is for plain old data types and simple classes.

            T* newData = static_cast<T*>(realloc(data_, newByteSize));
            if (newData == nullptr && newByteSize > 0)
            {
                throw std::bad_alloc();
            }
            data_ = newData;
        }
        else
        {
            // Allocate a new memory buffer if one isn't allocated yet,
            // or if the data type is complex enough that it's not trivially
            // moveable (e.g. std::string, which contains pointers that point
            // into the class address itself for the small string optimization).

            T* newData = static_cast<T*>(malloc(newByteSize));
            std::unique_ptr<T, decltype(std::free)*> newDataHolder(newData, &std::free);

            // Copy an any existing elements from the fixed size buffer.
            std::uninitialized_move(data_, data_ + size_, /*out*/newData);

            // Release the existing block, and assign the new one.
            if (dataIsAllocatedMemory_)
            {
                std::free(data_);
            }
            data_ = newDataHolder.release();
            dataIsAllocatedMemory_ = true;

            // Caller updates capacity_ and size_. This simple reallocates the memory block.
        }
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

    fast_vector(array_ref<T> initialValues)
    :   BaseClass(fast_vector_use_memory_buffer, GetFixedSizeArrayData(), initialValues)
    {
    }

    fast_vector(const fast_vector& otherVector)
    :   BaseClass(otherVector)
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
        transfer_from(otherVector);
        return *this;
    }

    fast_vector& operator=(BaseClass&& otherVector) // Throws std::bad_alloc if low on memory.
    {
        transfer_from(otherVector);
        return *this;
    }

    fast_vector& operator=(const fast_vector& otherVector)
    {
        assign(otherVector);
        return *this;
    }

    fast_vector& operator=(const BaseClass& otherVector)
    {
        assign(otherVector);
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
    uint8_t fixedSizedArrayData_[DefaultArraySize][sizeof(T)];
};

#ifdef USE_GSL_SPAN_INSTEAD_OF_ARRAY_REF
#undef array_ref
#endif
