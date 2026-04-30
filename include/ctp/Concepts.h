#pragma once

#include <concepts>
#include <tuple>

#include "TypeTraits.h"


namespace ctp
{
template<auto Concept, typename Tuple>
struct AllTypesSatisfyConcept;

template<auto Concept, typename First, typename... Rest>
struct AllTypesSatisfyConcept<Concept, std::tuple<First, Rest...>>
    : std::bool_constant<
        Concept.template operator()<First>() &&
        AllTypesSatisfyConcept<
            Concept,
            std::tuple<Rest...>
        >::value
    > {};

template<auto Concept>
struct AllTypesSatisfyConcept<Concept, std::tuple<>>
    : std::true_type {};


template<typename T>
concept HasEmptyTupleValue = requires(T a)
{
    { a.value };

    requires std::same_as<decltype(a.value), std::tuple<>>;
};

template<
    auto Concept,
    typename T
>
concept HasTupleValueSatisfyingConcept = requires(T a)
{
    { a.value };
    requires IsTuple<decltype(a.value)>::value == true;
    requires
        AllTypesSatisfyConcept<
            Concept,
            decltype(a.value)
        >::value;
};


template<typename T>
concept IsFunction = requires
{
    requires std::default_initializable<T>;
};

template<typename T>
concept IsDataField = requires(T a)
{
    { a.value };
};

template<typename T>
concept IsParent = requires(T a)
{
    { a.accessSpecifier };
    { a.typeTable };
};

template<typename T>
concept IsHierarchy =
    HasEmptyTupleValue<T> ||
    HasTupleValueSatisfyingConcept<
        [] <typename U> () consteval { return IsParent<U>; },
        T
    >;

template<typename T>
concept IsFunctions =
    HasEmptyTupleValue<T> ||
    HasTupleValueSatisfyingConcept<
        [] <typename U> () consteval { return IsFunction<U>; },
        T
    >;

template<typename T>
concept IsDataFields =
    HasEmptyTupleValue<T> ||
    HasTupleValueSatisfyingConcept<
        [] <typename U> () consteval { return IsDataField<U>; },
        T
    >;


// satisfies_concept<Concept, T> — bridges a concept to a value-level bool.
// Requires further investigation for Clang 17 compatibility.
// See explorations/sketches.cpp for the original attempt.
} // namespace ctp
