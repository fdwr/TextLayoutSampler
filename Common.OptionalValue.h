//----------------------------------------------------------------------------
//  Author:     Dwayne Robinson
//  History:    2015-06-19 Created
//----------------------------------------------------------------------------
#pragma once


// Bare minimum partial substitute for C++11 std::optional.
// This class also exists to work around some cases where default
// constructors are not defined by a class or the assignment operator
// was made private.
template <typename T>
class optional_value
{
public:
    T& value()
    {
        return *reinterpret_cast<T*>(t_);
    }

    T const& value() const
    {
        return *reinterpret_cast<T const*>(t_);
    }

    template <typename... Args>
    void emplace(Args&&... args)
    {
        clear();
        ::new(t_) T(args...);
        valueExists_ = true;
    }

    void clear()
    {
        if (valueExists_)
        {
            valueExists_ = false;
            value().~T();
        }
    }

    ~optional_value()
    {
        clear();
    }

    optional_value& operator=(optional_value const& otherOptional)
    {
        // Forward call to the assignment operator for the contained type,
        // or if assigning an empty optional, just clear this one.

        if (otherOptional.isCached)
        {
            operator=(otherOptional.value());
        }
        else
        {
            clear();
        }
        return *this;
    }

    optional_value& operator=(T const& t)
    {
        // If a value already exists, use the type's assignment operator.
        // Otherwise construct via in-place new.

        if (valueExists_)
        {
            value() = t;
        }
        else
        {
            new(t_) T(t);
            valueExists_ = true;
        }
        return *this;
    }

    T* operator->()
    {
        return reinterpret_cast<T*>(t_);
    }

    T const* operator->() const
    {
        return reinterpret_cast<T const*>(t_);
    }

    T& operator*()
    {
        return value();
    }

    T const& operator*() const
    {
        return value();
    }

    bool empty()
    {
        return !valueExists_;
    }

private:
    char t_[sizeof(T)];
    bool valueExists_ = false;
};
