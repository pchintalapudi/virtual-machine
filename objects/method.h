#ifndef OBJECTS_METHOD
#define OBJECTS_METHOD

#include <cstdint>

#include "field.h"
#include "../utils/utils.h"

namespace oops {
    namespace objects {
        class clazz;

        class handle_map {
            private:
            char* real;
            public:
            handle_map(char* real) : real(real) {}

            std::uint32_t operator[](std::uint16_t index) const {
                return utils::pun_read<std::uint32_t>(this->real + static_cast<std::uint32_t>(index) * sizeof(std::uint32_t));
            }

            std::uint16_t size() const {
                return utils::pun_read<std::uint32_t>(this->real - sizeof(std::uint32_t));
            }
        };

        class method {
            private:
            char* real;

            std::uint16_t bytecode_length() const;
            public:
            method(char* real) : real(real) {}

            char* unwrap() const {
                return this->real;
            }

            enum class type {
                REGULAR, INTERFACE, NATIVE, JIT
            };

            friend bool operator<(method m1, method m2) {
                return m1.real < m2.real;
            }

            friend bool operator==(method m1, method m2) {
                return m1.real == m2.real;
            }

            type get_type() const;

            field::type get_return_type() const;

            std::uint16_t arg_count() const;

            std::uint64_t arg_offset_pack(std::uint16_t arg_index) const;

            clazz enclosing_class() const;

            handle_map get_stack_handle_map() const;

            std::uint16_t stack_frame_size() const;

            char* bytecode_begin() const;
        };
    }
}
#endif /* OBJECTS_METHOD */
