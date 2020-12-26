#include "../gc/semispace_allocator.h"

using namespace oops::gc;

bool semispace::initialize(std::uintptr_t max_size) {
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