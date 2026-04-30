# Compile-Time Polymorphism

A header-only C++23 library that implements compile-time type composition using tuples and concepts. Types are namespaces. Structure is a `using` directive. Polymorphic dispatch is resolved entirely at compile time with zero runtime overhead.

This is a working implementation of the kind of programmatic class composition that Herb Sutter's metaclasses proposal ([P0707](https://wg21.link/P0707)) described as requiring a language extension. It uses only features that shipped in C++20 and C++23.

## The Idea

A class in C++ bundles data, behavior, hierarchy, and access control into a single monolithic declaration. This library decomposes that into composable parts:

```cpp
namespace Circle
{
    struct Radius { double value; };

    struct Draw : public Shape::Draw
    {
        template<typename ThisContext>
        void operator()(const ThisContext& _this) const
        {
            std::cout << "Circle with radius "
                      << _this.template get<Radius>().value << "\n";
        }
    };

    using Type =
        TypeTable<
            Hierarchy<Parent<Public, Shape::Type>>,
            Functions<Draw>,
            DataFields<Radius>
        >;
}
```

A `TypeTable<Hierarchy, Functions, DataFields>` is a class — decomposed into its constituent parts, stored as tuples, validated by concepts at compile time. Polymorphic dispatch works through compile-time type search (`DerivedTypeIndex`), not virtual tables:

```cpp
constexpr auto shapes = std::make_tuple(
    Circle::Type{ Circle::Radius{ 5.0 } },
    Rectangle::Type{ Rectangle::Height{ 3.0 }, Rectangle::Width{ 7.0 } }
);

ctp::forEach([](const auto& shape) {
    shape.access([](const auto& _this) {
        _this.template invoke<Shape::Draw>();
    });
}, shapes);
```

No `virtual`. No vtable. No runtime indirection. Each type resolved statically.

## Metaclass Patterns

The library demonstrates several of Sutter's proposed metaclass patterns:

| Metaclass | What it enforces | Example |
|-----------|-----------------|---------|
| `interface` | Functions with virtual tags, no data | [`examples/interface.cpp`](examples/interface.cpp) |
| `plain_struct` | Data only, no functions, no hierarchy | [`examples/plain_struct.cpp`](examples/plain_struct.cpp) |
| `value` | Data with equality semantics | *planned* |
| `ordered` | Extends value with comparison | *planned* |

## Building

Requires C++23, CMake 3.12+, and a compatible compiler (tested with Clang 17).

```bash
mkdir build && cd build
cmake -G Ninja -DCMAKE_CXX_COMPILER=clang++ ..
ninja
```

Run examples:
```bash
./bin/interface
./bin/plain_struct
```

## Library Headers

Header-only. Single include via `#include <ctp/ctp.h>`, or individual headers:

| Header | Contents |
|--------|----------|
| `ctp/TypeTable.h` | `TypeTable<H,F,D>` with `ThisContext` for `get<>` and `invoke<>` |
| `ctp/TypeTraits.h` | `HasType`, `HasDerivedType`, `DerivedTypeIndex` — compile-time tuple search |
| `ctp/Concepts.h` | `IsFunction`, `IsDataField`, `IsHierarchy` — structural validation |
| `ctp/Hierarchy.h` | `Parent<AccessSpecifier, TypeTable>`, `Hierarchy<Parents...>` |
| `ctp/Functions.h` | `Functions<Fs...>` — concept-constrained function tuple |
| `ctp/DataFields.h` | `DataFields<Ds...>` — concept-constrained data tuple |
| `ctp/AccessSpecifiers.h` | `Public`, `Protected`, `Private` tag types |
| `ctp/ForEach.h` | Heterogeneous tuple iteration via fold expressions |

## Key Techniques

- **Concept-as-consteval-lambda**: `[] <typename U>() consteval { return IsParent<U>; }` — passes a concept as a value for validating heterogeneous tuple contents
- **DerivedTypeIndex**: Compile-time linear search through a tuple for a type derived from a given base — this IS the "virtual dispatch," resolved by the compiler
- **Namespace-as-class**: Types are namespaces with `using Type = TypeTable<...>`. No `class` keyword needed for the domain model
- **Inheritance as dispatch tag**: `Circle::Draw : public Shape::Draw` — C++ inheritance repurposed as a compile-time dispatch marker

## Relationship to P0707

Sutter's metaclasses proposal argued that C++ needs a new language feature for programmatic class generation. This library demonstrates that C++23's existing features — concepts for validation, constexpr for computation, tuples for heterogeneous storage, fold expressions for iteration — are sufficient to implement the same patterns as a library, and to go further (functional update, type-level search, evolutionary type composition) than the original proposal described.

## License

MIT
