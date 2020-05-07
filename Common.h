//----------------------------------------------------------------------------
//  History:    2015-06-19 Dwayne Robinson - Created
//----------------------------------------------------------------------------
#pragma once

#ifdef __cpp_modules
#define USE_CPP_MODULES 1
#elif !defined(USE_CPP_MODULES)
#define USE_CPP_MODULES 0
#endif


#if USE_CPP_MODULES // Is there no __cpp_modules feature test macro?
#define MODULE(moduleName) export module moduleName;
#else
#define MODULE(moduleName)
#endif


#if USE_CPP_MODULES
#define EXPORT export
#define EXPORT_BEGIN export {
#define EXPORT_END }
#else // Modules not supported by compiler. Define to nothing.
#define EXPORT
#define EXPORT_BEGIN
#define EXPORT_END
#endif


#ifndef IFR
#define IFR(hrIn) { HRESULT hrOut = (hrIn); if (FAILED(hrOut)) {return hrOut; } }
#endif

#ifndef ISR
#define ISR(hrIn) { HRESULT hrOut = (hrIn); if (SUCCEEDED(hrOut)) {return hrOut; } }
#endif

#ifndef IFRV
#define IFRV(hrIn) { HRESULT hrOut = (hrIn); if (FAILED(hrOut)) {return; } }
#endif

#ifndef RETURN_ON_ZERO // For null, zero, or false
#define RETURN_ON_ZERO(exp, retval) if (!(exp)) {return (retval);}
#endif

bool ThrowIf(bool value, _In_opt_z_ char const* message = nullptr);

#define DEBUG_ASSERT assert

template <typename T, size_t N>
constexpr size_t countof(const T(&a)[N])
{
    return N;// _countof(a);
}

template<typename T> void ZeroStructure(T& structure)
{
    memset(&structure, 0, sizeof(T));
}

bool TestBit(void const* memoryBase, uint32_t bitIndex) noexcept;
bool ClearBit(void* memoryBase, uint32_t bitIndex) noexcept;
bool SetBit(void* memoryBase, uint32_t bitIndex) noexcept;

// Returns true if current flags were updated (false if unchanged).
template<typename EnumType>
bool UpdateFlags(IN OUT EnumType& currentFlags, bool condition, EnumType flagsWhenTrue)
{
    EnumType previousFlags = currentFlags;
    currentFlags = (previousFlags & ~flagsWhenTrue) | (condition ? flagsWhenTrue : EnumType(0));
    return previousFlags != currentFlags;
}

template<typename T>
T* PtrAddByteOffset(T* p, size_t offset)
{
    return reinterpret_cast<T*>(reinterpret_cast<unsigned char*>(p) + offset);
}

template<typename T>
const T* PtrAddByteOffset(const T* p, size_t offset)
{
    return reinterpret_cast<const T*>(reinterpret_cast<const unsigned char*>(p) + offset);
}

template <typename FunctorType>
struct DeferCleanupType
{
public:
    explicit DeferCleanupType(FunctorType const& f) : f_(f) {}
    ~DeferCleanupType() { f_(); }

private:
    FunctorType f_;
};

template <typename FunctorType>
DeferCleanupType<FunctorType> inline DeferCleanup(FunctorType const& f) { return DeferCleanupType<FunctorType>(f); }


template <typename FunctorType>
struct DismissableCleanupType
{
public:
    explicit DismissableCleanupType(FunctorType const& f) : f_(f) {}
    ~DismissableCleanupType() { if (!isDismissed_) f_(); }
    void Dismiss() { isDismissed_ = true; }

private:
    FunctorType f_;
    bool isDismissed_ = false;
};

template <typename FunctorType>
DismissableCleanupType<FunctorType> inline DismissableCleanup(FunctorType const& f) { return DismissableCleanupType<FunctorType>(f); }


// Range of iterators that can be used in a ranged for loop.
template<typename ForwardIteratorType>
class iterator_range
{
public: // types
    using iterator = ForwardIteratorType;
    using const_iterator = ForwardIteratorType;
    using size_type = size_t;

public: // construction, assignment
    template<typename iterator>
    iterator_range(iterator begin, iterator end)
        : begin_(begin), end_(end)
    { }

    template<typename IteratorRangeType>
    iterator_range(const IteratorRangeType& range)
        : begin_(range.begin()), end_(range.end())
    { }

    template<typename IteratorRangeType>
    iterator_range& operator=(const IteratorRangeType& range)
    {
        begin_ = range.begin();
        end_ = range.end();
        return *this;
    }

    iterator const& begin() const
    {
        return begin_;
    }

    iterator const& end() const
    {
        return end_;
    }

    bool equals(const iterator_range& other) const
    {
        return begin_ == other.begin_ && end_ == other.end_;
    }

    bool operator ==(const iterator_range& other) const
    {
        return equals(other);
    }

    bool empty() const
    {
        return begin_ == end_;
    }

    size_type size() const
    {
        return std::distance(begin_, end_);
    }

protected:
    iterator begin_;
    iterator end_;
};


template<typename T>
iterator_range<T> make_iterator_range(T begin, T end)
{
    return iterator_range<T>(begin, end);
}


template<typename T>
iterator_range<T> make_iterator_range(T p, size_t beginIndex, size_t endIndex)
{
    return iterator_range<T>(p + beginIndex, p + endIndex);
}


// Helper to return multiple supported interfaces.
//
// Example:
//
//  STDMETHOD(QueryInterface)(IID const& iid, __out void** object) override
//  {
//      COM_BASE_RETURN_INTERFACE(iid, IUnknown, object);
//      COM_BASE_RETURN_INTERFACE(iid, IDWriteInlineObject, object);
//      COM_BASE_RETURN_NO_INTERFACE(object);
//  }
//
#define COM_BASE_RETURN_INTERFACE(iid, U, object) \
    if (iid == __uuidof(U)) \
    { \
        U* p = static_cast<U*>(this); \
        p->AddRef(); \
        *object = p; \
        return S_OK; \
    }

// For those cases when diamond inheritance causes the ambiguous cast compilation error.
#define COM_BASE_RETURN_INTERFACE_AMBIGUOUS(iid, U, object, subthis) \
    if (iid == __uuidof(U)) \
    { \
        U* p = static_cast<U*>(subthis); \
        p->AddRef(); \
        *object = p; \
        return S_OK; \
    }

#define COM_BASE_RETURN_NO_INTERFACE(object) \
        *object = nullptr; \
        return E_NOINTERFACE;


class ComObject : public IUnknown
{
public:
    // Default implementation for simple class.
    // Anything that implements more UUID's needs to override this method.
    //
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(IID const& iid, _Out_ void** object) throw() override
    {
        if (iid == __uuidof(IUnknown))
        {
            *object = static_cast<IUnknown*>(this);
            AddRef();
            return S_OK;
        }
        *object = nullptr;
        return E_NOINTERFACE;
    }

    virtual ULONG STDMETHODCALLTYPE AddRef() throw() override
    {
        return InterlockedIncrement(&refCount_);
    }

    virtual ULONG STDMETHODCALLTYPE Release() throw() override
    {
        auto newRefCount = InterlockedDecrement(&refCount_);
        if (newRefCount == 0)
        {
            delete this;
        };
        return newRefCount;
    }

protected:
    // Ensure 'delete this' calls the destructor of the most derived class.
    virtual ~ComObject()
    { }

    ULONG refCount_ = 0;
};

// Place these around usage if Visual Studio unnecessarily complains:
// #pragma warning(push)
// #pragma warning(disable:4307) // "The overflow is deliberate - warning C4307: '*': integral constant overflow"
// #pragma warning(pop)
constexpr size_t constexpr_hash(_In_z_ const char* input)
{
    size_t hash = sizeof(size_t) == 8 ? 0XCBF29CE484222325 : 0X811C9DC5;
    const size_t prime = sizeof(size_t) == 8 ? 0X00000100000001B3 : 0X01000193;

    while (*input)
    {
        hash ^= static_cast<size_t>(*input);
        hash *= prime;
        ++input;
    }

    return hash;
}
