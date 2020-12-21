#ifndef OOPS_CORE_ARITHMETIC_H
#define OOPS_CORE_ARITHMETIC_H

#include <cmath>
#include <cstdint>
#include <type_traits>

namespace oops {
namespace core {
namespace arithmetic {

#define integer_method(name, op)                                              \
  template <typename integer>                                                 \
  std::enable_if_t<                                                           \
      std::is_signed_v<integer> and std::is_integral_v<integer> and           \
          std::is_same_v<std::common_type_t<integer, std::int32_t>, integer>, \
      integer>                                                                \
      name##_safe(integer src1, integer src2) {                               \
    return static_cast<integer>(static_cast<std::make_unsigned_t<integer>>(   \
        src1) op static_cast<std::make_unsigned_t<integer>>(src2));           \
  }
#define numeric_method(name, op)                                              \
  template <typename numeric>                                                 \
  std::enable_if_t<                                                           \
      std::is_signed_v<numeric> and                                           \
          std::is_same_v<std::common_type_t<numeric, std::int32_t>, numeric>, \
      numeric>                                                                \
      name##_safe(numeric src1, numeric src2) {                               \
    if constexpr (std::is_floating_point_v<numeric>) {                        \
      return src1 op src2;                                                    \
    } else {                                                                  \
      return static_cast<numeric>(static_cast<std::make_unsigned_t<numeric>>( \
          src1) op static_cast<std::make_unsigned_t<numeric>>(src2));         \
    }                                                                         \
  }

#define decl

decl numeric_method(add, +);
decl numeric_method(sub, -);
decl numeric_method(mul, *);

template <typename numeric>
std::enable_if_t<
    std::is_signed_v<numeric> and
        std::is_same_v<std::common_type_t<numeric, std::int32_t>, numeric>,
    std::optional<numeric>>
div_safe(numeric src1, numeric src2) {
  if constexpr (std::is_integral_v<numeric>) {
    if (src2 == 0) return {};
  }
  return src1 / src2;
}

template <typename numeric>
std::enable_if_t<
    std::is_signed_v<numeric> and
        std::is_same_v<std::common_type_t<numeric, std::int32_t>, numeric>,
    std::optional<numeric>>
mod_safe(numeric src1, numeric src2) {
  if (src2 == 0) return {};
  if constexpr (std::is_integral_v<numeric>) {
    return src1 % src2;
  } else {
    return fmod(src1, src2);
  }
}

decl integer_method(and, &);
decl integer_method(or, |);
decl integer_method(xor, ^);
decl integer_method(sll, <<);
decl integer_method(srl, >>);
template <typename integer>
std::enable_if_t<
    std::is_signed_v<integer> and std::is_integral_v<integer> and
        std::is_same_v<std::common_type_t<integer, std::int32_t>, integer>,
    integer>
sra_safe(integer src1, integer src2) {
  return src1 >> src2;
}

#undef decl
#undef numeric_method
#undef integer_method
}  // namespace arithmetic
}  // namespace core
}  // namespace oops

#endif