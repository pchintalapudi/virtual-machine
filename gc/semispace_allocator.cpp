#include "semispace_allocator.h"

#include <algorithm>
#include <cmath>

#include "../platform/memory.h"

using namespace oops::gc;

std::optional<void *> semispace::gc_prologue() {
  if (this->allocators[!this->use_second_space].commit(
          this->allocators[this->use_second_space].get_committed_memory())) {
    return this->allocators[!this->use_second_space].root();
  } else {
    return {};
  }
}

namespace {
std::uintptr_t round_for_commit(std::uintptr_t amount) {
  auto &sysinfo = oops::platform::get_system_info();
  auto scaled = amount + sysinfo.page_size - 1;
  return scaled - scaled % sysinfo.page_size;
}
}  // namespace

void semispace::gc_epilogue(void *used) {
  std::uintptr_t amount_used =
      static_cast<char *>(used) -
      static_cast<char *>(this->allocators[!this->use_second_space].root());
  std::uintptr_t amount_committed =
      this->allocators[!this->use_second_space].get_committed_memory();
  double saturation = static_cast<double>(amount_used) / amount_committed;
  if (saturation > max_heap_saturation) {
    std::uintptr_t needs_to_commit = static_cast<std::uintptr_t>(std::ceil(
                                         amount_used / max_heap_saturation)) -
                                     amount_committed;
    this->allocators[!this->use_second_space].commit_to_max(
        round_for_commit(needs_to_commit));
  } else if (saturation < min_heap_saturation) {
    std::uintptr_t committed_paged_amount =
        round_for_commit(std::max(static_cast<std::uintptr_t>(std::ceil(
                                      amount_used / min_heap_saturation)),
                                  this->min_heap_size));
    if (committed_paged_amount < amount_committed) {
      this->allocators[!this->use_second_space].decommit(
          committed_paged_amount);
    }
  }
  this->allocators[this->use_second_space].decommit_all();
  this->use_second_space = !this->use_second_space;
}

std::optional<void *> semispace::allocate(std::uintptr_t amount) {
  return this->allocators[this->use_second_space].allocate(amount);
}

void *semispace::low_bound() {
  return this->allocators[this->use_second_space].low_bound();
}
void *semispace::high_bound() {
  return this->allocators[this->use_second_space].high_bound();
}