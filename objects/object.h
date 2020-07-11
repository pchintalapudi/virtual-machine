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
            
            type get_type() const;

            std::uint64_t malloc_size() const;

            std::uint8_t metadata() const;

            std::uint32_t tail_data() const;

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
            std::enable_if_t<std::is_base_of_v<base_object, pointer>, pointer> read(std::uint32_t offset) const {
                return pointer(oops::utils::pun_read<char*>(this->real + sizeof(char*) + offset));
            }

            template<typename primitive>
            std::enable_if_t<std::is_signed_v<primitive>, void> write(std::uint32_t offset, primitive value) {
                oops::utils::pun_write(this->real + sizeof(char*) + offset, value);
            }

            template<typename pointer>
            std::enable_if_t<std::is_base_of_v<base_object, pointer>, void> write(std::uint32_t offset, pointer obj) {
                oops::utils::pun_write(this->real + sizeof(char*) + offset, obj.unwrap());
            }
        };

        class array : public base_object {
            public:

            array(char* real) : base_object(real) {}

            static std::uint64_t size(field::type array_type, std::uint32_t length);

            template<typename primitive>
            std::enable_if_t<std::is_signed_v<primitive>, primitive> read(std::uint64_t offset) const {
                return oops::utils::pun_read<primitive>(this->real + sizeof(char*) + sizeof(std::uint64_t) + offset);
            }

            template<typename pointer>
            std::enable_if_t<std::is_base_of_v<base_object, pointer>, pointer> read(std::uint64_t offset) const {
                return pointer(oops::utils::pun_read<char*>(this->real + sizeof(char*) + sizeof(std::uint64_t) + offset));
            }

            template<typename primitive>
            std::enable_if_t<std::is_signed_v<primitive>, void> write(std::uint64_t offset, primitive value) {
                oops::utils::pun_write(this->real + sizeof(char*) + sizeof(std::uint64_t) + offset, value);
            }

            template<typename pointer>
            std::enable_if_t<std::is_base_of_v<base_object, pointer>, void> write(std::uint64_t offset, pointer obj) {
                oops::utils::pun_write(this->real + sizeof(char*) + sizeof(std::uint64_t) + offset, obj.unwrap());
            }

            std::uint32_t length() const {
                return oops::utils::pun_read<std::uint32_t>(this->real + sizeof(char*));
            }
        };
    }
}

#endif /* OBJECTS_OBJECT */
