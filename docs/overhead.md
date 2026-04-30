# Overhead Analysis

## Runtime overhead

**Zero.** No virtual tables, no indirection, no heap allocation. All dispatch resolved at compile time via template instantiation. The optimizer sees through every abstraction — verified by inspecting assembly output at `-O2`.

The `__COUNTER__` consteval dispatch path (`explorations/counter_dispatch.cpp`) compiles to direct inlined calls. No table lookup, no function pointer indirection in the generated assembly.

## Memory overhead

`std::tuple<>` has size 1 on MSVC ABI (Clang and MSVC on Windows). This means empty `Hierarchy<>` and `Functions<>` each occupy 1 byte, and alignment padding brings a TypeTable with only DataFields to 8 bytes larger than the equivalent raw struct.

| Platform | Overhead per empty tuple member |
|----------|-------------------------------|
| GCC / Linux | 0 bytes (`[[no_unique_address]]` works) |
| Clang / Linux | 0 bytes |
| Clang / Windows (MSVC ABI) | 1 byte + padding |
| MSVC | 1 byte + padding |

Example: `DataFields<X, Y, Z>` where each field is a double.

| Type | Size |
|------|------|
| `struct { double x, y, z; }` | 24 bytes |
| `DataFields<X, Y, Z>` | 24 bytes |
| `TypeTable<Hierarchy<>, Functions<>, DataFields<X, Y, Z>>` | 32 bytes (Windows), 24 bytes (Linux) |

The 8-byte overhead on Windows is a platform limitation, not an abstraction cost.

## Compile-time overhead

Template instantiation and concept validation add compile-time cost. For the examples in this library (6 examples, ~20 types), compilation is under 1 second. For large codebases with deep hierarchies and many types, compile time would increase proportionally to the number of DerivedTypeIndex searches and concept validations.

The constexpr step limits used in cto-assets-rtis (`-fconstexpr-steps=4290000000`) are not required by this library.
