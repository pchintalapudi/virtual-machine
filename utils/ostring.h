#ifndef UTILS_OSTRING_H
#define UTILS_OSTRING_H
#include <cstdint>
#include <functional>
#include "../punning/puns.h"
namespace oops {
    namespace utils {
        class ostring {
            private:
            char* real;
            mutable std::uint64_t hash = 0;
            friend class std::hash<ostring>;
            
            public:
            ostring(char* real) : real(real) {}

            std::uint32_t length() const {
                PUN(std::uint32_t, length, this->real);
                return length;
            }

            char operator[](std::uint32_t index) const {
                return this->real[index + 4];
            }

            bool operator==(const ostring& other) {
                if (other.length() == this->length()) {
                    if (this->hash && other.hash && this->hash != other.hash) return false;
                    return std::memcmp(this->real + sizeof(std::uint32_t), other.real + sizeof(std::uint32_t), this->length()) == 0;
                }
                return false;
            }

            bool operator!=(ostring other) {
                return !(*this == other);
            }
        };
    }
}

namespace std {
    template<> struct hash<oops::utils::ostring> {

        private:
        std::size_t compute_hash(const oops::utils::ostring& string) {
            std::size_t hash = 0x1full;
            for (std::size_t i = string.length(); i --> 0;) {
                hash = hash * 0x7full ^ string[i];
            }
            return hash | 1;//Cannot be 0, so we cop out here
        }
        public:

        std::size_t operator()(const oops::utils::ostring &string) {
            return string.hash ? string.hash : (string.hash = compute_hash(string));
        }
    };
}

#endif