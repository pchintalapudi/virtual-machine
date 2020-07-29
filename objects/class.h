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

            char *meta_region() const;
            char *vtable() const;
            char *ctable() const;
            char *static_memory() const;
            char *static_pointers() const;
            char *virtual_pointers() const;
            char *mtable() const;
            char *itable() const;
            char *stable() const;

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

            bool operator<(clazz c2)
            {
                return this->real < c2.real;
            }

            bool operator==(clazz c2)
            {
                return this->real == c2.real;
            }

            bool operator<=(clazz c2)
            {
                return this->real <= c2.real;
            }

            bool operator>(clazz c2)
            {
                return this->real > c2.real;
            }

            bool operator!=(clazz c2)
            {
                return this->real != c2.real;
            }

            bool operator>=(clazz c2)
            {
                return this->real >= c2.real;
            }

            std::uint64_t object_malloc_required_size() const;

            std::uint64_t static_variables_size() const;

            std::uint32_t virtual_handle_count() const;
            std::uint32_t static_handle_count() const;

            std::uint32_t total_method_count() const;
            std::uint32_t total_virtual_variable_count() const;
            std::uint32_t total_static_variable_count() const;
            std::uint32_t total_class_count() const;

            std::uint32_t self_virtual_method_count() const;
            std::uint32_t self_static_method_count() const;
            std::uint32_t self_virtual_variable_count() const;
            std::uint32_t self_static_variable_count() const;

            bool requires_finalization() const;

            std::optional<clazz> superclass() const;

            std::variant<clazz, utils::ostring> lookup_class_offset(std::uint32_t offset) const;

            void dynamic_loaded_class(std::uint32_t offset, clazz cls);

            std::variant<std::pair<utils::ostring, std::uint32_t>, std::uint32_t> get_external_method_offset(std::uint32_t internal_offset) const;

            std::variant<std::pair<utils::ostring, std::uint32_t>, std::uint32_t> get_external_virtual_field_offset(std::uint32_t internal_offset);
            
            std::variant<std::pair<utils::ostring, std::uint32_t>, char*> get_external_static_field(std::uint32_t internal_offset);

            method direct_method_lookup(std::uint32_t external_offset);

            void set_external_method_offset(std::uint32_t internal_offset, std::uint32_t external_offset);
            void set_external_virtual_field_offset(std::uint32_t internal_offset, std::uint32_t external_offset);
            void set_external_static_field(std::uint32_t internal_offset, char* external_field);

            std::optional<std::uint32_t> get_real_method_offset(utils::ostring name);
            std::optional<std::uint32_t> get_real_virtual_field_offset(utils::ostring name);
            std::optional<char*> get_real_static_field(utils::ostring name);

            template <typename primitive>
            std::enable_if_t<std::is_signed_v<primitive>, primitive> read(std::uint32_t offset) const
            {
                return utils::pun_read<primitive>(this->static_memory() + offset);
            }

            template <typename pointer>
            std::enable_if_t<std::is_base_of_v<base_object, pointer>, pointer> read(std::uint32_t offset) const
            {
                return utils::pun_read<char *>(this->static_memory() + offset);
            }

            template <typename primitive>
            std::enable_if_t<std::is_signed_v<primitive>, void> write(std::uint32_t offset, primitive value) const
            {
                return utils::pun_write(this->static_memory() + offset, value);
            }

            template <typename pointer>
            std::enable_if_t<std::is_base_of_v<base_object, pointer>, void> write(std::uint32_t offset, pointer obj) const
            {
                return utils::pun_write(this->static_memory() + offset, obj.unwrap());
            }

            operator bool() const
            {
                return this->real != nullptr;
            }
        };
    } // namespace objects
} // namespace oops

#endif /* OBJECTS_CLASS */
