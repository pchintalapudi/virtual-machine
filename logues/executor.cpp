#include "../core/executor.h"

using namespace oops::core;

bool executor::initialize(const executor_options& options) {
    if (this->vm_stack.initialize(options.stack_size)) {
        this->vm_heap = static_cast<memory::heap*>(options.heap.heap);
        return true;
    }
    return false;
}

void executor::destroy() {
    this->vm_stack.destroy();
}