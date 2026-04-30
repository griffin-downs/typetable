#pragma once

#include <tuple>
#include <utility>


namespace ctp
{
template<typename FunctionType, typename Tuple>
constexpr void forEach(FunctionType function, Tuple&& tuple)
{
    const auto apply =
    [&]<size_t... Indices>(std::index_sequence<Indices...>)
    {
        (function(std::get<Indices>(std::forward<Tuple>(tuple))), ...);
    };

    return apply(
        std::make_index_sequence<
            std::tuple_size<
                typename std::decay<decltype(tuple)>::type
            >::value
        >{});
}
} // namespace ctp
