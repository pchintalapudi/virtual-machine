#ifndef OOPS_GC_SEMISPACE_ALLOCATOR_H
#define OOPS_GC_SEMISPACE_ALLOCATOR_H

#include <cstdint>
#include <optional>

#include "../memory/bump_allocator.h"

namespace oops {
namespace gc {
class semispace {
 private:
  memory::bump_allocator allocators[2];
  std::uintptr_t min_heap_size;
  double min_heap_saturation;
  double max_heap_saturation;
  bool use_second_space;

 public:
  std::optional<void *> gc_prologue();
  void gc_epilogue(void *used);

  bool initialize(std::uintptr_t min_size, std::uintptr_t max_size, double min_heap_saturation, double max_heap_saturation);

  std::optional<void *> allocate(std::uintptr_t amount);

  void *low_bound();
  void *high_bound();

  void destroy();
};
}  // namespace gc
}  // namespace oops

#endif