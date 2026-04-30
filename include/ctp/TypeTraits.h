#pragma once

#include <tuple>
#include <type_traits>


namespace ctp
{
template<typename Tuple, typename TypeToFind>
struct HasType;

template<typename TypeToFind, typename... TupleElementTypes>
struct HasType<std::tuple<TupleElementTypes...>, TypeToFind>
    : std::disjunction<
        std::is_same<TypeToFind, TupleElementTypes>...
    > {};


template<typename Tuple, typename TypeToFind>
struct HasDerivedType;

template<typename TypeToFind, typename... TupleElementTypes>
struct HasDerivedType<std::tuple<TupleElementTypes...>, TypeToFind>
    : std::disjunction<
        std::is_base_of<TypeToFind, TupleElementTypes>...
    > {};


template<typename T, typename = void>
struct HasVirtualTag : std::false_type {};

template<typename T>
struct HasVirtualTag<
    T,
    std::void_t<typename T::Virtual>
> : std::true_type {};


template<typename T, typename = void>
struct HasPublicTag : std::false_type {};

template<typename T>
struct HasPublicTag<
    T,
    std::void_t<typename T::Public>
> : std::true_type {};


template<typename T, typename = void>
struct HasProtectedTag : std::false_type {};

template<typename T>
struct HasProtectedTag<
    T,
    std::void_t<typename T::Protected>
> : std::true_type {};


template<typename T, typename = void>
struct HasPrivateTag : std::false_type {};

template<typename T>
struct HasPrivateTag<
    T,
    std::void_t<typename T::Private>
> : std::true_type {};


template<typename T>
struct IsTuple : std::false_type {};

template<typename... Arguments>
struct IsTuple<
    std::tuple<Arguments...>
> : std::true_type {};


template<typename Tuple, typename DerivedTypeToFind, int N = 0, typename = void>
struct DerivedTypeIndex;

template<typename Tuple, typename DerivedTypeToFind, int N>
struct DerivedTypeIndex<
    Tuple,
    DerivedTypeToFind,
    N,
    std::enable_if_t<(N < std::tuple_size<Tuple>::value)>
>
{
    static constexpr int value =
    []
    {
        using CurrentElementType =
            std::tuple_element<N, Tuple>::type;

        if constexpr (
            std::is_base_of<
                DerivedTypeToFind,
                CurrentElementType
            >::value)
        {
            return N;
        }

        return DerivedTypeIndex<
            Tuple,
            DerivedTypeToFind,
            N + 1
        >::value;
    }();
};

template<typename Tuple, typename DerivedTypeToFind, int N>
struct DerivedTypeIndex<
    Tuple,
    DerivedTypeToFind,
    N,
    std::enable_if_t<N == std::tuple_size<Tuple>::value>
>
{
    static constexpr int value = -1;
};


template<typename Tuple, typename DerivedTypeToFind>
struct HasTypeDerivedFromVirtualTaggedBase
{
    static constexpr bool value =
        DerivedTypeIndex<
            Tuple,
            DerivedTypeToFind
        >::value != -1;
};


template<typename DerivedTypeToFind, typename Tuple>
constexpr const auto& getDerivedElement(const Tuple& tuple)
{
    constexpr int index =
        DerivedTypeIndex<
            Tuple,
            DerivedTypeToFind
        >::value;

    return std::get<index>(tuple);
}
} // namespace ctp
