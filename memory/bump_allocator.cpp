#include "bump_allocator.h"

#include "../platform/memory.h"

using namespace oops::memory;

std::uintptr_t bump_allocator::get_committed_memory() const {
  return static_cast<char *>(this->committed) - static_cast<char *>(this->base);
}

void *bump_allocator::root() const { return this->base; }

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

void bump_allocator::deallocate(std::uintptr_t amount) {
  this->used = static_cast<char *>(this->used) - amount;
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

void *bump_allocator::low_bound() { return this->base; }
void *bump_allocator::high_bound() { return this->cap; }