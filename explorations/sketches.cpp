#include <iostream>
#include <optional>
#include <tuple>
#include <typeinfo>


// // find_and_invoke implementation
// template<typename TagType, typename TupleType, typename Lambda>
// auto find_and_invoke(TupleType& tuple, TagType, Lambda lambda) {
//     static_assert(has_type<TagType, TupleType>::value, "Tag type not found in tuple.");

//     // Logic to iterate over the tuple and find the matching tag, then invoke lambda.
//     // Placeholder for actual tuple iteration and invocation logic.
// }
// }
// template<typename TagType, typename Lambda, typename Hierarchy, typename... Tuples>
// auto find_and_invoke_in_hierarchy(Lambda lambda, Hierarchy& hierarchy, Tuples&... tuples) {
//     // Check current class's tuples
//     // If not found, recurse into the parent class in the hierarchy
// }

// template<typename AccessingType, typename Hierarchy, typename DataField>
// concept HasAccess = requires
// {
//     {  }
// }

template<typename Tuple, typename TypeToFind>
struct HasType;

template<typename TypeToFind, typename... TupleElementTypes>
struct HasType<std::tuple<TupleElementTypes...>, TypeToFind>
    : std::disjunction<
        std::is_same<TypeToFind, TupleElementTypes>...
    > {};

template<typename Tuple, typename TypeToFind>
struct HasDerivedType;

template<typename TypeToFind, typename... TupleElementTypes>
struct HasDerivedType<std::tuple<TupleElementTypes...>, TypeToFind>
    : std::disjunction<
        std::is_base_of<TypeToFind, TupleElementTypes>...
    > {};

template<typename T, typename = void>
struct HasVirtualTag : std::false_type {};

template<typename T>
struct HasVirtualTag<
    T,
    std::void_t<typename T::Virtual>
> : std::true_type {};

template<typename Tuple, typename DerivedTypeToFind, int N = 0, typename = void>
struct DerivedTypeIndex;

template<typename Tuple, typename DerivedTypeToFind, int N>
struct DerivedTypeIndex<
    Tuple,
    DerivedTypeToFind,
    N,
    std::enable_if_t<(N < std::tuple_size<Tuple>::value)>
>
{
    static constexpr int value =
    []
    {
        using CurrentElementType = std::tuple_element<N, Tuple>::type;
        if constexpr (
            std::is_base_of<
                DerivedTypeToFind,
                CurrentElementType
            >::value)
        {
            return N;
        }

        return DerivedTypeIndex<
            Tuple,
            DerivedTypeToFind,
            N + 1
        >::value;
    }();
};

template<typename Tuple, typename DerivedTypeToFind, int N>
struct DerivedTypeIndex<
    Tuple,
    DerivedTypeToFind,
    N,
    std::enable_if_t<N == std::tuple_size<Tuple>::value>
>
{
    static constexpr int value = -1;
};

template<typename Tuple, typename DerivedTypeToFind>
struct HasTypeDerivedFromVirtualTaggedBase
{
    static constexpr bool value =
        DerivedTypeIndex<
            Tuple,
            DerivedTypeToFind
        >::value != -1;
};


template<typename DerivedTypeToFind, typename Tuple>
constexpr const auto& GetDerivedElementFromVirtualTaggedBase(const Tuple& tuple)
{
    constexpr int index =
        DerivedTypeIndex<
            Tuple,
            DerivedTypeToFind
        >::value;

    return std::get<index>(tuple);
}

template<typename T, typename = void>
struct HasTypeDerivedFromVirtualBase : std::false_type {};

template<typename TypeToFind, typename... TupleElementTypes>
struct HasTypeDerivedFromVirtualBase<std::tuple<TupleElementTypes...>, TypeToFind>
    : std::conjunction<
        HasDerivedType<TypeToFind, TupleElementTypes...>
    > {};

template<typename T, typename = void>
struct HasPublicTag : std::false_type {};

template<typename T>
struct HasPublicTag<
    T,
    std::void_t<typename T::Public>
> : std::true_type {};

template<typename T, typename = void>
struct HasProtectedTag : std::false_type {};

template<typename T>
struct HasProtectedTag<
    T,
    std::void_t<typename T::Protected>
> : std::true_type {};

template<typename T, typename = void>
struct HasPrivateTag : std::false_type {};

template<typename T>
struct HasPrivateTag<
    T,
    std::void_t<typename T::Private>
> : std::true_type {};

template<typename Callable>
concept CallableConcept = std::invocable<Callable>;

template<typename T>
concept IsFunction = requires(T a)
{
    { a.operator() };
    requires std::default_initializable<T>;
    requires
        HasVirtualTag<T>::value == true ||
        HasVirtualTag<T>::value == false;

    requires
        HasPublicTag<T>::value ||
        HasProtectedTag<T>::value ||
        HasPrivateTag<T>::value ||
        (
            !HasPublicTag<T>::value &&
            !HasProtectedTag<T>::value &&
            !HasPrivateTag<T>::value
        );
};

template<typename T>
concept IsDataField = requires(T a)
{
    { a.value };
    // typename T::Type;
    // requires std::is_empty<typename T::Type>::value;
};

template<auto Concept, typename Tuple>
struct AllTypesSatisfyConcept;

template<auto Concept, typename First, typename... Rest>
struct AllTypesSatisfyConcept<Concept, std::tuple<First, Rest...>>
    : std::bool_constant<
        Concept.template operator()<First>() &&
        AllTypesSatisfyConcept<
            Concept,
            std::tuple<Rest...>
        >::value
    > {};

// Terminal case for when there are no more types in the tuple
template<auto Concept>
struct AllTypesSatisfyConcept<Concept, std::tuple<>> : std::true_type {};

template<typename T>
struct IsTuple : std::false_type {};

template<typename... Arguments>
struct IsTuple<
    std::tuple<Arguments...>
> : std::true_type {};

template<typename T>
concept HasEmptyTupleValue = requires(T a)
{
    { a.value };

    requires std::same_as<decltype(a.value), std::tuple<>>;
};

template<
    auto Concept,
    typename T
>
concept HasTupleValueSatisfyingConcept = requires(T a)
{
    { a.value };
    requires IsTuple<decltype(a.value)>::value == true;
    requires
        AllTypesSatisfyConcept<
            Concept,
            decltype(a.value)
        >::value;
};

// Type trait to check if a type satisfies a given concept
template<template<typename> typename Concept, typename T>
struct satisfies_concept : std::false_type {};

// Specialization when the type satisfies the concept
template<template<typename> typename Concept, typename T>
requires Concept<T>
struct satisfies_concept<Concept, T> : std::true_type {};

template<typename T>
concept IsParent = requires(T a)
{
    { a.accessSpecifier };
    { a.typeTable };
};

template<typename AccessSpecifier, typename TypeTable>
struct Parent
{
    AccessSpecifier accessSpecifier;
    TypeTable typeTable;
};

template<typename T>
concept IsHierarchy = 
    HasEmptyTupleValue<T> ||
    HasTupleValueSatisfyingConcept<
        [] <typename T> () consteval { return IsParent<T>; },
        T
    >;

template<typename... Parents>
struct Hierarchy
{
    std::tuple<Parents...> value;
};

template<typename T>
concept IsFunctions = 
    HasEmptyTupleValue<T> ||
    HasTupleValueSatisfyingConcept<
        [] <typename T> () consteval { return IsFunction<T>; },
        T
    >;

template<IsFunction... Functions>
struct Functions
{
    std::tuple<Functions...> value;
};

template<typename T>
concept IsDataFields =
    HasEmptyTupleValue<T> ||
    HasTupleValueSatisfyingConcept<
        [] <typename T> () consteval { return IsDataField<T>; },
        T
    >;

template<IsDataField... DataFields>
struct DataFields
{
    std::tuple<DataFields...> value;

    constexpr DataFields(DataFields... fields)
    : value{ fields... }
    {}
};

struct Public {};
struct Protected {};
struct Private {};

template<
    IsHierarchy HierarchyType,
    IsFunctions FunctionsType,
    IsDataFields DataFieldsType
>
class TypeTable
{
    HierarchyType hierarchy;
    FunctionsType functions;
    DataFieldsType dataFields;

public:
    template<typename... Arguments>
    constexpr TypeTable(Arguments... arguments)
    : dataFields(std::forward<Arguments>(arguments)...)
    {
    }

    template<typename AccessingType>
    class ThisContext
    {
    public:
        ThisContext(const TypeTable& typeTable)
        : thisTypeTable(typeTable)
        {
        }

        template<typename RequestedDataField>
        constexpr const auto& get() const
        {
            return
                getFromTuple<RequestedDataField>
                (thisTypeTable.dataFields.value).get();
        }

        template<typename RequestedMethod, typename... Arguments>
        constexpr auto invoke(Arguments&&... arguments) const
        // requires HasAccess<AccessingType, Hierarchy, DataField>
        {
            return
                getFromTuple<RequestedMethod>
                (thisTypeTable.functions.value)
                (*this, std::forward<Arguments>(arguments)...);
        }

    private:
        const TypeTable& thisTypeTable;

        template<typename T, typename Tuple>
        constexpr auto getFromTuple(const Tuple& tuple) const
        {
            if constexpr (HasType<Tuple, T>::value)
            {
                return std::cref(std::get<T>(tuple));
            } else if constexpr (
                constexpr auto derivedTypeIndex =
                    DerivedTypeIndex<
                        Tuple,
                        T
                    >::value;
                derivedTypeIndex != -1)
            {
                return
                    std::cref(std::get<derivedTypeIndex>(tuple));
            } else {
                return std::nullopt;
            }
        }
    };

    template<typename Callable, typename AccessingType = Public>
    constexpr auto access(Callable&& callable) const
    {
        return callable(ThisContext<AccessingType>(*this));
    }
};

// type table
    // functions
    // data
    // hierarchy

    // ThisContext is supplied to all functors within the type
    //  The ThisContext will give access to 

    // No access function
    // invoke
        // Takes the function to invoke
        // Searches for a type derived from the tag being looked for
        //      Checks the accessibility modifier if a candidate is found
        //          Won't return anything that isn't public
        // When found, it will supply the TypeTable of the object it was found in
    // get ()
        // Searches for the type that was supplied, checking for accessibility
        // returns <type>::value

// show things off via tests
    // tests that show things working and not working



namespace Base
{
    struct MyInt { int value = 5; };

    struct PrintMyInt
    {
        struct Virtual {};
        template<typename ThisContext>
        void operator()(const ThisContext& _this) const
        {
            std::cout << _this.get<MyInt>().value << std::endl;
        }
    };

    using Type = TypeTable<
        Hierarchy<>,
        Functions<
            PrintMyInt
        >,
        DataFields<
            MyInt
        >
    >;
}

namespace Derived
{
    struct PrintMyInt : public Base::PrintMyInt
    {
        // using Type = Base::PrintMyInt::Type;
        // struct Virtual {};

        template<typename ThisContext>
        void operator()(const ThisContext& _this) const
        {
            std::cout << _this.get<Base::MyInt>() + 1 << std::endl;
        }
    };

    struct Double { double value; };
    struct MyFirstDouble : public Double {};
    struct MySecondDouble : public Double {};

    using Type = TypeTable<
        Hierarchy<
            // Public<Base::Type>,
            Parent<Public, Base::Type>
        >,
        Functions<
            PrintMyInt
            // Private<
                // PrintMyInt
                // , variadic...
            // >
        >,
        DataFields<
            MyFirstDouble
        >
    >;
}

template <typename T, std::size_t ExpectedSize, typename Enable = void>
struct staticAssertSizeof {
    static constexpr std::size_t value = staticAssertSizeof<T, ExpectedSize - 1>::value;
};

template <typename T, std::size_t ExpectedSize>
struct staticAssertSizeof<T, ExpectedSize, std::enable_if_t<(ExpectedSize == 0)>> {
    static_assert(ExpectedSize != 0, "Reached the end of recursive descent");
    static constexpr std::size_t value = ExpectedSize;
};

// struct Foo
// {
//     virtual const char* hello() const { return "hello\n"; }
//     constexpr virtual ~Foo() {}
// };

// struct Bar : public Foo
// {
//     constexpr virtual const char* hello() const override { return "goodbye\n"; }
// };


// #include <array>

// constexpr Foo foo;
// constexpr Bar bar;
// constexpr std::array<Foo&, 2> arr = std::array<Foo&, 2>{ foo, bar };

// namespace Base
// {
//     struct TypeTag {};

//     struct GetTypeTag
//     {
//         template<typename T>
//         constexpr GetTypeTag(const TypeTag& tag) : tag(tag)
//         {
//             struct MyLocal : public MyHolder
//             {
//                 auto f = T{};
//             }
//         }

//         constexpr const TypeTag& get() const { return tag; }

//         const TypeTag& tag =  ;
//     };
// }

// namespace Derived
// {
//     struct TypeTag : public Base::TypeTag {};
// }

// template<>
// constexpr int* getTypeIdentifier<bool>(bool) {
//     // return reinterpret_cast<const int *>(&value<const bool>);
//     return const int*();
//     // return reinterpret_cast<uintptr_t>(&uniqueAddress);
// }

// template<typename PreviousGeneration, std::size_t CurrentGenerationIndex>
// struct SimulateGeneration
// {
//     // Placeholder for logic to simulate a generation based on the previous generation
//     using type = /* logic to determine the organism types for this generation */;
// };

// template<int TotalGenerationCount, typename InitialPopulation>
// constexpr auto simulateOrganisms(std::index_sequence<TotalGenerationCount>)
// {
//     return (... , typename SimulateGeneration<OrganismsTuple, Is>::type(orgs, Is));
// }


// template<typename OrganismType, size_t NumberOfGenerations>
// struct SimulateOrganisms
// {
//     static constexpr auto value =
//     []
//     {
//         const auto apply =
//         []<size_t... Indices>(std::index_sequence<Indices...>) constexpr
//         {
//             struct Generation
//             {
//                 constexpr Generation operator+(const Generation& rhs) const
//                 {
//                     return Generation(rhs.i + this->i);
//                 }

//                 constexpr Generation(size_t j) : i(j) {}

//                 size_t i;
//             };

//             return (Generation(Indices) + ...);
//         };

//         return apply(std::make_index_sequence<NumberOfGenerations>{});
//     }();
// };

// namespace InitialOrganism
// {
//     struct Reproduce;

//     using Type =
//         TypeTable<
//             Hierarchy<>,
//             Functions<Reproduce>,
//             DataFields<>
//         >;

//     struct Reproduce
//     {
//         template<typename ThisContext, typename OtherOrganism>
//         constexpr auto operator()(
//             const ThisContext& _this,
//             const OtherOrganism& otherOrganism) const
//         {
//             return Reproduce<Type, OtherOrganism::Type>;
//                 TypeTable<
//                     Hierarchy<>,
//                     Functions<
//                         Crossover<Type, OtherOrganism::Type>::Type
//                     >
//                 >{};
//         }

//     private:
         
//     };
// }

namespace Shape
{
    struct Draw
    {
        template <typename ThisContext>
        void operator()(const ThisContext&) const {
            std::cout << "Base draw\n";
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
        template <typename ThisContext>
        void operator()(const ThisContext& _this) const
        {
            std::cout
                << "Drawing a Circle with proportions:\n"
                << "\tRadius: " << _this.get<Radius>().value << "\n";
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
        template <typename ThisContext>
        void operator()(const ThisContext& _this) const
        {
            std::cout
                << "Drawing a Rectangle with proportions:\n"
                << "\tHeight: " << _this.get<Height>().value << "\n"
                << "\tWidth: " << _this.get<Width>().value << "\n";
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

template <typename FunctionType, typename Tuple>
constexpr void forEach(FunctionType function, Tuple&& tuple) {
    const auto apply =
    [&]<size_t... Indices>(std::index_sequence<Indices...>) {
        (function(std::get<Indices>(std::forward<Tuple>(tuple))), ...);
    };

    return apply(
        std::make_index_sequence<
            std::tuple_size<
                typename std::decay<decltype(tuple)>::type
            >::value
        >{});
}

int main()
{
    constexpr auto shapes =
        std::make_tuple(
            Rectangle::Type{
                Rectangle::Height{ 5 },
                Rectangle::Width{ 10 }
            },
            Circle::Type{
                Circle::Radius{ 5 }
            });

    forEach(
    [](const auto& shape)
    {
        shape.access(
        [](const auto& _this)
        {
            _this.invoke<Shape::Draw>();
        });
    }, shapes);

    // constexpr auto circle = std::get<Circle>(shapes);

    // constexpr auto circleReconstructed =
    //     Circle::Type
    //     {
    //         Radius{ circle.get<Radius>() + 5 }
    //     };

    // constexpr auto circleCopyAssigned =
    //     circle.set<Radius>(circle.get<Radius>() + 5);
}

// struct Reproduce
// {
//     using Type = int;
// };

// int main()
// {
//     constexpr auto numberOfGenerations = 100;
//     constexpr auto ResultPopulation =
//         SimulateOrganisms<
//             std::tuple<InitialOrganism::type>,
//             numberOfGenerations
//         >::value;

//     std::cout << ResultPopulation.i << std::endl;

//     constexpr Base::Type base{};
//     constexpr Derived::Type derived{};
//     base.access(
//     [](const auto& context)
//     {
//         context.invoke<Base::PrintMyInt>();
//         context.get<Base::MyInt>();
//     });
// }