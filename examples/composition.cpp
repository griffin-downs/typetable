// Cross-cutting composition via Hierarchy.
//   - A Serializable behavior added to any type without modification
//   - Behavior composition is just tuple concatenation through Parents
//   - The original type's data fields stay in the child
//   - The composed behavior's functions are found via hierarchy traversal
//
// This is the metaclass vision beyond what P0707 explicitly shows:
// behaviors as mixins, composed at the type-table level, resolved
// at compile time through the parent chain.

#include <iostream>
#include <sstream>
#include <string>
#include <tuple>

#include <ctp/ctp.h>

using namespace ctp;


namespace Serializable
{
    struct Serialize
    {
        struct Virtual {};

        template<typename ThisContext>
        std::string operator()(const ThisContext&) const
        {
            return "{}";
        }
    };

    using Type =
        TypeTable<
            Hierarchy<>,
            Functions<Serialize>,
            DataFields<>
        >;
}

namespace Player
{
    struct Name  { std::string value; };
    struct Score { int value; };

    struct Serialize : public Serializable::Serialize
    {
        template<typename ThisContext>
        std::string operator()(const ThisContext& _this) const
        {
            std::ostringstream out;
            out
                << "{ name: \""
                << _this.template get<Name>().value
                << "\", score: "
                << _this.template get<Score>().value
                << " }";
            return out.str();
        }
    };

    using Type =
        TypeTable<
            Hierarchy<
                Parent<Public, Serializable::Type>
            >,
            Functions<Serialize>,
            DataFields<Name, Score>
        >;
}

namespace Enemy
{
    struct Kind   { std::string value; };
    struct Health { int value; };

    struct Serialize : public Serializable::Serialize
    {
        template<typename ThisContext>
        std::string operator()(const ThisContext& _this) const
        {
            std::ostringstream out;
            out
                << "{ kind: \""
                << _this.template get<Kind>().value
                << "\", health: "
                << _this.template get<Health>().value
                << " }";
            return out.str();
        }
    };

    using Type =
        TypeTable<
            Hierarchy<
                Parent<Public, Serializable::Type>
            >,
            Functions<Serialize>,
            DataFields<Kind, Health>
        >;
}


int main()
{
    auto player =
        Player::Type{
            Player::Name{ "Griffy" },
            Player::Score{ 42000 }
        };

    auto goblin =
        Enemy::Type{
            Enemy::Kind{ "Goblin" },
            Enemy::Health{ 30 }
        };

    auto dragon =
        Enemy::Type{
            Enemy::Kind{ "Dragon" },
            Enemy::Health{ 500 }
        };

    // Polymorphic serialization — both types share the Serializable
    // parent, but each provides its own Serialize override.
    // The dispatch is resolved through DerivedTypeIndex on
    // Serializable::Serialize as the base.

    std::cout << "Serialized entities:\n";

    // Each type is distinct, so we iterate a heterogeneous tuple.
    // Polymorphic dispatch resolves Serializable::Serialize through
    // DerivedTypeIndex — each type's override is found at compile time.
    forEach(
    [](const auto& entity)
    {
        auto json = entity.access(
        [](const auto& _this)
        {
            return _this.template invoke<Serializable::Serialize>();
        });

        std::cout << "  " << json << "\n";
    }, std::tie(player, goblin, dragon));

    // Functional update — damage the goblin
    auto damagedGoblin = goblin.set(Enemy::Health{ 10 });

    std::cout << "\nAfter damage:\n";
    damagedGoblin.access(
    [](const auto& _this)
    {
        std::cout
            << "  " << _this.template get<Enemy::Kind>().value
            << " health: " << _this.template get<Enemy::Health>().value
            << "\n";
    });
}
