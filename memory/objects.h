#ifndef MEMORY_OBJECTS_H
#define MEMORY_OBJECTS_H

#include <algorithm>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cstdlib>
#include <optional>
#include <type_traits>

namespace oops
{
    typedef std::size_t field_size_t;

    namespace objects
    {
        enum class primitives
        {
            OBJECT,
            CLASS,
            INTERFAZE
        };

        template <typename enum_type, enum_type ptr_type>
        struct inconvertible
        {
            char *pointer;
        };

        typedef inconvertible<primitives, primitives::OBJECT> object;
        typedef object method;
        class field
        {
        private:
            std::uint32_t name_length;
            std::uint32_t offset;
            const char *name; //Must be 8-byte aligned and have allocated length as a multiple of 2 (including null terminator).

        public:
            enum class field_type
            {
                CHAR,
                SHORT,
                INT,
                FLOAT,
                LONG,
                DOUBLE,
                OBJECT,
                METHOD
            };

            template <field_type tp>
            struct to_type
            {
            };

            field(const char *name, std::uint32_t name_length, std::uint32_t offset, field_type type, bool immutable) : name_length((name_length + 1) & ~1ull), offset(offset), name(name)
            {
                std::uintptr_t nameptr;
                std::memcpy(&nameptr, &this->name, sizeof(char *));
                nameptr |= static_cast<uintptr_t>(type);
                std::memcpy(&this->name, &nameptr, sizeof(char *));
                this->name_length |= immutable;
            }

            field_type get_type() const
            {
                std::uintptr_t name_int;
                std::memcpy(&name_int, &this->name, sizeof(const char *));
                return static_cast<field_type>(name_int & 0b111);
            }

            bool is_final() const
            {
                return name_length & 1;
            }

            const char *get_name() const
            {
                std::uintptr_t name_int;
                std::memcpy(&name_int, &this->name, sizeof(const char *));
                name_int &= ~0b111ull;
                const char *out;
                std::memcpy(&out, &name_int, sizeof(const char *));
                return out;
            }

            std::uint32_t get_name_length() const
            {
                return this->get_name()[this->name_length - 1] == 0 ? this->name_length - 1 : this->name_length;
            }
        };

#pragma region type specializations
        template <>
        struct field::to_type<field::field_type::CHAR>
        {
            typedef std::int8_t type;
        };

        template <>
        struct field::to_type<field::field_type::SHORT>
        {
            typedef std::int16_t type;
        };

        template <>
        struct field::to_type<field::field_type::INT>
        {
            typedef std::int32_t type;
        };

        template <>
        struct field::to_type<field::field_type::FLOAT>
        {
            typedef float type;
        };

        template <>
        struct field::to_type<field::field_type::LONG>
        {
            typedef std::int64_t type;
        };

        template <>
        struct field::to_type<field::field_type::DOUBLE>
        {
            typedef double type;
        };

        template <>
        struct field::to_type<field::field_type::OBJECT>
        {
            typedef char *type;
        };

        template <>
        struct field::to_type<field::field_type::METHOD>
        {
            typedef char *type;
        };
#pragma endregion

        struct class_def
        {
            std::uint32_t class_size;
            std::uint32_t method_count;
            std::uint32_t object_size;
            std::uint32_t handle_count;
            method **methods;
            object class_object;
            std::uint32_t field_count;
            std::uint32_t name_length;
            field *fields;
            char *name;
        };

        struct interface_def
        {
            std::uint32_t name_length;
            std::uint32_t field_length;
            char *name;
            field *fields;
        };

        namespace clazz
        {
            /*
            struct class {
                std::uint32_t class_size;
                std::uint32_t method_count;
                std::uint32_t object_size;
                std::uint32_t handle_count;
                method* methods[method_count];
                object* class_object;
                std::uint32_t field_count;
                std::uint32_t name_length;
                field* fields;
                char* name;
            };
            */

            typedef inconvertible<primitives, primitives::CLASS> _class;

            inline method *lookup_method(_class clazz, std::uint32_t offset)
            {
                method *target;
                std::memcpy(&target, clazz.pointer + 4 * sizeof(std::uint32_t) + offset * sizeof(method *), sizeof(method *));
                return target;
            }

            const field *lookup_field(_class clazz, const char *name, std::uint32_t name_length);

            void construct_class(char *aligned_8_byte_location, class_def &definition);

            void destruct_class(char *aligned_8_byte_location);

        } // namespace clazz

        /*
        struct object {
            std::uint32_t size; // has 3 low bits empty
            std::uint32_t handle_count;
            char* class_def; // also has 3 low bits empty
            char* handles[handle_count]; // each handle has 3 low bits empty
            char primitives[size - handle_count * sizeof(char*)];
        };
        */
        //Possibly controversially, I'm going to reuse the object struct to represent methods as well.
        //Simply replacing the class definition with a pointer to either a native method or a bytecode
        //executable will allow the method to maintain captures, which is a nifty little feature :)

        inline method *lookup_method(object obj, std::uint32_t offset)
        {
            return clazz::lookup_method({obj.pointer + sizeof(std::uint32_t) * 2}, offset);
        }

        template <typename p_t>
        std::enable_if_t<std::is_signed<p_t>::value, p_t> read_field(object obj, std::uint32_t offset)
        {
            p_t value;
            std::memcpy(&value, obj.pointer + offset, sizeof(p_t));
            return value;
        }

        template <typename p_t>
        std::enable_if_t<std::is_same<object, p_t>::value, p_t> read_field(object obj, std::uint32_t offset)
        {
            char *ptr;
            std::memcpy(&ptr, obj.pointer + offset, sizeof(char *));
            return {ptr};
        }

        template <typename p_t>
        std::enable_if_t<std::is_signed<p_t>::value, void> write_field(object obj, std::uint32_t offset, p_t value)
        {
            std::memcpy(obj.pointer + offset, &value, sizeof(p_t));
        }

        template <typename p_t>
        std::enable_if_t<std::is_same<object, p_t>::value, void> write_field(object obj, std::uint32_t offset, object value)
        {
            std::memcpy(obj.pointer + offset, &value.pointer, sizeof(value.pointer));
        }

        void construct_object(char *aligned_8_byte_location, clazz::_class class_ptr);

        namespace interfaze
        {
            /*
            struct interface {
                std::uint32_t name_length;
                std::uint32_t field_count;
                char *name;
                field fields[field_count];
            };
            */
            typedef inconvertible<primitives, primitives::INTERFAZE> _interface;
            inline const field *lookup_interface_field(object obj, _interface iface, std::uint32_t offset)
            {
                static_assert(std::is_standard_layout<field>::value, "Field is not standard layout...");
                clazz::_class class_pointer{};
                std::memcpy(&class_pointer.pointer, obj.pointer + sizeof(std::uint32_t) * 2, sizeof(char *));
                char *name;
                std::uint32_t name_length;
                std::memcpy(&name, iface.pointer + sizeof(std::uint32_t) * 2 + sizeof(char *) + sizeof(field) * offset + sizeof(std::uint32_t) * 2, sizeof(char *));
                std::memcpy(&name_length, iface.pointer + sizeof(std::uint32_t) * 2 + sizeof(char *) + sizeof(field) * offset, sizeof(std::uint32_t));
                return clazz::lookup_field(class_pointer, name, name_length);
            }

            inline void construct_interface(char *aligned_8_byte_location, interface_def &iface_def)
            {
                std::memcpy(aligned_8_byte_location, &iface_def.name_length, sizeof(std::uint32_t));
                std::memcpy(aligned_8_byte_location += sizeof(std::uint32_t), &iface_def.field_length, sizeof(std::uint32_t));
                std::memcpy(aligned_8_byte_location += sizeof(std::uint32_t), &iface_def.name, sizeof(char *));
                std::memcpy(aligned_8_byte_location += sizeof(char *), iface_def.fields, sizeof(field) * iface_def.field_length);
            }
        } // namespace interfaze
    }     // namespace objects
} // namespace oops

#endif