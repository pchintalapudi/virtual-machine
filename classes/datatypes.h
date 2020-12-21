#ifndef OOPS_CLASSES_DATATYPES_H
#define OOPS_CLASSES_DATATYPES_H

#include <cstdint>
#include <type_traits>

#include "object.h"

namespace oops {
namespace classes {
enum class datatype { BYTE, SHORT, INT, FLOAT, LONG, DOUBLE, OBJECT };

unsigned datatype_size(datatype dt);

template <typename dt>
constexpr bool is_valid_datatype =
    std::is_same_v<dt, classes::base_object> or std::is_signed_v<dt>;
}  // namespace classes
}  // namespace oops
#endif /* CLASSES_DATATYPES */
