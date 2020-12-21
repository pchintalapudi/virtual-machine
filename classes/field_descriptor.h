#ifndef OOPS_CLASSES_FIELD_DESCRIPTOR_H
#define OOPS_CLASSES_FIELD_DESCRIPTOR_H

#include <variant>

#include "class.h"
#include "object.h"
#include "../methods/method.h"

namespace oops {
namespace classes {

struct field_descriptor {
  class_descriptor clazz;
  std::variant<string, std::uint32_t> field_index;
};

struct static_method_descriptor {
    class_descriptor clazz;
    std::variant<string, methods::method> static_method;
};

struct virtual_method_descriptor {
    class_descriptor clazz;
    std::variant<string, std::uint32_t> virtual_method_index;
};

struct dynamic_method_descriptor {
    string dynamic_method_name;
};
}  // namespace classes
}  // namespace oops

#endif