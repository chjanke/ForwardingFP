#include <cstddef>
#include <utility>
#include <type_traits>
#include <array>


//================================================================================
//                              Sequence Types
//================================================================================

template<typename... Ts>
struct signature{};

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
//                       sequence add_front
//================================================================================

template<typename Seq, Seq::value_type NewElement>
struct add_front;

template<size_t... Indices, size_t NewIndex>
struct add_front<std::index_sequence<Indices...>, NewIndex>
{
    using type = std::index_sequence<NewIndex, Indices...>;
};

template<typename Seq, Seq::value_type New>
using add_front_t = typename add_front<Seq, New>::type;



static_assert(std::is_same_v<add_front_t<std::index_sequence<0>, 1>, std::index_sequence<1,0>>);

//================================================================================
//                      std::index_sequence filter refs
//================================================================================
//TODO: reduce instantiations ? less recursion? transform seq to seq of 0 or 1 element seq and concat that

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
//                             test bit of number
//================================================================================

inline constexpr bool test_bit(size_t n, size_t bitIndex){
    if(bitIndex > sizeof(size_t) * 8) return 0;
    return n & (1 << bitIndex);
}



static_assert(test_bit(0,0) == false && test_bit(0, 500) == false); //0...
static_assert(test_bit(1,0) == true && test_bit(1,1) == false && test_bit(1,20) == false); //01
static_assert(test_bit(2,0) == false && test_bit(2,1) == true); //10
static_assert(test_bit(3,0) == true && test_bit(3,1) == true); //11
static_assert(test_bit(4,0) == false && test_bit(4,1) == false && test_bit(4,2) == true); //100
static_assert(test_bit(5,0) == true && test_bit(5,1) == false && test_bit(5,2) == true); //101
static_assert(test_bit(6,0) == false && test_bit(6,1) == true && test_bit(6,2) == true); //110
static_assert(test_bit(7,0) == true && test_bit(7,1) == true && test_bit(7,2) == true); //110
