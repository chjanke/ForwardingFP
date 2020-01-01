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

//================================================================================
//                      std::index_sequence filter refs
//================================================================================

template<typename T>
static constexpr bool is_value_type = !(std::is_lvalue_reference_v<T> || std::is_rvalue_reference_v<T>);



template<typename Signature, size_t CurrentIndex, typename Output>
struct get_value_indices;

template<typename... Types, size_t CurrentIndex, size_t... ValueIndices>
struct get_value_indices<signature<Types...>, CurrentIndex, std::index_sequence<ValueIndices...>>
{

    using output =  std::conditional_t<is_value_type<nth_element_t<CurrentIndex, Types...>>,
     add_front_t<std::index_sequence<ValueIndices...>, CurrentIndex>,
      std::index_sequence<ValueIndices...>>;

    using type = typename get_value_indices<signature<Types...>, CurrentIndex-1, output>::type;
};

template<typename... Types, size_t... ValueIndices>
struct get_value_indices<signature<Types...>, 0, std::index_sequence<ValueIndices...>>
{
    using type = std::conditional_t<is_value_type<nth_element_t<0, Types...>>,
     add_front_t<std::index_sequence<ValueIndices...>, size_t{0}>,
      std::index_sequence<ValueIndices...>>;
};



template<typename Signature>
struct value_indices;

template<typename... Types>
struct value_indices<signature<Types...>>
{
    using type = typename get_value_indices<signature<Types...>, signature_size_v<signature<Types...>>-1, std::index_sequence<>>::type;
};

template<typename Signature>
using value_indices_t = typename value_indices<Signature>::type;



//test
static_assert(std::is_same_v<value_indices_t<signature<int, bool&, const size_t&, unsigned, char&&, int*>>, std::index_sequence<0,3,5>>);

//================================================================================
//                             size_t to bool_sequence
//================================================================================

//TODO: support C++17 ?

template<size_t N, size_t bits, typename IndexSeq>
struct to_bool_sequence_impl;

template<size_t N, size_t bits, size_t... Powers>
struct to_bool_sequence_impl<N, bits, std::index_sequence<Powers...>>
{
    using type = bool_sequence<((N / (1<<(sizeof...(Powers) - Powers -1)) % 2) == 1)...>; //TODO: formula
};

template<size_t N, size_t BitWidth = std::log2p1(N)>
struct to_bool_sequence
{
    using type = typename to_bool_sequence_impl<N, BitWidth, std::make_index_sequence<BitWidth>>::type;
};

template<size_t N, size_t BitWidth = std::log2p1(N)>
using to_bool_sequence_t = typename to_bool_sequence<N, BitWidth>::type;



//some tests
static_assert(std::is_same_v<to_bool_sequence_t<4>, bool_sequence<true,false,false>>);
static_assert(to_bool_sequence_t<5>::size == 3);
static_assert(std::is_same_v<to_bool_sequence_t<5>,bool_sequence<true,false,true>>);
static_assert(std::is_same_v<to_bool_sequence_t<15>, bool_sequence<true,true,true,true>>);
static_assert(std::is_same_v<to_bool_sequence_t<0>, bool_sequence<>>);
static_assert(to_bool_sequence_t<5,5>::size == 5);
static_assert(std::is_same_v<to_bool_sequence_t<5, 5>, bool_sequence<false, false,true,false,true>>);
