#pragma once

#include <tuple>

#include "AccessSpecifiers.h"


namespace ctp
{
template<typename AccessSpecifier, typename TypeTable>
struct Parent
{
    AccessSpecifier accessSpecifier;
    TypeTable typeTable;
};

template<typename... Parents>
struct Hierarchy
{
    std::tuple<Parents...> value;
};
} // namespace ctp
