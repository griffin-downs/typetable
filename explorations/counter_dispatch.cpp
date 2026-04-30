// Experiment: can __COUNTER__ + a type registry give us
// compile-time virtual dispatch through a constexpr integer ID?
//
// The idea: each type gets a unique constexpr int via __COUNTER__.
// A constexpr dispatch table maps IDs to function pointers.
// In a consteval context, the dispatch resolves to a direct call.

#include <iostream>
#include <array>
#include <tuple>
#include <string_view>


// --- Type registry via __COUNTER__ ---

template<typename T>
struct TypeId;

static constexpr int counter_base = __COUNTER__;

#define REGISTER_TYPE(T) \
    template<> struct TypeId<T> { \
        static constexpr int value = __COUNTER__ - counter_base - 1; \
    };


// --- Types ---

struct Circle  { double radius; };
struct Rectangle { double width; double height; };
struct Triangle { double base; double height; };

REGISTER_TYPE(Circle)
REGISTER_TYPE(Rectangle)
REGISTER_TYPE(Triangle)

static constexpr int type_count = __COUNTER__ - counter_base - 1;


// --- Constexpr dispatch table ---

using DrawFn = void(*)(const void*);

template<typename T>
void drawImpl(const void* ptr);

template<>
void drawImpl<Circle>(const void* ptr)
{
    const auto& c = *static_cast<const Circle*>(ptr);
    std::cout << "Circle r=" << c.radius << "\n";
}

template<>
void drawImpl<Rectangle>(const void* ptr)
{
    const auto& r = *static_cast<const Rectangle*>(ptr);
    std::cout << "Rectangle " << r.width << "x" << r.height << "\n";
}

template<>
void drawImpl<Triangle>(const void* ptr)
{
    const auto& t = *static_cast<const Triangle*>(ptr);
    std::cout << "Triangle b=" << t.base << " h=" << t.height << "\n";
}

// The dispatch table — constexpr array indexed by TypeId
static constexpr std::array<DrawFn, type_count> draw_table = {
    &drawImpl<Circle>,
    &drawImpl<Rectangle>,
    &drawImpl<Triangle>
};


// --- Type-erased wrapper with compile-time ID ---

struct AnyShape
{
    const void* data;
    int typeId;

    template<typename T>
    AnyShape(const T& value)
    : data(&value)
    , typeId(TypeId<T>::value)
    {}

    void draw() const
    {
        draw_table[this->typeId](this->data);
    }
};


// --- Consteval experiment ---
// Can we resolve the dispatch at compile time?

template<typename T>
consteval int getTypeId()
{
    return TypeId<T>::value;
}

template<typename T>
consteval DrawFn getDrawFn()
{
    return draw_table[TypeId<T>::value];
}


int main()
{
    // Runtime dispatch through type-erased wrapper
    auto circle = Circle{ 5.0 };
    auto rect = Rectangle{ 7.0, 3.0 };
    auto tri = Triangle{ 4.0, 6.0 };

    std::array<AnyShape, 3> shapes = { circle, rect, tri };

    std::cout << "=== Runtime dispatch via ID ===\n";
    for (const auto& s : shapes)
    {
        s.draw();
    }

    // Compile-time: can we resolve the function at compile time?
    std::cout << "\n=== Consteval ID resolution ===\n";

    constexpr auto circleId = getTypeId<Circle>();
    constexpr auto rectId = getTypeId<Rectangle>();
    constexpr auto triId = getTypeId<Triangle>();

    std::cout << "Circle ID:    " << circleId << "\n";
    std::cout << "Rectangle ID: " << rectId << "\n";
    std::cout << "Triangle ID:  " << triId << "\n";

    // Consteval function pointer resolution
    constexpr auto circleDraw = getDrawFn<Circle>();
    constexpr auto rectDraw = getDrawFn<Rectangle>();

    std::cout << "\n=== Consteval dispatch (direct call, no table lookup) ===\n";
    circleDraw(&circle);
    rectDraw(&rect);

    // The key question: is circleDraw a direct call or does it
    // go through the table at runtime?
    // With -O2, the compiler should inline this completely.

    // Verify IDs are unique and sequential
    static_assert(circleId != rectId);
    static_assert(rectId != triId);
    static_assert(circleId != triId);
    static_assert(type_count == 3);

    std::cout << "\n=== Type count: " << type_count << " ===\n";
}
