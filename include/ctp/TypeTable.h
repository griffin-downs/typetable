#pragma once

#include <functional>
#include <optional>

#include "Concepts.h"
#include "TypeTraits.h"
#include "AccessSpecifiers.h"


namespace ctp
{
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
    constexpr TypeTable() = default;

    template<typename... Arguments>
        requires (sizeof...(Arguments) > 0)
    constexpr TypeTable(Arguments... arguments)
    : dataFields(std::forward<Arguments>(arguments)...)
    {
    }

    constexpr const auto& getDataFields() const
    {
        return this->dataFields;
    }

    constexpr const auto& getFunctions() const
    {
        return this->functions;
    }

    constexpr const auto& getHierarchy() const
    {
        return this->hierarchy;
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
            constexpr bool foundLocally =
                findInTuple<RequestedDataField, decltype(DataFieldsType::value)>();

            if constexpr (foundLocally)
            {
                const auto& result =
                    this->getFromTuple<RequestedDataField>
                    (this->thisTypeTable.dataFields.value);

                this->enforceAccess<RequestedDataField, AccessingType>();

                return result.get();
            }
            else
            {
                return this->getFromHierarchy<RequestedDataField>(
                    this->thisTypeTable.hierarchy.value);
            }
        }

        template<typename RequestedMethod, typename... Arguments>
        constexpr auto invoke(Arguments&&... arguments) const
        {
            constexpr bool foundLocally =
                findInTuple<RequestedMethod, decltype(FunctionsType::value)>();

            if constexpr (foundLocally)
            {
                const auto& method =
                    this->getFromTuple<RequestedMethod>
                    (this->thisTypeTable.functions.value);

                this->enforceAccess<RequestedMethod, AccessingType>();

                return method.get()(*this, std::forward<Arguments>(arguments)...);
            }
            else
            {
                return this->invokeFromHierarchy<RequestedMethod>(
                    this->thisTypeTable.hierarchy.value,
                    std::forward<Arguments>(arguments)...);
            }
        }

        constexpr auto set(auto newFieldValue) const
        {
            return this->thisTypeTable.set(newFieldValue);
        }

    private:
        const TypeTable& thisTypeTable;

        template<typename T, typename Tuple>
        static consteval bool findInTuple()
        {
            if constexpr (HasType<Tuple, T>::value)
                return true;
            else if constexpr (DerivedTypeIndex<Tuple, T>::value != -1)
                return true;
            else
                return false;
        }

        template<typename T, typename Tuple>
        constexpr auto getFromTuple(const Tuple& tuple) const
        {
            if constexpr (HasType<Tuple, T>::value)
            {
                return std::cref(std::get<T>(tuple));
            }
            else if constexpr (
                constexpr auto derivedTypeIndex =
                    DerivedTypeIndex<
                        Tuple,
                        T
                    >::value;
                derivedTypeIndex != -1)
            {
                return
                    std::cref(std::get<derivedTypeIndex>(tuple));
            }
            else
            {
                return std::nullopt;
            }
        }

        template<typename Member, typename Accessor>
        static constexpr void enforceAccess()
        {
            if constexpr (HasPrivateTag<Member>::value)
            {
                static_assert(
                    std::is_same_v<Accessor, Private>,
                    "Cannot access a Private member from outside");
            }

            if constexpr (HasProtectedTag<Member>::value)
            {
                static_assert(
                    std::is_same_v<Accessor, Private> ||
                    std::is_same_v<Accessor, Protected>,
                    "Cannot access a Protected member from outside");
            }
        }

        // Depth-first left-to-right hierarchy traversal for data fields
        template<typename RequestedDataField, typename ParentsTuple, size_t I = 0>
        constexpr const auto& getFromHierarchy(const ParentsTuple& parents) const
        {
            if constexpr (I < std::tuple_size_v<ParentsTuple>)
            {
                const auto& parent = std::get<I>(parents);
                using ParentTypeTable = std::decay_t<decltype(parent.typeTable)>;

                constexpr bool found =
                    [&]<typename TT>(const TT*)
                    {
                        return ParentTypeTable::template ThisContext<AccessingType>::
                            template canResolveDataField<RequestedDataField>();
                    }(static_cast<const ParentTypeTable*>(nullptr));

                if constexpr (found)
                {
                    return parent.typeTable.access(
                        [](const auto& _this) -> const auto&
                        {
                            return _this.template get<RequestedDataField>();
                        });
                }
                else
                {
                    return this->getFromHierarchy<RequestedDataField, ParentsTuple, I + 1>(parents);
                }
            }
            else
            {
                static_assert(I < std::tuple_size_v<ParentsTuple>,
                    "DataField not found in hierarchy");
            }
        }

        // Depth-first left-to-right hierarchy traversal for functions.
        // The function object is found in the parent, but invoked with
        // the child's ThisContext so it can access the child's fields.
        template<typename RequestedMethod, typename ParentsTuple, size_t I = 0, typename... Arguments>
        constexpr auto invokeFromHierarchy(const ParentsTuple& parents, Arguments&&... arguments) const
        {
            if constexpr (I < std::tuple_size_v<ParentsTuple>)
            {
                const auto& parent = std::get<I>(parents);
                using ParentTypeTable = std::decay_t<decltype(parent.typeTable)>;

                constexpr bool found =
                    [&]<typename TT>(const TT*)
                    {
                        return ParentTypeTable::template ThisContext<AccessingType>::
                            template canResolveFunction<RequestedMethod>();
                    }(static_cast<const ParentTypeTable*>(nullptr));

                if constexpr (found)
                {
                    const auto& method =
                        extractFunction<RequestedMethod>(parent.typeTable);

                    return method(*this, std::forward<Arguments>(arguments)...);
                }
                else
                {
                    return this->invokeFromHierarchy<RequestedMethod, ParentsTuple, I + 1>(
                        parents, std::forward<Arguments>(arguments)...);
                }
            }
            else
            {
                static_assert(I < std::tuple_size_v<ParentsTuple>,
                    "Function not found in hierarchy");
            }
        }

        // Extract a function object from a TypeTable, searching its local
        // functions first, then recursively into its own parents.
        template<typename RequestedMethod, typename TT>
        static constexpr const auto& extractFunction(const TT& typeTable)
        {
            using FuncTuple = std::decay_t<decltype(typeTable.getFunctions().value)>;

            if constexpr (findInTuple<RequestedMethod, FuncTuple>())
            {
                if constexpr (HasType<FuncTuple, RequestedMethod>::value)
                {
                    return std::get<RequestedMethod>(typeTable.getFunctions().value);
                }
                else
                {
                    constexpr auto idx = DerivedTypeIndex<FuncTuple, RequestedMethod>::value;
                    return std::get<idx>(typeTable.getFunctions().value);
                }
            }
            else
            {
                return extractFunctionFromParents<RequestedMethod>(
                    typeTable.getHierarchy().value);
            }
        }

        template<typename RequestedMethod, typename ParentsTuple, size_t I = 0>
        static constexpr const auto& extractFunctionFromParents(const ParentsTuple& parents)
        {
            static_assert(I < std::tuple_size_v<ParentsTuple>,
                "Function not found in parent hierarchy");

            const auto& parent = std::get<I>(parents);
            using ParentTT = std::decay_t<decltype(parent.typeTable)>;

            constexpr bool found =
                [&]<typename TT>(const TT*)
                {
                    return ParentTT::template ThisContext<AccessingType>::
                        template canResolveFunction<RequestedMethod>();
                }(static_cast<const ParentTT*>(nullptr));

            if constexpr (found)
            {
                return extractFunction<RequestedMethod>(parent.typeTable);
            }
            else
            {
                return extractFunctionFromParents<RequestedMethod, ParentsTuple, I + 1>(parents);
            }
        }

    public:
        template<typename RequestedDataField>
        static consteval bool canResolveDataField()
        {
            using DataTuple = decltype(DataFieldsType::value);

            if constexpr (findInTuple<RequestedDataField, DataTuple>())
                return true;
            else
                return canResolveInHierarchy<RequestedDataField, true>();
        }

        template<typename RequestedMethod>
        static consteval bool canResolveFunction()
        {
            using FuncTuple = decltype(FunctionsType::value);

            if constexpr (findInTuple<RequestedMethod, FuncTuple>())
                return true;
            else
                return canResolveInHierarchy<RequestedMethod, false>();
        }

    private:
        template<typename Requested, bool IsData, typename Parents = decltype(HierarchyType::value), size_t I = 0>
        static consteval bool canResolveInHierarchy()
        {
            if constexpr (I < std::tuple_size_v<Parents>)
            {
                using ParentType = std::tuple_element_t<I, Parents>;
                using ParentTT = decltype(ParentType::typeTable);

                constexpr bool found = [&]
                {
                    if constexpr (IsData)
                        return ParentTT::template ThisContext<AccessingType>::
                            template canResolveDataField<Requested>();
                    else
                        return ParentTT::template ThisContext<AccessingType>::
                            template canResolveFunction<Requested>();
                }();

                if constexpr (found)
                    return true;
                else
                    return canResolveInHierarchy<Requested, IsData, Parents, I + 1>();
            }
            else
            {
                return false;
            }
        }
    };

    template<typename Callable, typename AccessingType = Public>
    constexpr auto access(Callable&& callable) const
    {
        return callable(ThisContext<AccessingType>(*this));
    }

    template<typename RequestedDataField>
    constexpr auto set(RequestedDataField newValue) const
    {
        return this->withUpdatedField<RequestedDataField>(
            this->dataFields.value, newValue,
            std::make_index_sequence<
                std::tuple_size_v<decltype(this->dataFields.value)>
            >{});
    }

private:
    template<typename RequestedDataField, typename Tuple, typename NewValue, size_t... Is>
    constexpr auto withUpdatedField(
        const Tuple& fields,
        const NewValue& newValue,
        std::index_sequence<Is...>) const
    {
        return TypeTable{
            [&]<size_t I>()
            {
                if constexpr (std::is_same_v<
                    std::tuple_element_t<I, Tuple>,
                    RequestedDataField>)
                {
                    return newValue;
                }
                else
                {
                    return std::get<I>(fields);
                }
            }.template operator()<Is>()...
        };
    }
};
} // namespace ctp
