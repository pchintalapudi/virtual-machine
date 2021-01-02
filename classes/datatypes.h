#ifndef OOPS_CLASSES_DATATYPES_H
#define OOPS_CLASSES_DATATYPES_H

#include <cstdint>
#include <type_traits>

#include "object.h"

namespace oops {
namespace classes {
enum class datatype { BYTE, SHORT, INT, FLOAT, LONG, DOUBLE, OBJECT };

static constexpr unsigned sizes[] = {sizeof(std::int8_t),  sizeof(std::int16_t),
                                     sizeof(std::int32_t), sizeof(float),
                                     sizeof(std::int64_t), sizeof(double),
                                     sizeof(void *)};

constexpr std::size_t datatype_size(datatype dt) {
  return sizes[static_cast<int>(dt)];
}

template <typename dt>
constexpr bool is_valid_datatype =
    std::is_same_v<dt, classes::base_object> or std::is_signed_v<dt>;
}  // namespace classes
}  // namespace oops
#endif /* CLASSES_DATATYPES */
