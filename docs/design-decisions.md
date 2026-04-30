# Design Decisions

Choices made during the development of the TypeTable library and the reasoning behind them.

## Types are namespaces

A type is a namespace containing struct members and a `using Type = TypeTable<...>` declaration. No `class` keyword is used for domain modeling.

**Why:** Namespaces provide scoping without the baggage of classes (access control syntax, constructor rules, inheritance mechanics). The `using Type` convention is predictable — `Parent<Public, Shape::Type>` works because `Type` always exists in the namespace.

## Strong types via struct wrapping

`struct Width { int value; };` instead of `using Width = int;`.

**Why:** `using` aliases are weak — two aliases to `int` are interchangeable. Struct wrapping creates a distinct type per field. The compiler rejects `Width` where `Height` is expected. The `.value` convention provides a uniform accessor that concepts can validate.

## Compile-time dispatch via DerivedTypeIndex

Polymorphic dispatch resolved by searching a tuple for a type derived from a given base, rather than virtual function tables.

**Why:** Virtual dispatch requires base references, which erase type information even in constexpr contexts. DerivedTypeIndex never erases — the tuple holds concrete types, and `std::is_base_of` searches at compile time. The optimizer eliminates all indirection.

## C++ inheritance repurposed as dispatch tags

`Circle::Draw : public Shape::Draw` — the inheritance has no runtime effect. It exists solely so `DerivedTypeIndex` can find `Circle::Draw` when searching for `Shape::Draw`.

**Why:** `std::is_base_of` is the only standard compile-time mechanism for expressing "this type is a refinement of that type." By inheriting from the base function struct, the derived version becomes findable via type traits without any runtime cost.

## Hierarchy traversal: depth-first left-to-right

When ThisContext searches for a function or data field not found locally, it walks Parent chains depth-first, left-to-right.

**Why:** Matches C++'s own method resolution order for multiple inheritance. Predictable for users already familiar with C++ dispatch semantics.

## Invoked parent functions receive child's ThisContext

When a function is found in a parent's Functions tuple, it is called with the child's ThisContext, not the parent's.

**Why:** This matches virtual dispatch semantics where `this` refers to the derived object. A parent's function can access fields that exist on the child — enabling inherited behavior that operates on derived data.

## Functional update returns same type

`set<Field>(newValue)` returns a new TypeTable instance of the same type. It cannot add or remove fields.

**Why:** Type-changing update would require returning a different TypeTable type, which complicates the API significantly. Same-type update is sufficient for value semantics and is consistent with functional programming conventions (persistent data structures).

## Empty tuples have size 1 (platform limitation)

`std::tuple<>` has size 1 on MSVC ABI (including Clang on Windows). This adds 8 bytes of padding to TypeTables with empty Hierarchy or Functions.

**Why not fix:** `[[no_unique_address]]` is not supported by Clang targeting MSVC ABI. On GCC/Linux the overhead is zero. Documented, not worth a workaround that would complicate the library for a platform-specific issue.

## Concept-as-consteval-lambda

`[] <typename U>() consteval { return IsParent<U>; }` passes a concept as a value to a template.

**Why:** C++ concepts cannot be passed as template parameters directly. The consteval lambda wraps the concept check in a callable that can be passed as an NTTP. This enables `AllTypesSatisfyConcept` to validate that every element of a tuple satisfies an arbitrary concept.

## Five failed approaches to compile-time virtual dispatch

Documented in `explorations/sketches.cpp`:
1. Constexpr virtual through base reference (type erased by reference)
2. TypeTag inheritance chains (base reference loses derived identity)
3. Address-based type identity (reinterpret_cast forbidden in constexpr)
4. sizeof probing (sizeof not unique across types)
5. Constexpr mutable counter (C++ forbids compile-time mutable state)

**Why document failures:** The failures define the boundary of what's expressible. DerivedTypeIndex succeeds precisely because it avoids the erasure that all five approaches hit. Understanding why they fail is essential to understanding why the solution works.
