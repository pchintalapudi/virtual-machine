#ifndef OBJECTS_CLASS
#define OBJECTS_CLASS

#include <cstdint>
#include <type_traits>

#include "method.h"

namespace oops {
    namespace objects {

        class method;
        class base_object;

        class clazz {
            private:
            char* real;
            public:

            enum class type {
                CLASS, INTERFACE, ENUM
            };
            clazz(char* real) : real(real) {}

            char* unwrap() const {
                return this->real;
            }

            friend bool operator<(clazz c1, clazz c2) {
                return c1.real < c2.real;
            }

            friend bool operator==(clazz c1, clazz c2) {
                return c1.real == c2.real;
            }

            std::size_t object_malloc_required_size() const;

            char* handle_map() const;

            std::uint16_t handle_map_length() const;
            
            method lookup_method(std::uint32_t method_offset) const;

            clazz lookup_class(std::uint32_t class_offset) const;

            type get_class_type() const;

            std::optional<method> lookup_interface_method_direct(method imethod);

            template<typename primitive>
            std::enable_if_t<std::is_signed_v<primitive>, primitive> read(std::uint16_t offset) const;

            template<typename pointer>
            std::enable_if_t<std::is_base_of_v<base_object, pointer>, pointer> read(std::uint16_t offset) const;

            template<typename primitive>
            std::enable_if_t<std::is_signed_v<primitive>, void> write(std::uint16_t offset, primitive value);

            template<typename pointer>
            std::enable_if_t<std::is_base_of_v<base_object, pointer>, void> write(std::uint16_t offset, pointer obj);
        };
    }
}

#endif /* OBJECTS_CLASS */
