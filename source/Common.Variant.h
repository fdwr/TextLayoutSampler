//----------------------------------------------------------------------------
//  History:    2018-11-29 Dwayne Robinson - Created
//  Summary:    Extended std::variant which contains more intuitive methods
//              rather awkward and poorly named free functions.
// 
//              Extended methods:
//
//              v.get<T>()
//              v.is_type<T>()
//              v.index_of_type<T>()
//              v.call<Callback>(Callback& callback)
//
//----------------------------------------------------------------------------
#pragma once

template<typename VariantType, typename T, std::size_t index = 0>
constexpr std::size_t variant_index()
{
    if constexpr (index == std::variant_size_v<VariantType>)
    {
        return index;
    }
    else if constexpr (std::is_same_v<std::variant_alternative_t<index, VariantType>, T>)
    {
        return index;
    }
    else
    {
        return variant_index<VariantType, T, index + 1>();
    }
}

template <typename... Ts>
class variantex : public std::variant<Ts...>
{
public:
    using base = std::variant<Ts...>;

    using base::base;

    template<typename T>
    T& get()
    {
        return std::get<T>(*this);
    }

    template<typename T>
    bool is_type() const
    {
        return std::holds_alternative<T>(*this);
    }

    template<typename T>
    constexpr static size_t index_of_type()
    {
        return variant_index<base, T>();
    }

    template<typename T>
    void call(T& t)
    {
        base& b = *this;
        std::visit(t, b);
    }
};
