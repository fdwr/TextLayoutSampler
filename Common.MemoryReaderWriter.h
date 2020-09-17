// Adapter that can wrap a byte container, either fixed size or variable, and progressively append
// new content or read/write various offsets.
//
// e.g.
//      std::vector<std::byte> someVector;
//      MemoryReaderWriter someVectorWriter(someVector);
//
//      std::array<std::byte, 4> someArray;
//      MemoryReaderWriter someArrayWriter(someArray);
//
//      MyFixedArrayThatsNonTrivial<std::byte, 4> myFixedArray;
//      MemoryReaderWriter myFixedArrayWriter(myFixedArray, MemoryReaderWriter::FixedResizeFunction);
//
//      MyVector<std::byte> myVector;
//      auto myVectorResizer = [&](uint32_t newSizeInBytes)
//      {
//          myVector.Resize(newSizeInBytes);
//          return make_span<std::byte>(myVector.GetBegin(), myVector.GetSize());
//      }
//      MemoryReaderWriter writerMyVector(myVector, myArraymyVectorResizerResizer);
//      MemoryReaderWriter<uint32_t> writerMyVectorUint32(myVector, myArraymyVectorResizerResizer);
//
//      uint32_t floatValueByteOffset = constantsWriter.Append(someFloatValue);
//      uint32_t trivialStructByteOffset = someVectorWriter.AppendAligned(someTrivialStruct);
//      uint32_t someFloatVectorByteOffset = someVectorWriter.AppendArray(someFloatVector);
//      someVectorWriter.Write(floatValueByteOffset, x);
//      float y = someVectorWriter.Read(floatValueByteOffset);
//      auto floatVectorSpan = someVectorWriter.ReadArray<float>(someFloatVectorByteOffset, 3);
//
// Notes:
//      - This class is intended to be used transiently on the stack to temporarily wrap a container.
//      - The caller must keep the underlying container alive during usage.
//      - If resizeable, the caller must not clear/reserve/resize the container during usage.
//        The same rules as for invalidation of iterators applies. Modifying values is fine.
//      - If resizeable, two MemoryReaderWriter's must not use same underlying buffer at the same time.
//      - Two counts exist, one for the total number of readable bytes and one for the number of bytes
//        written, which is initially zero until values are appended/skipped.
//
template <typename MinimumElementType = uint8_t> // No element should be smaller than this. All offsets are relative to this.
class MemoryReaderWriter
{
public:
    MemoryReaderWriter() = default;
    MemoryReaderWriter(const MemoryReaderWriter& container) = delete; // Unwise to have two at the same time.
    MemoryReaderWriter(MemoryReaderWriter&& container) = default;

    constexpr static uint32_t MinimumElementTypeByteSize = sizeof(MinimumElementType);

    // The resizing function is used to grow the container and get a span for the new
    // container data. For most uses, the simplest constructor suffices and will choose
    // a suitable resizing function. If your container supports resizing but has no
    // resize() method, you'll need to pass your own function, or if resizing is not
    // intended, use the stock FixedResizeFunction.
    using ResizeFunctionTypeSignature = gsl::span<std::byte>(uint32_t sizeInBytes);

    // The container to wrap can be any STL-compatible container which has data() and size()
    // methods and which is implicitly convertible to a span of bytes. Otherwise the caller
    // should use make_span. The caller is responsible for ensuring the lifetime of the
    // container outlives outlives the memory reader/writer.
    //
    // This constructor provides a default resizing function depending on whether the container
    // is trivial (e.g. a raw C array or std::array of POD's) or nontrivial, in which case it
    // expects an STL-compatible resize() method (e.g. std::vector, std::string). If your
    // container doesn't satisfy that, call the other constructor with a specific resizer.
    template <typename Container>
    explicit MemoryReaderWriter(Container& container)
    :   m_data(gsl::make_span(container))
    {
        if constexpr (std::is_trivial_v<Container>)
        {
            // Trivial data structs and fixed sized buffers such as a std::array of POD's cannot
            // be resized. Attempts to do so will throw.
            m_resizeFunction = FixedResizeFunction;
        }
        else
        {
            // Otherwise assume it's a complex container supporting resize().
            m_resizeFunction = [&](uint32_t newSizeInBytes) -> gsl::span<std::byte>
            {
                container.resize(newSizeInBytes);
                return gsl::span(container.data(), container.size());
            };
        }
    }

    template <typename Container, typename ResizeFunction>
    MemoryReaderWriter(Container& container, const ResizeFunction& function)
    :   m_data(gsl::make_span(container)),
        m_resizeFunction(function)
    {
    }

    explicit MemoryReaderWriter(gsl::span<std::byte> data)
    :   m_data(data),
        m_resizeFunction(FixedResizeFunction)
    {
    }

    MemoryReaderWriter(_In_reads_(valuesCount) void* data, uint32_t dataByteCount)
    :   m_data(gsl::make_span(reinterpret_cast<std::byte*>(data), dataByteCount)),
        m_resizeFunction(FixedResizeFunction)
    {
    }

    // Stock implementation of a fixed resizing function for backing stores which do not
    // grow, such as std::array and C arrays.
    static gsl::span<std::byte> FixedResizeFunction(uint32_t newSizeInBytes)
    {
        throw std::bad_alloc();
    }

    inline uint32_t GetWrittenByteCount() const noexcept
    {
        return m_writtenByteCount; // Return written byte amount, not just m_data.size() which can be larger.
    }

    inline uint32_t GetWrittenElementCount() const noexcept
    {
        return m_writtenByteCount / MinimumElementTypeByteSize;
    }

    uint32_t GetReadableByteCount() const noexcept
    {
        return gsl::narrow_cast<uint32_t>(m_data.size_bytes());
    }

    // Append a new value to the end of the writeable region, advancing the written count and returning the offset of the written value.
    template <typename T>
    uint32_t Append(const T& value)
    {
        return AppendArrayInternal(m_writtenByteCount, WrapValueAsByteSpan(value));
    }

    // Align to the natural alignment of the given type first, returning the aligned offset.
    template <typename T>
    uint32_t AppendAligned(const T& value)
    {
        return AppendArrayInternal(RoundUpToMultiple(m_writtenByteCount, uint32_t(alignof(T))), WrapValueAsByteSpan(value));
    }

    // Append the contents to the buffer, where Container can be anything castable to a span.
    template <typename Container>
    uint32_t AppendArray(const Container& values)
    {
        auto span = gsl::make_span(values);
        return AppendArrayInternal(m_writtenByteCount, gsl::as_bytes(span));
    }

    // Special case for broken initializer_list which should have a data() method.
    template <typename T>
    uint32_t AppendArray(std::initializer_list<T> values)
    {
        auto span = gsl::make_span(values.begin(), values.end());
        return AppendArrayInternal(m_writtenByteCount, gsl::as_bytes(span));
    }

    template <typename T>
    uint32_t AppendArray(_In_count_(valueCount) const T* values, uint32_t valueCount)
    {
        auto span = gsl::make_span(values, valueCount);
        return AppendArrayInternal(m_writtenByteCount, gsl::as_bytes(span));
    }

    // Align first and then write the array elements.
    template <typename Container>
    uint32_t AppendArrayAligned(const Container& values)
    {
        auto span = gsl::make_span(values);
        using T = decltype(*span.data());
        return AppendArrayInternal(RoundUpToMultiple(m_writtenByteCount, uint32_t(alignof(T))), gsl::as_bytes(values));
    }

    // Write to a specific offset. Unlike Append, this does not grow GetWrittenByteCount.
    template <typename T>
    void Write(uint32_t offsetInElements, const T& value)
    {
        // Types must be trivial byte-wise copyable, lacking constructors and destructors.
        // They can be non-standard layout though.
        static_assert(std::is_trivial_v<T>);
        Get<T>(offsetInElements) = value;
    }

    template <typename T>
    void WriteArray(uint32_t offsetInElements, span<const T> values)
    {
        static_assert(std::is_trivial_v<T>);
        const uint32_t offsetInBytes = offsetInElements * MinimumElementTypeByteSize;
        WriteArrayInternal(offsetInBytes, gsl::as_bytes(values));
    }

    // Skip by the number of writeable bytes, returning the offset before the skip.
    // Unlike Append, this simply reserves blank space, and no data is written.
    uint32_t Skip(uint32_t bytesToSkip)
    {
        assert(bytesToSkip % MinimumElementTypeByteSize == 0);
        return SkipInternal(m_writtenByteCount, bytesToSkip);
    }

    // Skip one element of the given type. e.g. writer.Skip<int32_t>().
    template <typename T>
    uint32_t Skip()
    {
        static_assert(sizeof(T) % MinimumElementTypeByteSize == 0);
        return SkipInternal(m_writtenByteCount, sizeof(T));
    }

    // Align to the natural alignment of the type first, then skip ahead the byte size of the type.
    template <typename T>
    uint32_t SkipAligned()
    {
        static_assert(sizeof(T) % MinimumElementTypeByteSize == 0);
        return SkipInternal(RoundUpToMultiple(m_writtenByteCount, uint32_t(alignof(T))), sizeof(T));
    }

    // Align to the given multiple. If already aligned, nothing happens.
    uint32_t Align(uint32_t byteAlignment)
    {
        assert(byteAlignment % MinimumElementTypeByteSize == 0);
        return SkipInternal(RoundUpToMultiple(m_writtenByteCount, byteAlignment), 0);
    }

    template <typename T>
    T& Get(uint32_t offsetInElements)
    {
        const uint32_t offsetInBytes = offsetInElements * MinimumElementTypeByteSize;
        assert(offsetInBytes + sizeof(T) <= size_t(m_data.size_bytes()));
        return *GetPointerInternal<T>(offsetInBytes);
    }

    template <typename T>
    const T& Read(uint32_t offsetInElements) const
    {
        const uint32_t offsetInBytes = offsetInElements * MinimumElementTypeByteSize;
        assert(offsetInBytes + sizeof(T) <= size_t(m_data.size_bytes()));
        return *GetPointerInternal<T>(offsetInBytes);
    }

    template <typename T>
    gsl::span<const T> ReadArray(uint32_t offsetInElements, uint32_t elementCount) const
    {
        const uint32_t offsetInBytes = offsetInElements * MinimumElementTypeByteSize;
        assert(offsetInBytes + sizeof(T) * elementCount <= size_t(m_data.size_bytes()));
        return gsl::make_span<const T>(GetPointerInternal<T>(offsetInBytes), elementCount);
    }

    void EnsureWriteableSize(uint32_t minimumByteSize, uint32_t additionalSizeInBytes = 0)
    {
        uint32_t newSizeInBytes = minimumByteSize + additionalSizeInBytes;
        if (newSizeInBytes < minimumByteSize) // overflow check.
            std::bad_array_new_length();

        if (newSizeInBytes > gsl::narrow_cast<uint32_t>(m_data.size_bytes()))
        {
            if (!m_resizeFunction)
                throw std::logic_error("MemoryReaderWriter - tried to resize a container that has no resize functionality.");

            m_data = m_resizeFunction(newSizeInBytes);
        }
        m_writtenByteCount = newSizeInBytes;
    }

protected:
    template <typename T>
    inline T* GetPointerInternal(uint32_t offsetInBytes) const noexcept // Logically const.
    {
        return reinterpret_cast<T*>(const_cast<std::byte*>(m_data.data()) + offsetInBytes);
    }

    // Append the byte array, returning the offset in elements.
    uint32_t AppendArrayInternal(uint32_t offsetInBytes, gsl::span<const std::byte> values)
    {
        EnsureWriteableSize(offsetInBytes, gsl::narrow_cast<uint32_t>(values.size_bytes()));
        WriteArrayInternal(offsetInBytes, values);
        return offsetInBytes / MinimumElementTypeByteSize;
    }

    void WriteArrayInternal(uint32_t offsetInBytes, gsl::span<const std::byte> values)
    {
        assert(offsetInBytes + values.size_bytes() <= gsl::narrow_cast<uint32_t>(m_data.size_bytes()));
        memcpy(m_data.data() + offsetInBytes, values.data(), values.size_bytes());
    }

    // Skip the given number of bytes, returning the offset in elements.
    uint32_t SkipInternal(uint32_t offsetInBytes, uint32_t additionalBytesToGrow)
    {
        EnsureWriteableSize(offsetInBytes, additionalBytesToGrow);
        return offsetInBytes / MinimumElementTypeByteSize;
    }

    template <typename T>
    static gsl::span<const std::byte> WrapValueAsByteSpan(const T& value)
    {
        // Ensure we're not wrapping anything except simple structs and types,
        // but especially not std::vector by accident, since you really want
        // the bytes the vector points to and not the vector class itself.
        static_assert(std::is_trivial_v<T>);
        return gsl::span<const std::byte>(reinterpret_cast<const std::byte*>(std::addressof(value)), sizeof(T));
    }

protected:
    gsl::span<std::byte> m_data;
    std::function<ResizeFunctionTypeSignature> m_resizeFunction;
    uint32_t m_writtenByteCount = 0; // Last appended byte offset into m_data.
};
