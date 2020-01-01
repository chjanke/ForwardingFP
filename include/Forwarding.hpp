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

//================================================================================
//                     sequence add_front, add_back
//================================================================================

//TODO: generic implementation for both cases

template<typename Seq, Seq::value_type NewElement>
struct add_back;

template<bool... Bs, bool NewBool>
struct add_back<bool_sequence<Bs...>, NewBool>
{
    using type = bool_sequence<Bs..., NewBool>;
};

template<typename Seq, Seq::value_type New>
using add_back_t = typename add_back<Seq, New>::type;



template<typename Seq, Seq::value_type NewElement>
struct add_front;

template<size_t... Indices, size_t NewIndex>
struct add_front<std::index_sequence<Indices...>, NewIndex>
{
    using type = std::index_sequence<NewIndex, Indices...>;
};

template<bool... Bs, bool NewBool>
struct add_front<bool_sequence<Bs...>, NewBool>
{
    using type = bool_sequence<NewBool, Bs...>;
};

template<typename Seq, Seq::value_type New>
using add_front_t = typename add_front<Seq, New>::type;



//some tests
static_assert(std::is_same_v<add_front_t<std::index_sequence<0>, 1>, std::index_sequence<1,0>>);
static_assert(std::is_same_v<add_front_t<bool_sequence<false>, true>, bool_sequence<true,false>>);
