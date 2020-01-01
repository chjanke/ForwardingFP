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

//================================================================================
//                          "std::tuple_size" for signature
//================================================================================

template<typename T>
struct signature_size;

template<typename... Types>
struct signature_size<signature<Types...>>{
    static constexpr size_t value = sizeof...(Types);
};

template<typename T>
static constexpr size_t signature_size_v = signature_size<T>::value;

//================================================================================
//                          PACK ACCESS
//================================================================================

//Using a recursive implementation because our TMP will access all types anyway

template<size_t Index, typename T, typename... Types>
struct nth_element {
    using type = typename nth_element<Index - 1, Types...>::type;
};

template<typename T, typename... Types>
struct nth_element<0, T, Types...>
{
    using type = T;
};

template<size_t Index, typename... Types>
using nth_element_t = typename nth_element<Index, Types...>::type;
