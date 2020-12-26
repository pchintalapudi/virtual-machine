#include "../memory/bump_allocator.h"

#include "../platform/memory.h"

using namespace oops::memory;

bool bump_allocator::initialize(std::uintptr_t max_size) {
  auto base = platform::reserve_virtual_memory(max_size);
  if (!base) {
    return false;
  }
  this->base = *base;
  this->used = this->base;
  this->committed = this->base;
  this->cap = static_cast<char *>(this->base) + max_size;
  return true;
}

void bump_allocator::destroy() {
  this->decommit_all();
  platform::dereserve_virtual_memory(this->base);
}