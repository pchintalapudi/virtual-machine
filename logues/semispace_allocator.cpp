#include "../gc/semispace_allocator.h"

using namespace oops::gc;

bool semispace::initialize(std::uintptr_t min_size, std::uintptr_t max_size,
                           double min_heap_saturation,
                           double max_heap_saturation) {
  this->min_heap_size = min_size;
  this->min_heap_saturation = min_heap_saturation;
  this->max_heap_saturation = max_heap_saturation;
  if (this->allocators[0].initialize(max_size)) {
    if (this->allocators[1].initialize(max_size)) {
      this->use_second_space = false;
      return true;
    }
    this->allocators[0].destroy();
  }
  return false;
}

void semispace::destroy() {
  this->allocators[0].destroy();
  this->allocators[1].destroy();
}