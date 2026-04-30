// Sutter's "interface" metaclass:
//   - All functions are public and virtual-tagged
//   - No data fields
//   - Derived types provide concrete implementations
//
// In P0707 this would be:
//   interface Shape { void draw(); };
//
// Here, a type IS a namespace. Its structure IS a using directive.

#include <iostream>
#include <tuple>

#include <ctp/ctp.h>

using namespace ctp;


namespace Shape
{
    struct Draw
    {
        struct Virtual {};

        template<typename ThisContext>
        void operator()(const ThisContext&) const
        {
            std::cout << "Shape::Draw (base)\n";
        }
    };

    using Type =
        TypeTable<
            Hierarchy<>,
            Functions<Draw>,
            DataFields<>
        >;
}

namespace Circle
{
    struct Radius { double value; };

    struct Draw : public Shape::Draw
    {
        template<typename ThisContext>
        void operator()(const ThisContext& _this) const
        {
            std::cout
                << "Circle with radius "
                << _this.template get<Radius>().value << "\n";
        }
    };

    using Type =
        TypeTable<
            Hierarchy<
                Parent<Public, Shape::Type>
            >,
            Functions<Draw>,
            DataFields<Radius>
        >;
}

namespace Rectangle
{
    struct Height { double value; };
    struct Width { double value; };

    struct Draw : public Shape::Draw
    {
        template<typename ThisContext>
        void operator()(const ThisContext& _this) const
        {
            std::cout
                << "Rectangle "
                << _this.template get<Width>().value << "x"
                << _this.template get<Height>().value << "\n";
        }
    };

    using Type =
        TypeTable<
            Hierarchy<
                Parent<Public, Shape::Type>
            >,
            Functions<Draw>,
            DataFields<Height, Width>
        >;
}


int main()
{
    constexpr auto shapes =
        std::make_tuple(
            Circle::Type{
                Circle::Radius{ 5.0 }
            },
            Rectangle::Type{
                Rectangle::Height{ 3.0 },
                Rectangle::Width{ 7.0 }
            });

    // Polymorphic dispatch — no vtable, no virtual keyword,
    // no runtime indirection. Each type resolved at compile time
    // via DerivedTypeIndex searching the Functions tuple.
    forEach(
    [](const auto& shape)
    {
        shape.access(
        [](const auto& _this)
        {
            _this.template invoke<Shape::Draw>();
        });
    }, shapes);
}
