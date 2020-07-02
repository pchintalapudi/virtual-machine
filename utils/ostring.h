#ifndef UTILS_OSTRING_H
#define UTILS_OSTRING_H

#include <cstdint>
#include <cstring>
#include <functional>

namespace oops {
    namespace utils {
        class ostring {
            private:
            char* real;
            mutable std::size_t hash;
            friend struct std::hash<ostring>;
            public:
            ostring(char* real) : real(real) {}

            char operator[](std::uint32_t idx) const {
                return this->real[4ull + idx];
            }

            std::uint32_t length() const {
                std::uint32_t length;
                std::memcpy(&length, this->real, sizeof(length));
                return length;
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