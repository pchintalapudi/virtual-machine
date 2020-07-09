#ifndef PLATFORM_SPECIFIC_MEMORY
#define PLATFORM_SPECIFIC_MEMORY

#include <cstdint>
#include <optional>

namespace oops {
    namespace platform {

        struct platform_info {
            std::uint32_t page_size, allocation_granularity;
            std::uint32_t processor_count;
        };

        platform_info get_platform_info();

        std::optional<char*> reserve(std::size_t amount);

        bool commit(char* memory, std::size_t amount);

        void decommit(char* memory, std::size_t amount);

        void dereserve(char* memory);
    }
}

#endif /* PLATFORM_SPECIFIC_MEMORY */
