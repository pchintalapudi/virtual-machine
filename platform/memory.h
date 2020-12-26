#ifndef OOPS_PLATFORM_MEMORY_H
#define OOPS_PLATFORM_MEMORY_H

#include <cstdint>
#include <optional>

namespace oops {
namespace platform {
struct system_info {
  std::uintptr_t allocation_granularity;
  std::uint32_t page_size;
  std::uint32_t processor_count;
};
const system_info &get_system_info();
std::optional<void *> reserve_virtual_memory(std::size_t amount);
std::optional<void *> commit_virtual_memory(void *reserved, std::size_t amount);
void decommit_virtual_memory(void *committed, std::size_t amount);
void dereserve_virtual_memory(void *reserved);
}  // namespace platform
}  // namespace oops

#endif