#pragma once

#include <tuple>

#include "Concepts.h"


namespace ctp
{
template<IsDataField... Ds>
struct DataFields
{
    std::tuple<Ds...> value = {};

    constexpr DataFields() = default;

    constexpr DataFields(Ds... fields)
        requires (sizeof...(Ds) > 0)
    : value{ fields... }
    {}
};
} // namespace ctp
