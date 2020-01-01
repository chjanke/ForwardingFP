#include <cstddef>
#include <utility>
#include <type_traits>
#include <bit>

//================================================================================
//                              Sequence Types
//================================================================================

template<bool... Bs>
struct bool_sequence{
    static constexpr size_t size = sizeof...(Bs);
    using value_type = bool;
};

template<typename... Ts>
struct signature{};

template<typename... IndexedOverload>
struct indexed_overload_sequence{};
