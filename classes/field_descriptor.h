#ifndef OOPS_CLASSES_FIELD_DESCRIPTOR_H
#define OOPS_CLASSES_FIELD_DESCRIPTOR_H

#include <variant>

#include "class.h"
#include "object.h"

namespace oops {
namespace classes {

struct field_descriptor {
  class_descriptor clazz;
  std::variant<string, std::uint32_t> field_index;
};
}  // namespace classes
}  // namespace oops

#endif