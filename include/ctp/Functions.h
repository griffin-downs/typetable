#pragma once

#include <tuple>

#include "Concepts.h"


namespace ctp
{
template<IsFunction... Fs>
struct Functions
{
    std::tuple<Fs...> value;
};
} // namespace ctp
