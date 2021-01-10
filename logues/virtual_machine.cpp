#include "../vm/virtual_machine.h"

using namespace oops;

std::optional<oops::core::executor *> virtual_machine::initialize(
    const vm_options &options) {
  this->options = options;
  if (this->vm_heap.initialize(options.heap_args)) {
    core::executor *bootstrap_executor = new core::executor;
    if (bootstrap_executor->initialize(options.executor_args, &this->vm_heap)) {
        this->vm_heap.register_executor(bootstrap_executor);
        return bootstrap_executor;
    }
    delete bootstrap_executor;
    this->vm_heap.destroy();
  }
  return {};
}

void virtual_machine::destroy() {
    for (auto executor : this->vm_heap.vm_executors) {
        executor->destroy();
        delete executor;
    }
    this->vm_heap.destroy();
}