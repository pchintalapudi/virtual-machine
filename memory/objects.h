#ifndef MEMORY_OBJECTS_H
#define MEMORY_OBJECTS_H

#include <algorithm>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cstdlib>
#include <optional>
#include <type_traits>

#include "../punning/puns.h"

namespace oops
{

    template <typename std_layout>
    struct maybe
    {
        bool present;
        std::enable_if_t<std::is_standard_layout<std_layout>::value, std_layout> value;

        explicit operator bool()
        {
            return this->present;
        }
    };
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
            enum class field_type : std::uint8_t
            {
                CHAR,
                SHORT,
                INT,
                FLOAT,
                LONG,
                DOUBLE,
                OBJECT,
                METHOD,
                VD = METHOD
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

        template <typename size_type, typename self>
        class aliased
        {
        protected:
            char *real;

        private:
            template <typename integer>
            using next_size = std::conditional_t<sizeof(integer) == 1, std::uint16_t, std::conditional_t<sizeof(integer) == 2, std::uint32_t, std::conditional_t<sizeof(integer) == 4, std::uint64_t, __uint128_t>>>;

            static_assert(std::is_same<next_size<std::uint8_t>, std::uint16_t>::value);
            static_assert(std::is_same<next_size<std::uint16_t>, std::uint32_t>::value);
            static_assert(std::is_same<next_size<std::uint32_t>, std::uint64_t>::value);
            static_assert(std::is_same<next_size<std::uint64_t>, __uint128_t>::value);

        public:
            explicit aliased(char *real) : real(real)
            {
            }

            operator bool() const
            {
                return this->real != nullptr;
            }

            char *unwrap() const
            {
                return this->real;
            }

            next_size<size_type> size() const
            {
                PUN(size_type, sz, this->real);
                return static_cast<next_size<size_type>>(sz) * 8 + static_cast<const self *>(this)->static_offset();
            }

            size_type handle_count() const
            {
                PUN(size_type, hc, this->real + sizeof(size_type));
                return hc;
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
        class clazz;

        class instruction
        {
        private:
            char *real;

        public:
            char *unwrap() const
            {
                return real;
            }

            operator bool() const
            {
                return this->real != nullptr;
            }

            instruction(char *real) : real(real) {}

            instruction &operator++()
            {
                this->real += sizeof(std::uint64_t);
                return *this;
            }

            instruction operator++(int)
            {
                instruction ret(this->real);
                this->real += sizeof(std::uint64_t);
                return ret;
            }

            instruction &operator+=(std::int32_t offset)
            {
                this->real += offset * sizeof(std::uint64_t);
                return *this;
            }

            instruction &operator--()
            {
                this->real -= sizeof(std::uint64_t);
                return *this;
            }

            instruction operator--(int)
            {
                instruction ret(this->real);
                this->real -= sizeof(std::uint64_t);
                return ret;
            }

            instruction &operator-=(std::int32_t offset)
            {
                this->real -= offset * sizeof(std::uint64_t);
                return *this;
            }

            std::uint64_t operator*()
            {
                PUN(std::uint64_t, instr, this->real);
                return instr;
            }

            bool operator==(const instruction &other) const
            {
                return this->real == other.real;
            }

            bool operator!=(const instruction &other) const
            {
                return this->real != other.real;
            }

            bool operator<(const instruction &other) const
            {
                return this->real < other.real;
            }

            bool operator<=(const instruction &other) const
            {
                return this->real <= other.real;
            }

            bool operator>(const instruction &other) const
            {
                return this->real > other.real;
            }

            bool operator>=(const instruction &other) const
            {
                return this->real >= other.real;
            }
        };

        class method : public aliased<std::uint16_t, method>
        {
        private:
            friend class aliased;

            std::uint32_t static_offset() const
            {
                return this->bytecode_begin().unwrap() - this->real;
            }

        public:
            enum struct method_type
            {
                VIRTUAL,
                STATIC,
                NATIVE
            };

            method(char *real) : aliased(real) {}

            std::uint32_t stack_size()
            {
                PUN(std::uint16_t, ss, this->real + sizeof(std::uint16_t) * 2);
                return static_cast<std::uint32_t>(ss) << 3;
            }

            std::uint16_t arg_count() const
            {
                PUN(std::uint16_t, ac, this->real + sizeof(std::uint16_t) * 3);
                return ac;
            }

            clazz owner();

            instruction bytecode_begin() const
            {
                return instruction(this->real + sizeof(std::uint16_t) * 4 + sizeof(char *) + (std::min((this->arg_count() + 2u) / 3u, 1u) * sizeof(std::uint64_t)));
            }

            objects::field::field_type rval_type()
            {
                PUN(std::uint64_t, arg1, this->real + sizeof(std::uint16_t) * 4 + sizeof(char *));
                return static_cast<objects::field::field_type>(arg1 >> 9 & 0b111ull);
            }

            method_type m_type()
            {
                PUN(std::uint64_t, arg1, this->real + sizeof(std::uint16_t) * 4 + sizeof(char *));
                return static_cast<method_type>(arg1 >> 12 & 0b11ull);
            }

            struct argument_definition
            {
                std::uint16_t arg_offsets[3];
                objects::field::field_type arg_types[3];
            };

            argument_definition read_arg(std::uint16_t arg_index)
            {
                argument_definition args;
                PUN(std::uint64_t, arg, this->real + sizeof(std::uint16_t) * 4 + sizeof(char *) + sizeof(std::uint64_t) * arg_index);
                args.arg_offsets[0] = arg >> 16;
                args.arg_offsets[1] = arg >> 32;
                args.arg_offsets[2] = arg >> 48;
                args.arg_types[0] = static_cast<objects::field::field_type>(arg & 0b111);
                args.arg_types[1] = static_cast<objects::field::field_type>(arg >> 3 & 0b111);
                args.arg_types[2] = static_cast<objects::field::field_type>(arg >> 6 & 0b111);
                return args;
            }
        };

        class clazz : public aliased<std::uint32_t, clazz>
        {
        private:
            friend class aliased;
            static constexpr std::uint64_t offset_size = sizeof(std::uint32_t) * 4 + sizeof(std::uint16_t) * 4 + sizeof(char *);

            std::uint32_t static_offset() const
            {
                return this->offset_size;
            }

        public:
            explicit clazz(char *real) : aliased(real) {}

            std::uint32_t method_count()
            {
                PUN(std::uint32_t, mc, this->real + sizeof(std::uint32_t) * 2);
                return mc;
            }

            std::uint32_t class_count()
            {
                PUN(std::uint32_t, cc, this->real + sizeof(std::uint32_t) * 3);
                return cc;
            }

            std::uint32_t static_fields_size()
            {
                PUN(std::uint16_t, sfs, this->real + sizeof(std::uint32_t) * 4);
                return static_cast<std::uint32_t>(sfs) << 3;
            }

            std::uint16_t static_handle_count()
            {
                PUN(std::uint16_t, shc, this->real + sizeof(std::uint32_t) * 4 + sizeof(std::uint16_t));
                return shc;
            }

            std::uint16_t class_name_length()
            {
                PUN(std::uint16_t, cnl, this->real + sizeof(std::uint32_t) * 4 + sizeof(std::uint16_t) * 2);
                return cnl;
            }

            std::uint16_t interface_count()
            {
                PUN(std::uint16_t, ic, this->real + sizeof(std::uint32_t) * 4 + sizeof(std::uint16_t) * 3);
                return ic;
            }

            clazz superclass()
            {
                PUN(char *, sc, this->real + sizeof(std::uint32_t) * 4 + sizeof(std::uint16_t) * 4);
                return clazz(sc);
            }

            template <typename primitive>
            std::enable_if_t<std::is_signed<primitive>::value, primitive> read(std::uint16_t offset)
            {
                PUN(primitive, p, this->real + offset_size + (this->method_count() + this->class_count()) * sizeof(char *) + offset);
                return p;
            }

            template <typename pointer>
            std::enable_if_t<std::is_same<pointer, object>::value, pointer> read(std::uint16_t offset)
            {
                PUN(char *, obj, this->real + offset_size + (this->method_count() + this->class_count()) * sizeof(char *) + offset);
                return object(obj);
            }

            template <typename primitive>
            std::enable_if_t<std::is_signed<primitive>::value, void> write(std::uint16_t offset, primitive p)
            {
                std::memcpy(this->real + offset_size + (this->method_count() + this->class_count()) * sizeof(char *) + offset, &p, sizeof(primitive));
            }

            template <typename pointer>
            std::enable_if_t<std::is_same<pointer, object>::value, void> write(std::uint16_t offset, pointer p)
            {
                auto ptr = p.unwrap();
                std::memcpy(this->real + offset_size + (this->method_count() + this->class_count()) * sizeof(char *) + offset, &ptr, sizeof(char *));
            }

            method get_method(std::uint32_t offset)
            {
                PUN(char *, mtd, this->real + offset_size + offset);
                return method(mtd);
            }

            clazz get_import(std::uint32_t offset)
            {
                PUN(char *, clz, this->real + offset_size + this->method_count() * sizeof(char *) + offset);
                return clazz(clz);
            }

            bool instanceof (clazz clz);
        };

        class object
        {
        private:
            char *real;
            friend class memory::memory_manager;

        public:
            static std::uint64_t size32to64(std::uint32_t compressed)
            {
                return (static_cast<std::uint64_t>(compressed) + 1) * 8;
            }

            static std::uint32_t size64to32(std::uint64_t expanded)
            {
                return expanded / 8 - 1;
            }
            explicit object(char *real) : real(real) {}

            void construct(clazz clz);

            void destruct();

            operator bool() const
            {
                return this->real != nullptr;
            }

            char *unwrap()
            {
                return this->real;
            }

            std::uint64_t size() const
            {
                PUN(std::uint32_t, sz, this->real - sizeof(std::uint32_t));
                return static_cast<std::uint64_t>(sz) * 8 + sizeof(char *);
            }

            bool operator==(const object &other) const
            {
                return this->real == other.real;
            }

            bool operator!=(const object &other) const
            {
                return this->real != other.real;
            }

            bool operator<(const object &other) const
            {
                return this->real < other.real;
            }

            bool operator<=(const object &other) const
            {
                return this->real <= other.real;
            }

            bool operator>(const object &other) const
            {
                return this->real > other.real;
            }

            bool operator>=(const object &other) const
            {
                return this->real >= other.real;
            }

            clazz get_class()
            {
#pragma GCC diagnostic ignored "-Wsizeof-pointer-memaccess"
                PUN(std::uintptr_t, clz, this->real);
#pragma GCC diagnostic pop
                clz &= ~0ull << 3;
                PUN(char *, cls, &clz);
                return clazz(cls);
            }

            template <typename pointer>
            std::enable_if_t<std::is_same<pointer, object>::value, pointer> read(std::uint32_t offset)
            {
#pragma GCC diagnostic ignored "-Wsizeof-pointer-memaccess"
                PUN(char *, clz, this->real + sizeof(char *) + (offset * sizeof(pointer)));
#pragma GCC diagnostic pop
                return object(clz);
            }

            template <typename primitive>
            std::enable_if_t<std::is_signed<primitive>::value, primitive> read(std::uint32_t offset)
            {
                PUN(primitive, p, this->real + sizeof(char *) + (offset * sizeof(primitive)));
                return p;
            }

            template <typename pointer>
            std::enable_if_t<std::is_same<pointer, object>::value, void> write(std::uint32_t offset, pointer p)
            {
                std::memcpy(this->real + sizeof(char *) + offset, &p.real, sizeof(char *));
            }

            template <typename primitive>
            std::enable_if_t<std::is_signed<primitive>::value, void> write(std::uint32_t offset, primitive p)
            {
                std::memcpy(this->real + sizeof(char *) + offset, &p, sizeof(primitive));
            }

            method get_virtual_method(std::uint32_t offset)
            {
                return this->get_class().get_method(offset);
            }
        };

        inline clazz method::owner()
        {
            PUN(char *, cls, this->real + sizeof(std::uint16_t) * 4);
            return clazz(cls);
        }

        static_assert(std::is_standard_layout<object>::value);
        static_assert(std::is_standard_layout<clazz>::value);

        static_assert(std::is_trivially_copyable<object>::value);
        static_assert(std::is_trivially_copyable<clazz>::value);
    } // namespace objects
} // namespace oops

#endif