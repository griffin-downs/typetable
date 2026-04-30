# Mapping to Sutter's P0707 Metaclasses

How each metaclass from Herb Sutter's P0707 proposal maps to TypeTable.

## interface

**P0707:** All functions pure virtual, no data members, virtual destructor.

**TypeTable:** A type with `Functions<Draw>` where `Draw` has a `struct Virtual {}` tag, empty `DataFields<>`. Derived types override via C++ inheritance on the function struct (`Circle::Draw : public Shape::Draw`). DerivedTypeIndex resolves the override at compile time.

**Example:** `examples/interface.cpp`

## value

**P0707:** Regular type with equality, copyable, default constructible.

**TypeTable:** `DataFields<X, Y>` with `Functions<Equals>`. Functional update via `set<Field>(newValue)` returns a new instance. Immutability by default — no mutation, only transformation.

**Example:** `examples/value.cpp`

## plain_struct

**P0707:** All public members, no invariants, no functions.

**TypeTable:** `TypeTable<Hierarchy<>, Functions<>, DataFields<...>>`. The simplest possible TypeTable. The concept constraints (`IsDataFields`) validate that all members have `.value`.

**Example:** `examples/plain_struct.cpp`

## ordered

**P0707:** Extends value with comparison operators.

**TypeTable:** Child type inherits `Equals` from parent via `Hierarchy<Parent<Public, Measurement::Type>>`. Adds `LessThan` locally. Hierarchy traversal finds `Equals` in the parent chain. `std::sort` works directly on TypeTable instances.

**Example:** `examples/ordered.cpp`

## property

**P0707:** Auto-generated getters and setters from data members.

**TypeTable:** `PropertyMetaclass<DataFields<Name, Age>>::Type` generates `Functions<Get<Name>, Set<Name>, Get<Age>, Set<Age>>` via pack expansion. The user writes data. The metaclass generates behavior. 46 lines of library code.

**Example:** `examples/property.cpp`

## Beyond P0707

The TypeTable approach enables patterns that P0707 did not describe:

- **Cross-cutting composition:** Adding behavior (Serializable) to any type via Hierarchy without modifying the original type (`examples/composition.cpp`)
- **Compile-time dispatch tables:** `__COUNTER__`-based type registry with consteval function pointer resolution, verified zero-overhead in assembly (`explorations/counter_dispatch.cpp`)
- **Type-level search:** DerivedTypeIndex as a compile-time alternative to virtual dispatch
- **Concept-as-consteval-lambda:** `[] <typename U>() consteval { return IsParent<U>; }` for validating heterogeneous tuple contents
