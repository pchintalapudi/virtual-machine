#ifndef OBJECTS_CLASS
#define OBJECTS_CLASS

#include <cstdint>
#include <type_traits>
#include <variant>

#include "method.h"

namespace oops
{
    namespace objects
    {

        class method;
        class base_object;

        class clazz
        {
        private:
            char *real;

        public:
            enum class type
            {
                CLASS,
                INTERFACE,
                ENUM
            };
            clazz(char *real) : real(real) {}

            char *unwrap() const
            {
                return this->real;
            }

            friend bool operator<(clazz c1, clazz c2)
            {
                return c1.real < c2.real;
            }

            friend bool operator==(clazz c1, clazz c2)
            {
                return c1.real == c2.real;
            }

            std::uint64_t object_malloc_required_size() const;

            std::uint64_t static_variables_size() const;

            std::uint32_t static_handle_count() const;
            std::uint32_t virtual_handle_count() const;

            bool requires_finalization() const;

            std::optional<std::uint32_t> lookup_interface_method(utils::ostring name) const;
            std::optional<std::uint32_t> lookup_interface_field(utils::ostring name) const;
            std::optional<char *> lookup_static_interface_field(utils::ostring name) const;

            std::variant<clazz, utils::ostring> lookup_class_offset(std::uint32_t offset) const;

            std::variant<std::pair<utils::ostring, std::uint32_t>, method> get_method_by_offset(std::uint32_t offset) const;

            std::variant<std::pair<utils::ostring, std::uint32_t>, char *> get_static_by_offset(std::uint32_t offset) const;

            std::variant<std::pair<utils::ostring, std::uint32_t>, std::uint32_t> get_virtual_by_offset(std::uint32_t offset) const;

            void dynamic_loaded_class(std::uint32_t offset, clazz cls);

            void dynamic_loaded_method(std::uint32_t offset, method method);

            void dynamic_loaded_static_field(std::uint32_t offset31, char *field);

            void dynamic_loaded_virtual_field(std::uint32_t offset, std::uint32_t field24);

            template <typename primitive>
            std::enable_if_t<std::is_signed_v<primitive>, primitive> read(std::uint32_t offset) const;

            template <typename pointer>
            std::enable_if_t<std::is_base_of_v<base_object, pointer>, pointer> read(std::uint32_t offset) const;

            template <typename primitive>
            std::enable_if_t<std::is_signed_v<primitive>, void> write(std::uint32_t offset, primitive value) const;

            template <typename pointer>
            std::enable_if_t<std::is_base_of_v<base_object, pointer>, void> write(std::uint32_t offset, pointer obj) const;

            operator bool() const
            {
                return this->real != nullptr;
            }
        };
    } // namespace objects
} // namespace oops

#endif /* OBJECTS_CLASS */
