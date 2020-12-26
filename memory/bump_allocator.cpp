#include "bump_allocator.h"

#include "../platform/memory.h"

using namespace oops::memory;

std::uintptr_t bump_allocator::get_committed_memory() const {
  return static_cast<char *>(this->committed) - static_cast<char *>(this->base);
}

void *bump_allocator::root() const { return this->base; }

void bump_allocator::destroy() {
  this->decommit_all();
  platform::dereserve_virtual_memory(this->base);
}

void bump_allocator::decommit_all() {
  platform::decommit_virtual_memory(this->base, this->get_committed_memory());
  this->committed = this->base;
  this->used = base;
}

std::optional<void *> bump_allocator::allocate(std::uintptr_t amount) {
  auto location = static_cast<char *>(this->used);
  if (location + amount > this->committed) {
    return {};
  } else {
    this->used = location + amount;
    return location;
  }
}

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

bool bump_allocator::commit(std::uintptr_t amount) {
  auto start = static_cast<char *>(this->committed);
  if (start + amount > this->cap) {
    return false;
  }
  if (platform::commit_virtual_memory(this->committed, amount)) {
    this->committed = start + amount;
    return true;
  }
  return false;
}

std::uintptr_t bump_allocator::amount_used() const {
  return static_cast<char *>(this->used) - static_cast<char *>(this->base);
}