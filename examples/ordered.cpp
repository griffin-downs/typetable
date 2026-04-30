// Sutter's "ordered" metaclass — extends value with ordering.
//   - Inherits Equals from a value-like parent via Hierarchy
//   - Adds LessThan for ordering
//   - Demonstrates hierarchy traversal: Equals resolved through parent
//
// In P0707 this would be:
//   ordered Temperature : value { double kelvin; };
//
// Here, the parent TypeTable carries Equals. The child adds LessThan.
// ThisContext's depth-first search finds Equals in the parent when
// it isn't in the child's own Functions tuple.

#include <algorithm>
#include <iostream>
#include <tuple>
#include <vector>

#include <ctp/ctp.h>

using namespace ctp;


namespace Measurement
{
    struct Kelvin { double value; };

    struct Equals
    {
        template<typename ThisContext>
        constexpr bool operator()(
            const ThisContext& _this,
            const ThisContext& other) const
        {
            return
                _this.template get<Kelvin>().value ==
                other.template get<Kelvin>().value;
        }
    };

    using Type =
        TypeTable<
            Hierarchy<>,
            Functions<Equals>,
            DataFields<Kelvin>
        >;
}

namespace Temperature
{
    struct LessThan
    {
        template<typename ThisContext>
        constexpr bool operator()(
            const ThisContext& _this,
            const ThisContext& other) const
        {
            return
                _this.template get<Measurement::Kelvin>().value <
                other.template get<Measurement::Kelvin>().value;
        }
    };

    using Type =
        TypeTable<
            Hierarchy<
                Parent<Public, Measurement::Type>
            >,
            Functions<LessThan>,
            DataFields<Measurement::Kelvin>
        >;
}


int main()
{
    auto boiling =
        Temperature::Type{
            Measurement::Kelvin{ 373.15 }
        };

    auto freezing =
        Temperature::Type{
            Measurement::Kelvin{ 273.15 }
        };

    auto body =
        Temperature::Type{
            Measurement::Kelvin{ 310.15 }
        };

    // LessThan — resolved locally
    auto isLess = freezing.access(
    [&](const auto& _this)
    {
        return boiling.access(
        [&](const auto& other)
        {
            return _this.template invoke<Temperature::LessThan>(other);
        });
    });

    std::cout
        << "freezing < boiling? "
        << (isLess ? "yes" : "no") << "\n";

    // Equals — resolved through hierarchy traversal into Measurement parent
    auto isEqual = boiling.access(
    [&](const auto& _this)
    {
        return boiling.access(
        [&](const auto& other)
        {
            return _this.template invoke<Measurement::Equals>(other);
        });
    });

    std::cout
        << "boiling == boiling? "
        << (isEqual ? "yes" : "no") << "\n";

    // Ordering a collection
    std::vector<Temperature::Type> temps = { boiling, freezing, body };

    std::sort(temps.begin(), temps.end(),
    [](const auto& a, const auto& b)
    {
        return a.access(
        [&](const auto& _this)
        {
            return b.access(
            [&](const auto& other)
            {
                return _this.template invoke<Temperature::LessThan>(other);
            });
        });
    });

    std::cout << "Sorted temperatures:\n";

    for (const auto& t : temps)
    {
        t.access(
        [](const auto& _this)
        {
            std::cout
                << "  "
                << _this.template get<Measurement::Kelvin>().value
                << " K\n";
        });
    }
}
