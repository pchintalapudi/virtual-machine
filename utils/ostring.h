#ifndef UTILS_OSTRING_H
#define UTILS_OSTRING_H

#include <cstdint>
#include <functional>

namespace oops {
    namespace utils {
        class ostring {
            private:
            char* real;
            mutable std::size_t hash;
            friend struct std::hash<ostring>;
            public:

            char* unwrap() const {
                return this->real;
            }

            ostring(char* real) : real(real), hash(0) {}

            char operator[](std::uint32_t idx) const;

            std::uint32_t length() const;

            bool operator==(const ostring& other) const;
            bool operator!=(const ostring& other) const;
            bool operator<(const ostring& other) const;
            bool operator<=(const ostring& other) const;
            bool operator>(const ostring& other) const;
            bool operator>=(const ostring& other) const;

            char* begin() {
                return this->real;
            }

            char* end() {
                return this->real + this->length();
            }

        };
    }
}

namespace std {
    template<>
    struct hash<oops::utils::ostring> {

        std::size_t compute_hash(const oops::utils::ostring &string) const {
            std::size_t seed = 0x1f;
            for (auto length = string.length(); length --> 0;) {
                seed = seed * 0x7f ^ string[length];
            }
            return seed;
        }

        std::size_t operator()(const oops::utils::ostring &string) const {
            return string.hash ? string.hash : (string.hash = this->compute_hash(string));
        }
    };
}

#endif