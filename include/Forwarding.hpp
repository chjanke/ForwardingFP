#include <cstddef>
#include <utility>
#include <type_traits>
#include <array>


//================================================================================
//                              Sequence Types
//================================================================================

template<typename... Ts>
struct signature{};



template<typename... Overloads>
struct overload_set{};

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
    using type = typename get_value_indices<signature<Types...>, sizeof...(Types) - 1, std::index_sequence<>>::type;
};

template<typename Signature>
using value_indices_t = typename value_indices<Signature>::type;



//test
static_assert(std::is_same_v<value_indices_t<signature<int, bool&, const size_t&, unsigned, char&&, int*>>, std::index_sequence<0,3,5>>);

//================================================================================
//                             test_bit fn
//================================================================================

//returns true if the binary representation of n has a '1' at position bitIndex
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

//================================================================================
//                              find_index_of fn
//================================================================================

//returns the array index of the provided value, or N if indices does not contain it
template<size_t N>
inline constexpr size_t find_index_of(std::array<size_t, N> const& indices, size_t value)
{
    //std::find is not constexpr until c++20
    for(size_t i{0}; i != N; ++i){
        if(indices[i] == value) return i;
    }
    return N;
}



static_assert(find_index_of(std::array<size_t, 4>{0,2,4,5}, 0) == 0);
static_assert(find_index_of(std::array<size_t, 4>{0,2,4,5}, 2) == 1);
static_assert(find_index_of(std::array<size_t, 4>{0,2,4,5}, 4) == 2);
static_assert(find_index_of(std::array<size_t, 4>{0,2,4,5}, 5) == 3);

static_assert(find_index_of(std::array<size_t, 4>{0,2,4,5}, 3) == 4);
static_assert(find_index_of(std::array<size_t, 4>{0,2,4,5}, 1) == 4);

//================================================================================
//                              seq_to_array fn
//================================================================================

template<typename IndexSeq>
struct seq_to_array;

template<size_t... Indices>
struct seq_to_array<std::index_sequence<Indices...>>
{
    static constexpr std::array<size_t, sizeof...(Indices)> value()
    {
        return {Indices...};
    }
};



static_assert(seq_to_array<std::index_sequence<2,4>>::value()[0] == 2 && seq_to_array<std::index_sequence<2,4>>::value()[1] == 4);

//================================================================================
//                      make_overload_signature
//================================================================================

template<typename SignatureType, bool IsRvalueRef>
using overload_type_t = std::conditional_t<IsRvalueRef, SignatureType&&, SignatureType&>;



template<typename Signature, typename SigIndices, typename FwdIndices>
struct make_overload_signature;

template<typename... SigTypes, size_t... SigIndices, size_t... FwdIndices>
struct make_overload_signature<signature<SigTypes...>, std::index_sequence<SigIndices...>, std::index_sequence<FwdIndices...>>
{
    template<size_t Index>
    static constexpr size_t get_index = find_index_of(seq_to_array<std::index_sequence<FwdIndices...>>::value(), Index);

    template<size_t SigIndex, size_t OverloadID>
    using type_at = std::conditional_t<get_index<SigIndex> == sizeof...(FwdIndices),
    nth_element_t<get_index<SigIndex>, SigTypes...>,
    overload_type_t<nth_element_t<SigIndex, SigTypes...>, test_bit(OverloadID, get_index<SigIndex>)>>;

    template<size_t OverloadID>
    using type = signature<type_at<SigIndices, OverloadID>...>;
};

template<typename Signature, typename SigIndices, typename FwdIndices, size_t OverloadID>
using make_overload_signature_t = make_overload_signature<Signature, SigIndices, FwdIndices>::template type<OverloadID>;

//================================================================================
//                          make_overload_set from
//================================================================================

template<typename Signature, typename OverloadIDs>
struct make_overload_set;

template<typename... SigTypes, size_t... OverloadIDs>
struct make_overload_set<signature<SigTypes...>, std::index_sequence<OverloadIDs...>>
{
    using type = overload_set<make_overload_signature_t<signature<SigTypes...>,std::index_sequence_for<SigTypes...>, value_indices_t<signature<SigTypes...>>, OverloadIDs>...>;
};

template<typename Signature>
using make_overload_set_t = make_overload_set<Signature, std::make_index_sequence<2 << (value_indices_t<Signature>::size() - 1)>>::type;



static_assert(std::is_same_v<make_overload_set_t<signature<int,bool>>,overload_set<signature<int&, bool&>, signature<int&&, bool&>, signature<int&, bool&&>, signature<int&&, bool&&>>>);
static_assert(std::is_same_v<make_overload_set_t<signature<int>>, overload_set<signature<int&>, signature<int&&>>>);
