#ifndef OOPS_MEMORY_BYTEBLOCK_H
#define OOPS_MEMORY_BYTEBLOCK_H

#include <cstdint>
#include <cstring>
#include <type_traits>

namespace oops {
    namespace memory {
        template<bool writeable = true>
        class byteblock {
            private:
            void* block;
            public:

            void initialize(void* block) {
                this->block = block;
            }

            template<typename out_t>
            std::enable_if_t<std::is_trivially_copy_constructible_v<out_t> and std::is_trivially_constructible_v<out_t>, out_t> read(std::uintptr_t offset) const {
                out_t out;
                std::memcpy(&out, static_cast<char*>(block) + offset, sizeof(out_t));
                return out;
            }

            template<typename in_t>
            std::enable_if_t<std::is_trivially_copy_constructible_v<in_t> and std::is_trivially_constructible_v<in_t> and writeable, void> write(std::uintptr_t offset, in_t value) {
                std::memcpy(static_cast<char*>(block) + offset, &value, sizeof(in_t));
            }
        };
    }
}

#endif