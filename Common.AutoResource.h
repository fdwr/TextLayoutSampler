//+---------------------------------------------------------------------------
//  Automatic resource management helper class.
//
//  History:    2007-07-30   Dwayne Robinson - Created
//              2014-10-06   Dwayne Robinson - Changed to template typedefs
//----------------------------------------------------------------------------
#pragma once


// This class mostly obviates the following custom classes
// (which are mostly just variants of various smart resource holders):
//
//      Common/SmartPtr.h : ScopedPtr
//      Platform/AccessToken.h : AccessToken
//      Platform/Win32/Win32Handle.h : Win32Handle
//      Platform/Win32/SecurityHelpers.h : AutoLocalFree
//      Platform/Win32/GdiHelpers.h : GdiHandle
//
// It partially obviates these, satisfying the destructors
// and getters, but the classes have more complex constructors,
// meaning they would require inheritance.
//
//      Platform/MemorySection.h : MemoryView, MemorySection
//
// With a free function (given a resource releaser that requires multiple
// parameters), it satisfies these too.
//
//      Platform/Win32/WaitHandle.h
//
// Example usage:
//
//  GdiPenHandle gdiPenHandle;
//  WaitHandleResource waitHandle;
//  ComPtr<IDWriteTextLayout> comPtr;


// Policy to determine how the resource is released when done, acquired if more owners want to hold
// a reference to it, or zero-initialized. The resource type itself should generally be a simple
// trivially copyable thing such as a pointer or handle type.
template <typename ResourceType = void*> // type of the resource held onto
struct DefaultResourceTypePolicy
{
    inline static void InitializeEmpty(_Out_ ResourceType* resource) noexcept
    {
        // Most resources (pointers to memory, HGLOBALS, HGDIOBJ...)
        // are indicated as empty by setting to 0/NULL. If a resource
        // has special semantics, override this method, or supply a
        // different policy class. Any implementations of this function
        // should directly initialize the resource to the empty state
        // without calling any freeing functions. The AutoResource will
        // call Release first, which is where that policy should happen.

        *resource = 0;
    }

    inline static bool IsNull(ResourceType resource) noexcept
    {
        return (resource == 0);
    }

    inline static void Acquire(ResourceType resource) noexcept
    {
        // Do nothing.
    }

    inline static void Release(ResourceType resource) noexcept
    {
        // Do nothing.
    }

    // Define as true if the type's policy (such as a COM interface) allows multiple AutoResource's
    // to hold the same resource, either because it is reference counted or because releasing it
    // will not destroy the resource. If false, then exactly one AutoResource should be the true
    // owner, and any other references should be weak. To avoid run-time surprises, it is a
    // compile-time error to call certain methods using another AutoResource as a parameter, since
    // that would lead to a two-owner situation. You can always still initialize from the raw
    // unscoped resource though (such as void* or HANDLE).
    static const bool AllowsMultipleReferences = false;
};


// Holds either a simple non-ref counted resource, such as a handle, raw
// memory, or a ref-counted interface, freeing it upon destruction (or not,
// depending on the policy implementation).
template <
    typename ResourceType = void*, // type of the resource held onto
    typename ResourceTypePolicy = DefaultResourceTypePolicy<ResourceType>,
    typename BaseResourceType = ResourceType // type as known by the resource releaser (like HGDIOBJ vs HBITMAP)
    >
class AutoResource
{
public:
    using Self = AutoResource<ResourceType, ResourceTypePolicy, BaseResourceType>;

    AutoResource(ResourceType resource)
    :   resource_(resource)
    {
        // Acquire is generally a no-op, but it matters for ref-counted objects.
        ResourceTypePolicy::Acquire(resource_);
    }

    AutoResource(Self const& other)
    :   resource_(other.resource_)
    {
        // Acquire is generally a no-op, but it matters for ref-counted objects.
        static_assert(ResourceTypePolicy::AllowsMultipleReferences, "This function is only useable on resource types that allow multiple strong references.");
        ResourceTypePolicy::Acquire(resource_);
    }

    inline AutoResource()
    {
        ResourceTypePolicy::InitializeEmpty(Cast(&resource_));
    }

    inline ~AutoResource() noexcept
    {
        // Notice we merely free the resource and do not zero the resource,
        // since we actually prefer to leave the value intact, not because
        // it's one fewer write back out to memory, but because it leaves a
        // more debuggable trace). If the derived class is a non-owning pointer,
        // the free is a no-op.

        ResourceTypePolicy::Release(resource_);
    }

    // Compiler generated assignment and copy construction do the right thing
    // here, since simple resources don't have any complex behavior.

    // Implicitly promote to the resource type for all the times the resource
    // handle/pointer is passed by value to a function.
    inline operator ResourceType() const noexcept
    {
        return resource_;
    }

    // Explicit getter for times when the compiler becomes confused
    // during overload resolution.
    inline ResourceType Get() const noexcept
    {
        return resource_;
    }

    // Implicitly promote to a reference to the resource type for when passed
    // to functions that want a reference, and for swapping resources in/out.
    // If modified directly, the caller is responsible for managing the object.
    // This intentionally does NOT have evil behavior of freeing the resource
    // since needing the address of a resource is not only for the sake of
    // out params - it's perfectly legitimate as an in param too, such as with
    // WaitForMultipleObjects().
    inline operator ResourceType&() noexcept
    {
        return resource_;
    }

    // Used when passed as an out parameter to any function,
    // or when the caller needs a pointer to a pointer.
    // If modified directly, the caller is responsible for managing the object.
    inline ResourceType* operator&() noexcept
    {
        return &resource_;
    }

    // Explicitly named form.
    inline ResourceType* Address() noexcept
    {
        return &resource_;
    }

    // Explicitly named form.
    inline ResourceType& Reference() noexcept
    {
        return resource_;
    }

    // For calling methods of an interface.
    //
    // ComPtr<ICat> cat;
    // cat->Meow();
    inline ResourceType operator->() const noexcept
    {
        return resource_;
    }

    // Set a new resource, acquiring the new resource before releasing the
    // old one, to prevent premature freeing issues with ref-counted objects
    // and because acquiring a resource is more likely to fail (with a
    // potential exception) than realising. For non-ref-counted objects,
    // the acquire is generally a no-op.
    inline ResourceType Set(ResourceType resource)
    {
        if (resource != resource_)
        {
            std::swap(resource_, resource);
            ResourceTypePolicy::Acquire(resource_);
            ResourceTypePolicy::Release(resource);
        }
        return resource_;
    }

    inline ResourceType Set(const Self& other)
    {
        static_assert(ResourceTypePolicy::AllowsMultipleReferences, "This function is only useable on resource types that allow multiple strong references.");
        return Set(other.resource_);
    }

    inline ResourceType Set(Self&& other)
    {
        if (other.resource_ != resource_)
        {
            ResourceType oldResource = resource_;
            resource_ = other.resource_;
            ResourceTypePolicy::InitializeEmpty(Cast(&other.resource_));
            ResourceTypePolicy::Release(oldResource);
        }
        return resource_;
    }

    inline Self& operator=(ResourceType resource)
    {
        Set(resource);
        return *this;
    }

    inline Self& operator=(const Self& other)
    {
        static_assert(ResourceTypePolicy::AllowsMultipleReferences, "This function is only useable on resource types that allow multiple strong references.");
        Set(other.resource_);
        return *this;
    }

    // No check. Just set it directly.
    inline ResourceType SetDirectly(ResourceType resource)
    {
        DEBUG_ASSERT(IsNull());
        resource_ = resource;
        return resource_;
    }

    void Clear()
    {
        ResourceType oldResource = resource_;
        ResourceTypePolicy::InitializeEmpty(Cast(&resource_));
        ResourceTypePolicy::Release(oldResource);
    }

    // Abandon the resource without freeing it.
    inline void Abandon() noexcept
    {
        ResourceTypePolicy::InitializeEmpty(Cast(&resource_));
    }

    // Like ATL's CComPtr, VS's _com_ptr_t, and the CLR's ptr::Attach,
    // this Attach does not increment the ref count, which is symmetric
    // to Detach().
    // * No self-assignment checking is done here.
    ResourceType Attach(ResourceType resource) noexcept
    {
        DEBUG_ASSERT(resource != resource_);
        std::swap(resource_, resource);
        ResourceTypePolicy::Release(resource);
        return resource_;
    }

    // Lets go of the resource without freeing it, but returning it
    // to the caller.
    ResourceType Detach() noexcept
    {
        ResourceType resource = resource_;
        ResourceTypePolicy::InitializeEmpty(Cast(&resource_));
        return resource;
    }

    // For the common transfer usage, where one container directly steals another,
    // it simplifies to:
    //
    //  p1.Attach(p2.Detach())   ->   p1.Steal(p2)
    //
    inline void Steal(Self& other) noexcept
    {
        swap(other);
        other.Clear();
    }

    inline bool IsNull() const noexcept
    {
        return ResourceTypePolicy::IsNull(resource_);
    }

    inline bool IsSet() const noexcept
    {
        return !ResourceTypePolicy::IsNull(resource_);
    }

    ////////////////////
    // STL aliases

    inline void clear()
    {
        Clear();
    }

    inline void swap(ResourceType& resource) noexcept
    {
        std::swap(resource_, resource);
    }

    inline void reset(ResourceType resource)
    {
        Set(resource);
    }

    // there is no 'release' alias, since the C++ auto_ptr does something
    // different than typically expected (detaches rather than frees).

protected:
    inline BaseResourceType* Cast(ResourceType* resource)
    {
        // Cast resource to the base type, such as an interface:
        //      IDWriteFont/IDWriteTextLayout/IShellFolder -> IUnknown
        // or handle:
        //      HBITMAP/HPEN/HFONT -> HGDIOBJ.
        //
        // Such an explicit cast isn't normally needed since the resource
        // is passed by pointer and implicitly castable, but a pointer to
        // a pointer is not directly castable. Trying to pass IShellFolder**
        // to a function that takes IUnknown** is normally a compile error
        // (even though it would actually work just fine).

        return reinterpret_cast<BaseResourceType*>(resource);
    }

    ResourceType resource_; // could be void*, HANDLE, FILE*, GDIOBJ...
};


// Overload std::swap for AutoResource<T> so that it can work with standard algorithms.
namespace std
{
    template <
        typename ResourceType,          // type of the resource held onto
        typename ResourceTypePolicy,    // policy for acquiring/releasing this resource
        typename BaseResourceType       // type as known by the resource releaser (like HGDIOBJ vs HBITMAP)
        >
    void swap(
        AutoResource<ResourceType, ResourceTypePolicy, BaseResourceType>& lhs,
        AutoResource<ResourceType, ResourceTypePolicy, BaseResourceType>& rhs
        ) noexcept
    {
        lhs.swap(rhs);
    }
}


////////////////////////////////////////
// Raw unowned pointer, which is not notably useful, but it can be used as a
// template argument without the caller/container worrying about whether it
// is raw or owned, and the reader is clear that it's a weak pointer.

static_assert(sizeof(int*) == sizeof(void*), "Expect all pointers have same size, such as void* and resourceType*");

template <typename ResourceType>
struct UnownedMemoryPointerPolicy : public DefaultResourceTypePolicy<ResourceType>
{
    // Allow multiple references because the non-owning AutoResource does not strongly hold it anyway.
    static const bool AllowsMultipleReferences = true;
};

template<typename ResourceType>
using UnownedMemoryPointer = AutoResource<ResourceType*, UnownedMemoryPointerPolicy<void*>, void*>;


////////////////////////////////////////
// Scoped memory pointer.
// No two scoped pointers may own the same object, and trying to assign one to another is a compile
// error (to avoid suprises such as with std::auto_ptr), but you can transfer them explicitly by
// Steal'ing from another, or by Detach/Attach.

template <typename ResourceType>
struct OwnedMemoryPointerPolicy : public DefaultResourceTypePolicy<ResourceType>
{
    inline static void Release(ResourceType resource) noexcept
    {
        if (resource != nullptr)
        {
            delete resource;
        }
    }

    static const bool AllowsMultipleReferences = false;
};

template<typename ResourceType>
using OwnedMemoryPointer = AutoResource<ResourceType*, OwnedMemoryPointerPolicy<ResourceType*>, ResourceType*>;
