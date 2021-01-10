#include "../memory/ostack.h"
#include "../platform/memory.h"
using namespace oops::memory;

bool stack::initialize(std::uintptr_t max_size) {
    auto reserved = platform::reserve_virtual_memory(max_size);
    if (reserved) {
        auto committed = platform::commit_virtual_memory(*reserved, max_size);
        if (committed) {
            this->stack_root = *committed;
            this->max_stack_size = max_size;
            this->current = frame(this->stack_root);
            std::memset(this->stack_root, 0, 32);
            this->current->mem.write<std::uint32_t>(28, 32);
        }
        platform::dereserve_virtual_memory(*reserved);
    }
    return false;
}

void stack::destroy() {
    platform::decommit_virtual_memory(this->stack_root, this->max_stack_size);
    platform::dereserve_virtual_memory(this->stack_root);
}