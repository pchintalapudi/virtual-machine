#ifndef OOPS_CLASSES_FIELD_import_H
#define OOPS_CLASSES_FIELD_import_H

#include <variant>

#include "class.h"
#include "object.h"
#include "../methods/method.h"

namespace oops {
namespace classes {

struct field_import {
  class_import clazz;
  std::variant<classloading::raw_string, std::uint32_t> field_index;
};

struct static_method_import {
    class_import clazz;
    std::variant<classloading::raw_string, methods::method> static_method;
};

struct virtual_method_import {
    class_import clazz;
    std::variant<classloading::raw_string, std::uint32_t> virtual_method_index;
};

struct dynamic_method_import {
    classloading::raw_string dynamic_method_name;
};
}  // namespace classes
}  // namespace oops

#endif