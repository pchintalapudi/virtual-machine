#ifndef OOPS_CLASSES_CLASS_H
#define OOPS_CLASSES_CLASS_H

#include <optional>
#include "../memory/byteblock.h"

namespace oops {
    namespace methods {
        class method;
    }
    namespace classes {

        class clazz {
            private:
            memory::byteblock<> class_data;
            public:

            clazz(void* cls);

            std::optional<clazz> superclass() const;

            std::uint32_t object_size() const;

            std::uint16_t instance_pointer_count() const;
            std::uint16_t instance_double_count() const;
            std::uint16_t instance_long_count() const;
            std::uint16_t instance_float_count() const;
            std::uint16_t instance_int_count() const;
            std::uint16_t instance_short_count() const;
            std::uint16_t instance_byte_count() const;

            std::uint16_t static_pointer_count() const;
            std::uint16_t static_double_count() const;
            std::uint16_t static_long_count() const;
            std::uint16_t static_float_count() const;
            std::uint16_t static_int_count() const;
            std::uint16_t static_short_count() const;
            std::uint16_t static_byte_count() const;

            std::uint32_t virtual_method_count() const;

            std::uint32_t total_reflection_count() const;

            std::uint32_t total_bytecode_size() const;

            std::uint32_t total_string_pool_size() const;
        };
    }
}

#endif