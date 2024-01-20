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

namespace CompileTime
{
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

template<typename Tuple, typename DerivedTypeToFind, size_t N = 0, typename = void>
struct DerivedTypeFromVirtualTaggedBaseIndex;

template<typename Tuple, typename DerivedTypeToFind, size_t N>
struct DerivedTypeFromVirtualTaggedBaseIndex<
    Tuple,
    DerivedTypeToFind,
    N,
    std::enable_if_t<(N < std::tuple_size<Tuple>::value)>
>
{
    static constexpr std::optional<size_t> value =
    []
    {
        using CurrentElementType = std::tuple_element<N, Tuple>::type;
        if constexpr (std::is_base_of<
                    DerivedTypeToFind,
                    CurrentElementType
                >::value &&
                HasVirtualTag<CurrentElementType>::value)
        {
            return std::optional{ N };
        }

        return DerivedTypeFromVirtualTaggedBaseIndex<
            Tuple,
            DerivedTypeToFind,
            N + 1
        >::value;
    }();
};

template<typename Tuple, typename DerivedTypeToFind, size_t N>
struct DerivedTypeFromVirtualTaggedBaseIndex<
    Tuple,
    DerivedTypeToFind,
    N,
    std::enable_if_t<N == std::tuple_size<Tuple>::value>
>
{
    static constexpr std::optional<size_t> value = std::nullopt;
};

template<typename Tuple, typename DerivedTypeToFind>
struct HasTypeDerivedFromVirtualTaggedBase
{
    static constexpr bool value =
        DerivedTypeFromVirtualTaggedBaseIndex<
            Tuple,
            DerivedTypeToFind
        >::value.has_value();
};


template<typename Tuple, typename DerivedTypeToFind>
constexpr auto GetDerivedElementFromVirtualTaggedBase(const Tuple& tuple)
{
    constexpr int index =
        DerivedTypeFromVirtualTaggedBaseIndex<
            Tuple,
            DerivedTypeToFind
        >::value;

    static_assert(index != -1, "No derived type with found in tuple");

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
concept FunctionConcept = requires(T a)
{
    // typename T::Type;
    // requires std::is_empty<typename T::Type>::value;
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
concept DataFieldConcept = requires(T a)
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
    
    // std::is_same<decltype(a.value), std::tuple<>>::value;
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
concept ParentConcept = requires(T a)
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
concept HierarchyConcept = requires(T a)
{
    requires //all_satisfy_concept<ParentConcept, decltype(a.value)>::value;
        HasEmptyTupleValue<T> ||
        HasTupleValueSatisfyingConcept<
            ParentConcept,
            T
        >;
};

template<typename... ParentTypes>
struct Hierarchy
{
    using type = std::tuple<ParentTypes...>;
    type value;
};

// type table variadic constructor taking in DerivedTypes
// DerivedTypes is passed to the constructor of hierarchy
// hierarchy 



// MetaHierarchy is a type specialized on HierarchyType and decltype(*this) TypeTable

template<typename T>
concept Foo = false;

template<typename T>
concept Bar = true;

template<typename T>
concept FunctionsConcept = requires(T a)
{
    requires 
        HasTupleValueSatisfyingConcept<
            [] <typename T> () consteval { return FunctionConcept<T>; },
            T
        >;
};

template<FunctionConcept... FunctionTypes>
struct Functions
{
    std::tuple<FunctionTypes...> value;
};

template<typename T>
concept DataFieldsConcept = requires
{
    requires
        HasEmptyTupleValue<T> ||
        HasTupleValueSatisfyingConcept<
            DataFieldConcept,
            T
        >;
};

template<typename... DataFieldTypes>
struct DataFields
{
    std::tuple<DataFieldTypes...> value;
};

struct Public {};
struct Protected {};
struct Private {};

template<
    typename HierarchyType,
    FunctionsConcept FunctionsType,
    typename DataFieldsType
>
struct TypeTable
{
    struct Type {};

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
        // requires HasAccess<AccessingType, Hierarchy, DataField>
        {
            return tryGetFromTuple<RequestedDataField>(thisTypeTable.dataFields.value)->get();
                // .value_or(
                //     tryFindInHierarchy<RequestedDataField>(
                //     {
                //         .accessor = [](const auto& typeTable)
                //         {
                //             return typeTable.dataFields;
                //         }
                //     }))->get().value;

                    
                // .or_else(
                //     []
                //     {
                //         static_assert(
                //             false,
                //             "Requested DataField not found in Type Table.");
                //     })
        }

        template<typename RequestedMethod, typename... Arguments>
        constexpr auto invoke(Arguments&&... arguments) const
        // requires HasAccess<AccessingType, Hierarchy, DataField>
        {
            return tryGetFromTuple<RequestedMethod>(thisTypeTable.functions.value)->get()(*this, std::forward<Arguments>(arguments)...);
                // .value_or(tryFindInHierarchy(thisTypeTable.hierarchy.value))
        }

    private:
        const TypeTable& thisTypeTable;

        template<typename T>
        using OptionalReference =
            std::optional<
                std::reference_wrapper<const T>
            >;

        template<typename T, typename Tuple>
        constexpr OptionalReference<T> tryGetFromTuple(const Tuple& tuple) const
        {
            if constexpr (
                HasType<Tuple, T>::value == false &&
                HasTypeDerivedFromVirtualTaggedBase<Tuple, T>::value == false)
            {
                return std::nullopt;
            }

            return std::cref(std::get<T>(tuple));
        };

        template<typename T, typename ParentType, typename Accessor>
        constexpr OptionalReference<T> tryFindInHierarchy(const Accessor& accessor) const
        {
            // TODO support public / private / protected access levels
            return std::nullopt;
            // const auto& parentTypeTable = parent.typeTable;
            // return
            //     tryGetFromTuple(accessor(parentTypeTable))
            //         .value_or(
            //             tryFindInHierarchy(parentTypeTable.hierarchy, accessor));
        };
    };

    template<typename Callable, typename AccessingType = Public>
    constexpr auto access(Callable&& callable) const
    {
        return callable(ThisContext<AccessingType>(*this));
    }

    constexpr ~TypeTable()
    {
        // callDestructorsInHierarchyInOrder();
    }

// private:
    HierarchyType hierarchy;
    FunctionsType functions;
    DataFieldsType dataFields;
};

template<typename T>
concept TypeTableContext = requires
{
    requires T::get;
};

}

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
    using namespace CompileTime;

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
    using namespace CompileTime;

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

    struct MyDouble { double value; };

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
            MyDouble
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

int main()
{
    // constexpr auto numberOfGenerations = 100;
    // using ResultPopulation =
    //     std::invoke_result(
    //         simulateOrganisms<
    //             std::index_sequence<numberOfGenerations>,
    //             std::tuple<
    //                 InitialOrganism::Type
    //             >
    //         >)::type;

    // static constexpr auto resultPopulation = ResultPopulation{};

    // std::cout << resultPopulation << std::endl;


    constexpr Base::Type base{};
    constexpr Derived::Type derived{};
    base.access(
    [](const auto& context)
    {
        context.invoke<Base::PrintMyInt>();
        context.get<Base::MyInt>();
    });


    // // staticAssertSizeof<Base::Type, sizeof(Base::Type)>::value;

    // struct MyPOD
    // {
    //     int myInt;
    //     double myDouble;
    //     float myFloat;

    //     virtual ~MyPOD() {}
    // };

    // // struct MyDerivedPOD

    // std::cout << sizeof(base) << std::endl;
    // // std::cout << sizeof(int) << std::endl;
    // std::cout << sizeof(MyPOD) << std::endl;
    // // std::cout << typeid(Base::Type).name << std::endl;
    // derived.invoke<Derived::PrintMyInt>();
    // constexpr auto base& = derived.getReference<Base::Type>();
    // base.invoke<Base::PrintMyInt>();
    // Bar bar;
    // Foo& fooRef = bar;
    // fooRef.hello();

    // constexpr auto myCollection = make_tuple(base, derived);

    // forEachTuple(myCollection,
    // [](const auto& item)
    // {
    //     item.access<Base::Type>([](const auto& _this)
    //     {
    //         _this.invoke<Base::PrintMyInt>();
    //     });
    // });

    // static constexpr auto derivedTag = Derived::TypeTag();
    // static constexpr Base::GetTypeTag getTypeTag(derivedTag);
    // // static_assert(decltype(getTypeTag.get()) == void, "Uh oh!");
    // static constexpr auto& f = static_cast<const Derived::TypeTag&>(getTypeTag.get());
    // std::cout << typeid(f).name() << std::endl;
}


// template<typename AccessSpecifier, typename TypeTable>
// struct Parent
// {
//     AccessSpecifier accessSpecifier;
//     TypeTable typeTable;
// };
// template<typename... ParentTypes>
// struct Hierarchy
// {
//     using type = std::tuple<ParentTypes...>;
//     type value;
// };

// template<typename... FunctionTypes>
// struct Functions
// {
//     std::tuple<FunctionTypes...> value;
// };

// struct Public {};

// namespace Base
// {
//     struct PrintMyInt { struct Virtual {}; constexpr void operator() const; };
//     struct GetMyInt { constexpr int operator() const; };
//     struct MyInt { struct Private {}; int value = 10; };

//     using Type = TypeTable<
//         Hierarchy<>,
//         Functions<
//             PrintMyInt
//         >,
//         DataFields<
//             MyInt
//         >
//     >;
// }

// namespace Derived
// {
//     struct PrintMyInt { struct Override {}; constexpr void operator() const; };

//     using Type = TypeTable<
//         Hierarchy<
//             Parent<Public, Base::Type>
//         >,
//         Functions<
//             PrintMyInt
//         >,
//         DataFields<
//             MyInt
//         >
//     >;
// }

// constexpr Base::Type base;
// constexpr Derived::Type derived;
// constexpr Base::Type& baseRef = derived.GetReference<Base::Type>();

// static constexpr auto myIntArray = std::array<int, 2>{ base.get<Base::MyInt>(), derived.get<Base::MyInt>() };

// // baseRef.invoke implements compile time virtual dispatch to invoke Derived::PrintMyInt
// baseRef.invoke<Base::PrintMyInt>();

// constexpr auto myBaseRefArray =
//     std::array<Base::Type&, 2>
//     {
//         base.getReference(),
//         baseRef
//     };

// for (const Base::Type& b : myBaseRefArray)
// {
//     b.invoke<Base::PrintMyInt>();
// }





















// template<typename... TypeTagAndSpecifierTagPairs>
// struct Hierarchy { std::tuple<TypeTagAndSpecifierTagPairs> value };


// template<typename... TypeTagAndFunctionPointerPairs>
// struct FunctionSet
// {
//     std::tuple<TypeTagAndFunctionPointerPairs...> publicFunctions;
//     std::tuple<TypeTagAndFunctionPointerPairs...> protectedFunctions;
//     std::tuple<TypeTagAndFunctionPointerPairs...> privateFunctions;
// };

// template<typename... TypeTagAndValuePairs>
// struct DataFieldSet
// {
//     std::tuple<TypeTagAndFunctionPointerPairs...> publicFields;
//     std::tuple<TypeTagAndFunctionPointerPairs...> protectedFields;
//     std::tuple<TypeTagAndFunctionPointerPairs...> privateFields;
// };

// template<typename Hierarchy, typename FunctionSet, typename DataFieldSet>
// class TypeTable
// {
// public:

// public:

//     class Context
//     {};

//     template<typename... Arguments>
//     TypeTable(Arguments&&... arguments)
//         : hierarchy(),
//         functionSet(),
//         dataSet()
//     {
        
//     }

//     template<typename DataTag, typename T>
//     constexpr const T& get<DataTag>()
//     {
//         if constexpr (!hasAccess)
//     }




//     Hierarchy hierarchy;
//     FunctionSet functionSet;
//     DataFieldSet dataFieldSet;
// };




// namespace CompileTime
// {

// // Helper function to get a tuple element by type
// template<typename T, typename Tuple>
// constexpr T& get_by_type(Tuple& tuple)
// {
//     return std::get<T>(tuple);
// }

// // FunctionTable and DataField for demonstration
// template <typename... Functions>
// struct FunctionTable
// {
//     std::tuple<Functions...> functions;
// };

// template <typename... Fields>
// struct DataField
// {
//     std::tuple<Fields...> fields;
// };

// // CompileTimeTypeTable Definition
// template <typename Hierarchy, typename Table, typename Data>
// class CompileTimeTypeTable
// {
// public:
//     Hierarchy hierarchy;
//     Table table;
//     Data data;

//     template <typename Tag>
//     constexpr auto Get() const
//     {
//         return get_by_type<std::pair<Tag, decltype(get_data<Tag>(data.fields))>>(data.fields).second;
//     }

//     template <typename Tag, typename NewValue>
//     constexpr auto Updated(NewValue&& newValue) const {
//         auto newData = set_data<Tag>(data.fields, std::forward<NewValue>(newValue));
//         return CompileTimeTypeTable<Hierarchy, Table, decltype(newData)>{hierarchy, table, newData};
//     }

//     template <typename Tag>
//     constexpr auto& GetReference() const
//     {
//         return get_reference<Tag>(hierarchy);
//     }

// private:
//     template <typename Tag, typename Tuple, typename Value, std::size_t Index = 0>
//     constexpr auto set_data(const Tuple& tuple, Value&& value) const
//     {
//         if constexpr (Index < std::tuple_size_v<Tuple>)
//         {
//             if constexpr (std::is_same_v<typename std::tuple_element_t<Index, Tuple>::first_type, Tag>)
//             {
//                 return std::tuple_cat(
//                     std::make_tuple(std::pair<Tag, Value>(std::forward<Value>(value))),
//                     std::tuple_slice<Index + 1>(tuple)
//                 );
//             }
//             else
//             {
//                 return std::tuple_cat(
//                     std::make_tuple(std::get<Index>(tuple)),
//                     set_data<Tag, Tuple, Value, Index + 1>(tuple, std::forward<Value>(value))
//                 );
//             }
//         }
//         else
//         {
//             static_assert(Index < std::tuple_size_v<Tuple>, "Tag not found in DataField");
//         }
//     }

//     // Implementation of get_reference
//     template <typename Tag, typename Tuple, std::size_t Index = 0>
//     constexpr auto& get_reference(const Tuple& tuple) const
//     {
//         if constexpr (Index < std::tuple_size_v<Tuple>)
//         {
//             using ElementType = typename std::tuple_element_t<Index, Tuple>;
//             if constexpr (std::is_same_v<typename ElementType::first_type, Tag>)
//             {
//                 return std::get<Index>(tuple).second;
//             }
//             else
//             {
//                 return get_reference<Tag, Tuple, Index + 1>(tuple);
//             }
//         } else
//         {
//             static_assert(Index < std::tuple_size_v<Tuple>, "Tag not found in CompileTimeHierarchy");
//         }
//     }


//     // Implementation of get_data
//     template <typename Tag, typename Tuple, std::size_t Index = 0>
//     constexpr auto get_data(const Tuple& tuple) const
//     {
//         if constexpr (Index < std::tuple_size_v<Tuple>)
//         {
//             if constexpr (std::is_same_v<typename std::tuple_element_t<Index, Tuple>::first_type, Tag>)
//             {
//                 return std::get<Index>(tuple).second;
//             }
//             else
//             {
//                 return get_data<Tag, Tuple, Index + 1>(tuple);
//             }
//         }
//         else
//         {
//             static_assert(Index < std::tuple_size_v<Tuple>, "Tag not found in DataField");
//         }
//     }
// };

// namespace Base
// {
//     struct Type {};  // Unique tag for Base

//     using Type = CompileTimeTypeTable<
//         Istd::tuple<>,
//         FunctionTable< /* ... */ >,
//         DataField<std::pair<Type, int>>  // Example data field
//     >;
//     // Other definitions for Base...
// }

// namespace Derived
// {
//     struct DerivedTag {};  // Unique tag for Derived

//     using Type = CompileTimeTypeTable<
//         std::tuple<std::pair<Base::Type, Base::Type>>,  // Inherit from Base
//         FunctionTable< /* ... */ >,
//         DataField< /* ... */ >  // Data fields specific to Derived
//     >;
//     // Other definitions for Derived...
// }

// }


// int main()
// {
//     // Create a Derived instance
//     constexpr Derived::Type derivedInstance;

//     // Create a Base reference to the Derived instance
//     constexpr auto& baseRef = derivedInstance.GetReference<Base::DataTag>();

//     // Retrieve the current value and increment it
//     constexpr int currentValue = baseRef.Get<Base::DataTag>();
//     constexpr int incrementedValue = currentValue + 1;

//     // Update the Base instance with the incremented value
//     constexpr auto updatedBaseInstance = baseRef.Updated<Base::DataTag>(incrementedValue);

//     std::cout << updatedBaseInstance.Get<Base::DataTag>() << std::endl;

//     return 0;
// }
