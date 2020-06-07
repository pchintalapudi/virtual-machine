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

    namespace objects
    {
        class clazz;
        class object;
    } // namespace objects

    namespace memory
    {
        class memory_manager;
    } // namespace memory

    namespace objects
    {
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

            field(const char *name, std::uint32_t name_length, std::uint32_t offset, field_type type, bool is_static) : name_length((name_length + 1) & ~1ull), offset(offset), name(name)
            {
                std::uintptr_t nameptr;
                std::memcpy(&nameptr, &this->name, sizeof(char *));
                nameptr |= static_cast<uintptr_t>(type);
                std::memcpy(&this->name, &nameptr, sizeof(char *));
                this->name_length |= is_static;
            }

            field_type get_type() const
            {
                std::uintptr_t name_int;
                std::memcpy(&name_int, &this->name, sizeof(const char *));
                return static_cast<field_type>(name_int & 0b111);
            }

            bool is_static() const
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

        class aliased
        {
        protected:
            char *real;

            void construct(std::uint32_t size, std::uint32_t handle_count)
            {
                size <<= 1;
                handle_count <<= 1;
                std::memcpy(this->real, &size, sizeof(std::uint32_t));
                std::memcpy(this->real + sizeof(std::uint32_t), &handle_count, sizeof(std::uint32_t));
            }

        private:
            void mark();

            bool marked();

            void reset();
            template <typename o_type>
            friend class memory_manager;

        public:
            explicit aliased(char *real) : real(real)
            {
            }

            std::uint32_t size() const
            {
                std::uint32_t sz;
                std::memcpy(&sz, this->real, sizeof(std::uint32_t));
                return sz >> 1;
            }

            std::uint32_t handle_count() const
            {
                std::uint32_t hc;
                std::memcpy(&hc, this->real + sizeof(std::uint32_t), sizeof(std::uint32_t));
                return hc >> 1;
            }

            bool operator==(const aliased &other) const
            {
                return this->real == other.real;
            }

            bool operator!=(const aliased &other) const
            {
                return this->real != other.real;
            }

            bool operator<(const aliased &other) const
            {
                return this->real < other.real;
            }

            bool operator<=(const aliased &other) const
            {
                return this->real <= other.real;
            }

            bool operator>(const aliased &other) const
            {
                return this->real > other.real;
            }

            bool operator>=(const aliased &other) const
            {
                return this->real >= other.real;
            }
        };
        struct object_header
        {
            std::uint32_t size;
            std::uint32_t handle_count;
        };

        class object;

        class clazz : public aliased
        {
        public:
            explicit clazz(char *real) : aliased(real) {}

            constexpr static std::uint64_t offset_size = sizeof(std::uint32_t) * 6 + sizeof(field *);

            std::uint32_t method_count()
            {
                std::uint32_t mc;
                std::memcpy(&mc, this->real + sizeof(std::uint32_t) * 2, sizeof(std::uint32_t));
                return mc;
            }

            std::uint32_t field_count()
            {
                std::uint32_t fc;
                std::memcpy(&fc, this->real + sizeof(std::uint32_t) * 3, sizeof(std::uint32_t));
                return fc;
            }

            object_header object_info()
            {
                object_header ret;
                std::memcpy(&ret.size, this->real + sizeof(std::uint32_t) * 4, sizeof(std::uint32_t));
                std::memcpy(&ret.handle_count, this->real + sizeof(std::uint32_t) * 5, sizeof(std::uint32_t));
                return ret;
            }

            field *get_fields()
            {
                field *fields;
                std::memcpy(&fields, this->real + sizeof(std::uint32_t) * 6, sizeof(field *));
                return fields;
            }

            template <typename pointer>
            std::enable_if_t<std::is_same<pointer, object>::value, pointer> read_static_field(std::uint32_t offset)
            {
                char *clz;
                std::memcpy(&clz, this->real + offset_size + offset, sizeof(char *));
                return object(clz);
            }

            template <typename primitive>
            std::enable_if_t<std::is_signed<primitive>::value, primitive> read_static_field(std::uint32_t offset)
            {
                primitive p;
                std::memcpy(&p, this->real + offset_size + offset, sizeof(primitive));
                return p;
            }

            template <typename pointer>
            std::enable_if_t<std::is_same<pointer, object>::value, void> write_static_field(std::uint32_t offset, pointer p)
            {
                std::memcpy(this->real + offset_size + offset, &p.real, sizeof(char *));
            }

            template <typename primitive>
            std::enable_if_t<std::is_signed<primitive>::value, void> write_static_field(std::uint32_t offset, primitive p)
            {
                std::memcpy(this->real + offset_size + offset, &p, sizeof(primitive));
            }

            char *unwrap()
            {
                return this->real;
            }
        };

        class object : public aliased
        {
        public:
            explicit object(char *real) : aliased(real) {}

            constexpr static std::uint64_t offset_size = sizeof(std::uint32_t) * 2 + sizeof(char *);

            void construct(clazz clz);

            void destruct();

            clazz get_class()
            {
                std::uintptr_t clz;
                #pragma GCC diagnostic ignored "-Wsizeof-pointer-memaccess"
                std::memcpy(&clz, this->real + 2 * sizeof(std::uint32_t), sizeof(char *));
                #pragma GCC diagnostic pop
                clz &= ~0ull << 3;
                char *cls;
                std::memcpy(&cls, &clz, sizeof(char *));
                return clazz(cls);
            }

            template <typename pointer>
            std::enable_if_t<std::is_same<pointer, object>::value, pointer> read_instance_field(std::uint32_t offset)
            {
                char *clz;
                #pragma GCC diagnostic ignored "-Wsizeof-pointer-memaccess"
                std::memcpy(&clz, this->real + offset_size + offset, sizeof(char *));
                #pragma GCC diagnostic pop
                return object(clz);
            }

            template <typename primitive>
            std::enable_if_t<std::is_signed<primitive>::value, primitive> read_instance_field(std::uint32_t offset)
            {
                primitive p;
                std::memcpy(&p, this->real + offset_size + offset, sizeof(primitive));
                return p;
            }

            template <typename pointer>
            std::enable_if_t<std::is_same<pointer, object>::value, void> write_instance_field(std::uint32_t offset, pointer p)
            {
                std::memcpy(this->real + offset_size + offset, &p.real, sizeof(char *));
            }

            template <typename primitive>
            std::enable_if_t<std::is_signed<primitive>::value, void> write_instance_field(std::uint32_t offset, primitive p)
            {
                std::memcpy(this->real + offset_size + offset, &p, sizeof(primitive));
            }

            char *unwrap()
            {
                return this->real;
            }
        };

        static_assert(std::is_standard_layout<object>::value);
        static_assert(std::is_standard_layout<clazz>::value);
        static_assert(std::is_standard_layout<aliased>::value);

        static_assert(std::is_trivially_copyable<object>::value);
        static_assert(std::is_trivially_copyable<clazz>::value);
        static_assert(std::is_trivially_copyable<aliased>::value);
    } // namespace objects
} // namespace oops

#endif