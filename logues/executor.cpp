#include "../core/executor.h"

using namespace oops::core;

bool executor::initialize(const executor_options& options, memory::heap *heap) {
    if (this->vm_stack.initialize(options.default_stack_size)) {
        this->vm_heap = heap;
        return true;
    }
    return false;
}

void executor::destroy() {
    this->vm_stack.destroy();
}