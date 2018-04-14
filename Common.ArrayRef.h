//----------------------------------------------------------------------------
//  Author:     Dwayne Robinson
//  History:    2015-06-19 Created
//              2015-10-20 Forked into stand-alone file and added unit test.
//----------------------------------------------------------------------------
#pragma once


// View of contiguous memory, which may come from an std::vector,
// std::wstring, std::initializer_list, std::array, plain C array,
// or even raw memory.
//
// No resource ownership is implied by this class, as another
// container actually owns the data.
//
// e.g. DrawGlyphs(array_ref<uint16_t const> glyphIds)
//      ConvertToLowerCase(IN OUT array_ref<char16_t> text)
//      GetPixelRowView(OUT array_ref<uint32_t>& pixelRow)
//
template<typename T>
class array_ref
{
    // All array_ref's are friends of each other to enable constructors to
    // read other variants, mainly for copy constructors from a non-const to
    // const array_ref but also for reinterpret_reset.
    template <typename U>
    friend class array_ref;

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

    // construct/copy
    constexpr array_ref() = default;
    constexpr array_ref(array_ref<typename T> const& other) = default;

    constexpr array_ref(pointer array, size_t elementCount) : begin_(array), end_(array + elementCount) {}
    constexpr array_ref(pointer begin, pointer end) : begin_(begin), end_(end) {}

    using NonConstArrayRefType = array_ref<typename std::remove_const<T>::type>;

    // Generic constructor to accept any container which uses contiguous memory
    // and exposes a data() member.
    //
    // Sadly some bugs/holes in the standard complicate genericity in getting
    // the data pointer, including std::initializer_list missing a data() member
    // and std::string lacking the non-const overload of data(). See details in
    // get_container_pointer. Otherwise it would simply be std::data(v).
    //
    // Use enable_if to ensure the compiler correctly chooses the default copy
    // constructor when copying another array_ref of the same constness,
    // instead of always calling this templated copy constructor.
    //
    template<
        typename ContiguousContainer,
        typename std::enable_if<
            !std::is_same<ContiguousContainer, array_ref<T> >::value &&
            !std::is_same<ContiguousContainer, NonConstArrayRefType>::value,
            int
        >::type = 0
    >
    constexpr array_ref(ContiguousContainer& container)
    :   begin_(get_container_pointer(container)),
        end_(begin_ + std::size(container))
    {
    }

    // Constructor for copying non-const array_ref's to const array_ref's.
    // The default constructor handles const to const and non-const to non-const,
    // but not non-const to const conversion. Implement it specially rather than
    // using the generic constructor below to avoid pointless division and
    // multiplication. The enable_if prevents the template from stealing all
    // overload calls away from the default copy constructor.
    template<
        typename OtherArrayRef,
        typename std::enable_if<
            !std::is_same<OtherArrayRef, typename array_ref<T> >::value &&
            std::is_same<OtherArrayRef, NonConstArrayRefType>::value,
            int
        >::type = 0
    >
    array_ref(OtherArrayRef const& other)
    :   begin_(other.begin()),
        end_(other.end())
    {
    }

    // Reset to a new range using a compatible data type, possibly differing in constness
    // but only from non-const to const.
    //
    // e.g. glyphIds.reset(newGlyphIdsArray);
    //      glyphIds.reset(glyphRun.glyphIds, glyphRun.glyphCount);
    //      glyphIds.reset(oldGlyphs.begin(), oldGlyphs.end());
    //
    template<typename ContiguousContainer> void reset(ContiguousContainer& container)
    {
        begin_ = get_container_pointer(container);
        end_ = begin_ + std::size(container);
    }

    template <typename T> void reset(_In_reads_(count) T* begin, size_type count)
    {
        begin_ = begin;
        end_ = begin + count;
    }

    template <typename T> void reset(_In_reads_(end-begin) T* begin, T* end)
    {
        begin_ = begin;
        end_ = end;
    }

    // Because sometimes your generic array of floats would be easier to work with as
    // a vec3f, or because you know your array of bytes from a buffer is actually a
    // specific data type. If the new type does not yield an exact multiple of the
    // old size, the new size is floored to the nearest whole unit. That means that
    // if there isn't enough room for even one of the new type, the array_ref will
    // be empty.
    //
    // e.g. glyphOffsets.reset(floatArray.reinterpret_as<DWRITE_GLYPH_OFFSET>());
    //      auto featureRecords = byteArray.reinterpret_as<FeatureRecord const>();
    //
    template<typename NewType> array_ref<NewType> reinterpret_as()
    {
        size_type adjustedByteSize = size_in_bytes();
        adjustedByteSize -= adjustedByteSize % sizeof(NewType);
        return array_ref<NewType>(
            reinterpret_cast<NewType*>(begin_),
            reinterpret_cast<NewType*>(to_byte_pointer(begin_) + adjustedByteSize)
            );
    }

    // Iterators
    iterator begin() const                  { return begin_; }
    iterator end() const                    { return end_; }
    const_iterator cbegin() const           { return begin_; }
    const_iterator cend() const             { return end_; };
    reverse_iterator rbegin() const         { return reverse_iterator(begin_); }
    reverse_iterator rend() const           { return reverse_iterator(end_); }
    const_reverse_iterator crbegin() const  { return const_reverse_iterator(begin_); }
    const_reverse_iterator crend() const    { return const_reverse_iterator(end_); }

    // Capacity
    size_type size() const                  { return end_ - begin_; }
    size_type size_in_bytes() const         { return to_byte_pointer(end_) - to_byte_pointer(begin_); }
    size_type max_size() const              { return SIZE_MAX / sizeof(T); }
    bool empty() const                      { return begin_ == end_; }

    // Element access
    T& operator[](size_t i) const           { return begin_[i]; }
    T& front() const                        { return *begin_; }
    T& back() const                         { return *(end_ - 1); }
    T* data() const                         { return begin_; }
    T* data_end() const                     { return end_; }

    // Mutators
    void clear()                            { begin_ = end_ = nullptr; }
    void remove_prefix(size_type n)         { begin_ += n; assert(begin_ <= end_); }
    void remove_suffix(size_type n)         { end_   -= n; assert(begin_ <= end_); }
    void pop_back()                         { --end_;   assert(begin_ <= end_); }
    void pop_front()                        { ++begin_; assert(begin_ <= end_); }

    // Return a new indexed slice into the array.
    array_ref get_slice(size_type beginIndex, size_type endIndex) const
    {
        endIndex   = std::min(endIndex,   size());
        beginIndex = std::max(beginIndex, size_type(0));
        beginIndex = std::min(beginIndex, endIndex);
        return array_ref(begin_ + beginIndex, begin_ + endIndex);
    }

    // Clamp one range to another.
    void clamp(array_ref<T const> other) const
    {
        begin_ = std::max(begin_, other.begin_);
        end_ = std::min(end_, other.end_);
        begin_ = std::min(begin_, end_);
    }

protected:
    // Mini-helpers.
    static inline uint8_t const* to_byte_pointer(void const* p) { return reinterpret_cast<uint8_t const*>(p); }
    static inline uint8_t* to_byte_pointer(void* p) { return reinterpret_cast<uint8_t*>(p); }

    // Unwraps the data pointer from the container passed.
    template <typename ContiguousContainer>
    inline static T* get_container_pointer(ContiguousContainer& container)
    {
        // This crazy function can be deleted once std::string is fixed.
        // http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-active.html#2391
        //
        // std::data() and std::string::data() return a const pointer for a
        // mutable string even when s[0] and begin() are mutable. So we have to
        // cast away the constness while still staying const correct in the
        // case of an actual const string. To get the true type, dereference an
        // iterator (cannot use s[0] because std::initializer_list lacks that
        // operator overload, despite basically being an array), then get the
        // address of it (but not via operator '&' because some wrapping
        // classes have an operator overload that returns their base type
        // rather than the actual type), and then finally use the type of the
        // address for the const_cast.

        return const_cast<decltype(std::addressof(*std::begin(container)))>(std::data(container));
    }

protected:
    pointer begin_ = nullptr;
    pointer end_ = nullptr;
};


// Wraps multiple contiguous elements. Alternately you could just pass this
// to the array_ref constructor, but this conveniently deduces the type.
template <typename T>
array_ref<T> make_array_ref(_In_reads_(count) T* t, size_t count)
{
    return array_ref<T>(t, count);
}

template <typename T>
array_ref<T> make_array_ref(T* begin, T* end)
{
    return array_ref<T>(begin, end);
}

template <typename ContiguousContainer>
auto make_array_ref(ContiguousContainer& container) -> array_ref<typename std::remove_reference<decltype(*std::begin(container))>::type>
{
    // The remove_reference is necessary because decltype retains the reference
    // from std::vector's dereferenced iterator.
    using ArrayRefType = typename std::remove_reference<decltype(*std::begin(container))>::type;
    return array_ref<ArrayRefType>(container);
}

template <typename T>
auto make_array_ref(std::initializer_list<T> container) -> array_ref<T const>
{
    return array_ref<T const>(container);
}


// Wrap a single instance of a type into an array_ref with a single element.
// You shouldn't use this on containers like std::vector if you want the data,
// since this actually actually wraps the type itself, not the type's contents
// (unless you actually DO want to wrap a single instance of the vector).
// Otherwise just use the ordinary array_ref constructor or make_array_ref.
//
template <typename T>
static array_ref<T> wrap_single_array_ref(T& t)
{
    return array_ref<T>(std::addressof(t), std::addressof(t) + 1);
}


using byte_array_ref = array_ref<uint8_t>;
using const_byte_array_ref = array_ref<uint8_t const>;
