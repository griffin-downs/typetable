// Sutter's "property" metaclass:
//   - Data fields only — the user writes no functions
//   - The metaclass generates typed getters and setters automatically
//   - Setters return new instances (functional update)
//
// This is the key demonstration of metaclasses as type transformations:
// the user describes data, the metaclass produces behavior.

#include <iostream>
#include <string>
#include <tuple>

#include <ctp/ctp.h>

using namespace ctp;


namespace Person
{
    struct Name { std::string value; };
    struct Age  { int value; };

    using Type = PropertyMetaclass<DataFields<Name, Age>>::Type;
}

namespace Vec3
{
    struct X { double value; };
    struct Y { double value; };
    struct Z { double value; };

    using Type = PropertyMetaclass<DataFields<X, Y, Z>>::Type;
}


int main()
{
    // --- Person ---

    auto person =
        Person::Type{
            Person::Name{ "Herb" },
            Person::Age{ 60 }
        };

    std::cout << "Person (initial):\n";

    person.access(
    [](const auto& _this)
    {
        std::cout
            << "  name: " << _this.template invoke<Get<Person::Name>>()
            << "\n"
            << "  age:  " << _this.template invoke<Get<Person::Age>>()
            << "\n";
    });

    // Functional update via generated setter
    auto older = person.access(
    [](const auto& _this)
    {
        return _this.template invoke<Set<Person::Age>>(61);
    });

    auto renamed = older.access(
    [](const auto& _this)
    {
        return _this.template invoke<Set<Person::Name>>(
            std::string{ "Herbert" });
    });

    std::cout << "\nPerson (after setters):\n";

    renamed.access(
    [](const auto& _this)
    {
        std::cout
            << "  name: " << _this.template invoke<Get<Person::Name>>()
            << "\n"
            << "  age:  " << _this.template invoke<Get<Person::Age>>()
            << "\n";
    });

    // Original unchanged
    std::cout << "\nPerson (original unchanged):\n";

    person.access(
    [](const auto& _this)
    {
        std::cout
            << "  name: " << _this.template invoke<Get<Person::Name>>()
            << "\n"
            << "  age:  " << _this.template invoke<Get<Person::Age>>()
            << "\n";
    });


    // --- Vec3 ---

    std::cout << "\n";

    constexpr auto v =
        Vec3::Type{
            Vec3::X{ 1.0 },
            Vec3::Y{ 2.0 },
            Vec3::Z{ 3.0 }
        };

    std::cout << "Vec3 (initial):\n";

    v.access(
    [](const auto& _this)
    {
        std::cout
            << "  (" << _this.template invoke<Get<Vec3::X>>()
            << ", " << _this.template invoke<Get<Vec3::Y>>()
            << ", " << _this.template invoke<Get<Vec3::Z>>()
            << ")\n";
    });

    auto w = v.access(
    [](const auto& _this)
    {
        return _this.template invoke<Set<Vec3::Z>>(10.0);
    });

    std::cout << "Vec3 (after set Z=10):\n";

    w.access(
    [](const auto& _this)
    {
        std::cout
            << "  (" << _this.template invoke<Get<Vec3::X>>()
            << ", " << _this.template invoke<Get<Vec3::Y>>()
            << ", " << _this.template invoke<Get<Vec3::Z>>()
            << ")\n";
    });
}
