#include "../memory/oheap.h"

using namespace oops::memory;

void heap::destroy() {
  this->allocator.destroy();
  this->bootstrap_classloader.destroy();
  this->native_references.clear();
  this->scratch.clear();
  this->vm_executors.clear();
}

bool heap::initialize(const heap_options &options) {
    if (this->allocator.initialize(options.min_heap_size, options.max_heap_size, options.min_heap_saturation, options.max_heap_saturation)) {
        if (this->bootstrap_classloader.initialize(options.classloader_args)) {
            return true;
        }
        this->allocator.destroy();
    }
    return false;
}