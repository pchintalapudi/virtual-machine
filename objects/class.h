#ifndef OBJECTS_CLASS
#define OBJECTS_CLASS

#include <cstdint>
#include <type_traits>

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
            char *static_variables_start() const;
            char *method_symbol_table() const;
            char *class_symbol_table() const;

            std::uint32_t method_count() const;
            std::uint32_t class_count() const;

            method resolve_symbolic_method(char *name) const;
            clazz resolve_symbolic_class(char *name) const;

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

            method lookup_method(std::uint32_t method_offset) const;

            clazz lookup_class(std::uint32_t class_offset) const;

            std::optional<std::uint32_t> lookup_interface_method(utils::ostring name);

            template <typename primitive>
            std::enable_if_t<std::is_signed_v<primitive>, primitive> read(std::uint16_t offset) const
            {
                std::uint32_t real_offset = offset;
                real_offset *= sizeof(primitive);
                return utils::pun_read<primitive>(this->static_variables_start() + real_offset);
            }

            template <typename pointer>
            std::enable_if_t<std::is_base_of_v<base_object, pointer>, pointer> read(std::uint16_t offset) const
            {
                std::uint32_t real_offset = offset;
                real_offset *= sizeof(char *);
                return base_object(utils::pun_read<char *>(this->static_variables_start() + real_offset));
            }

            template <typename primitive>
            std::enable_if_t<std::is_signed_v<primitive>, void> write(std::uint16_t offset, primitive value) const
            {
                std::uint32_t real_offset = offset;
                real_offset *= sizeof(primitive);
                utils::pun_write(this->static_variables_start() + real_offset, value);
            }

            template <typename pointer>
            std::enable_if_t<std::is_base_of_v<base_object, pointer>, void> write(std::uint16_t offset, pointer obj) const
            {
                std::uint32_t real_offset = offset;
                real_offset *= sizeof(char *);
                utils::pun_write(this->static_variables_start() + real_offset, obj.unwrap());
            }

            operator bool() const
            {
                return this->real != nullptr;
            }
        };
    } // namespace objects
} // namespace oops

#endif /* OBJECTS_CLASS */
