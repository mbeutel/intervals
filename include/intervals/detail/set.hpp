
#ifndef INCLUDED_INTERVALS_DETAIL_SET_HPP_
#define INCLUDED_INTERVALS_DETAIL_SET_HPP_


#include <array>
#include <type_traits>  // for is_same<>

#include <makeshift/metadata.hpp>  // for metadata::values()


namespace intervals {

namespace detail {


// The 4-valued extension of the 3-valued Kleene logic represented by `Set<bool>` has the following states:
//
//     0 : uninitialized
//     1 : false
//     2 : true
//     3 : both
//
// The uninitialized state is infectious, i.e. every function of an undefined value yields an undefined value.
// (TODO: do we want infection traps? do we want them to be configurable?)
//
// Truth tables:
//
//      ⋀  |     y           ⋁  |     y           ⊻  |     y          ¬   |         =  |     y           ≠  |     y     
//         | 0  1  2  3         | 0  1  2  3         | 0  1  2  3     ————+———         | 0  1  2  3         | 0  1  2  3
//     ————+————————————    ————+————————————    ————+————————————      0 | 0 —    ————+————————————    ————+———————————
//       0 | 0  0  0  0       0 | 0  0  0  0       0 | 0  0  0  0     x 1 | 2        0 | 0  0  0  0       0 | 0  0  0  0
//     x 1 | 0  1  1  1     x 1 | 0  1  2  3     x 1 | 0  1  2  3       2 | 1      x 1 | 0  2  1  3     x 1 | 0  1  2  3
//       2 | 0  1  2  3       2 | 0  2  2  2       2 | 0  2  1  3       3 | 3        2 | 0  1  2  3       2 | 0  2  1  3
//       3 | 0  1  3  3       3 | 0  3  2  3       3 | 0  3  3  3                    3 | 0  3  3  3       3 | 0  3  3  3
//
//      ≥  |     y           >  |     y
//         | 0  1  2  3         | 0  1  2  3
//     ————+————————————    ————+————————————
//       0 | 0  0  0  0       0 | 0  0  0  0
//     x 1 | 0  2  1  3     x 1 | 0  1  1  1
//       2 | 0  2  2  2       2 | 0  2  1  3
//       3 | 0  2  2  3       3 | 0  3  1  3
//
// The following constants are essentially 32-bit look-up tables for truth values, which we use for efficient implementation
// of 4-valued logic.
//
//                                  y: |     3     |     2     |     1     |     0     |
//                                  x: | 3  2  1  0| 3  2  1  0| 3  2  1  0| 3  2  1  0|
constexpr std::uint32_t lut_4vK_and = 0b11'11'01'00'11'10'01'00'01'01'01'00'00'00'00'00u;
constexpr std::uint32_t lut_4vK_or  = 0b11'10'11'00'10'10'10'00'11'10'01'00'00'00'00'00u;
constexpr std::uint32_t lut_4vK_xor = 0b11'11'11'00'11'01'10'00'11'10'01'00'00'00'00'00u;
constexpr std::uint32_t lut_4vK_eq  = 0b11'11'11'00'11'10'01'00'11'01'10'00'00'00'00'00u;
constexpr std::uint32_t lut_4vK_neq = 0b11'11'11'00'11'01'10'00'11'10'01'00'00'00'00'00u;  // `== lut_4vK_xor`
constexpr std::uint32_t lut_4vK_geq = 0b11'10'11'00'10'10'01'00'10'10'10'00'00'00'00'00u;
constexpr std::uint32_t lut_4vK_gt  = 0b11'11'01'00'01'01'01'00'11'10'01'00'00'00'00'00u;
//                                  x: | 3  2  1  0|
constexpr std::uint8_t  lut_4vK_not = 0b11'01'10'00u;


template <typename T0, typename... Ts>
struct enforce_same
{
    static_assert((std::is_same_v<T0, Ts> && ...));
    using type = T0;
};


}  // namespace detail

} // namespace intervals


#endif // INCLUDED_INTERVALS_DETAIL_SET_HPP_
