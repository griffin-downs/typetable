// Sutter's "value" metaclass:
//   - Data fields with named construction
//   - Equality comparison via an Equals function
//   - Functional update — derive new instances from existing ones
//
// In P0707 this would be:
//   value Point { int x; int y; };
//
// Here, equality is just another function in the Functions tuple.
// Functional update uses TypeTable::set<> to produce a new instance
// with one field replaced — the original is never mutated.

#include <iostream>
#include <tuple>

#include <ctp/ctp.h>

using namespace ctp;


namespace Point
{
    struct X { int value; };
    struct Y { int value; };

    struct Equals
    {
        template<typename ThisContext>
        constexpr bool operator()(
            const ThisContext& _this,
            const ThisContext& other) const
        {
            return
                _this.template get<X>().value == other.template get<X>().value &&
                _this.template get<Y>().value == other.template get<Y>().value;
        }
    };

    using Type =
        TypeTable<
            Hierarchy<>,
            Functions<Equals>,
            DataFields<X, Y>
        >;
}


int main()
{
    constexpr auto origin =
        Point::Type{
            Point::X{ 0 },
            Point::Y{ 0 }
        };

    constexpr auto point =
        Point::Type{
            Point::X{ 3 },
            Point::Y{ 4 }
        };

    // Named field access
    point.access(
    [](const auto& _this)
    {
        std::cout
            << "Point: ("
            << _this.template get<Point::X>().value << ", "
            << _this.template get<Point::Y>().value << ")\n";
    });

    // Equality via invoke
    auto equal = origin.access(
    [&](const auto& _this)
    {
        return point.access(
        [&](const auto& other)
        {
            return _this.template invoke<Point::Equals>(other);
        });
    });

    std::cout
        << "origin == point? "
        << (equal ? "yes" : "no") << "\n";

    auto selfEqual = point.access(
    [&](const auto& _this)
    {
        return point.access(
        [&](const auto& other)
        {
            return _this.template invoke<Point::Equals>(other);
        });
    });

    std::cout
        << "point == point? "
        << (selfEqual ? "yes" : "no") << "\n";

    // Functional update — create a new point with X changed
    constexpr auto moved = point.set(Point::X{ 10 });

    moved.access(
    [](const auto& _this)
    {
        std::cout
            << "After set<X>(10): ("
            << _this.template get<Point::X>().value << ", "
            << _this.template get<Point::Y>().value << ")\n";
    });
}
