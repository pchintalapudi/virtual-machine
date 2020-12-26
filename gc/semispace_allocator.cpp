#include "semispace_allocator.h"

using namespace oops::gc;

std::optional<void *> semispace::gc_prologue() {
    if (this->allocators[!this->use_second_space].commit(this->allocators[this->use_second_space].get_committed_memory())) {
        return this->allocators[!this->use_second_space].root();
    } else {
        return {};
    }
}
constexpr double upper_memory_saturation_limit = 0.75, lower_memory_saturation_limit = 0.5;
void semispace::gc_epilogue(void *used) {
    std::uintptr_t amount_used = static_cast<char*>(used) - static_cast<char*>(this->allocators[!this->use_second_space].root());
    std::uintptr_t amount_committed = this->allocators[!this->use_second_space].get_committed_memory();
    double saturation = static_cast<double>(amount_used) / amount_committed;
    if (saturation > upper_memory_saturation_limit) {
        //TODO commit more memory
    } else if (saturation < lower_memory_saturation_limit) {
        //TODO decommit memory
    }
    this->allocators[this->use_second_space].decommit_all();
    this->use_second_space = !this->use_second_space;
}

std::optional<void *> semispace::allocate(std::uintptr_t amount) {
    return this->allocators[this->use_second_space].allocate(amount);
}