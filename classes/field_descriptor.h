#ifndef OOPS_CLASSES_FIELD_DESCRIPTOR_H
#define OOPS_CLASSES_FIELD_DESCRIPTOR_H

#include "object.h"
#include "class.h"

#include <variant>

namespace oops {
    namespace classes {

        struct field_descriptor {
            class_descriptor clazz;
            std::variant<string, std::uint32_t> field_index;
        };
    }
}

#endif