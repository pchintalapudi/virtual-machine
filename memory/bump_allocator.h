#ifndef OOPS_MEMORY_BUMP_ALLOCATOR_H
#define OOPS_MEMORY_BUMP_ALLOCATOR_H

#include <cstdint>
#include <optional>

namespace oops {
namespace memory {

class bump_allocator {
 private:
  void *base, *used, *committed, *cap;

 public:
  bool initialize(std::uintptr_t max_size);

  bool commit(std::uintptr_t amount);

  std::optional<void *> allocate(std::uintptr_t amount);

  void decommit_all();
  void decommit(std::uintptr_t new_amount);

  std::uintptr_t get_committed_memory() const;
  std::uintptr_t amount_used() const;
  void *root() const;

  void* low_bound();
  void* high_bound();

  void destroy();
};
}  // namespace memory
}  // namespace oops

#endif