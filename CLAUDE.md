# TypeTable

Header-only C++23 library implementing compile-time metaclasses via tuples and concepts. Repo: https://github.com/griffin-downs/typetable

## Build

```bash
mkdir build && cd build
cmake -G Ninja -DCMAKE_CXX_COMPILER=clang++ ..
ninja
```

All examples must compile clean with `-Wall -Wextra -Wpedantic -Werror` on Clang 17+.

## Code Style

Craft mode. Match Griffin's conventions exactly:
- IILEs for inline derivation
- `this->` for member access
- camelCase functions, PascalCase types
- Vertical rhythm between logical blocks
- Namespace `ctp` for library code, `using namespace ctp;` in examples only
- No explanatory comments for standard C++ features

## Architecture

`TypeTable<Hierarchy, Functions, DataFields>` decomposes a class into three concept-validated tuple containers. `ThisContext` provides `get<DataField>()` and `invoke<Function>()` with hierarchy traversal and access control.

Types are namespaces. Structure is a `using Type = TypeTable<...>` directive. Strong types via `struct Name { T value; };`.

## Techniques Catalog

`docs/techniques.json` — 79 cataloged TMP patterns with tags and reusability flags. Query with `/techniques <query>` skill or:
```bash
jq '[.[] | select(.tags[] == "dispatch")]' docs/techniques.json
jq '[.[] | select(.file | startswith("CompileTimePolymorphism"))]' docs/techniques.json
```

## Key Files

- `include/ctp/TypeTable.h` — core: TypeTable, ThisContext, hierarchy traversal, access control, functional update
- `include/ctp/TypeTraits.h` — HasType, DerivedTypeIndex, tag detectors
- `include/ctp/Concepts.h` — IsFunction, IsDataField, concept-as-consteval-lambda
- `include/ctp/Metaclass.h` — PropertyMetaclass type transform (Get/Set generation)
- `examples/` — six metaclass patterns (interface, plain_struct, value, ordered, composition, property)
- `explorations/sketches.cpp` — archaeological record of original experiments and failed dispatch attempts
- `explorations/counter_dispatch.cpp` — __COUNTER__ registry with verified zero-overhead assembly

## Docs

- `docs/p0707-mapping.md` — how each Sutter metaclass maps to TypeTable
- `docs/design-decisions.md` — architectural choices and reasoning
- `docs/overhead.md` — runtime (zero), memory (platform-dependent), compile-time analysis

## Open Questions

- Rename `ctp` namespace to `tt` to match repo name?
- `satisfies_concept` trait broken on Clang 17 (see Concepts.h comment)
- Compile-time virtual dispatch through base references remains unsolved in C++ as specified; TypeTable makes it unnecessary
