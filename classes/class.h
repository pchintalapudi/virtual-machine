#ifndef OOPS_CLASSES_CLASS_H
#define OOPS_CLASSES_CLASS_H

#include <optional>
#include "../memory/byteblock.h"
#include "object.h"

namespace oops {
    namespace methods {
        class method;
    }
    namespace classes {
        
        class string;
        struct field_descriptor;
        enum class datatype;

        class clazz {
            private:
            memory::byteblock<> class_data;
            public:

            clazz(void* cls);

            void* get_raw() const {
                return class_data.get_raw();
            }

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

            field_descriptor get_field_descriptor(std::uint32_t index);

            std::optional<std::uint32_t> reflect_object_field_index(string str, datatype expected_type);
            std::optional<std::uint32_t> reflect_class_field_index(string str, datatype expected_type);
            std::optional<std::uint32_t> reflect_method_index(string str);

            std::uintptr_t get_static_memory_idx(std::uintptr_t idx) {
                return idx + 56; // Size of header region, defined in /spec/ClassStructure.md
            }

            template<typename out_t>
            std::optional<out_t> checked_read_static_memory(std::uint32_t idx) {
                if constexpr (std::is_same_v<out_t, base_object>) {
                    void* raw = this->class_data.read<void*>(get_static_memory_idx(idx));
                    return base_object(raw);
                } else {
                    return this->class_data.read<out_t>(get_static_memory_idx(idx));
                }
            }

            template<typename in_t>
            bool checked_write_static_memory(std::uint32_t idx, in_t value) {
                if constexpr (std::is_same_v<in_t, base_object>) {
                    this->class_data.write(get_static_memory_idx(idx), value.get_raw());
                    return true;
                } else {
                    this->class_data.write(get_static_memory_idx(idx), value);
                    return true;
                }
            }
        };
    }
}

#endif