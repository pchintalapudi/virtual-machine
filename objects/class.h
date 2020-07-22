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

            char *meta_start() const
            {
                return this->real;
            }
            char *resolved_method_start() const;
            char *resolved_class_start() const;
            char *resolved_static_variable_start() const;
            char *resolved_virtual_variable_start() const;
            char *static_memory_start() const;
            char *symbol_table() const;

            std::uint32_t method_count() const;
            std::uint32_t class_count() const;
            std::uint32_t static_variable_count() const;
            std::uint32_t virtual_variable_count() const;
            std::uint32_t symbol_count() const;

            std::optional<std::uint32_t> lookup_symbol(utils::ostring name) const;

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

            std::optional<std::uint32_t> lookup_interface_method(utils::ostring name) const
            {
                return this->lookup_symbol(name);
            }
            std::optional<std::uint32_t> lookup_interface_field(utils::ostring name) const
            {
                return this->lookup_symbol(name);
            }
            std::optional<std::uint32_t> lookup_static_interface_field(utils::ostring name) const
            {
                return this->lookup_symbol(name);
            }

            std::variant<clazz, utils::ostring> lookup_class_offset(std::uint32_t offset) const;

            std::variant<method, std::pair<std::uint32_t, utils::ostring>> lookup_method_offset(std::uint32_t offset) const;

            std::pair<std::uint32_t, std::variant<std::uint32_t, utils::ostring>> lookup_static_field_offset(std::uint32_t offset) const;

            std::variant<std::uint32_t, std::pair<std::uint32_t, utils::ostring>> lookup_virtual_field_offset(std::uint32_t offset24) const;

            void dynamic_loaded_class(std::uint32_t offset, clazz cls);

            void dynamic_loaded_method(std::uint32_t offset, method method);

            void dynamic_loaded_static_field(std::uint32_t offset31, std::uint32_t class_index, std::uint32_t field31);

            void dynamic_loaded_virtual_field(std::uint32_t offset, std::uint32_t field24);

            template <typename primitive>
            std::enable_if_t<std::is_signed_v<primitive>, primitive> read(std::uint32_t offset) const
            {
                return utils::pun_read<primitive>(this->static_memory_start() + offset);
            }

            template <typename pointer>
            std::enable_if_t<std::is_base_of_v<base_object, pointer>, pointer> read(std::uint32_t offset) const
            {
                return base_object(utils::pun_read<char *>(this->static_memory_start() + offset));
            }

            template <typename primitive>
            std::enable_if_t<std::is_signed_v<primitive>, void> write(std::uint32_t offset, primitive value) const
            {
                utils::pun_write(this->static_memory_start() + offset, value);
            }

            template <typename pointer>
            std::enable_if_t<std::is_base_of_v<base_object, pointer>, void> write(std::uint32_t offset, pointer obj) const
            {
                utils::pun_write(this->static_memory_start() + offset, obj.unwrap());
            }

            operator bool() const
            {
                return this->real != nullptr;
            }
        };
    } // namespace objects
} // namespace oops

#endif /* OBJECTS_CLASS */
