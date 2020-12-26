#include "../memory/oheap.h"

using namespace oops::memory;

void heap::destroy() {
  this->allocator.destroy();
  this->bootstrap_classloader.destroy();
  this->native_references.clear();
  this->scratch.clear();
  this->vm_stacks.clear();
}