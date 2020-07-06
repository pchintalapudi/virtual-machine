#ifndef MEMORY_MEMUTILS
#define MEMORY_MEMUTILS

#include <cstdint>
namespace oops
{
    namespace memory
    {
        constexpr std::uint64_t size32to64(std::uint32_t compressed_size)
        {
            return (static_cast<std::uint64_t>(compressed_size) + 2) * 8;
        }

        constexpr std::uint32_t size64to32(std::uint64_t expanded_size)
        {
            return expanded_size / 8 - 2;
        }
    } // namespace memory
} // namespace oops

#endif /* MEMORY_MEMUTILS */
