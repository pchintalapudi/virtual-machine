#ifndef OBJECTS_OBJECT
#define OBJECTS_OBJECT

#include <type_traits>

#include "../utils/utils.h"

#include "class.h"

namespace oops {
    namespace objects {
        class base_object {
            protected:
            char* real;
            public:

            char* unwrap() {
                return this->real;
            }

            enum class type {
                OBJECT, ARRAY
            };

            base_object(char* real) : real(real) {}

            std::size_t malloc_inclusive_memory_size() const {
                std::uint64_t compressed_size = oops::utils::pun_read<std::uint32_t>(this->real - sizeof(std::uint32_t)) >> 1;//Low bit is for other stuff
                compressed_size *= sizeof(char*);
                compressed_size += sizeof(char*) * 2;//1 for the class pointer, 1 for the malloc overhead
                return compressed_size;
            }
            
            type get_type() const;

            clazz get_clazz() const;

            friend bool operator<(base_object b1, base_object b2) {
                return b1.real < b2.real;
            }

            friend bool operator==(base_object b1, base_object b2) {
                return b1.real == b2.real;
            }
        };

        class object : public base_object {
            public:

            object(char* real) : base_object(real) {}

            template<typename primitive>
            std::enable_if_t<std::is_signed_v<primitive>, primitive> read(std::uint32_t offset) const {
                return oops::utils::pun_read<primitive>(this->real + sizeof(char*) + offset);
            }

            template<typename pointer>
            std::enable_if_t<std::is_same_v<pointer, base_object>, base_object> read(std::uint32_t offset) const {
                return base_object(oops::utils::pun_read<char*>(this->real + sizeof(char*) + offset));
            }

            template<typename primitive>
            std::enable_if_t<std::is_signed_v<primitive>, void> write(std::uint32_t offset, primitive value) {
                oops::utils::pun_write(this->real + sizeof(char*) + offset, value);
            }

            template<typename pointer>
            std::enable_if_t<std::is_same_v<pointer, base_object>, void> write(std::uint32_t offset, base_object obj) {
                oops::utils::pun_write(this->real + sizeof(char*) + offset, obj.real);
            }
        };

        class array : public base_object {
            public:

            array(char* real) : base_object(real) {}

            template<typename primitive>
            std::enable_if_t<std::is_signed_v<primitive>, primitive> read(std::uint64_t offset) const {
                return oops::utils::pun_read<primitive>(this->real + sizeof(char*) + sizeof(std::uint64_t) + offset);
            }

            template<typename pointer>
            std::enable_if_t<std::is_same_v<pointer, base_object>, base_object> read(std::uint64_t offset) const {
                return base_object(oops::utils::pun_read<char*>(this->real + sizeof(char*) + sizeof(std::uint64_t) + offset));
            }

            template<typename primitive>
            std::enable_if_t<std::is_signed_v<primitive>, void> write(std::uint64_t offset, primitive value) {
                oops::utils::pun_write(this->real + sizeof(char*) + sizeof(std::uint64_t) + offset, value);
            }

            template<typename pointer>
            std::enable_if_t<std::is_same_v<pointer, base_object>, void> write(std::uint64_t offset, base_object obj) {
                oops::utils::pun_write(this->real + sizeof(char*) + sizeof(std::uint64_t) + offset, obj.real);
            }
        };
    }
}

#endif /* OBJECTS_OBJECT */
