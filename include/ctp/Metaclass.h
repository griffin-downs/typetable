#pragma once

#include "DataFields.h"
#include "Functions.h"
#include "Hierarchy.h"
#include "TypeTable.h"


namespace ctp
{
template<typename Field>
struct Get
{
    template<typename ThisContext>
    constexpr auto operator()(const ThisContext& _this) const
    {
        return _this.template get<Field>().value;
    }
};

template<typename Field>
struct Set
{
    template<typename ThisContext, typename Value>
    constexpr auto operator()(const ThisContext& _this, Value newValue) const
    {
        return _this.set(Field{ newValue });
    }
};


template<typename DataFieldsTuple>
struct PropertyMetaclass;

template<typename... Ds>
struct PropertyMetaclass<DataFields<Ds...>>
{
    using GeneratedFunctions = Functions<Get<Ds>..., Set<Ds>...>;

    using Type = TypeTable<
        Hierarchy<>,
        GeneratedFunctions,
        DataFields<Ds...>
    >;
};
} // namespace ctp
