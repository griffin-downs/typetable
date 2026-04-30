// Sutter's "plain_struct" metaclass:
//   - All fields public
//   - No functions
//   - No hierarchy
//
// In P0707 this would be:
//   plain_struct Config { int width; int height; bool fullscreen; };
//
// Here, a plain_struct is a TypeTable with empty Functions and Hierarchy.
// The concept enforces: no behavior, no inheritance, just data.

#include <iostream>
#include <tuple>

#include <ctp/ctp.h>

using namespace ctp;


namespace Config
{
    struct Width      { int value; };
    struct Height     { int value; };
    struct Fullscreen { bool value; };

    using Type =
        TypeTable<
            Hierarchy<>,
            Functions<>,
            DataFields<Width, Height, Fullscreen>
        >;
}

namespace Color
{
    struct R { float value; };
    struct G { float value; };
    struct B { float value; };
    struct A { float value; };

    using Type =
        TypeTable<
            Hierarchy<>,
            Functions<>,
            DataFields<R, G, B, A>
        >;
}


int main()
{
    constexpr auto config =
        Config::Type{
            Config::Width{ 1920 },
            Config::Height{ 1080 },
            Config::Fullscreen{ true }
        };

    config.access(
    [](const auto& _this)
    {
        std::cout
            << "Config: "
            << _this.template get<Config::Width>().value << "x"
            << _this.template get<Config::Height>().value
            << ((_this.template get<Config::Fullscreen>().value)
                ? " fullscreen" : " windowed")
            << "\n";
    });

    constexpr auto color =
        Color::Type{
            Color::R{ 0.2f },
            Color::G{ 0.4f },
            Color::B{ 0.8f },
            Color::A{ 1.0f }
        };

    color.access(
    [](const auto& _this)
    {
        std::cout
            << "Color: ("
            << _this.template get<Color::R>().value << ", "
            << _this.template get<Color::G>().value << ", "
            << _this.template get<Color::B>().value << ", "
            << _this.template get<Color::A>().value << ")\n";
    });
}
